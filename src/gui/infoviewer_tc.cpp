/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Bugfixes/cleanups (C) 2007-2013,2015 Stefan Seyfried
	(C) 2008 Novell, Inc. Author: Stefan Seyfried

	rejoin/cleanup (C) 2017 TangoCash

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "infoviewer_tc.h"

#include <algorithm>
#include <time.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/timeb.h>
#include <sys/param.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include <global.h>
#include <neutrino.h>

#include <gui/bouquetlist.h>
#include <gui/color_custom.h>
#include <gui/widget/icons.h>
#include <gui/widget/hintbox.h>
#include <gui/movieplayer.h>
#include <gui/infoclock.h>
#include <gui/pictureviewer.h>
#include <gui/keybind_setup.h>
#include <gui/components/cc_timer.h>

#include <system/helpers.h>
#include <system/hddstat.h>

#include <daemonc/remotecontrol.h>
#include <driver/record.h>
#include <driver/display.h>
#include <driver/volume.h>
#include <driver/radiotext.h>
#include <driver/fontrenderer.h>

#include <zapit/satconfig.h>
#include <zapit/femanager.h>
#include <zapit/zapit.h>
#include <zapit/capmt.h>
#include <eitd/sectionsd.h>
#include <video.h>

extern CRemoteControl *g_RemoteControl;	/* neutrino.cpp */
extern CBouquetList * bouquetList;       /* neutrino.cpp */
extern CPictureViewer * g_PicViewer;
extern cVideo * videoDecoder;

#define COL_INFOBAR_BUTTONS_BACKGROUND (COL_MENUFOOT_PLUS_0)
#define NEUTRINO_ICON_LOGO "/tmp/logo.png"
#define INFOFILE "/tmp/infobar.txt"

event_id_t CInfoViewer::last_curr_id = 0, CInfoViewer::last_next_id = 0;

static bool sortByDateTime (const CChannelEvent& a, const CChannelEvent& b)
{
	return a.startTime < b.startTime;
}

extern bool timeset;

CInfoViewer::CInfoViewer ()
	: fader(g_settings.theme.infobar_alpha)
{
	header = NULL;
	body = rec = NULL;
	txt_cur_start = txt_cur_event = txt_cur_event_rest = txt_next_start = txt_next_event = txt_next_in = NULL;
	timescale = NULL;
	info_CurrentNext.current_zeit.startzeit = 0;
	info_CurrentNext.current_zeit.dauer = 0;
	info_CurrentNext.flags = 0;
	frameBuffer = CFrameBuffer::getInstance();
	ecmInfoBox = NULL;
	md5_ecmInfo = "0";
	InfoHeightY = 0;
	ButtonWidth = 0;
	rt_dx = 0;
	rt_dy = 0;
	ChanNameX = 0;
	ChanNameY = 0;
	ChanWidth = 0;
	ChanHeight = 0;
	time_width = 0;
	header_height = 0;
	aspectRatio = 0;
	ChanInfoX = 0;
	oldinfo.current_uniqueKey = 0;
	oldinfo.next_uniqueKey = 0;
	isVolscale = false;
	info_time_width = 0;
	timeoutEnd = 0;
	sec_timer_id = 0;
	ecminfo_toggle = false;
	is_visible		= false;
	scrambledErr		= false;
	scrambledErrSave	= false;
	scrambledNoSig		= false;
	scrambledNoSigSave	= false;
	scrambledT		= 0;
	hddscale 		= NULL;
	bbIconInfo[0].x = 0;
	bbIconInfo[0].h = 0;
	BBarY = 0;
	BBarFontY = 0;
	foot			= NULL;
	ca_bar			= NULL;
	recordsbox = NULL;
	recordsblink = NULL;
	showBBIcons_width = 0;
	Init();
}

CInfoViewer::~CInfoViewer()
{
	if(scrambledT)
	{
		pthread_cancel(scrambledT);
		scrambledT = 0;
	}
	ResetModules();
}

void CInfoViewer::Init()
{
	BoxStartX = BoxStartY = BoxEndX = BoxEndY = 0;
	recordModeActive = false;
	is_visible = false;
	showButtonBar = false;
	//gotTime = g_Sectionsd->getIsTimeSet ();
	gotTime = timeset;
	zap_mode = IV_MODE_DEFAULT;
	newfreq = true;
	chanready = 1;
	fileplay = 0;
	SDT_freq_update = false;

	getIconInfo();
	/* after font size changes, Init() might be called multiple times */
	ResetPBars();

	current_channel_id = CZapit::getInstance()->GetCurrentChannelID();;
	current_epg_id = 0;
	lcdUpdateTimer = 0;
	rt_x = rt_y = rt_h = rt_w = 0;

	infobar_txt = NULL;


	_livestreamInfo1.clear();
	_livestreamInfo2.clear();

	// Hardcoded some Settings in neutrino.cpp
	/*
	// these has to be
	g_settings.infobar_show_dd_available=1;
	g_settings.infobar_show_res=1;
	g_settings.infobar_show_sysfs_hdd=0;
	*/

	hddwidth		= 0;
	bbIconMaxH 		= 0;
	bbButtonMaxH 		= 0;
	bbIconMinX 		= 0;
	bbButtonMaxX 		= 0;
	fta			= true;
	minX			= 0;

	DecEndx = 0;
	decode = UNKNOWN;
	camCI = false;
	useCI = false;
	int CiSlots = cCA::GetInstance()->GetNumberCISlots();
	int acc = 0;
	while (acc < CiSlots && acc < 2)
	{
		if (cCA::GetInstance()->ModulePresent(CA_SLOT_TYPE_CI, acc))
		{
			printf("CI: CAM found in Slot %i\n", acc);
			camCI = true;
		}
		else
			printf("CI: CAM not found\n");
		acc++;
	}

	for (int i = 0; i < CInfoViewer::BUTTON_MAX; i++)
	{
		tmp_bbButtonInfoText[i] = "";
		bbButtonInfo[i].x   = -1;
	}

	InfoHeightY_Info = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_FOOT]->getHeight() + 5;
	initBBOffset();

	ResetPBars();
}

/*
 * This nice ASCII art should hopefully explain how all the variables play together ;)
 *

              ___BoxStartX
             |           |
             | recording icons
 BoxStartY---+-ChanWidth-+
     |       | ##################### infobar_txt ###################
     |       +-------------------------------------------------------+--+-ChanNameY-----+
     |       | Channelname (header)                [DD][16:9]| clock |  | header height |
 ChanHeight--|-------------------------------------------------------+--+               |
             |                    B---O---D---Y                      |                  |InfoHeightY
             |   01:23     Current Event                             |                  |
             |   02:34     Next Event                                |                  |
             |                                                       |                  |
     BoxEndY-+-------------------------------------------------------+--+---------------+
             |                        ca_bar                         |  bottom_bar_offset
     BBarY---+-------------------------------------------------------+--+
             | * red         * green      * yellow            * blue |  InfoHeightY_Info
             +-------------------------------------------------------+--+
             |                         asize                         |
                                                             BoxEndX-/
*/
void CInfoViewer::start ()
{
	info_time_width = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth("22:22") + 10;

	InfoHeightY = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getHeight() * 9/8 +
	              2 * g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight() + 25;

	ChanWidth = max(125, 4 * g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->getMaxDigitWidth() + 10);

	ChanHeight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->getHeight()/* * 9/8*/;
	ChanHeight += g_SignalFont->getHeight()/2;
	ChanHeight = max(75, ChanHeight);

	BoxStartX = g_settings.screen_StartX + 10;
	BoxEndX = g_settings.screen_EndX - 10;
	BoxEndY = g_settings.screen_EndY - 10 - InfoHeightY_Info - bottom_bar_offset;
	BoxStartY = BoxEndY - InfoHeightY - ChanHeight / 2;

	ChanNameY = BoxStartY + (ChanHeight / 2);	//oberkante schatten?
	ChanInfoX = BoxStartX;

}

void CInfoViewer::showRecords()
{
	CRecordManager * crm	= CRecordManager::getInstance();

	int box_posX = BoxStartX;
	int box_posY = BoxStartY - OFFSET_SHADOW;

	if (!(access(INFOFILE, F_OK) == 0))
		box_posY += g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight() + 2 + OFFSET_SHADOW;

	if (!recordsblink)
		recordsblink = new CComponentsTimer(1);

	if (crm->RecordingStatus())
	{
		if (recordsbox)
		{
			recordsbox->kill();
			delete recordsbox;
			recordsbox = NULL;
		}
		recordsbox = new CComponentsForm();
		recordsbox->setWidth(0);
		recordsbox->doPaintBg(false);
		recordsbox->setCornerType(CORNER_NONE);
		//recordsbox->setColorBody(COL_BACKGROUND_PLUS_0);

		recmap_t recmap = crm->GetRecordMap();
		std::vector<CComponentsPicture*> images;
		CComponentsForm *recline = NULL;
		CComponentsText *rec_name = NULL;
		int y_recline = 0;
		int w_recbox = 0;
		int w_shadow = OFFSET_SHADOW/2;

		for(recmap_iterator_t it = recmap.begin(); it != recmap.end(); it++)
		{
			CRecordInstance * inst = it->second;

			recline = new CComponentsForm(0, y_recline, 0);
			recline->doPaintBg(true);
			recline->setColorBody(COL_INFOBAR_PLUS_0);
			recline->enableShadow(CC_SHADOW_ON, w_shadow);
			recline->setCorner(CORNER_RADIUS_MID);
			recordsbox->addCCItem(recline);

			CComponentsPicture *iconf = new CComponentsPicture(OFFSET_INNER_MID, 0, NEUTRINO_ICON_REC, recline, CC_SHADOW_OFF, COL_RED, COL_INFOBAR_PLUS_0);
			iconf->setCornerType(CORNER_NONE);
			iconf->doPaintBg(true);
			iconf->SetTransparent(CFrameBuffer::TM_BLACK);
			images.push_back(iconf);
			recline->setHeight(iconf->getHeight());

			std::string records_msg = inst->GetEpgTitle();

			rec_name = new CComponentsText(iconf->getWidth()+2*OFFSET_INNER_MID, 0, 0, 0, records_msg, CTextBox::AUTO_WIDTH, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]);
			rec_name->doPaintBg(false);
			rec_name->setTextColor(COL_INFOBAR_TEXT);

			recline->setHeight(std::max(rec_name->getHeight(), iconf->getHeight()));
			recline->setWidth(OFFSET_INNER_MIN+iconf->getWidth()+OFFSET_INNER_MID+rec_name->getWidth()+OFFSET_INNER_MID);
			w_recbox = (std::max(recline->getWidth(), recordsbox->getWidth()));
			recordsbox->setWidth(w_recbox);
			recline->addCCItem(rec_name);

			y_recline += recline->getHeight() + OFFSET_SHADOW;
		}

		int h_rbox = 0;
		for(size_t i = 0; i< recordsbox->size(); i++)
			h_rbox += recordsbox->getCCItem(i)->getHeight() + OFFSET_SHADOW;

		recordsbox->setDimensionsAll(box_posX, box_posY-h_rbox, w_recbox+w_shadow, h_rbox);
		recordsbox->paint0();

		for(size_t j = 0; j< images.size(); j++)
		{
			images[j]->kill();
			images[j]->paintBlink(recordsblink);
		}

	}
}

void CInfoViewer::paintHead(t_channel_id channel_id,std::string channel_name)
{
	int head_x = BoxStartX;
	int head_w = BoxEndX-head_x;
	int head_h = max(ChanHeight / 2,g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getHeight());

	CInfoClock::getInstance()->disableInfoClock();

	if (header == NULL)
	{
		header = new CComponentsHeader();
		header->enableShadow(CC_SHADOW_RIGHT_CORNER_ALL);
		header->setCorner(RADIUS_LARGE, CORNER_TOP);
	}

	header->setDimensionsAll(head_x, ChanNameY, head_w, head_h);

	header->setCaptionFont(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]);
	header->setColorBody(g_settings.theme.infobar_gradient_top ? COL_MENUHEAD_PLUS_0 : COL_INFOBAR_PLUS_0);
	header->enableColBodyGradient(g_settings.theme.infobar_gradient_top, COL_INFOBAR_PLUS_0, g_settings.theme.infobar_gradient_top_direction);
	if (!g_settings.infobar_anaclock || g_settings.channellist_show_numbers)
	{
		header->enableClock(true, "%H:%M ", "%H.%M ", true);
		header->getClockObject()->setCorner(RADIUS_LARGE, CORNER_TOP_RIGHT);
		header->getClockObject()->setTextColor(g_settings.theme.infobar_gradient_top ? COL_MENUHEAD_TEXT : COL_INFOBAR_TEXT);
		header->getClockObject()->setClockFont(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]);
		time_width = header->getClockObject()->getWidth();
	}

	header->setChannelLogo(channel_id,channel_name,CCHeaderTypes::CC_LOGO_LEFT);
	header->setCaptionMaxWidth(header->getWidth() - time_width - 3*OFFSET_INNER_MID - showBBIcons_width);

	if (!header->getChannelLogoObject()->hasLogo())
		header->setCaption(channel_name);
	else
		header->setCaption("");

	header->paint(CC_SAVE_SCREEN_NO);
	header_height = header->getHeight();
}

void CInfoViewer::paintBody()
{
	int h_body = InfoHeightY - header_height;// - OFFSET_SHADOW;
	if(h_body < 0)
		h_body = 0;

	initBBOffset();
	if (!zap_mode)
		h_body += bottom_bar_offset;

	int y_body = ChanNameY + header_height;

	if (body == NULL)
	{
		body = new CComponentsShapeSquare(ChanInfoX, y_body, BoxEndX-ChanInfoX, h_body);
	}
	else
	{
		body->setDimensionsAll(ChanInfoX, y_body, BoxEndX-ChanInfoX, h_body);
	}

	//set corner and shadow modes, consider virtual zap mode
	body->setCorner(RADIUS_LARGE, (zap_mode) ? CORNER_BOTTOM : CORNER_NONE);
	body->enableShadow(zap_mode ? CC_SHADOW_ON : CC_SHADOW_RIGHT | CC_SHADOW_CORNER_TOP_RIGHT | CC_SHADOW_CORNER_BOTTOM_RIGHT);

	body->setColorBody(g_settings.theme.infobar_gradient_body ? COL_MENUHEAD_PLUS_0 : COL_INFOBAR_PLUS_0);
	body->enableColBodyGradient(g_settings.theme.infobar_gradient_body, COL_INFOBAR_PLUS_0, g_settings.theme.infobar_gradient_body_direction);

	body->paint(CC_SAVE_SCREEN_NO);
}

void CInfoViewer::show_current_next(bool new_chan, int  epgpos)
{
	CEitManager::getInstance()->getCurrentNextServiceKey(current_epg_id, info_CurrentNext);
	if (!evtlist.empty())
	{
		if (new_chan)
		{
			for ( eli=evtlist.begin(); eli!=evtlist.end(); ++eli )
			{
				if ((uint)eli->startTime >= info_CurrentNext.current_zeit.startzeit + info_CurrentNext.current_zeit.dauer)
					break;
			}
			if (eli == evtlist.end()) // the end is not valid, so go back
				--eli;
		}

		if (epgpos != 0)
		{
			info_CurrentNext.flags = 0;
			if ((epgpos > 0) && (eli != evtlist.end()))
			{
				++eli; // next epg
				if (eli == evtlist.end()) // the end is not valid, so go back
					--eli;
			}
			else if ((epgpos < 0) && (eli != evtlist.begin()))
			{
				--eli; // prev epg
			}
			info_CurrentNext.flags = CSectionsdClient::epgflags::has_current;
			info_CurrentNext.current_uniqueKey      = eli->eventID;
			info_CurrentNext.current_zeit.startzeit = eli->startTime;
			info_CurrentNext.current_zeit.dauer     = eli->duration;
			if (eli->description.empty())
				info_CurrentNext.current_name   = g_Locale->getText(LOCALE_INFOVIEWER_NOEPG);
			else
				info_CurrentNext.current_name   = eli->description;
			info_CurrentNext.current_fsk            = '\0';

			if (eli != evtlist.end())
			{
				++eli;
				if (eli != evtlist.end())
				{
					info_CurrentNext.flags                  = CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::has_next;
					info_CurrentNext.next_uniqueKey         = eli->eventID;
					info_CurrentNext.next_zeit.startzeit    = eli->startTime;
					info_CurrentNext.next_zeit.dauer        = eli->duration;
					if (eli->description.empty())
						info_CurrentNext.next_name      = g_Locale->getText(LOCALE_INFOVIEWER_NOEPG);
					else
						info_CurrentNext.next_name      = eli->description;
				}
				--eli;
			}
		}
	}

	if (!(info_CurrentNext.flags & (CSectionsdClient::epgflags::has_later | CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::not_broadcast)))
	{
		neutrino_locale_t loc;
		if (!gotTime)
			loc = LOCALE_INFOVIEWER_WAITTIME;
		else if (showButtonBar)
			loc = LOCALE_INFOVIEWER_EPGWAIT;
		else
			loc = LOCALE_INFOVIEWER_EPGNOTLOAD;

		_livestreamInfo1.clear();
		_livestreamInfo2.clear();
		if (!showLivestreamInfo())
			display_Info(g_Locale->getText(loc), NULL);
	}
	else
	{
		show_Data ();
	}
}

void CInfoViewer::showMovieTitle(const int playState, const t_channel_id &Channel_Id, const std::string &Channel,
                                 const std::string &g_file_epg, const std::string &g_file_epg1,
                                 const int duration, const int curr_pos,
                                 const int repeat_mode, const int _zap_mode)
{
	if (g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_LEFT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_RIGHT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_CENTER ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_HIGHER_CENTER)
		isVolscale = CVolume::getInstance()->hideVolscale();
	else
		isVolscale = false;

	initBBOffset();
	start();
	aspectRatio = 0;
	last_curr_id = last_next_id = 0;
	showButtonBar = true;
	fileplay = true;
	zap_mode = _zap_mode;
	ResetPBars();
	if (!gotTime)
		gotTime = timeset;

	if (g_settings.radiotext_enable && g_Radiotext)
	{
		g_Radiotext->RT_MsgShow = true;
	}

	if(!is_visible)
		fader.StartFadeIn();

	is_visible = true;
	is_visible = true;

	ChannelName = Channel;
	t_channel_id old_channel_id = current_channel_id;
	current_channel_id = Channel_Id;

	ChanNameX = BoxStartX + ChanWidth + OFFSET_SHADOW;

	paintHead(current_channel_id,ChannelName);
	paintBody();

	showRecords();
	ana_clock_size = (BoxEndY - (ChanNameY + header_height) - 6);
	if (!g_settings.channellist_show_numbers && g_settings.infobar_anaclock)
		showClock_analog(ChanInfoX + 10 + ana_clock_size/2,BoxEndY - ana_clock_size / 2 - 3, ana_clock_size / 2);

	ChanNumWidth = g_settings.infobar_anaclock ? ana_clock_size : g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->getRenderWidth("888") + 5;

	if (!zap_mode)
		paintshowButtonBar();

	int renderFlag = ((g_settings.theme.infobar_gradient_top) ? Font::FULLBG : 0) | Font::IS_UTF8;

	// show_Data
	if (CMoviePlayerGui::getInstance().file_prozent > 100)
		CMoviePlayerGui::getInstance().file_prozent = 100;

	const char *unit_short_minute = g_Locale->getText(LOCALE_UNIT_SHORT_MINUTE);
	char runningRest[32]; // %d can be 10 digits max...
	snprintf(runningRest, sizeof(runningRest), "%d / %d %s", (curr_pos + 30000) / 60000, (duration - curr_pos + 30000) / 60000, unit_short_minute);
	display_Info(g_file_epg.c_str(), g_file_epg1.c_str(), false, CMoviePlayerGui::getInstance().file_prozent, NULL, runningRest);

	int speed = CMoviePlayerGui::getInstance().GetSpeed();
	const char *playicon = NULL;
	switch (playState)
	{
	case CMoviePlayerGui::PLAY:
		switch (repeat_mode)
		{
		case CMoviePlayerGui::REPEAT_ALL:
			playicon = NEUTRINO_ICON_PLAY_REPEAT_ALL;
			break;
		case CMoviePlayerGui::REPEAT_TRACK:
			playicon = NEUTRINO_ICON_PLAY_REPEAT_TRACK;
			break;
		default:
			playicon = NEUTRINO_ICON_PLAY;
		}
		speed = 0;
		break;
	case CMoviePlayerGui::PAUSE:
		playicon = NEUTRINO_ICON_PAUSE;
		break;
	case CMoviePlayerGui::REW:
		playicon = NEUTRINO_ICON_REW;
		speed = abs(speed);
		break;
	case CMoviePlayerGui::FF:
		playicon = NEUTRINO_ICON_FF;
		speed = abs(speed);
		break;
	default:
		/* NULL crashes in getIconSize, just use something */
		playicon = NEUTRINO_ICON_BUTTON_HELP;
		break;
	}
	int icon_w = 0,icon_h = 0;
	frameBuffer->getIconSize(playicon, &icon_w, &icon_h);
	int speedw = 0;
	if (speed)
	{
		sprintf(runningRest, "%dx", speed);
		speedw = 5 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(runningRest);
		icon_w += speedw;
	}
	int icon_x = ChanInfoX + 10 + ChanNumWidth / 2 - icon_w / 2;
	int icon_y = (BoxEndY + ChanNameY + header_height) / 2 - icon_h / 2;
	if (speed)
	{
		int sh = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight();
		int sy = (BoxEndY + ChanNameY + header_height) / 2 - sh/2 + sh;
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(icon_x, sy, ChanHeight, runningRest, COL_INFOBAR_TEXT, 0, renderFlag);
		icon_x += speedw;
	}
	frameBuffer->paintIcon(playicon, icon_x, icon_y);
	showLcdPercentOver ();
	showInfoFile();

	if ((access("/tmp/cover.jpg", F_OK) == 0))
	{
		icon_w = ChanNumWidth;
		icon_h = BoxEndY - (ChanNameY + header_height) - 6 ;
		icon_x =  ChanInfoX + 10 + ChanNumWidth / 2 - icon_w / 2;
		icon_y = (BoxEndY + ChanNameY + header_height) / 2 - icon_h / 2;
		g_PicViewer->DisplayImage("/tmp/cover.jpg", icon_x, icon_y, icon_w, icon_h, 1);
	}

	loop();
	aspectRatio = 0;
	fileplay = 0;
	current_channel_id = old_channel_id;
}

void CInfoViewer::showTitle(t_channel_id chid, const bool calledFromNumZap, int epgpos)
{
	CZapitChannel * channel = CServiceManager::getInstance()->FindChannel(chid);

	if(channel)
		showTitle(channel, calledFromNumZap, epgpos);
}

void CInfoViewer::showTitle(CZapitChannel * channel, const bool calledFromNumZap, int epgpos)
{
	ecminfo_toggle = false;

	if(!calledFromNumZap && !(zap_mode & IV_MODE_DEFAULT))
		resetSwitchMode();

	std::string Channel = channel->getName();
	t_channel_id new_channel_id = channel->getChannelID();
	int ChanNum = channel->number;
	char strChanNum[10];

	current_epg_id = channel->getEpgID();

	if (g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_LEFT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_RIGHT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_CENTER ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_HIGHER_CENTER)
		isVolscale = CVolume::getInstance()->hideVolscale();
	else
		isVolscale = false;

	initBBOffset();
	start();
	aspectRatio = 0;
	last_curr_id = last_next_id = 0;
	showButtonBar = !calledFromNumZap;

	fileplay = (ChanNum == 0);
	newfreq = true;

	ResetPBars();
	if (!gotTime)
		gotTime = timeset;

	if(!is_visible && !calledFromNumZap)
		fader.StartFadeIn();

	is_visible = true;
	is_visible = true;

	ChannelName = Channel;
	bool new_chan = false;

	if (zap_mode & IV_MODE_VIRTUAL_ZAP)
	{
		if ((current_channel_id != new_channel_id) || (evtlist.empty()))
		{
			CEitManager::getInstance()->getEventsServiceKey(current_epg_id, evtlist);
			if (!evtlist.empty())
				sort(evtlist.begin(),evtlist.end(), sortByDateTime);
			new_chan = true;
		}
	}
	if (! calledFromNumZap && !(g_RemoteControl->subChannels.empty()) && (g_RemoteControl->selected_subchannel > 0))
	{
		current_channel_id = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].getChannelID();
		ChannelName = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].subservice_name;
	}
	else
	{
		current_channel_id = new_channel_id;
	}

	ChanNameX = BoxStartX + ChanWidth + OFFSET_SHADOW;

	paintHead(current_channel_id,ChannelName);
	paintBody();

	showRecords();
	ana_clock_size = (BoxEndY - (ChanNameY + header_height) - 6);
	if (!g_settings.channellist_show_numbers && g_settings.infobar_anaclock)
		showClock_analog(ChanInfoX + 10 + ana_clock_size/2,BoxEndY - ana_clock_size / 2 - 3, ana_clock_size / 2);

	if (showButtonBar)
	{
		paintshowButtonBar();
	}

	if ((ChanNum) && (g_settings.channellist_show_numbers))
	{
		snprintf (strChanNum, sizeof(strChanNum), "%d", ChanNum);
		ChanNumWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->getRenderWidth(strChanNum) + 5;
	}
	else
		ChanNumWidth = g_settings.infobar_anaclock ? ana_clock_size : 5;

	if (fileplay)
	{
		show_Data ();
	}
	else
	{
		show_current_next(new_chan, epgpos);
	}

	showLcdPercentOver ();
	showInfoFile();

	int ChanNumYPos = (BoxEndY + ChanNameY + header_height) /2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->getHeight() /2;

	if ((g_settings.channellist_show_numbers) && (ChanNum))
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER]->RenderString(
		    ChanInfoX + 10, ChanNumYPos,
		    ChanNumWidth, strChanNum, COL_INFOBAR_TEXT);

	// Radiotext
	if (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio)
	{
		if ((g_settings.radiotext_enable) && (!recordModeActive) && (!calledFromNumZap))
			showRadiotext();
		else
			showIcon_RadioText(false);
	}

	if (!calledFromNumZap)
	{
		loop();
	}
	else
		frameBuffer->blit();
	aspectRatio = 0;
	fileplay = 0;
}

void CInfoViewer::setInfobarTimeout(int timeout_ext)
{
	int mode = CNeutrinoApp::getInstance()->getMode();
	//define timeouts
	switch (mode)
	{
	case NeutrinoMessages::mode_tv:
	case NeutrinoMessages::mode_webtv:
		timeoutEnd = CRCInput::calcTimeoutEnd (g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] + timeout_ext);
		break;
	case NeutrinoMessages::mode_radio:
		timeoutEnd = CRCInput::calcTimeoutEnd (g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR_RADIO] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR_RADIO] + timeout_ext);
		break;
	case NeutrinoMessages::mode_ts:
		timeoutEnd = CRCInput::calcTimeoutEnd (g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR_MOVIE] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR_MOVIE] + timeout_ext);
		break;
	default:
		timeoutEnd = CRCInput::calcTimeoutEnd(6 + timeout_ext);
		break;
	}
}

bool CInfoViewer::showLivestreamInfo()
{
	CZapitChannel * cc = CZapit::getInstance()->GetCurrentChannel();
	if (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv &&
	        cc->getEpgID() == 0 && !cc->getScriptName().empty())
	{
		std::string livestreamInfo1 = "";
		std::string livestreamInfo2 = "";
		std::string tmp1            = "";
		CMoviePlayerGui::getInstance().getLivestreamInfo(&livestreamInfo1, &tmp1);

		if (!(videoDecoder->getBlank()))
		{
			int xres, yres, framerate;
			std::string tmp2;
			videoDecoder->getPictureInfo(xres, yres, framerate);
			switch (framerate)
			{
			case 0:
				tmp2 = "23.976fps";
				break;
			case 1:
				tmp2 = "24fps";
				break;
			case 2:
				tmp2 = "25fps";
				break;
			case 3:
				tmp2 = "29,976fps";
				break;
			case 4:
				tmp2 = "30fps";
				break;
			case 5:
				tmp2 = "50fps";
				break;
			case 6:
				tmp2 = "50,94fps";
				break;
			case 7:
				tmp2 = "60fps";
				break;
			default:
				tmp2 = g_Locale->getText(LOCALE_STREAMINFO_FRAMERATE_UNKNOWN);
				break;
			}
			livestreamInfo2 = to_string(xres) + "x" + to_string(yres) + ", " + tmp2;
			if (!tmp1.empty())
				livestreamInfo2 += (std::string)", " + tmp1;
		}

		if (livestreamInfo1 != _livestreamInfo1 || livestreamInfo2 != _livestreamInfo2)
		{
			display_Info(livestreamInfo1.c_str(), livestreamInfo2.c_str(), false);
			_livestreamInfo1 = livestreamInfo1;
			_livestreamInfo2 = livestreamInfo2;
			showButtons(true /*paintFooter*/);
		}
		return true;
	}
	return false;
}

void CInfoViewer::loop()
{
	bool hideIt = true;
	resetSwitchMode(); //no virtual zap
	//bool fadeOut = false;
	timeoutEnd=0;;
	setInfobarTimeout();

	int res = messages_return::none;
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	if (isVolscale)
		CVolume::getInstance()->showVolscale();

	_livestreamInfo1.clear();
	_livestreamInfo2.clear();

	bool blink = true;

	while (!(res & (messages_return::cancel_info | messages_return::cancel_all)))
	{
		frameBuffer->blit();
		g_RCInput->getMsgAbsoluteTimeout (&msg, &data, &timeoutEnd);

		showLivestreamInfo();

#ifdef ENABLE_PIP
		if ((msg == (neutrino_msg_t) g_settings.key_pip_close) ||
		        (msg == (neutrino_msg_t) g_settings.key_pip_setup) ||
		        (msg == (neutrino_msg_t) g_settings.key_pip_swap))
		{
			g_RCInput->postMsg(msg, data);
			res = messages_return::cancel_info;
		}
		else
#endif
			if (msg == (neutrino_msg_t) g_settings.key_screenshot)
			{
				res = CNeutrinoApp::getInstance()->handleMsg(msg, data);

			}
			else if (CNeutrinoApp::getInstance()->listModeKey(msg))
			{
				g_RCInput->postMsg (msg, 0);
				res = messages_return::cancel_info;
			}
			else if (msg == CRCInput::RC_help || msg == CRCInput::RC_info)
			{
				if (fileplay)
				{
					CMoviePlayerGui::getInstance().setFromInfoviewer(true);
					g_RCInput->postMsg (msg, data);
					hideIt = true;
					res = messages_return::cancel_info;
				}
				else if (g_settings.show_ecm_pos)
				{
					if (ecminfo_toggle)
					{
						ecmInfoBox_hide();
						g_RCInput->postMsg (NeutrinoMessages::SHOW_EPG, 0);
						res = messages_return::cancel_info;
					}
					else
					{
						showIcon_CA_Status(0);
					}
					ecminfo_toggle = !ecminfo_toggle;
				}
				else
				{
					g_RCInput->postMsg (NeutrinoMessages::SHOW_EPG, 0);
					res = messages_return::cancel_info;
				}
			}
			else if ((msg == NeutrinoMessages::EVT_TIMER) && (data == fader.GetFadeTimer()))
			{
				if(fader.FadeDone())
					res = messages_return::cancel_info;
			}
			else if ((msg == CRCInput::RC_ok) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_timeout))
			{
				if ((g_settings.mode_left_right_key_tv == SNeutrinoSettings::VZAP) && (msg == CRCInput::RC_ok))
				{
					if (fileplay)
					{
						// in movieplayer mode process vzap keys in movieplayer.cpp
						//printf("%s:%d: imitate VZAP; RC_ok\n", __func__, __LINE__);
						CMoviePlayerGui::getInstance().setFromInfoviewer(true);
						g_RCInput->postMsg (msg, data);
						hideIt = true;
					}
				}
				if(fader.StartFadeOut())
					timeoutEnd = CRCInput::calcTimeoutEnd (1);
				else
					res = messages_return::cancel_info;
			}
			else if ((g_settings.mode_left_right_key_tv == SNeutrinoSettings::VZAP) && ((msg == CRCInput::RC_right) || (msg == CRCInput::RC_left )))
			{
				if (fileplay)
				{
					// in movieplayer mode process vzap keys in movieplayer.cpp
					//printf("%s:%d: imitate VZAP; RC_left/right\n", __func__, __LINE__);
					CMoviePlayerGui::getInstance().setFromInfoviewer(true);
					g_RCInput->postMsg (msg, data);
					hideIt = true;
				}
				else
					setSwitchMode(IV_MODE_VIRTUAL_ZAP);
				res = messages_return::cancel_all;
				hideIt = true;
			}
			else if ((msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id))
			{
				// doesn't belong here, but easiest way to check for a change ...
				if (is_visible && showButtonBar)
					showIcon_CA_Status(0);
				showIcon_Update (blink);
				blink = !blink;
				showInfoFile();
				if (!g_settings.channellist_show_numbers && g_settings.infobar_anaclock)
					showClock_analog(ChanInfoX + 10 + ana_clock_size/2,BoxEndY - ana_clock_size / 2 - 3, ana_clock_size / 2);
				if ((g_settings.radiotext_enable) && (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio))
					showRadiotext();

				showIcon_16_9();
				//showIcon_CA_Status(0);
				if(file_exists("/tmp/ecm.info"))
				{
					std::string md5_tmp = filehash((char *)"/tmp/ecm.info");
					//printf("CInfoViewer::loop() ecm.info.tmp = %s\nCInfoViewer::loop() ecm.info     = %s\n",md5_ecmInfo.c_str(),md5_tmp.c_str());
					if(md5_ecmInfo != md5_tmp)
					{
						puts("CInfoViewer::loop() CA reload");
						showIcon_CA_Status(0);
					}
				}
				showIcon_Resolution();
			}
			else if ((msg == NeutrinoMessages::EVT_RECORDMODE) &&
			         (CMoviePlayerGui::getInstance().timeshift) && (CRecordManager::getInstance()->GetRecordCount() == 1))
			{
				res = CNeutrinoApp::getInstance()->handleMsg(msg, data);
			}
			else if (!fileplay && !CMoviePlayerGui::getInstance().timeshift)
			{
				CNeutrinoApp *neutrino = CNeutrinoApp::getInstance ();
				if ((msg == (neutrino_msg_t) g_settings.key_quickzap_up) || (msg == (neutrino_msg_t) g_settings.key_quickzap_down) || (msg == CRCInput::RC_0) || (msg == NeutrinoMessages::SHOW_INFOBAR))
				{
					hideIt = false; // default
					if ((g_settings.radiotext_enable) && (neutrino->getMode() == NeutrinoMessages::mode_radio))
						hideIt =  true;

					int rec_mode = CRecordManager::getInstance()->GetRecordMode();

					/* hide, if record (not timeshift only) is running -> neutrino will show channel list */
					if (rec_mode & CRecordManager::RECMODE_REC)
						hideIt = true;

					g_RCInput->postMsg (msg, data);
					res = messages_return::cancel_info;
				}
				else if (msg == NeutrinoMessages::EVT_TIMESET)
				{
					/* handle timeset event in upper layer, ignore here */
					res = neutrino->handleMsg (msg, data);
				}
				else
				{
					if (msg == CRCInput::RC_standby)
					{
						g_RCInput->killTimer (sec_timer_id);
						fader.StopFade();
					}
					res = neutrino->handleMsg (msg, data);
					if (res & messages_return::unhandled)
					{
						// raus hier und im Hauptfenster behandeln...
						g_RCInput->postMsg (msg, data);
						res = messages_return::cancel_info;
					}
				}
			}
			else if (fileplay || CMoviePlayerGui::getInstance().timeshift)
			{

				/* this debug message will only hit in movieplayer mode, where console is
				 * spammed to death anyway... */
				printf("%s:%d msg->MP: %08lx, data: %08lx\n", __func__, __LINE__, (long)msg, (long)data);

				bool volume_keys = (
				                       msg == CRCInput::RC_spkr
				                       || msg == (neutrino_msg_t) g_settings.key_volumeup
				                       || msg == (neutrino_msg_t) g_settings.key_volumedown
				                   );

				if (msg < CRCInput::RC_Events && !volume_keys)
				{
					g_RCInput->postMsg (msg, data);
					res = messages_return::cancel_info;
				}
				else
					res = CNeutrinoApp::getInstance()->handleMsg(msg, data);
			}
	}

	if (hideIt)
	{
		CVolume::getInstance()->hideVolscale();
		killTitle ();
	}

	g_RCInput->killTimer (sec_timer_id);
	fader.StopFade();
	if (zap_mode & IV_MODE_VIRTUAL_ZAP)
	{
		/* if bouquet cycle set, do virtual over current bouquet */
		if (/*g_settings.zap_cycle && */ /* (bouquetList != NULL) && */ !(bouquetList->Bouquets.empty()))
			bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->virtual_zap_mode(msg == CRCInput::RC_right);
		else
			CNeutrinoApp::getInstance()->channelList->virtual_zap_mode(msg == CRCInput::RC_right);
	}
}

void CInfoViewer::showSubchan ()
{
	CFrameBuffer *lframeBuffer = CFrameBuffer::getInstance ();
	CNeutrinoApp *neutrino = CNeutrinoApp::getInstance ();

	std::string subChannelName;	// holds the name of the subchannel/audio channel
	int subchannel = 0;				// holds the channel index
	const int borderwidth = 4;

	if (!(g_RemoteControl->subChannels.empty ()))
	{
		// get info for nvod/subchannel
		subchannel = g_RemoteControl->selected_subchannel;
		if (g_RemoteControl->selected_subchannel >= 0)
			subChannelName = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].subservice_name;
	}
	else if (g_RemoteControl->current_PIDs.APIDs.size () > 1 && g_settings.audiochannel_up_down_enable)
	{
		// get info for audio channel
		subchannel = g_RemoteControl->current_PIDs.PIDs.selected_apid;
		subChannelName = g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].desc;
	}

	if (!(subChannelName.empty ()))
	{
		if ( g_settings.infobar_subchan_disp_pos == 4 )
		{
			g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
		}
		else
		{
			char text[100];
			snprintf (text, sizeof(text), "%d - %s", subchannel, subChannelName.c_str ());

			int dx = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth (text) + 20;
			int dy = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight(); // 25;

			if (g_RemoteControl->director_mode)
			{
				int w = 20 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth (g_Locale->getText (LOCALE_NVODSELECTOR_DIRECTORMODE)) + 20;
				int h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
				if (w > dx)
					dx = w;
				dy = dy + h + 5; //dy * 2;
			}
			else
				dy = dy + 5;

			int x = 0, y = 0;
			if (g_settings.infobar_subchan_disp_pos == 0)
			{
				// Rechts-Oben
				x = g_settings.screen_EndX - dx - 10;
				y = g_settings.screen_StartY + 10;
			}
			else if (g_settings.infobar_subchan_disp_pos == 1)
			{
				// Links-Oben
				x = g_settings.screen_StartX + 10;
				y = g_settings.screen_StartY + 10;
			}
			else if (g_settings.infobar_subchan_disp_pos == 2)
			{
				// Links-Unten
				x = g_settings.screen_StartX + 10;
				y = g_settings.screen_EndY - dy - 10;
			}
			else if (g_settings.infobar_subchan_disp_pos == 3)
			{
				// Rechts-Unten
				x = g_settings.screen_EndX - dx - 10;
				y = g_settings.screen_EndY - dy - 10;
			}

			fb_pixel_t pixbuf[(dx + 2 * borderwidth) * (dy + 2 * borderwidth)];
			lframeBuffer->SaveScreen (x - borderwidth, y - borderwidth, dx + 2 * borderwidth, dy + 2 * borderwidth, pixbuf);

			// clear border
			lframeBuffer->paintBackgroundBoxRel (x - borderwidth, y - borderwidth, dx + 2 * borderwidth, borderwidth);
			lframeBuffer->paintBackgroundBoxRel (x - borderwidth, y + dy, dx + 2 * borderwidth, borderwidth);
			lframeBuffer->paintBackgroundBoxRel (x - borderwidth, y, borderwidth, dy);
			lframeBuffer->paintBackgroundBoxRel (x + dx, y, borderwidth, dy);

			lframeBuffer->paintBoxRel (x, y, dx, dy, COL_MENUCONTENT_PLUS_0);
			//g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (x + 10, y + 30, dx - 20, text, COL_MENUCONTENT_TEXT);

			if (g_RemoteControl->director_mode)
			{
				lframeBuffer->paintIcon (NEUTRINO_ICON_BUTTON_YELLOW, x + 8, y + dy - 20);
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString (x + 30, y + dy - 2, dx - 40, g_Locale->getText (LOCALE_NVODSELECTOR_DIRECTORMODE), COL_MENUCONTENT_TEXT);
				int h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (x + 10, y + dy - h - 2, dx - 20, text, COL_MENUCONTENT_TEXT);
			}
			else
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (x + 10, y + dy - 2, dx - 20, text, COL_MENUCONTENT_TEXT);

			uint64_t timeoutEnd_tmp = CRCInput::calcTimeoutEnd (2);
			int res = messages_return::none;

			neutrino_msg_t msg;
			neutrino_msg_data_t data;

			while (!(res & (messages_return::cancel_info | messages_return::cancel_all)))
			{
				g_RCInput->getMsgAbsoluteTimeout (&msg, &data, &timeoutEnd_tmp);

				if (msg == CRCInput::RC_timeout)
				{
					res = messages_return::cancel_info;
				}
				else
				{
					res = neutrino->handleMsg (msg, data);

					if (res & messages_return::unhandled)
					{
						// raus hier und im Hauptfenster behandeln...
						g_RCInput->postMsg (msg, data);
						res = messages_return::cancel_info;
					}
				}
			}
			lframeBuffer->RestoreScreen (x - borderwidth, y - borderwidth, dx + 2 * borderwidth, dy + 2 * borderwidth, pixbuf);
		}
	}
	else
	{
		g_RCInput->postMsg (NeutrinoMessages::SHOW_INFOBAR, 0);
	}
}

void CInfoViewer::showFailure ()
{
	ShowHint (LOCALE_MESSAGEBOX_ERROR, g_Locale->getText (LOCALE_INFOVIEWER_NOTAVAILABLE), 430);	// UTF-8
}

void CInfoViewer::showMotorMoving (int duration)
{
	setInfobarTimeout(duration + 1);

	char text[256];
	snprintf(text, sizeof(text), "%s (%ds)", g_Locale->getText (LOCALE_INFOVIEWER_MOTOR_MOVING), duration);
	ShowHint (LOCALE_MESSAGEBOX_INFO, text, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth (text) + 10, duration);	// UTF-8
}

void CInfoViewer::killRadiotext()
{
	if (g_Radiotext->S_RtOsd)
		frameBuffer->paintBackgroundBox(rt_x, rt_y, rt_w, rt_h);
	rt_x = rt_y = rt_h = rt_w = 0;
	CInfoClock::getInstance()->enableInfoClock(true);
}

void CInfoViewer::showRadiotext()
{
	char stext[3][100];
	bool RTisIsUTF = false;

	if (g_Radiotext == NULL) return;
	showIcon_RadioText(g_Radiotext->haveRadiotext());

	bool blit = false;

	if (g_Radiotext->S_RtOsd)
	{
		CInfoClock::getInstance()->enableInfoClock(false);
		// dimensions of radiotext window
		int /*yoff = 8,*/ ii = 0;
		rt_dx = BoxEndX - BoxStartX;
		rt_dy = 25;
		rt_x = BoxStartX;
		rt_y = g_settings.screen_StartY + 10;
		rt_h = rt_y + 7 + rt_dy*(g_Radiotext->S_RtOsdRows+1)+OFFSET_SHADOW;
		rt_w = rt_x+rt_dx+OFFSET_SHADOW;

		int lines = 0;
		for (int i = 0; i < g_Radiotext->S_RtOsdRows; i++)
		{
			if (g_Radiotext->RT_Text[i][0] != '\0') lines++;
		}
		if (lines == 0)
			frameBuffer->paintBackgroundBox(rt_x, rt_y, rt_w, rt_h);

		if (g_Radiotext->RT_MsgShow)
		{

			if (g_Radiotext->S_RtOsdTitle == 1)
			{

				// Title
				//	sprintf(stext[0], g_Radiotext->RT_PTY == 0 ? "%s - %s %s%s" : "%s - %s (%s)%s",
				//	g_Radiotext->RT_Titel, tr("Radiotext"), g_Radiotext->RT_PTY == 0 ? g_Radiotext->RDS_PTYN : g_Radiotext->ptynr2string(g_Radiotext->RT_PTY), g_Radiotext->RT_MsgShow ? ":" : tr("  [waiting ...]"));
				if ((lines) || (g_Radiotext->RT_PTY !=0))
				{
					sprintf(stext[0], g_Radiotext->RT_PTY == 0 ? "%s %s%s" : "%s (%s)%s", tr("Radiotext"), g_Radiotext->RT_PTY == 0 ? g_Radiotext->RDS_PTYN : g_Radiotext->ptynr2string(g_Radiotext->RT_PTY), ":");

					// shadow
					frameBuffer->paintBoxRel(rt_x+OFFSET_SHADOW, rt_y+OFFSET_SHADOW, rt_dx, rt_dy, COL_SHADOW_PLUS_0, RADIUS_LARGE, CORNER_TOP);
					frameBuffer->paintBoxRel(rt_x, rt_y, rt_dx, rt_dy, COL_INFOBAR_PLUS_0, RADIUS_LARGE, CORNER_TOP);
					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rt_x+10, rt_y+ 30, rt_dx-20, stext[0], COL_INFOBAR_TEXT, 0, RTisIsUTF);
					blit = true;
				}
//				yoff = 17;
				ii = 1;

			}
			// Body
			if (lines)
			{
				frameBuffer->paintBoxRel(rt_x+OFFSET_SHADOW, rt_y+rt_dy+OFFSET_SHADOW, rt_dx, 7+rt_dy* g_Radiotext->S_RtOsdRows, COL_SHADOW_PLUS_0, RADIUS_LARGE, CORNER_BOTTOM);
				frameBuffer->paintBoxRel(rt_x, rt_y+rt_dy, rt_dx, 7+rt_dy* g_Radiotext->S_RtOsdRows, COL_INFOBAR_PLUS_0, RADIUS_LARGE, CORNER_BOTTOM);

				// RT-Text roundloop
				int ind = (g_Radiotext->RT_Index == 0) ? g_Radiotext->S_RtOsdRows - 1 : g_Radiotext->RT_Index - 1;
				int rts_x = rt_x+10;
				int rts_y = rt_y+ 30;
				int rts_dx = rt_dx-20;
				if (g_Radiotext->S_RtOsdLoop == 1)   // latest bottom
				{
					for (int i = ind+1; i < g_Radiotext->S_RtOsdRows; i++)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR_TEXT, 0, RTisIsUTF);
					for (int i = 0; i <= ind; i++)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR_TEXT, 0, RTisIsUTF);
				}
				else   // latest top
				{
					for (int i = ind; i >= 0; i--)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR_TEXT, 0, RTisIsUTF);
					for (int i = g_Radiotext->S_RtOsdRows-1; i > ind; i--)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR_TEXT, 0, RTisIsUTF);
				}
				blit = true;
			}
		}
	}
	g_Radiotext->RT_MsgShow = false;
	if (blit)
		frameBuffer->blit();

}

int CInfoViewer::handleMsg (const neutrino_msg_t msg, neutrino_msg_data_t data)
{
	if ((msg == NeutrinoMessages::EVT_CURRENTNEXT_EPG) || (msg == NeutrinoMessages::EVT_NEXTPROGRAM))
	{
//printf("CInfoViewer::handleMsg: NeutrinoMessages::EVT_CURRENTNEXT_EPG data %llx current %llx\n", *(t_channel_id *) data, current_channel_id & 0xFFFFFFFFFFFFULL);
		if ((*(t_channel_id *) data) == (current_channel_id & 0xFFFFFFFFFFFFULL))
		{
			getEPG (*(t_channel_id *) data, info_CurrentNext);
			if (is_visible)
				show_Data (true);
			showLcdPercentOver ();
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_GOTPIDS)
	{
		if ((*(t_channel_id *) data) == current_channel_id)
		{
			if (is_visible && showButtonBar)
			{
				showIcon_SubT();
				//showIcon_CA_Status(0);
				showIcon_Resolution();
				showIcon_Tuner();
			}
		}
		return messages_return::handled;
	}
	else if ((msg == NeutrinoMessages::EVT_ZAP_COMPLETE) ||
	         (msg == NeutrinoMessages::EVT_ZAP_ISNVOD))
	{
		current_channel_id = (*(t_channel_id *)data);
		//killInfobarText();
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_CA_ID)
	{
		if (is_visible && showButtonBar)
			showIcon_CA_Status(0);
		//Set_CA_Status (data);
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_TIMER)
	{
		if (data == fader.GetFadeTimer())
		{
			// here, the event can only come if there is another window in the foreground!
			fader.StopFade();
			return messages_return::handled;
		}
		else if (data == lcdUpdateTimer)
		{
//printf("CInfoViewer::handleMsg: lcdUpdateTimer\n");
			if (is_visible)
			{
				if (fileplay || CMoviePlayerGui::getInstance().timeshift)
					CMoviePlayerGui::getInstance().UpdatePosition();
				if (fileplay)
				{
					const char *unit_short_minute = g_Locale->getText(LOCALE_UNIT_SHORT_MINUTE);
					char runningRest[64]; // %d can be 10 digits max...
					int curr_pos = CMoviePlayerGui::getInstance().GetPosition();
					int duration = CMoviePlayerGui::getInstance().GetDuration();
					snprintf(runningRest, sizeof(runningRest), "%d / %d %s", (curr_pos + 30000) / 60000, (duration - curr_pos + 30000) / 60000, unit_short_minute);
					display_Info(NULL, NULL, false, CMoviePlayerGui::getInstance().file_prozent, NULL, runningRest);
				}
				else if (!IS_WEBTV(current_channel_id))
				{
					show_Data(false);
				}
			}
			showLcdPercentOver ();
			return messages_return::handled;
		}
		else if (data == sec_timer_id)
		{
			return messages_return::handled;
		}
	}
	else if (msg == NeutrinoMessages::EVT_RECORDMODE)
	{
		recordModeActive = data;
		if (is_visible) showRecords();
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_GOTAPIDS)
	{
		if ((*(t_channel_id *) data) == current_channel_id)
		{
			if (is_visible && showButtonBar)
			{
				showIcon_DD();
				showLivestreamInfo();
				showButtons(true /*paintFooter*/); // in case button text has changed
			}
			if (g_settings.radiotext_enable && g_Radiotext && !g_RemoteControl->current_PIDs.APIDs.empty() && ((CNeutrinoApp::getInstance()->getMode()) == NeutrinoMessages::mode_radio))
				g_Radiotext->setPid(g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].pid);
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_GOT_SUBSERVICES)
	{
		if ((*(t_channel_id *) data) == current_channel_id)
		{
			if (is_visible && showButtonBar)
				showButtons(true /*paintFooter*/); // in case button text has changed
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_SUB_COMPLETE)
	{
		//if ((*(t_channel_id *)data) == current_channel_id)
		{
			if (is_visible && showButtonBar && (!g_RemoteControl->are_subchannels))
				show_Data (true);
		}
		showLcdPercentOver ();
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_SUB_FAILED)
	{
		// show failure..!
		CVFD::getInstance ()->showServicename ("(" + g_RemoteControl->getCurrentChannelName () + ')', g_RemoteControl->getCurrentChannelNumber());
		printf ("zap failed!\n");
		showFailure ();
		CVFD::getInstance ()->showPercentOver (255);
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_FAILED)
	{
		if ((*(t_channel_id *) data) == current_channel_id)
		{
			// show failure..!
			CVFD::getInstance ()->showServicename ("(" + g_RemoteControl->getCurrentChannelName () + ')', g_RemoteControl->getCurrentChannelNumber());
			printf ("zap failed!\n");
			showFailure ();
			CVFD::getInstance ()->showPercentOver (255);
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_MOTOR)
	{
		chanready = 0;
		showMotorMoving (data);
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_TUNE_COMPLETE)
	{
		chanready = 1;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_MODECHANGED)
	{
		aspectRatio = (int8_t)data;
		if (is_visible && showButtonBar)
			showIcon_16_9 ();
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_TIMESET)
	{
		gotTime = true;
		return messages_return::handled;
	}

	return messages_return::unhandled;
}

void CInfoViewer::sendNoEpg(const t_channel_id for_channel_id)
{
	if (!zap_mode/* & IV_MODE_DEFAULT*/)
	{
		char *p = new char[sizeof(t_channel_id)];
		memcpy(p, &for_channel_id, sizeof(t_channel_id));
		g_RCInput->postMsg (NeutrinoMessages::EVT_NOEPG_YET, (const neutrino_msg_data_t) p, false);
	}
}

void CInfoViewer::getEPG(const t_channel_id for_channel_id, CSectionsdClient::CurrentNextInfo &info)
{
	/* to clear the oldinfo for channels without epg, call getEPG() with for_channel_id = 0 */
	if (for_channel_id == 0 || IS_WEBTV(for_channel_id))
	{
		oldinfo.current_uniqueKey = 0;
		return;
	}

	CEitManager::getInstance()->getCurrentNextServiceKey(current_epg_id, info);

	/* of there is no EPG, send an event so that parental lock can work */
	if (info.current_uniqueKey == 0 && info.next_uniqueKey == 0)
	{
		sendNoEpg(for_channel_id);
		oldinfo = info;
		return;
	}

	if (info.current_uniqueKey != oldinfo.current_uniqueKey || info.next_uniqueKey != oldinfo.next_uniqueKey)
	{
		if (info.flags & (CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::has_next))
		{
			CSectionsdClient::CurrentNextInfo *_info = new CSectionsdClient::CurrentNextInfo;
			*_info = info;
			neutrino_msg_t msg;
			if (info.flags & CSectionsdClient::epgflags::has_current)
				msg = NeutrinoMessages::EVT_CURRENTEPG;
			else
				msg = NeutrinoMessages::EVT_NEXTEPG;
			g_RCInput->postMsg(msg, (const neutrino_msg_data_t) _info, false);
		}
		else
		{
			sendNoEpg(for_channel_id);
		}
		oldinfo = info;
	}
}

void CInfoViewer::display_Info(const char *current, const char *next,
                               bool starttimes, const int pb_pos,
                               const char *runningStart, const char *runningRest,
                               const char *nextStart, const char *nextDuration,
                               bool update_current, bool update_next)
{
	/* dimensions of the two-line current-next "box":
	   top of box    == ChanNameY + header_height (bottom of channel name)
	   bottom of box == BoxEndY
	   height of box == BoxEndY - (ChanNameY + header_height)
	   middle of box == top + height / 2
			 == ChanNameY + header_height + (BoxEndY - (ChanNameY + header_height))/2
			 == ChanNameY + header_height + (BoxEndY - ChanNameY - header_height)/2
			 == ChanNameY / 2 + header_height / 2 + BoxEndY / 2
			 == (BoxEndY + ChanNameY + header_height)/2
	   The bottom of current info and the top of next info is == middle of box.
	 */

	int height = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight();
	int CurrInfoY = (BoxEndY + ChanNameY + header_height) / 2;
	int NextInfoY = CurrInfoY/* + height*/;	// lower end of next info box
	int InfoX = ChanInfoX + ChanNumWidth + 20;

	int xStart = InfoX;
	if (starttimes)
		xStart += info_time_width + 10;

	int pb_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight() - 4;
	CurrInfoY -= (pb_h/3);
	NextInfoY += (pb_h/3);

	if (pb_pos > -1)
	{
		int pb_w = 112;
		int pb_startx = BoxEndX - pb_w - OFFSET_SHADOW;
		int pb_starty = ChanNameY - (pb_h + 10);

		pb_startx = xStart;
		pb_w = BoxEndX - 10 - xStart;

		pb_starty = CurrInfoY + ((pb_h / 3)-(pb_h/5)) ;
		pb_h = (pb_h/5);

		int pb_p = pb_pos * pb_w / 100;
		if (pb_p > pb_w)
			pb_p = pb_w;

		timescale->setDimensionsAll(pb_startx, pb_starty, pb_w, pb_h);
		timescale->setActiveColor(COL_INFOBAR_PLUS_7);
		timescale->setPassiveColor(COL_INFOBAR_PLUS_1);
		timescale->enableShadow(CC_SHADOW_OFF, OFFSET_SHADOW/2);
		timescale->setValues(pb_p, pb_w);

		//printf("paintProgressBar(%d, %d, %d, %d)\n", BoxEndX - pb_w - OFFSET_SHADOW, ChanNameY - (pb_h + 10) , pb_w, pb_h);
	}

	if (showButtonBar)
	{
		if (!g_settings.theme.infobar_gradient_top)
			frameBuffer->paintHLine(ChanInfoX + 10, BoxEndX - 10, CurrInfoY - height - 2, COL_INFOBAR_PLUS_3);
		if ((g_settings.infobar_casystem_display < 2) && (!g_settings.infobar_casystem_frame))
			frameBuffer->paintHLine(ChanInfoX + 10, BoxEndX - 10, NextInfoY + height + 2, COL_INFOBAR_PLUS_3);
	}


	int currTimeW = 0;
	int nextTimeW = 0;
	if (runningRest)
		currTimeW = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(runningRest)*2;
	if (nextDuration)
		nextTimeW = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(nextDuration)*2;
	int currTimeX = BoxEndX - currTimeW - 10;
	int nextTimeX = BoxEndX - nextTimeW - 10;

	//colored_events init
	bool colored_event_C = (g_settings.theme.colored_events_infobar == 1);
	bool colored_event_N = (g_settings.theme.colored_events_infobar == 2);

	//current event
	if (current && update_current)
	{
		if (txt_cur_start)
		{
			txt_cur_start->hide();
			delete txt_cur_start;
			txt_cur_start = NULL;
		}

		if (txt_cur_event)
		{
			txt_cur_event->hide();
			delete txt_cur_event;
			txt_cur_event = NULL;
		}

		if (txt_cur_event_rest)
		{
			txt_cur_event_rest->hide();
			delete txt_cur_event_rest;
			txt_cur_event_rest = NULL;
		}

		txt_cur_event = new CComponentsTextTransp(NULL, xStart, CurrInfoY - height, currTimeX - xStart - 5, height, current,
		        CTextBox::NO_AUTO_LINEBREAK, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
		        CComponentsText::FONT_STYLE_REGULAR, colored_event_C ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);
		txt_cur_event->paint(CC_SAVE_SCREEN_YES);

		if (runningStart)
		{
			txt_cur_start = new CComponentsTextTransp(NULL, InfoX, CurrInfoY - height, info_time_width, height, runningStart,
			        CTextBox::NO_AUTO_LINEBREAK, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
			        CComponentsText::FONT_STYLE_REGULAR, colored_event_C ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);
			txt_cur_start->paint(CC_SAVE_SCREEN_YES);
		}
		if (runningRest)
		{
			txt_cur_event_rest = new CComponentsTextTransp(NULL, currTimeX, CurrInfoY - height, currTimeW, height, runningRest,
			        CTextBox::RIGHT, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
			        CComponentsText::FONT_STYLE_REGULAR, colored_event_C ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);
			txt_cur_event_rest->paint(CC_SAVE_SCREEN_YES);
		}
	}

	//next event
	if (next && update_next)
	{
		if (txt_next_start)
		{
			txt_next_start->hide();
			delete txt_next_start;
			txt_next_start = NULL;
		}

		if (txt_next_event)
		{
			txt_next_event->hide();
			delete txt_next_event;
			txt_next_event= NULL;
		}

		if (txt_next_in)
		{
			txt_next_in->hide();
			delete txt_next_in;
			txt_next_in = NULL;
		}

		txt_next_event = new CComponentsTextTransp(NULL, xStart, NextInfoY, nextTimeX - xStart - 5, height, next,
		        CTextBox::NO_AUTO_LINEBREAK, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
		        CComponentsText::FONT_STYLE_REGULAR, colored_event_N ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);
		txt_next_event->paint(CC_SAVE_SCREEN_YES);

		if (nextStart)
		{
			txt_next_start = new CComponentsTextTransp(NULL, InfoX, NextInfoY, info_time_width, height, nextStart,
			        CTextBox::NO_AUTO_LINEBREAK, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
			        CComponentsText::FONT_STYLE_REGULAR, colored_event_N ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);
			txt_next_start->paint(CC_SAVE_SCREEN_YES);
		}
		if (nextDuration)
		{
			txt_next_in = new CComponentsTextTransp(NULL, nextTimeX, NextInfoY, nextTimeW, height, nextDuration,
			                                        CTextBox::RIGHT, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO],
			                                        CComponentsText::FONT_STYLE_REGULAR, colored_event_N ? COL_COLORED_EVENTS_TEXT : COL_INFOBAR_TEXT);

			txt_next_in->paint(CC_SAVE_SCREEN_YES);
		}
	}

	//finally paint time scale
	if (pb_pos > -1)
		timescale->paint();

}

void CInfoViewer::show_Data (bool calledFromEvent)
{
	if (! is_visible)
		return;

	/* EPG data is not useful in movieplayer mode ;) */
	if (fileplay && !CMoviePlayerGui::getInstance().timeshift)
		return;

	char runningStart[32];
	char runningRest[32];
	char runningPercent = 0;

	char nextStart[32];
	char nextDuration[32];

	int is_nvod = false;

	if ((g_RemoteControl->current_channel_id == current_channel_id) && (!g_RemoteControl->subChannels.empty()) && (!g_RemoteControl->are_subchannels))
	{
		is_nvod = true;
		info_CurrentNext.current_zeit.startzeit = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].startzeit;
		info_CurrentNext.current_zeit.dauer = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].dauer;
	}

	time_t jetzt = time (NULL);

	const char *unit_short_minute = g_Locale->getText(LOCALE_UNIT_SHORT_MINUTE);

	if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current)
	{
		int seit = (abs(jetzt - info_CurrentNext.current_zeit.startzeit) + 30) / 60;
		int rest = (info_CurrentNext.current_zeit.dauer / 60) - seit;
		runningPercent = 0;
		if (!gotTime)
			snprintf(runningRest, sizeof(runningRest), "%d %s", info_CurrentNext.current_zeit.dauer / 60, unit_short_minute);
		else if (jetzt < info_CurrentNext.current_zeit.startzeit)
			snprintf(runningRest, sizeof(runningRest), "%s %d %s", g_Locale->getText(LOCALE_WORD_IN), seit, unit_short_minute);
		else
		{
			runningPercent = (jetzt - info_CurrentNext.current_zeit.startzeit) * 100 / info_CurrentNext.current_zeit.dauer;
			if (runningPercent > 100)
				runningPercent = 100;
			if (rest >= 0)
				snprintf(runningRest, sizeof(runningRest), "%d / %d %s", seit, rest, unit_short_minute);
			else
				snprintf(runningRest, sizeof(runningRest), "%d +%d %s", info_CurrentNext.current_zeit.dauer / 60, -rest, unit_short_minute);
		}

		struct tm *pStartZeit = localtime (&info_CurrentNext.current_zeit.startzeit);
		snprintf (runningStart, sizeof(runningStart), "%02d:%02d", pStartZeit->tm_hour, pStartZeit->tm_min);
	}
	else
		last_curr_id = 0;

	if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_next)
	{
		unsigned dauer = info_CurrentNext.next_zeit.dauer / 60;
		snprintf (nextDuration, sizeof(nextDuration), "%d %s", dauer, unit_short_minute);
		struct tm *pStartZeit = localtime (&info_CurrentNext.next_zeit.startzeit);
		snprintf (nextStart, sizeof(nextStart), "%02d:%02d", pStartZeit->tm_hour, pStartZeit->tm_min);
	}
	else
		last_next_id = 0;

//	int ChanInfoY = BoxStartY + ChanHeight + 15;	//+10

	if (showButtonBar)
	{
		showButtons(calledFromEvent);
	}

	if ((info_CurrentNext.flags & CSectionsdClient::epgflags::not_broadcast) ||
	        (calledFromEvent && !(info_CurrentNext.flags & (CSectionsdClient::epgflags::has_next|CSectionsdClient::epgflags::has_current))))
	{
		// no EPG available
		display_Info(g_Locale->getText(gotTime ? LOCALE_INFOVIEWER_NOEPG : LOCALE_INFOVIEWER_WAITTIME), NULL);
		/* send message. Parental pin check gets triggered on EPG events... */
		/* clear old info in getEPG */
		CSectionsdClient::CurrentNextInfo dummy;
		getEPG(0, dummy);
		sendNoEpg(current_channel_id);
		return;
	}

	// irgendein EPG gefunden
	const char *current   = NULL;
	const char *curr_time = NULL;
	const char *curr_rest = NULL;
	const char *next      = NULL;
	const char *next_time = NULL;
	const char *next_dur  = NULL;
	bool curr_upd = true;
	bool next_upd = true;

	if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current)
	{
		if (/*!calledFromEvent ||*/ info_CurrentNext.current_uniqueKey != last_curr_id)
		{
			last_curr_id = info_CurrentNext.current_uniqueKey;
			curr_time = runningStart;
			current = info_CurrentNext.current_name.c_str();
		}
		else
			curr_upd = false;
		curr_rest = runningRest;
	}
	else
		current = g_Locale->getText(LOCALE_INFOVIEWER_NOCURRENT);

	if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_next)
	{
		if (!(is_nvod && (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current))
		        && info_CurrentNext.next_uniqueKey != last_next_id)
		{
			/* if current is shown, show next only if !nvod. Why? I don't know */
			//printf("SHOWDATA: last_next_id = 0x%016llx next_id = 0x%016llx\n", last_next_id, info_CurrentNext.next_uniqueKey);
			last_next_id = info_CurrentNext.next_uniqueKey;
			next_time = nextStart;
			next = info_CurrentNext.next_name.c_str();
			next_dur = nextDuration;
		}
		else
			next_upd = false;
	}
	display_Info(current, next, true, runningPercent,
	             curr_time, curr_rest, next_time, next_dur, curr_upd, next_upd);

}

void CInfoViewer::killInfobarText()
{
	if (infobar_txt)
	{
		if (infobar_txt->isPainted())
			infobar_txt->kill();
		delete infobar_txt;
		infobar_txt = NULL;
		showRecords();
	}
}


void CInfoViewer::showInfoFile()
{
	//read textcontent from this file
	std::string infobar_file = INFOFILE;

	//exit if file not found, don't create an info object, delete old instance if required
	if (!file_size(infobar_file.c_str()))
	{
		killInfobarText();
		return;
	}

	//set position of info area
	const int xStart	= ChanInfoX;
	const int yStart	= BoxStartY - 5;
	const int width		= BoxEndX - ChanInfoX;
	const int height	= g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight() + 2;

	//create info object
	if (infobar_txt == NULL)
	{
		infobar_txt = new CComponentsInfoBox();
		//set some properties for info object
		infobar_txt->setCorner(RADIUS_SMALL);
		infobar_txt->enableShadow(CC_SHADOW_ON, OFFSET_SHADOW/2);
		infobar_txt->setTextColor(COL_INFOBAR_TEXT);
		infobar_txt->setColorBody(COL_INFOBAR_PLUS_0);
		infobar_txt->doPaintTextBoxBg(false);
		infobar_txt->enableColBodyGradient(g_settings.theme.infobar_gradient_top, g_settings.theme.infobar_gradient_top ? COL_INFOBAR_PLUS_0 : header->getColorBody(), g_settings.theme.infobar_gradient_top_direction);
	}

	//get text from file and set it to info object, exit and delete object if failed
	string old_txt = infobar_txt->getText();
	string new_txt = infobar_txt->getTextFromFile(infobar_file);
	bool has_text = infobar_txt->setText(new_txt, CTextBox::CENTER, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]);
	if (new_txt.empty())
	{
		killInfobarText();
		return;
	}

	//check dimension, if changed then kill to force reinit
	if (infobar_txt->getWidth() != width)
		infobar_txt->kill();

	//consider possible size change
	infobar_txt->setDimensionsAll(xStart, yStart, width, height);

	//paint info if not painted or text has changed
	if (has_text || (zap_mode & IV_MODE_VIRTUAL_ZAP))
	{
		if ((old_txt != new_txt) || !infobar_txt->isPainted())
		{
			showRecords();
			infobar_txt->paint(CC_SAVE_SCREEN_NO);
		}
	}
}

void CInfoViewer::killTitle()
{
	if (is_visible)
	{
		is_visible = false;
		is_visible = false;
		int bottom = BoxEndY + OFFSET_SHADOW + bottom_bar_offset;
		if (showButtonBar)
			bottom += InfoHeightY_Info;
		if (getFooter())
			getFooter()->kill();
		if (getCABar())
			getCABar()->kill();
		if (rec)
			rec->kill();
		//printf("killTitle(%d, %d, %d, %d)\n", BoxStartX, BoxStartY, BoxEndX+ OFFSET_SHADOW-BoxStartX, bottom-BoxStartY);
		//frameBuffer->paintBackgroundBox(BoxStartX, BoxStartY, BoxEndX+ OFFSET_SHADOW, bottom);
		if (!(zap_mode & IV_MODE_VIRTUAL_ZAP))
		{
			if (infobar_txt)
				infobar_txt->kill();
		}

		header->kill();

		body->kill();

		if (g_settings.radiotext_enable && g_Radiotext)
		{
			g_Radiotext->S_RtOsd = g_Radiotext->haveRadiotext() ? 1 : 0;
			killRadiotext();
		}

		killInfobarText();

		if (ecminfo_toggle)
			ecmInfoBox_hide();

		if (recordsbox)
		{
			recordsbox->kill();
			delete recordsbox;
			recordsbox = NULL;
		}

		frameBuffer->blit();
	}
	showButtonBar = false;
	CInfoClock::getInstance()->enableInfoClock();
}

void CInfoViewer::showLcdPercentOver ()
{
	if (g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME] != 1)
	{
		if (fileplay || (NeutrinoMessages::mode_ts == CNeutrinoApp::getInstance()->getMode()))
		{
			CVFD::getInstance ()->showPercentOver (CMoviePlayerGui::getInstance().file_prozent);
			return;
		}
		int runningPercent = -1;
		time_t jetzt = time (NULL);

		if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current)
		{
			if (jetzt < info_CurrentNext.current_zeit.startzeit)
				runningPercent = 0;
			else
				runningPercent = MIN ((jetzt - info_CurrentNext.current_zeit.startzeit) * 100 / info_CurrentNext.current_zeit.dauer, 100);
		}
		CVFD::getInstance ()->showPercentOver (runningPercent);
	}
}

void CInfoViewer::showEpgInfo()   //message on event change
{
	int mode = CNeutrinoApp::getInstance()->getMode();
	/* show epg info only if we in TV- or Radio mode and current event is not the same like before */
	if ((eventname != info_CurrentNext.current_name) && (mode == 2 /*mode_radio*/ || mode == 1 /*mode_tv*/))
	{
		eventname = info_CurrentNext.current_name;
		if (g_settings.infobar_show)
			g_RCInput->postMsg(NeutrinoMessages::SHOW_INFOBAR, 0);
	}
}

void CInfoViewer::setUpdateTimer(uint64_t interval)
{
	g_RCInput->killTimer(lcdUpdateTimer);
	if (interval)
		lcdUpdateTimer = g_RCInput->addTimer(interval, false);
}

void CInfoViewer::ResetModules(bool kill)
{
	if (kill)
	{
		if (txt_cur_event)
			txt_cur_event->clearSavedScreen();
		if (txt_cur_event_rest)
			txt_cur_event_rest->clearSavedScreen();
		if (txt_cur_start)
			txt_cur_start->clearSavedScreen();
		if (txt_next_event)
			txt_next_event->clearSavedScreen();
		if (txt_next_in)
			txt_next_in->clearSavedScreen();
		if (txt_next_start)
			txt_next_start->clearSavedScreen();
	}
	delete header;
	header = NULL;
	delete body;
	body = NULL;
	delete infobar_txt;
	infobar_txt = NULL;
	delete txt_cur_start;
	txt_cur_start = NULL;
	delete txt_cur_event;
	txt_cur_event = NULL;
	delete txt_cur_event_rest;
	txt_cur_event_rest = NULL;
	delete txt_next_start;
	txt_next_start = NULL;
	delete txt_next_event;
	txt_next_event = NULL;
	delete txt_next_in;
	txt_next_in = NULL;
	delete rec;
	rec = NULL;

	if (hddscale)
	{
		delete hddscale;
		hddscale = NULL;
	}
	if (foot)
	{
		delete foot;
		foot = NULL;
	}
	if (ca_bar)
	{
		delete ca_bar;
		ca_bar = NULL;
	}
}

void CInfoViewer::ecmInfoBox_show(const char * txt, int w, int h, Font * font)
{
	if (ecmInfoBox != NULL)
		ecmInfoBox_hide();

	// manipulate title font
	int storedSize = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getSize();
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->setSize((int)(font->getSize() * 150 / 100));

	//create new window
	ecmInfoBox = new CComponentsWindowMax(LOCALE_ECMINFO, NEUTRINO_ICON_INFO);

	//calc available width (width of Infobar)
	int max_w = BoxEndX - BoxStartX;
	//calc available height (space between Top and Infobar)
	int max_h = BoxStartY - frameBuffer->getScreenY() - 2*OFFSET_SHADOW;

	//get window header object
	CComponentsHeader* winheader = ecmInfoBox->getHeaderObject();
	int h_header = winheader->getHeight();

	//remove window footer object
	ecmInfoBox->showFooter(false);

	//set new window dimensions
	int h_offset = 5;
	int w_offset = 10;
	ecmInfoBox->setWidth(min(max_w, w + 2*w_offset));
	ecmInfoBox->setHeight(min(max_h, h_header + h + 2*h_offset));
	ecmInfoBox->Refresh();

	//calc window position
	int pos_x;
	switch (g_settings.show_ecm_pos)
	{
	case 3: // right
		pos_x = BoxEndX - ecmInfoBox->getWidth();
		break;
	case 1: // left
		pos_x = BoxStartX;
		break;
	case 2: // center
	default:
		pos_x = frameBuffer->getScreenX() + (max_w/2) - (ecmInfoBox->getWidth()/2);
		break;
	}

	int pos_y = frameBuffer->getScreenY() + (max_h/2) - (ecmInfoBox->getHeight()/2);
	ecmInfoBox->setXPos(pos_x);
	ecmInfoBox->setYPos(pos_y);

	//get window body object
	CComponentsForm* winbody = ecmInfoBox->getBodyObject();

	// create textbox object
	CComponentsText* ecmText = new CComponentsText(0, 0, winbody->getWidth(), winbody->getHeight());
	ecmText->setTextBorderWidth(w_offset, h_offset);
	ecmText->setText(txt, CTextBox::TOP | CTextBox::NO_AUTO_LINEBREAK, font);
	ecmText->setCorner(RADIUS_LARGE, CORNER_BOTTOM);

	// add textbox object to window
	ecmInfoBox->addWindowItem(ecmText);
	ecmInfoBox->enableShadow(CC_SHADOW_ON);
	ecmInfoBox->paint(CC_SAVE_SCREEN_NO);

	// restore title font
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->setSize(storedSize);
}

void CInfoViewer::ecmInfoBox_hide()
{
	if (ecmInfoBox != NULL)
	{
		ecmInfoBox->kill();
		delete ecmInfoBox;
		ecmInfoBox = NULL;
	}
}

/*#############################################################################*/

bool CInfoViewer::checkIcon(const char * const icon, int *w, int *h)
{
	frameBuffer->getIconSize(icon, w, h);
	if ((*w != 0) && (*h != 0))
		return true;
	return false;
}

void CInfoViewer::getIconInfo()
{
	initBBOffset();
	bbIconMaxH 		= 0;
	showBBIcons_width = 0;
	BBarY 			= BoxEndY + bottom_bar_offset;
	BBarFontY 		= BBarY + InfoHeightY_Info - (InfoHeightY_Info - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_FOOT]->getHeight()) / 2; /* center in buttonbar */
	bbIconMinX 		= BoxEndX - 2*OFFSET_INNER_MID;
	CNeutrinoApp* neutrino	= CNeutrinoApp::getInstance();

	for (int i = 0; i < CInfoViewer::ICON_MAX; i++)
	{
		int w = 0, h = 0;
		bool iconView = false;
		switch (i)
		{
		case CInfoViewer::ICON_SUBT:  //no radio
			if (neutrino->getMode() != NeutrinoMessages::mode_radio)
				iconView = checkIcon(NEUTRINO_ICON_SUBT, &w, &h);
			break;
		case CInfoViewer::ICON_RT:
			if ((neutrino->getMode() == NeutrinoMessages::mode_radio) && g_settings.radiotext_enable)
				iconView = checkIcon(NEUTRINO_ICON_RADIOTEXTGET, &w, &h);
			break;
		case CInfoViewer::ICON_DD:
			if( g_settings.infobar_show_dd_available )
				iconView = checkIcon(NEUTRINO_ICON_DD, &w, &h);
			break;
		case CInfoViewer::ICON_16_9:  //no radio
			if (neutrino->getMode() != NeutrinoMessages::mode_radio)
				iconView = checkIcon(NEUTRINO_ICON_16_9, &w, &h);
			break;
		case CInfoViewer::ICON_RES:  //no radio
			if ((g_settings.infobar_show_res < 2) && (neutrino->getMode() != NeutrinoMessages::mode_radio))
				iconView = checkIcon(g_settings.infobar_show_res ? NEUTRINO_ICON_RESOLUTION_HD : NEUTRINO_ICON_RESOLUTION_1280, &w, &h);
			break;
		case CInfoViewer::ICON_CA:
			if (g_settings.infobar_casystem_display == 2 && neutrino->getMode() != NeutrinoMessages::mode_ts)
				iconView = checkIcon(NEUTRINO_ICON_SCRAMBLED2, &w, &h);
			break;
		case CInfoViewer::ICON_TUNER:
			if (CFEManager::getInstance()->getEnabledCount() > 1 && g_settings.infobar_show_tuner == 1 && !IS_WEBTV(get_current_channel_id()) && neutrino->getMode() != NeutrinoMessages::mode_ts)
				iconView = checkIcon(NEUTRINO_ICON_TUNER_1, &w, &h);
			break;
		case CInfoViewer::ICON_UPDATE:
			if ((access("/tmp/.update_avail", F_OK) == 0))
				iconView = checkIcon(NEUTRINO_ICON_UPDATE_AVAIL, &w, &h);
			break;
		case CInfoViewer::ICON_LOGO:
			if ((access(NEUTRINO_ICON_LOGO, F_OK) == 0))
				iconView = checkIcon(NEUTRINO_ICON_LOGO, &w, &h);
			break;
		default:
			break;
		}
		if (iconView)
		{
			if (i > 0)
				bbIconMinX -= OFFSET_INNER_MIN;
			bbIconMinX -= w;
			bbIconInfo[i].x = bbIconMinX;
			bbIconInfo[i].h = h;
			showBBIcons_width += w;
		}
		else
			bbIconInfo[i].x = -1;
	}
	for (int i = 0; i < CInfoViewer::ICON_MAX; i++)
	{
		if (bbIconInfo[i].x != -1)
			bbIconMaxH = std::max(bbIconMaxH, bbIconInfo[i].h);
	}
}

void CInfoViewer::getButtonInfo()
{
	bbButtonMaxH = 0;
	bbButtonMaxX = ChanInfoX;
	int bbButtonMaxW = 0;
	int mode = NeutrinoMessages::mode_unknown;
	for (int i = 0; i < CInfoViewer::BUTTON_MAX; i++)
	{
		int w = 0, h = 0;
		bool active;
		std::string text, icon;
		switch (i)
		{
		case CInfoViewer::BUTTON_RED:
			icon = NEUTRINO_ICON_INFO_RED;
			frameBuffer->getIconSize(icon.c_str(), &w, &h);
			mode = CNeutrinoApp::getInstance()->getMode();
			if (mode == NeutrinoMessages::mode_ts)
			{
				text = CKeybindSetup::getMoviePlayerButtonName(CRCInput::RC_red, active, g_settings.infobar_buttons_usertitle);
				if (!text.empty())
					break;
			}
			text = CUserMenu::getUserMenuButtonName(0, active, g_settings.infobar_buttons_usertitle);
			if (!text.empty())
				break;
			text = g_settings.usermenu[SNeutrinoSettings::BUTTON_RED]->title;
			break;
		case CInfoViewer::BUTTON_GREEN:
			icon = NEUTRINO_ICON_INFO_GREEN;
			frameBuffer->getIconSize(icon.c_str(), &w, &h);
			mode = CNeutrinoApp::getInstance()->getMode();
			if (mode == NeutrinoMessages::mode_ts)
			{
				text = CKeybindSetup::getMoviePlayerButtonName(CRCInput::RC_green, active, g_settings.infobar_buttons_usertitle);
				if (!text.empty())
					break;
			}
			text = CUserMenu::getUserMenuButtonName(1, active, g_settings.infobar_buttons_usertitle);
			if (!text.empty())
				break;
			text = g_settings.usermenu[SNeutrinoSettings::BUTTON_GREEN]->title;
			break;
		case CInfoViewer::BUTTON_YELLOW:
			icon = NEUTRINO_ICON_INFO_YELLOW;
			frameBuffer->getIconSize(icon.c_str(), &w, &h);
			mode = CNeutrinoApp::getInstance()->getMode();
			if (mode == NeutrinoMessages::mode_ts)
			{
				text = "Fileinfos"; //CKeybindSetup::getMoviePlayerButtonName(CRCInput::RC_yellow, active, g_settings.infobar_buttons_usertitle);
				if (!text.empty())
					break;
			}
			text = CUserMenu::getUserMenuButtonName(2, active, g_settings.infobar_buttons_usertitle);
			if (!text.empty())
				break;
			text = g_settings.usermenu[SNeutrinoSettings::BUTTON_YELLOW]->title;
			break;
		case CInfoViewer::BUTTON_BLUE:
			icon = NEUTRINO_ICON_INFO_BLUE;
			frameBuffer->getIconSize(icon.c_str(), &w, &h);
			mode = CNeutrinoApp::getInstance()->getMode();
			if (mode == NeutrinoMessages::mode_ts)
			{
				text = CKeybindSetup::getMoviePlayerButtonName(CRCInput::RC_blue, active, g_settings.infobar_buttons_usertitle);
				if (!text.empty())
					break;
			}
			text = CUserMenu::getUserMenuButtonName(3, active, g_settings.infobar_buttons_usertitle);
			if (!text.empty())
				break;
			text = g_settings.usermenu[SNeutrinoSettings::BUTTON_BLUE]->title;
			break;
		default:
			break;
		}
		//label audio control button in movieplayer mode
		if (mode == NeutrinoMessages::mode_ts && !CMoviePlayerGui::getInstance().timeshift)
		{
			if (text == g_Locale->getText(LOCALE_MPKEY_AUDIO) && !g_settings.infobar_buttons_usertitle)
				text = CMoviePlayerGui::getInstance(false).CurrentAudioName(); // use instance_mp
		}
		bbButtonInfo[i].w = w;
		bbButtonInfo[i].cx = std::min(g_Font[SNeutrinoSettings::FONT_TYPE_MENU_FOOT]->getRenderWidth(text),w);
		bbButtonInfo[i].h = h;
		bbButtonInfo[i].text = text;
		bbButtonInfo[i].icon = icon;
	}
	// Calculate position/size of buttons
	minX = std::min(bbIconMinX, ChanInfoX + (((BoxEndX - ChanInfoX) * 75) / 100));
	int MaxBr = (BoxEndX - 10) - (ChanInfoX + 10);
	bbButtonMaxX = ChanInfoX + 10;
	int br = 0, count = 0;
	for (int i = 0; i < CInfoViewer::BUTTON_MAX; i++)
	{
		{
			count++;
			bbButtonInfo[i].paint = true;
			br += bbButtonInfo[i].w;
			bbButtonInfo[i].x = bbButtonMaxX;
			bbButtonMaxX += bbButtonInfo[i].w;
			bbButtonMaxW = std::max(bbButtonMaxW, bbButtonInfo[i].w);
		}
	}
	if (br > MaxBr)
		printf("[infoviewer_bb:%s#%d] width br (%d) > MaxBr (%d) count %d\n", __func__, __LINE__, br, MaxBr, count);
	bbButtonMaxX = ChanInfoX + 10;
	int step = MaxBr / 4;
	if (count > 0)   /* avoid div-by-zero :-) */
	{
		step = MaxBr / count;
		count = 0;
		for (int i = 0; i < BUTTON_MAX; i++)
		{
			if (!bbButtonInfo[i].paint)
				continue;
			bbButtonInfo[i].x = bbButtonMaxX + step * count + (step/2 - bbButtonInfo[i].w /2);
			// printf("%s: i = %d count = %d b.x = %d\n", __func__, i, count, bbButtonInfo[i].x);
			count++;
		}
	}
	else
	{
		printf("[infoviewer_bb:%s#%d: count <= 0???\n", __func__, __LINE__);
		bbButtonInfo[BUTTON_RED].x	= bbButtonMaxX;
		bbButtonInfo[BUTTON_GREEN].x	= bbButtonMaxX + step;
		bbButtonInfo[BUTTON_YELLOW].x	= bbButtonMaxX + 2*step;
		bbButtonInfo[BUTTON_BLUE].x	= bbButtonMaxX + 3*step;
	}
}

void CInfoViewer::showButtons(bool)
{
	if (!is_visible)
		return;
	int i;
	bool paint = false;

	if (g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_LEFT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_RIGHT ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_BOTTOM_CENTER ||
	        g_settings.volume_pos == CVolumeBar::VOLUMEBAR_POS_HIGHER_CENTER)
		isVolscale = CVolume::getInstance()->hideVolscale();
	else
		isVolscale = false;

	getButtonInfo();
	for (i = 0; i < CInfoViewer::BUTTON_MAX; i++)
	{
		if (tmp_bbButtonInfoText[i] != bbButtonInfo[i].text)
		{
			paint = true;
			break;
		}
	}

	if (paint)
	{
		paintFoot(BoxEndX - BoxStartX - ChanInfoX);

		for (i = BUTTON_MAX; i > 0;)
		{
			--i;
			if ((bbButtonInfo[i].x <= ChanInfoX) || (bbButtonInfo[i].x + bbButtonInfo[i].w >= BoxEndX) || (!bbButtonInfo[i].paint))
				continue;
			if (bbButtonInfo[i].x > 0)
			{
				frameBuffer->paintIcon(bbButtonInfo[i].icon, bbButtonInfo[i].x, BBarY, InfoHeightY_Info);

				g_Font[SNeutrinoSettings::FONT_TYPE_MENU_FOOT]->RenderString(bbButtonInfo[i].x + (bbButtonInfo[i].w /2 - bbButtonInfo[i].cx /2), BBarFontY,
				        bbButtonInfo[i].w, bbButtonInfo[i].text, COL_MENUFOOT_TEXT);
			}
		}

		for (i = 0; i < CInfoViewer::BUTTON_MAX; i++)
		{
			tmp_bbButtonInfoText[i] = bbButtonInfo[i].text;
		}
	}
	if (isVolscale)
		CVolume::getInstance()->showVolscale();
}

void CInfoViewer::showIcons(const int modus, const std::string & icon)
{
	if ((bbIconInfo[modus].x <= ChanInfoX) || (bbIconInfo[modus].x >= BoxEndX))
		return;
	if ((modus >= CInfoViewer::ICON_SUBT) && (modus < CInfoViewer::ICON_MAX) && (bbIconInfo[modus].x != -1) && (is_visible))
	{
		frameBuffer->paintIcon(icon, bbIconInfo[modus].x - time_width, ChanNameY + (header_height - bbIconMaxH)/2,
		                       InfoHeightY_Info, 1, true, !g_settings.theme.infobar_gradient_top, COL_INFOBAR_BUTTONS_BACKGROUND);
	}
}

void CInfoViewer::paintshowButtonBar()
{
	if (!is_visible)
		return;
	getIconInfo();
	for (int i = 0; i < CInfoViewer::BUTTON_MAX; i++)
	{
		tmp_bbButtonInfoText[i] = "";
	}
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	if (g_settings.infobar_casystem_display < 2)
		paint_ca_bar();

	paintFoot();

	// Buttons
	showButtons();

	if (g_settings.infobar_casystem_display < 2)
		paint_cam_icons();
	// Icons, starting from right
	showIcon_SubT();
	showIcon_DD();
	showIcon_16_9();
	showIcon_CA_Status(0);
	showIcon_Resolution();
	showIcon_Tuner();
	showIcon_Logo();
	if (g_settings.infobar_casystem_display < 2)
		showScale_RecordingDir();
}

void CInfoViewer::showIcon_Update(bool show)
{
	showIcons(CInfoViewer::ICON_UPDATE, show ? NEUTRINO_ICON_UPDATE_AVAIL : NEUTRINO_ICON_UPDATE_AVAIL_GREY);
}

void CInfoViewer::showIcon_Logo()
{
	showIcons(CInfoViewer::ICON_LOGO, NEUTRINO_ICON_LOGO);
}

void CInfoViewer::paintFoot(int w)
{
	int width = (w == 0) ? BoxEndX - ChanInfoX : w;

	if (foot == NULL)
		foot = new CComponentsShapeSquare(ChanInfoX, BBarY, width, InfoHeightY_Info, NULL, CC_SHADOW_ON);

	foot->setColorBody(g_settings.theme.infobar_gradient_bottom ? COL_MENUHEAD_PLUS_0 : COL_INFOBAR_BUTTONS_BACKGROUND);
	foot->enableColBodyGradient(g_settings.theme.infobar_gradient_bottom, COL_INFOBAR_PLUS_0, g_settings.theme.infobar_gradient_bottom_direction);
	foot->setCorner(RADIUS_LARGE, CORNER_BOTTOM);

	foot->paint(CC_SAVE_SCREEN_NO);
}

void CInfoViewer::showIcon_SubT()
{
	if (!is_visible)
		return;
	bool have_sub = false;
	CZapitChannel * cc = CNeutrinoApp::getInstance()->channelList->getChannel(CNeutrinoApp::getInstance()->channelList->getActiveChannelNumber());
	if (cc && cc->getSubtitleCount())
		have_sub = true;

	showIcons(CInfoViewer::ICON_SUBT, (have_sub) ? NEUTRINO_ICON_SUBT : NEUTRINO_ICON_SUBT_GREY);
}

void CInfoViewer::showIcon_DD()
{
	if (!is_visible || !g_settings.infobar_show_dd_available)
		return;
	std::string dd_icon;
	if ((g_RemoteControl->current_PIDs.PIDs.selected_apid < g_RemoteControl->current_PIDs.APIDs.size()) &&
	        (g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].is_ac3))
		dd_icon = NEUTRINO_ICON_DD;
	else
		dd_icon = g_RemoteControl->has_ac3 ? NEUTRINO_ICON_DD_AVAIL : NEUTRINO_ICON_DD_GREY;

	showIcons(CInfoViewer::ICON_DD, dd_icon);
}

void CInfoViewer::showIcon_RadioText(bool rt_available)
{
	if (!is_visible || !g_settings.radiotext_enable)
		return;

	std::string rt_icon;
	if (rt_available)
		rt_icon = (g_Radiotext->S_RtOsd) ? NEUTRINO_ICON_RADIOTEXTGET : NEUTRINO_ICON_RADIOTEXTWAIT;
	else
		rt_icon = NEUTRINO_ICON_RADIOTEXTOFF;

	showIcons(CInfoViewer::ICON_RT, rt_icon);
}

void CInfoViewer::showIcon_16_9()
{
	if (!is_visible)
		return;

	if ((aspectRatio == 0) || ( g_RemoteControl->current_PIDs.PIDs.vpid == 0 ) || (aspectRatio != videoDecoder->getAspectRatio()))
	{
		if (chanready &&
		        (g_RemoteControl->current_PIDs.PIDs.vpid > 0 || IS_WEBTV(get_current_channel_id())))
			aspectRatio = videoDecoder->getAspectRatio();
		else
			aspectRatio = 0;

		showIcons(CInfoViewer::ICON_16_9, (aspectRatio > 2) ? NEUTRINO_ICON_16_9 : NEUTRINO_ICON_16_9_GREY);
	}
}

void CInfoViewer::showIcon_Resolution()
{
	if ((!is_visible) || (g_settings.infobar_show_res == 2)) //show resolution icon is off
		return;
	if (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio)
		return;
	const char *icon_name = NULL;
#if BOXMODEL_UFS910
	if (!chanready)
#else
	if (!chanready || videoDecoder->getBlank())
#endif
	{
		icon_name = NEUTRINO_ICON_RESOLUTION_000;
	}
	else
	{
		int xres, yres, framerate;
		if (g_settings.infobar_show_res == 0)  //show resolution icon on infobar
		{
			videoDecoder->getPictureInfo(xres, yres, framerate);
			switch (yres)
			{
			case 1920:
				icon_name = NEUTRINO_ICON_RESOLUTION_1920;
				break;
			case 1080:
			case 1088:
				icon_name = NEUTRINO_ICON_RESOLUTION_1080;
				break;
			case 1440:
				icon_name = NEUTRINO_ICON_RESOLUTION_1440;
				break;
			case 1280:
				icon_name = NEUTRINO_ICON_RESOLUTION_1280;
				break;
			case 720:
				icon_name = NEUTRINO_ICON_RESOLUTION_720;
				break;
			case 704:
				icon_name = NEUTRINO_ICON_RESOLUTION_704;
				break;
			case 576:
				icon_name = NEUTRINO_ICON_RESOLUTION_576;
				break;
			case 544:
				icon_name = NEUTRINO_ICON_RESOLUTION_544;
				break;
			case 528:
				icon_name = NEUTRINO_ICON_RESOLUTION_528;
				break;
			case 480:
				icon_name = NEUTRINO_ICON_RESOLUTION_480;
				break;
			case 382:
				icon_name = NEUTRINO_ICON_RESOLUTION_382;
				break;
			case 352:
				icon_name = NEUTRINO_ICON_RESOLUTION_352;
				break;
			case 288:
				icon_name = NEUTRINO_ICON_RESOLUTION_288;
				break;
			default:
				icon_name = NEUTRINO_ICON_RESOLUTION_000;
				break;
			}
		}
		if (g_settings.infobar_show_res == 1)  //show simple resolution icon on infobar
		{
			videoDecoder->getPictureInfo(xres, yres, framerate);
			if (yres > 576)
				icon_name = NEUTRINO_ICON_RESOLUTION_HD;
			else if (yres > 0)
				icon_name = NEUTRINO_ICON_RESOLUTION_SD;
			else
				icon_name = NEUTRINO_ICON_RESOLUTION_000;
		}
	}
	showIcons(CInfoViewer::ICON_RES, icon_name);
}

void CInfoViewer::showOne_CAIcon()
{
	std::string sIcon = "";
	sIcon = (fta) ? NEUTRINO_ICON_SCRAMBLED2_GREY : NEUTRINO_ICON_SCRAMBLED2;
	showIcons(CInfoViewer::ICON_CA, sIcon);
}

void CInfoViewer::showIcon_Tuner()
{
	if (CFEManager::getInstance()->getEnabledCount() <= 1 || !g_settings.infobar_show_tuner)
		return;

	std::string icon_name;
	switch (CFEManager::getInstance()->getLiveFE()->getNumber())
	{
	case 1:
		icon_name = NEUTRINO_ICON_TUNER_2;
		break;
	case 2:
		icon_name = NEUTRINO_ICON_TUNER_3;
		break;
	case 3:
		icon_name = NEUTRINO_ICON_TUNER_4;
		break;
	case 0:
	default:
		icon_name = NEUTRINO_ICON_TUNER_1;
		break;
	}
	showIcons(CInfoViewer::ICON_TUNER, icon_name);
}

void CInfoViewer::showClock_analog(int posx,int posy,int dia)
{
	int ts,tm,th,sx,sy,mx,my,hx,hy;
	double pi = 3.1415926535897932384626433832795, sAngleInRad, mAngleInRad, mAngleSave, hAngleInRad;

	time_t now = time(0);
	struct tm *tm_p = localtime(&now);

	ts = tm_p->tm_sec;
	tm = tm_p->tm_min;
	th = tm_p->tm_hour;

	sAngleInRad = ((6 * ts) * (2*pi / 360));
	sAngleInRad -= pi/2;

	sx = int((dia * 0.9 * cos(sAngleInRad)));
	sy = int((dia * 0.9 * sin(sAngleInRad)));

	mAngleInRad = ((6 * tm) * (2*pi / 360));
	mAngleSave = mAngleInRad;
	mAngleInRad -= pi/2;

	mx = int((dia * 0.7 * cos(mAngleInRad)));
	my = int((dia * 0.7 * sin(mAngleInRad)));

	hAngleInRad = ((30 * th)* (2*pi / 360));
	hAngleInRad += mAngleSave / 12;
	hAngleInRad -= pi/2;

	hx = int((dia * 0.6 * cos(hAngleInRad)));
	hy = int((dia * 0.6 * sin(hAngleInRad)));

	std::string clock_face = ICONSDIR_VAR "/clock_face.png";
	if (access(clock_face.c_str(), F_OK) != 0)
		clock_face = ICONSDIR "/clock_face.png";

	g_PicViewer->DisplayImage(clock_face, posx-dia, posy-dia, 2*dia, 2*dia);

	frameBuffer->paintLine(posx,posy,posx+hx,posy+hy,COL_MENUHEAD_TEXT);
	frameBuffer->paintLine(posx,posy,posx+mx,posy+my,COL_MENUHEAD_TEXT);
	frameBuffer->paintLine(posx,posy,posx+sx,posy+sy,COL_COLORED_EVENTS_TEXT);
}

void CInfoViewer::showScale_RecordingDir()
{
	hddwidth = (BoxEndX - ChanInfoX) * 0.2;
	if (g_settings.infobar_show_sysfs_hdd)
	{
		int percent = cHddStat::getInstance()->getPercent();
		int py = BoxEndY + (g_settings.infobar_casystem_frame ? 4 : 2);
		int px = ChanInfoX + (((BoxEndX - ChanInfoX) - hddwidth)/2);
		if (is_visible)
		{
			if (percent >= 0)
			{
				hddscale->setDimensionsAll(px, py, hddwidth, 18);
				hddscale->setValues(percent, 100);
				hddscale->paint();
			}
			else
			{
				frameBuffer->paintBoxRel(px, py, hddwidth, 18, COL_INFOBAR_PLUS_0);
				hddscale->reset();
			}
		}
	}
}

void CInfoViewer::paint_ca_icons(int caid, const char *icon, int &icon_space_offset)
{
	char buf[20];
	int endx = BoxEndX - (g_settings.infobar_casystem_frame ? 20 : 10);
	int py = BoxEndY + (g_settings.infobar_casystem_frame ? 4 : 2); /* hand-crafted, should be automatic */
	int px = 0;
	static map<int, std::pair<int,const char*> > icon_map;
	const int icon_number = 11;

	static int icon_offset[icon_number] = {0,0,0,0,0,0,0,0,0,0,0};
	static int icon_sizeW [icon_number] = {0,0,0,0,0,0,0,0,0,0,0};
	static bool init_flag = false;

	if (!init_flag)
	{
		init_flag = true;
		int icon_sizeH = 0, index = 0;
		map<int, std::pair<int,const char*> >::const_iterator it;

		icon_map[0x0000] = std::make_pair(index++,"dec");
		icon_map[0x0E00] = std::make_pair(index++,"powervu");
		icon_map[0x4A00] = std::make_pair(index++,"d");
		icon_map[0x2600] = std::make_pair(index++,"biss");
		icon_map[0x0600] = std::make_pair(index++,"ird");
		icon_map[0x0100] = std::make_pair(index++,"seca");
		icon_map[0x0500] = std::make_pair(index++,"via");
		icon_map[0x1800] = std::make_pair(index++,"nagra");
		icon_map[0x0B00] = std::make_pair(index++,"conax");
		icon_map[0x0D00] = std::make_pair(index++,"cw");
		icon_map[0x0900] = std::make_pair(index,"nds");

		for (it=icon_map.begin(); it!=icon_map.end(); ++it)
		{
			snprintf(buf, sizeof(buf), "%s_%s", (*it).second.second, (*it).second.first==0 ? "int" : "white");
			frameBuffer->getIconSize(buf, &icon_sizeW[(*it).second.first], &icon_sizeH);
		}

		for (int j = 0; j < icon_number; j++)
		{
			for (int i = j; i < icon_number; i++)
			{
				icon_offset[j] += icon_sizeW[i] + OFFSET_INNER_MIN;
			}
		}
	}
	caid &= 0xFF00;

	if (icon_offset[icon_map[caid].first] == 0)
		return;

	if (g_settings.infobar_casystem_display == 0)
	{
		px = endx - (icon_offset[icon_map[caid].first] - OFFSET_INNER_MIN );
	}
	else
	{
		icon_space_offset += icon_sizeW[icon_map[caid].first];
		px = endx - icon_space_offset;
		icon_space_offset += 4;
	}

	if (px)
	{
		snprintf(buf, sizeof(buf), "%s_%s", icon_map[caid].second, icon);
		if ((px >= (endx-8)) || (px <= 0))
			printf("#####[%s:%d] Error paint icon %s, px: %d,  py: %d, endx: %d, icon_offset: %d\n",
			       __FUNCTION__, __LINE__, buf, px, py, endx, icon_offset[icon_map[caid].first]);
		else if (strstr(buf,"dec_white") == 0)
			frameBuffer->paintIcon(buf, px, py);
	}
}

void CInfoViewer::showIcon_CA_Status(int notfirst)
{
	int acaid = 0;
	if (ecminfo_toggle && (notfirst || g_settings.infobar_casystem_display > 1)) //bad mess :(
		acaid = check_ecmInfo();

	if (g_settings.infobar_casystem_display == 3)
		return;
	if(NeutrinoMessages::mode_ts == CNeutrinoApp::getInstance()->getMode() && !CMoviePlayerGui::getInstance().timeshift)
	{
		if (g_settings.infobar_casystem_display == 2)
		{
			fta = true;
			showOne_CAIcon();
		}
		return;
	}

	int caids[] = {  0x900, 0xD00, 0xB00, 0x1800, 0x0500, 0x0100, 0x600,  0x2600, 0x4a00, 0x0E00, 0x0000 };
	const char *green = "green";
	const char *white = "white";
	const char *yellow = "yellow";
	int icon_space_offset = 0;
	const char *dec_icon_name[] = {"na","na","fta","int","card","net"};
	decode = UNKNOWN;

	if(!chanready)
	{
		if (g_settings.infobar_casystem_display == 2)
		{
			fta = true;
			showOne_CAIcon();
		}
		else if(g_settings.infobar_casystem_display == 0)
		{
			for (int i = 0; i < (int)(sizeof(caids)/sizeof(int)); i++)
			{
				paint_ca_icons(caids[i], white, icon_space_offset);
			}
		}
		return;
	}

	CZapitChannel * channel = CZapit::getInstance()->GetCurrentChannel();
	if(!channel)
		return;

	if (g_settings.infobar_casystem_display == 2)
	{
		fta = channel->camap.empty();
		showOne_CAIcon();
		return;
	}

	if(!notfirst)
	{
		acaid = check_ecmInfo();
		if (channel->scrambled && camCI && !useCI)
		{
			useCI = cCA::GetInstance()->checkChannelID(channel->getChannelID());
		}

		if(useCI)
			decode = CARD;

		if((acaid & 0xFF00)== 0x1700 && (caids[3]& 0xFF00) == 0x1800)
			acaid=0x1800;
		for (int i = 0; i < (int)(sizeof(caids)/sizeof(int)); i++)
		{
			bool dcaid = false;
			bool found = false;
			for(casys_map_iterator_t it = channel->camap.begin(); it != channel->camap.end(); ++it)
			{
				int caid = (*it) & 0xFF00;
				if (caid == 0x1700)
					caid = 0x0600;
				if((found = (caid == caids[i])))
				{
					dcaid = ((caid) == (acaid & 0xFF00));
					fta=false;
					break;
				}
			}
			if(i == (int)(sizeof(caids)/sizeof(int))-1)
			{
				paint_ca_icons(caids[i], fta ? "fta" : dec_icon_name[decode], icon_space_offset);
				continue;
			}

			if(g_settings.infobar_casystem_display == 0)
				paint_ca_icons(caids[i], (found ? (dcaid ? green : yellow) : white), icon_space_offset); //NI
			else if(found)
				paint_ca_icons(caids[i], (dcaid ? green : yellow), icon_space_offset);
		}

	}
}

void CInfoViewer::paint_ca_bar()
{
	initBBOffset();
	int ca_width = BoxEndX - ChanInfoX;

	if (g_settings.infobar_casystem_frame)
	{
		if (ca_bar == NULL)
			ca_bar = new CComponentsShapeSquare(ChanInfoX + OFFSET_INNER_MID, BoxEndY, ca_width - 2*OFFSET_INNER_MID, bottom_bar_offset - OFFSET_INNER_MID, NULL, CC_SHADOW_ON, COL_INFOBAR_CASYSTEM_PLUS_2, COL_INFOBAR_CASYSTEM_PLUS_0);
		ca_bar->enableShadow(CC_SHADOW_ON, OFFSET_SHADOW/2, true);
		ca_bar->setFrameThickness(2);
		ca_bar->setCorner(RADIUS_SMALL, CORNER_ALL);
		ca_bar->paint(CC_SAVE_SCREEN_NO);
	}
	else
	{
		paintBoxRel(ChanInfoX, BoxEndY, ca_width, bottom_bar_offset, COL_INFOBAR_CASYSTEM_PLUS_0);
	}
}

void CInfoViewer::ResetPBars()
{
	if (!hddscale)
	{
		hddscale = new CProgressBar();
		hddscale->setType(CProgressBar::PB_REDRIGHT);
	}

	hddscale->reset();

	if (!timescale)
	{
		timescale = new CProgressBar();
		timescale->setType(CProgressBar::PB_TIMESCALE);
	}

	timescale->reset();

}

void CInfoViewer::initBBOffset()
{
	bottom_bar_offset = (g_settings.infobar_casystem_display < 2) ? (g_settings.infobar_casystem_frame ? 36 : 22) : 0;
}

void* CInfoViewer::scrambledThread(void *arg)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
	CInfoViewer *infoViewer = static_cast<CInfoViewer*>(arg);
	while(1)
	{
		if (infoViewer->is_visible)
			infoViewer->scrambledCheck();
		usleep(500*1000);
	}
	return 0;
}

void CInfoViewer::scrambledCheck(bool force)
{
	scrambledErr = false;
	scrambledNoSig = false;
	if (videoDecoder->getBlank())
	{
		if (videoDecoder->getPlayState())
			scrambledErr = true;
		else
			scrambledNoSig = true;
	}

	if ((scrambledErr != scrambledErrSave) || (scrambledNoSig != scrambledNoSigSave) || force)
	{
		showIcon_CA_Status(0);
		showIcon_Resolution();
		scrambledErrSave = scrambledErr;
		scrambledNoSigSave = scrambledNoSig;
	}
}

typedef  void* (CInfoViewer::*MemFuncPtr)(void);
typedef  void* (*PthreadPtr)(void*);

void CInfoViewer::paint_cam_icons()
{
	MemFuncPtr   t = &CInfoViewer::Thread_paint_cam_icons;
	PthreadPtr   p = *(PthreadPtr*)&t;
	pthread_t thread_pci;
	if (pthread_create(&thread_pci, NULL, p, this) == 0)
		pthread_detach(thread_pci);
}

void* CInfoViewer::Thread_paint_cam_icons(void)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	std::ostringstream buf;
	std::stringstream fpath;
	int emu_pic_startx = ChanInfoX + (g_settings.infobar_casystem_frame ? 20 : 10);
	int py = BoxEndY + (g_settings.infobar_casystem_frame ? 4 : 2);
	const char *icon_name[] = {"mgcamd","oscam","gbox"};
	int icon_sizeH = 0;
	int icon_sizeW = 0;

	for (int i=0; i < (int)(sizeof(icon_name)/sizeof(icon_name[0])); i++)
	{
		if ( getpidof(icon_name[i]) )
		{
			buf.str("");
			buf << icon_name[i] << "_green";
			frameBuffer->paintIcon(buf.str().c_str(), emu_pic_startx, py );
			frameBuffer->getIconSize(buf.str().c_str(), &icon_sizeW, &icon_sizeH);
			emu_pic_startx += icon_sizeW + OFFSET_INNER_MIN;
		}
		else
		{
			fpath.str("");
			fpath << CONFIGDIR"/../../emu/" << icon_name[i];
			if ((access(fpath.str().c_str(), F_OK) == 0))
			{
				buf.str("");
				buf << icon_name[i] << "_yellow";
				frameBuffer->paintIcon(buf.str().c_str(), emu_pic_startx, py );
				frameBuffer->getIconSize(buf.str().c_str(), &icon_sizeW, &icon_sizeH);
				emu_pic_startx += icon_sizeW + OFFSET_INNER_MIN;
			}
			else if(g_settings.infobar_casystem_display == 0)
			{
				buf.str("");
				buf << icon_name[i] << "_white";
				frameBuffer->paintIcon(buf.str().c_str(), emu_pic_startx, py );
				frameBuffer->getIconSize(buf.str().c_str(), &icon_sizeW, &icon_sizeH);
				emu_pic_startx += icon_sizeW + OFFSET_INNER_MIN;
			}
		}
	}

	if (camCI)
	{
		if (useCI)
			frameBuffer->paintIcon("ci+_green", emu_pic_startx, py);
		else
			frameBuffer->paintIcon("ci+_white", emu_pic_startx, py);
	}
	pthread_exit(0);
}

int CInfoViewer::check_ecmInfo()
{
	int caid = 0;
	if (File_copy("/tmp/ecm.info", "/tmp/ecm.info.tmp"))
	{
		md5_ecmInfo = filehash((char *)"/tmp/ecm.info.tmp");
		caid = parse_ecmInfo("/tmp/ecm.info.tmp");
	}
	return caid;
}

int CInfoViewer::parse_ecmInfo(const char * file)
{
	int acaid = 0;
	char *buffer;
	ssize_t read;
	size_t len;
	std::string ecm_txt("");
	FILE *fh;
	int w = 0;
	int ecm_width = 0;
	int ecm_height = 0;
	Font * ecm_font = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_ECMINFO];
	int font_height = ecm_font->getHeight();

	buffer=NULL;
	if((fh = fopen(file, "r")))
	{
		while ((read = getline(&buffer, &len, fh)) != -1)
		{
			if (ecminfo_toggle)
			{
				w = ecm_font->getRenderWidth(buffer);
				ecm_width = std::max(w, ecm_width);
				ecm_height += font_height;
				ecm_txt += buffer;
			}

			if ( !acaid && strstr(buffer, "0x") )
			{
				if(sscanf(buffer, "%*[^9-0]%x", &acaid ) == 1)
					continue;
			}
			else if ( strstr(buffer, "source:") ||		//mgcamd
			          strstr(buffer, "decode:") ||		//gbox
			          strstr(buffer, "protocol:") ||	//oscam constcw
			          strstr(buffer, "from:"))		//oscam
			{
				if ( strstr(buffer, "emu") ||		//mgcamd
				        strstr(buffer, "constcw") ||	//oscam constcw
				        strstr(buffer, "Internal"))	//gbox
				{
					decode = LOCAL;
				}
				else if ( strstr(buffer, "slot") ||	//gbox
				          strstr(buffer, "local") ||	//oscam
				          strstr(buffer, "com"))
				{
					decode = CARD;
				}
				else if ( strstr(buffer, "net") ||	//mgcamd
				          strstr(buffer, "Network") ||	//gbox
				          strstr(buffer, "."))		//oscam
				{
					if ( strstr(buffer, "localhost") || strstr(buffer, "127.0.0.1"))
						decode = CARD;
					else
						decode = REMOTE;
				}
			}
		}
		fclose(fh);
		remove(file);
		if(buffer)
			free(buffer);
	}

	if (ecminfo_toggle)
	{
		if(decode == UNKNOWN || decode == NA || ecm_txt.empty())
		{
			ecmInfoBox_hide();
		}
		else
		{
			ecmInfoBox_show(ecm_txt.c_str(), ecm_width, ecm_height, ecm_font);
		}
	}

	return(acaid);
}
