/*
	lcd4l - Neutrino-GUI

	Copyright (C) 2012 'defans'
	Homepage: http://www.bluepeercrew.us/

	Copyright (C) 2012 'vanhofen'
	Homepage: http://www.neutrino-images.de/

	Modded    (C) 2016 'TangoCash'

	License: GPL
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <iomanip>

#include <global.h>
#include <neutrino.h>

#include <timerdclient/timerdclient.h>
#include <system/helpers.h>

#include <driver/record.h>
#include <driver/audioplay.h>
#include <zapit/zapit.h>
#include <gui/movieplayer.h>
#include <eitd/sectionsd.h>

#include "lcd4l.h"

extern CRemoteControl *g_RemoteControl;

#define LCD_DATADIR		"/tmp/lcd/"

#define LCD_ICONSDIR		"/share/lcd/icons/"
#define ICONSEXT		".png"

#define LOGO_DUMMY		LCD_ICONSDIR "blank.png"

#define TUNER			LCD_DATADIR "tuner"
#define VOLUME			LCD_DATADIR "volume"
#define MODE_REC		LCD_DATADIR "mode_rec"
#define MODE_REC_ICON	LCD_DATADIR "mode_rec_icon"
#define MODE_TSHIFT		LCD_DATADIR "mode_tshift"
#define MODE_TIMER		LCD_DATADIR "mode_timer"
#define MODE_ECM		LCD_DATADIR "mode_ecm"

#define SERVICE			LCD_DATADIR "service"
#define CHANNELNR		LCD_DATADIR "channelnr"
#define LOGO			LCD_DATADIR "logo"
#define MODE_LOGO		LCD_DATADIR "mode_logo"
#define LAYOUT			LCD_DATADIR "layout"

#define EVENT			LCD_DATADIR "event"
#define PROGRESS		LCD_DATADIR "progress"
#define DURATION		LCD_DATADIR "duration"
#define START			LCD_DATADIR "start"
#define END			LCD_DATADIR "end"

#define FONT			LCD_DATADIR "font"
#define FGCOLOR			LCD_DATADIR "fgcolor"
#define BGCOLOR			LCD_DATADIR "bgcolor"

#define FLAG_LCD4LINUX		"/tmp/.lcd4linux"
#define PIDFILE			"/tmp/lcd4linux.pid"

/* ----------------------------------------------------------------- */

CLCD4l::CLCD4l()
{
	thrLCD4l = 0;
}

CLCD4l::~CLCD4l()
{
	if (thrLCD4l)
		pthread_cancel(thrLCD4l);
	thrLCD4l = 0;
}

/* ----------------------------------------------------------------- */

void CLCD4l::InitLCD4l()
{
	if (thrLCD4l)
	{
		printf("[CLCD4l] %s: initializing\n", __FUNCTION__);
		Init();
	}
}

void CLCD4l::StartLCD4l()
{
	if (!thrLCD4l)
	{
		printf("[CLCD4l] %s: starting thread\n", __FUNCTION__);
		pthread_create(&thrLCD4l, NULL, LCD4lProc, (void*) this);
		pthread_detach(thrLCD4l);
	}
}

void CLCD4l::StopLCD4l()
{
	if (thrLCD4l)
	{
		printf("[CLCD4l] %s: stopping thread\n", __FUNCTION__);
		pthread_cancel(thrLCD4l);
		thrLCD4l = 0;
	}
}

void CLCD4l::SwitchLCD4l()
{
	if (thrLCD4l)
		StopLCD4l();
	else
		StartLCD4l();
}

int CLCD4l::CreateFile(const char *file, std::string content, bool convert)
{
	// returns 0 = ok; 1 = can't create file; -1 = thread not found

	int ret = 0;

	if (thrLCD4l)
	{
		if (WriteFile(file, content, convert) == false)
			ret = 1;
	}
	else
		ret = -1;

	return ret;
}

int CLCD4l::RemoveFile(const char *file)
{
	// returns 0 = ok; 1 = can't remove file;

	int ret = 0;

	if (access(file, F_OK) == 0)
	{
		if (unlink(file) != 0)
			ret = 1;
	}

	return ret;
}

/* ----------------------------------------------------------------- */

void CLCD4l::Init()
{
	m_ParseID	= 0;

	m_Tuner		= -1;
	m_Volume	= -1;
	m_ModeRec	= -1;
	m_ModeTshift	= -1;
	m_ModeTimer	= -1;
	m_ModeEcm	= -1;

	m_Service	= "n/a";
	m_ChannelNr	= -1;
	m_Logo		= "n/a";
	m_ModeLogo	= -1;

	m_Layout	= "n/a";

	m_Ev_Desc	= "n/a";
	m_Ev_Start	= "00:00";
	m_Ev_End	= "00:00";
	m_Progress	= -1;
	for (int i = 0; i < (int)sizeof(m_Duration); i++)
		m_Duration[i] = ' ';

	if (!access(LCD_DATADIR, F_OK) == 0)
		mkdir(LCD_DATADIR, 0755);
}

void* CLCD4l::LCD4lProc(void* arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	CLCD4l *PLCD4l = static_cast<CLCD4l*>(arg);

	PLCD4l->Init();

	sleep(5); //please wait !

	static bool FirstRun = true;
	uint64_t p_ParseID = 0;
	bool NewParseID = false;

	//printf("[CLCD4l] %s: starting loop\n", __FUNCTION__);
	while(1)
	{
		if ( (!access(PIDFILE, F_OK) == 0) && (!FirstRun) )
		{
			if (g_settings.lcd4l_support == 1) // automatic
			{
				//printf("[CLCD4l] %s: waiting for lcd4linux\n", __FUNCTION__);
				sleep(10);
				continue;
			}
		}

		for (int i = 0; i < 10; i++)
		{
			usleep(5 * 100 * 1000); // 0.5 sec
			NewParseID = PLCD4l->CompareParseID(p_ParseID);
			if (NewParseID || p_ParseID == MODE_AUDIO)
				break;
		}

		//printf("[CLCD4l] %s: m_ParseID: %llx (NewParseID: %d)\n", __FUNCTION__, p_ParseID, NewParseID ? 1 : 0);
		PLCD4l->ParseInfo(p_ParseID, NewParseID, FirstRun);

		if (FirstRun)
		{
			PLCD4l->WriteFile(FLAG_LCD4LINUX);
			FirstRun = false;
		}
	}
	return 0;
}

void CLCD4l::ParseInfo(uint64_t parseID, bool newID, bool firstRun)
{
	SNeutrinoTheme &t = g_settings.theme;

	std::string font = g_settings.font_file;

	if (m_font.compare(font))
	{
		WriteFile(FONT, font);
		m_font = font;
	}

	/* ----------------------------------------------------------------- */

	std::string fgcolor = hexStr(&t.infobar_Text_red)
	                      + hexStr(&t.infobar_Text_green)
	                      + hexStr(&t.infobar_Text_blue)
	                      + hexStr(&t.infobar_Text_alpha);

	if (m_fgcolor.compare(fgcolor))
	{
		WriteFile(FGCOLOR, fgcolor);
		m_fgcolor = fgcolor;
	}

	/* ----------------------------------------------------------------- */

	std::string bgcolor = hexStr(&t.infobar_red)
	                      + hexStr(&t.infobar_green)
	                      + hexStr(&t.infobar_blue)
	                      + hexStr(&t.infobar_alpha);

	if (m_bgcolor.compare(bgcolor))
	{
		WriteFile(BGCOLOR, bgcolor);
		m_bgcolor = bgcolor;
	}

	/* ----------------------------------------------------------------- */

	int Tuner = 1 + CFEManager::getInstance()->getLiveFE()->getNumber();

	if (m_Tuner != Tuner)
	{
		WriteFile(TUNER, to_string(Tuner));
		m_Tuner = Tuner;
	}

	/* ----------------------------------------------------------------- */

	int Volume = g_settings.current_volume;

	if (m_Volume != Volume)
	{
		WriteFile(VOLUME, to_string(Volume));
		m_Volume = Volume;
	}

	/* ----------------------------------------------------------------- */

	int ModeRec = 0;
	int ModeTshift = 0;

	int RecordMode = CRecordManager::getInstance()->GetRecordMode();
	switch (RecordMode)
	{
	case CRecordManager::RECMODE_REC_TSHIFT:
		ModeRec = 1;
		ModeTshift = 1;
		break;
	case CRecordManager::RECMODE_REC:
		ModeRec = 1;
		break;
	case CRecordManager::RECMODE_TSHIFT:
		ModeTshift = 1;
		break;
	default:
		break;
	}

	if (m_ModeRec != ModeRec)
	{
		WriteFile(MODE_REC, ModeRec ? "on" : "off");
		std::string rec_icon ="";
		if (ModeRec)
			rec_icon = ICONSDIR "/" NEUTRINO_ICON_REC ICONSEXT;
		else
			rec_icon = ICONSDIR "/" NEUTRINO_ICON_REC_GRAY ICONSEXT;
		WriteFile(MODE_REC_ICON, rec_icon);
		m_ModeRec = ModeRec;
	}

	if (m_ModeTshift != ModeTshift)
	{
		WriteFile(MODE_TSHIFT, ModeTshift ? "on" : "off");
		m_ModeTshift = ModeTshift;
	}

	/* ----------------------------------------------------------------- */

	int ModeTimer = 0;

	CTimerd::TimerList timerList;
	CTimerdClient TimerdClient;

	timerList.clear();
	TimerdClient.getTimerList(timerList);

	CTimerd::TimerList::iterator timer = timerList.begin();

	for (; timer != timerList.end(); timer++)
	{
		if (timer->alarmTime > time(NULL) && (timer->eventType == CTimerd::TIMER_ZAPTO || timer->eventType == CTimerd::TIMER_RECORD))
		{
			// Nur "true", wenn irgendein timer in der zukunft liegt
			// und dieser vom typ TIMER_ZAPTO oder TIMER_RECORD ist
			ModeTimer = 1;
			break;
		}
	}

	if (m_ModeTimer != ModeTimer)
	{
		WriteFile(MODE_TIMER, ModeTimer ? "on" : "off");
		m_ModeTimer = ModeTimer;
	}

	/* ----------------------------------------------------------------- */

	int ModeEcm = 0;

	if (access("/tmp/ecm.info", F_OK) == 0)
	{
		struct stat buf;
		stat("/tmp/ecm.info", &buf);
		if (buf.st_size > 0)
			ModeEcm = 1;
	}

	if (m_ModeEcm != ModeEcm)
	{
		WriteFile(MODE_ECM, ModeEcm ? "on" : "off");
		m_ModeEcm = ModeEcm;
	}

	/* ----------------------------------------------------------------- */

	if (newID || parseID == MODE_AUDIO || parseID == MODE_TS)
	{
		std::string Service = "";
		int ChannelNr = 0;
		std::string Logo = LOGO_DUMMY;
		int ModeLogo = 0;

		int ModeStandby	= 0;

		if (m_ModeChannel)
		{
			if (m_ModeChannel > 1)
				Service = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].subservice_name;
			else
				Service = g_RemoteControl->getCurrentChannelName();

			GetLogoName(parseID, Service, Logo);

			ChannelNr = CNeutrinoApp::getInstance()->channelList->getActiveChannelNumber();
		}
		else if (parseID == MODE_PIC)
		{
			Service = g_Locale->getText(LOCALE_PICTUREVIEWER_HEAD);
		}
		else if (parseID == MODE_TS)
		{
			if (ModeTshift)
				Service = g_Locale->getText(LOCALE_RECORDINGMENU_TIMESHIFT);
			else if (CMoviePlayerGui::getInstance().p_movie_info)
					if (!CMoviePlayerGui::getInstance().p_movie_info->channelName.empty())
						Service = CMoviePlayerGui::getInstance().p_movie_info->channelName;

			if (Service.empty())
				Service = g_Locale->getText(LOCALE_MOVIEPLAYER_HEAD);

			switch (CMoviePlayerGui::getInstance().getState())
			{
			case 6: /* rewind */
				Logo = ICONSDIR "/" NEUTRINO_ICON_REW ICONSEXT;
				break;
			case 5: /* fast forward */
				Logo = ICONSDIR "/" NEUTRINO_ICON_FF ICONSEXT;
				break;
			case 4: /* pause */
				Logo = ICONSDIR "/" NEUTRINO_ICON_PAUSE ICONSEXT;
				break;
			case 3: /* play */
				if (CMoviePlayerGui::getInstance().p_movie_info)
					if (!GetLogoName(	CMoviePlayerGui::getInstance().p_movie_info->channelId,
				    	                CMoviePlayerGui::getInstance().p_movie_info->channelName, Logo))
						Logo = ICONSDIR "/" NEUTRINO_ICON_PLAY ICONSEXT;
				else
					Logo = ICONSDIR "/" NEUTRINO_ICON_PLAY ICONSEXT;
				break;
			default: /* show movieplayer-icon */
				Logo = ICONSDIR "/" NEUTRINO_ICON_MOVIEPLAYER ICONSEXT;
			}
		}
		else if (parseID == MODE_STANDBY)
		{
			Service = "STANDBY";
			ModeStandby = 1;
		}

		/* --- */

		if (m_Service.compare(Service))
		{
			WriteFile(SERVICE, Service, true);
			m_Service = Service;
		}

		if (m_ChannelNr != ChannelNr)
		{
			WriteFile(CHANNELNR, to_string(ChannelNr));
			m_ChannelNr = ChannelNr;
		}

		if (m_Logo.compare(Logo))
		{
			WriteFile(LOGO, Logo);
			m_Logo = Logo;
		}

		if (Logo != LOGO_DUMMY)
			ModeLogo = 1;

		if (m_ModeLogo != ModeLogo)
		{
			WriteFile(MODE_LOGO, to_string(ModeLogo));
			m_ModeLogo = ModeLogo;
		}

		/* --- */

		std::string Layout;
		if (ModeStandby)
		{
			Layout = "standby";
		}
		else
		{
			switch (g_settings.lcd4l_skin)
			{
			case 2:
				Layout = "spf_large";
				break;
			case 1:
				Layout = "spf_middle";
				break;
			default:
				Layout = "spf_small";
			}
		}

		if (m_Layout.compare(Layout))
		{
			WriteFile(LAYOUT, Layout);
			m_Layout = Layout;
			if (!firstRun)
			{
				const char *buf = "lcd4linux";
				if (my_system(3, "killall", "-9", buf) != 0)
					printf("[CLCD4l] %s: terminating '%s' failed\n", __FUNCTION__, buf);
				sleep(2);
				if (my_system(1, buf) != 0)
					printf("[CLCD4l] %s: executing '%s' failed\n", __FUNCTION__, buf);
			}
		}
	}

	/* ----------------------------------------------------------------- */

	std::string Event = "";
	int Progress = 0;
	char Duration[sizeof(m_Duration)] = {0};

	char Start[6] = {0};
	char End[6] = {0};
	int todo = 0;

	if (m_ModeChannel)
	{
		CSectionsdClient::CurrentNextInfo CurrentNext;
		CEitManager::getInstance()->getCurrentNextServiceKey(parseID & 0xFFFFFFFFFFFFULL, CurrentNext);

		if (CSectionsdClient::epgflags::has_current)
		{
			if (!CurrentNext.current_name.empty())
				Event = CurrentNext.current_name;

			if ((CurrentNext.current_zeit.dauer > 0) && (CurrentNext.current_zeit.dauer < 86400))
			{
				Progress = 100 * (time(NULL) - CurrentNext.current_zeit.startzeit) / CurrentNext.current_zeit.dauer;

				int total = CurrentNext.current_zeit.dauer / 60;
				int done = (abs(time(NULL) - CurrentNext.current_zeit.startzeit) + 30) / 60;
				todo = total - done;
				if ((time(NULL) < CurrentNext.current_zeit.startzeit) && todo >= 0)
				{
					done = 0;
					todo = CurrentNext.current_zeit.dauer / 60;
				}
				snprintf(Duration, sizeof(Duration), "%d/%d", done, total);
			}

			tm_struct = localtime(&CurrentNext.current_zeit.startzeit);
			snprintf(Start, sizeof(Start), "%02d:%02d", tm_struct->tm_hour, tm_struct->tm_min);
		}

		if (CSectionsdClient::epgflags::has_next)
		{
			if (todo)
				Event += "\nin "+ to_string(todo) + " min:" + CurrentNext.next_name;
			else
				Event += "\n"+ CurrentNext.next_name;
			tm_struct = localtime(&CurrentNext.next_zeit.startzeit);
			snprintf(End, sizeof(End), "%02d:%02d", tm_struct->tm_hour, tm_struct->tm_min);
		}
	}
	else if (parseID == MODE_TS)
	{
		Event = "Mediaplayer";

		if (!CMoviePlayerGui::getInstance().pretty_name.empty())
			Event = CMoviePlayerGui::getInstance().pretty_name;

		if (CMoviePlayerGui::getInstance().p_movie_info)
			if (!CMoviePlayerGui::getInstance().p_movie_info->epgTitle.empty())
				Event = CMoviePlayerGui::getInstance().p_movie_info->epgTitle;

		if (CMoviePlayerGui::getInstance().p_movie_info)
			if (!CMoviePlayerGui::getInstance().p_movie_info->epgInfo1.empty())
				Event += "\n" + CMoviePlayerGui::getInstance().p_movie_info->epgInfo1;

		if (!ModeTshift)
		{
			int total = CMoviePlayerGui::getInstance().GetDuration();
			int done = CMoviePlayerGui::getInstance().GetPosition();
			snprintf(Duration, sizeof(Duration), "%d/%d", done / (60 * 1000), total / (60 * 1000));
			Progress = CMoviePlayerGui::getInstance().file_prozent;
		}

		time_t sTime = time(NULL);
		sTime -= (CMoviePlayerGui::getInstance().GetPosition()/1000);
		tm_struct = localtime(&sTime);

		snprintf(Start, sizeof(Start), "%02d:%02d", tm_struct->tm_hour, tm_struct->tm_min);

		time_t eTime = time(NULL);
		eTime +=(CMoviePlayerGui::getInstance().GetDuration()/1000) - (CMoviePlayerGui::getInstance().GetPosition()/1000);
		tm_struct = localtime(&eTime);

		snprintf(End, sizeof(End), "%02d:%02d", tm_struct->tm_hour, tm_struct->tm_min);
	}

	/* ----------------------------------------------------------------- */

	Event += "\n"; // make sure we have at least two lines in event-file
	if (m_Ev_Desc.compare(Event))
	{
		WriteFile(EVENT, Event, true);
		m_Ev_Desc = Event;
	}

	if (m_Ev_Start.compare((std::string)Start))
	{
		WriteFile(START, (std::string)Start);
		m_Ev_Start = (std::string)Start;
	}

	if (m_Ev_End.compare((std::string)End))
	{
		WriteFile(END, (std::string)End);
		m_Ev_End = (std::string)End;
	}

	if (Progress > 100)
		Progress = 100;

	if (m_Progress != Progress)
	{
		WriteFile(PROGRESS, to_string(Progress));
		m_Progress = Progress;
	}

	if (strcmp(m_Duration, Duration))
	{
		WriteFile(DURATION, (std::string)Duration);
		strcpy(m_Duration, Duration);
	}
}

/* ----------------------------------------------------------------- */

bool CLCD4l::WriteFile(const char *file, std::string content, bool convert)
{
	bool ret = true;

	if (FILE *f = fopen(file, "w"))
	{
		//printf("[CLCD4l] %s: %s -> %s\n", __FUNCTION__, content.c_str(), file);
		fprintf(f, "%s\n", content.c_str());
		fclose(f);
	}
	else
	{
		ret = false;
		printf("[CLCD4l] %s: %s failed!\n", __FUNCTION__, file);
	}

	return ret;
}

uint64_t CLCD4l::GetParseID()
{
	uint64_t ID = CNeutrinoApp::getInstance()->getMode();
	m_Mode = (int) ID;
	m_ModeChannel = 0;

	if (ID == MODE_TV || ID == MODE_RADIO)
	{
		if (!(g_RemoteControl->subChannels.empty()) && (g_RemoteControl->selected_subchannel > 0))
			m_ModeChannel = 2;
		else
			m_ModeChannel = 1;

		if (m_ModeChannel > 1)
			ID = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].getChannelID();
		else
			ID = CZapit::getInstance()->GetCurrentChannelID();
	}
	//printf("[CLCD4l] %s: %llx\n", __FUNCTION__, ID);
	return ID;
}

bool CLCD4l::CompareParseID(uint64_t &i_ParseID)
{
	bool ret = false;

	i_ParseID = GetParseID();
	if (m_ParseID != i_ParseID)
	{
		//printf("[CLCD4l] %s: i_%llx <-> m_%llx\n", __FUNCTION__, i_ParseID, m_ParseID);
		ret = true;
		m_ParseID = i_ParseID;
	}
	return ret;
}

#ifdef NOTNEEDED
// stolen from gui/movieinfo.cpp
void CLCD4l::strReplace(std::string & orig, const char *fstr, const std::string rstr)
{
	unsigned int index = 0;
	unsigned int fstrlen = strlen(fstr);
	int rstrlen = rstr.size();

	while ((index = orig.find(fstr, index)) != std::string::npos)
	{
		orig.replace(index, fstrlen, rstr);
		index += rstrlen;
	}
}
#endif

std::string CLCD4l::hexStr(unsigned char* data)
{
	std::stringstream ss;
	ss << std::hex;
	for(int i=0; i<1; ++i)
		ss << std::setw(2) << std::setfill('0') << (int)data[i]*2.55;
	return ss.str();
}

bool CLCD4l::GetLogoName(uint64_t channel_id, std::string channel_name, std::string &logo)
{
	int h, i, j;
	char str_channel_id[16];
	char *upper_name, *lower_name, *p;

	upper_name = strdup(channel_name.c_str());
	for (p = upper_name; *p != '\0'; p++)
		*p = (char) toupper(*p);

	lower_name = strdup(channel_name.c_str());
	for (p = lower_name; *p != '\0'; p++)
		*p = (char) tolower(*p);

	sprintf(str_channel_id, "%llx", channel_id & 0xFFFFFFFFFFFFULL);
	// the directorys to search in
	std::string strLogoDir[4] = { g_settings.lcd4l_logodir, LOGODIR_VAR, LOGODIR, g_settings.logo_hdd_dir };
	// first the channelname, then the upper channelname, then the lower channelname, then the channel-id
	std::string strLogoName[4] = { channel_name, (std::string)upper_name, (std::string)lower_name, (std::string)str_channel_id };
	// first png, then jpg, then gif
	std::string strLogoExt[3] = { ".png", ".jpg", ".gif" };

	//printf("[CLCD4l] %s: ID: %s, Name: %s (u: %s, l: %s)\n", __FUNCTION__, str_channel_id, channel_name.c_str(), upper_name, lower_name);

	for (h = 0; h < 4; h++)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 3; j++)
			{
				std::string tmp(strLogoDir[h] + "/" + strLogoName[i] + strLogoExt[j]);
				if (access(tmp.c_str(), R_OK) != -1)
				{
					logo = tmp;
					return true;
				}
			}
		}
	}

	return false;
}
