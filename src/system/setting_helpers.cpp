/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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

#include <system/setting_helpers.h>
#include "configure_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <libnet.h>
#include <linux/if.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#if HAVE_COOL_HARDWARE
#include <coolstream/control.h>
#endif

#include <config.h>

#include <global.h>
#include <neutrino.h>
#include <gui/widget/stringinput.h>
#include <gui/infoclock.h>
#include <driver/volume.h>

// obsolete #include <gui/streaminfo.h>

#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>

#include <gui/plugins.h>
#include <daemonc/remotecontrol.h>
#include <xmlinterface.h>
#include <audio.h>
#include <video.h>
#include <dmx.h>
#include <cs_api.h>
#include <pwrmngr.h>
#include "libdvbsub/dvbsub.h"
#include "libtuxtxt/teletext.h"
#include <zapit/satconfig.h>
#include <zapit/zapit.h>

extern CPlugins       * g_PluginList;    /* neutrino.cpp */
extern CRemoteControl * g_RemoteControl; /* neutrino.cpp */
extern cVideo *videoDecoder;
extern cAudio *audioDecoder;

extern cDemux *videoDemux;
extern cDemux *audioDemux;
extern cDemux *pcrDemux;

extern "C" int pinghost( const char *hostname );

// gui/moviebrowser.cpp
COnOffNotifier::COnOffNotifier( CMenuItem* a1,CMenuItem* a2,CMenuItem* a3,CMenuItem* a4,CMenuItem* a5)
{
        number = 0;
        if(a1 != NULL){ toDisable[0] =a1;number++;};
        if(a2 != NULL){ toDisable[1] =a2;number++;};
        if(a3 != NULL){ toDisable[2] =a3;number++;};
        if(a4 != NULL){ toDisable[3] =a4;number++;};
        if(a5 != NULL){ toDisable[4] =a5;number++;};
}

bool COnOffNotifier::changeNotify(const neutrino_locale_t, void *Data)
{
   if(*(int*)(Data) == 0)
   {
      for (int i=0; i<number ; i++)
        toDisable[i]->setActive(false);
   }
   else
   {
      for (int i=0; i<number ; i++)
        toDisable[i]->setActive(true);
   }
   return false;
}

//used in gui/miscsettings_menu.cpp
CMiscNotifier::CMiscNotifier( CMenuItem* i1, CMenuItem* i2)
{
   toDisable[0]=i1;
   toDisable[1]=i2;
}
bool CMiscNotifier::changeNotify(const neutrino_locale_t, void *)
{
   toDisable[0]->setActive(!g_settings.shutdown_real);
   toDisable[1]->setActive(!g_settings.shutdown_real);
   return false;
}

bool CSectionsdConfigNotifier::changeNotify(const neutrino_locale_t, void *)
{
        CNeutrinoApp::getInstance()->SendSectionsdConfig();
        return false;
}

bool CTouchFileNotifier::changeNotify(const neutrino_locale_t, void * data)
{
	if ((*(int *)data) != 0)
	{
		FILE * fd = fopen(filename, "w");
		if (fd)
			fclose(fd);
		else
			return false;
	}
	else
		remove(filename);
	return true;
}

bool CColorSetupNotifier::changeNotify(const neutrino_locale_t, void *)
{
	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();
//	unsigned char r,g,b;
	//setting colors-..
	frameBuffer->paletteGenFade(COL_MENUHEAD,
	                              convertSetupColor2RGB(g_settings.menu_Head_red, g_settings.menu_Head_green, g_settings.menu_Head_blue),
	                              convertSetupColor2RGB(g_settings.menu_Head_Text_red, g_settings.menu_Head_Text_green, g_settings.menu_Head_Text_blue),
	                              8, convertSetupAlpha2Alpha( g_settings.menu_Head_alpha ) );

	frameBuffer->paletteGenFade(COL_MENUCONTENT,
	                              convertSetupColor2RGB(g_settings.menu_Content_red, g_settings.menu_Content_green, g_settings.menu_Content_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );


	frameBuffer->paletteGenFade(COL_MENUCONTENTDARK,
	                              convertSetupColor2RGB(int(g_settings.menu_Content_red*0.6), int(g_settings.menu_Content_green*0.6), int(g_settings.menu_Content_blue*0.6)),
	                              convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );

	frameBuffer->paletteGenFade(COL_MENUCONTENTSELECTED,
	                              convertSetupColor2RGB(g_settings.menu_Content_Selected_red, g_settings.menu_Content_Selected_green, g_settings.menu_Content_Selected_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_Selected_Text_red, g_settings.menu_Content_Selected_Text_green, g_settings.menu_Content_Selected_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_Selected_alpha) );

	frameBuffer->paletteGenFade(COL_MENUCONTENTINACTIVE,
	                              convertSetupColor2RGB(g_settings.menu_Content_inactive_red, g_settings.menu_Content_inactive_green, g_settings.menu_Content_inactive_blue),
	                              convertSetupColor2RGB(g_settings.menu_Content_inactive_Text_red, g_settings.menu_Content_inactive_Text_green, g_settings.menu_Content_inactive_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.menu_Content_inactive_alpha) );

	frameBuffer->paletteGenFade(COL_INFOBAR,
	                              convertSetupColor2RGB(g_settings.infobar_red, g_settings.infobar_green, g_settings.infobar_blue),
	                              convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

/*	frameBuffer->paletteSetColor( COL_INFOBAR_SHADOW,
	                                convertSetupColor2RGB(
	                                    int(g_settings.infobar_red*0.4),
	                                    int(g_settings.infobar_green*0.4),
	                                    int(g_settings.infobar_blue*0.4)),
	                                g_settings.infobar_alpha);
*/
	frameBuffer->paletteGenFade(COL_INFOBAR_SHADOW,
	                              convertSetupColor2RGB(int(g_settings.infobar_red*0.4), int(g_settings.infobar_green*0.4), int(g_settings.infobar_blue*0.4)),
	                              convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );


	frameBuffer->paletteGenFade(COL_COLORED_EVENTS_INFOBAR,
	                              convertSetupColor2RGB(g_settings.infobar_red, g_settings.infobar_green, g_settings.infobar_blue),
	                              convertSetupColor2RGB(g_settings.colored_events_red, g_settings.colored_events_green, g_settings.colored_events_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

	frameBuffer->paletteGenFade(COL_COLORED_EVENTS_CHANNELLIST,
	                              convertSetupColor2RGB(int(g_settings.menu_Content_red*0.6), int(g_settings.menu_Content_green*0.6), int(g_settings.menu_Content_blue*0.6)),
	                              convertSetupColor2RGB(g_settings.colored_events_red, g_settings.colored_events_green, g_settings.colored_events_blue),
	                              8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

	frameBuffer->paletteSet();
	/* recalculate volumebar */
	CVolume::getInstance()->Init();
	/* recalculate infoclock */
	CInfoClock::getInstance()->Init();
	return false;
}

bool CAudioSetupNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	//printf("notify: %d\n", OptionName);
#if 0 //FIXME to do ? manual audio delay
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_PCMOFFSET))
	{
	}
#endif
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_ANALOG_MODE)) {
		g_Zapit->setAudioMode(g_settings.audio_AnalogMode);
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_ANALOG_OUT)) {
		audioDecoder->EnableAnalogOut(g_settings.analog_out ? true : false);
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_HDMI_DD)) {
		audioDecoder->SetHdmiDD((HDMI_ENCODED_MODE) g_settings.hdmi_dd);
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_SPDIF_DD)) {
		audioDecoder->SetSpdifDD(g_settings.spdif_dd ? true : false);
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_AVSYNC)) {
		videoDecoder->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		audioDecoder->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		videoDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		audioDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		pcrDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_CLOCKREC)) {
		//.Clock recovery enable/disable
		// FIXME add code here.
	} else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIO_SRS_ALGO) ||
			ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIO_SRS_NMGR) ||
			ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIO_SRS_VOLUME)) {
		audioDecoder->SetSRS(g_settings.srs_enable, g_settings.srs_nmgr_enable, g_settings.srs_algo, g_settings.srs_ref_volume);
	}
	return false;
}

// used in ./gui/osd_setup.cpp:
bool CTimingSettingsNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	for (int i = 0; i < SNeutrinoSettings::TIMING_SETTING_COUNT; i++)
	{
		if (ARE_LOCALES_EQUAL(OptionName, timing_setting[i].name))
		{
			g_settings.timing[i] = 	atoi(g_settings.timing_string[i]);
			return true;
		}
	}
	return false;
}

// used in ./gui/osd_setup.cpp:
bool CFontSizeNotifier::changeNotify(const neutrino_locale_t, void *)
{
	CHintBox hintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FONTSIZE_HINT)); // UTF-8
	hintBox.paint();

	CNeutrinoApp::getInstance()->SetupFonts();

	hintBox.hide();
	/* recalculate volumebar */
	CVolume::getInstance()->Init();
	/* recalculate infoclock */
	CInfoClock::getInstance()->Init();
	return true;
}

int CSubtitleChangeExec::exec(CMenuTarget* /*parent*/, const std::string & actionKey)
{
printf("CSubtitleChangeExec::exec: action %s\n", actionKey.c_str());
	if(actionKey == "off") {
		tuxtx_stop_subtitle();
		dvbsub_stop();
		return menu_return::RETURN_EXIT;
	}
	if(!strncmp(actionKey.c_str(), "DVB", 3)) {
		char const * pidptr = strchr(actionKey.c_str(), ':');
		int pid = atoi(pidptr+1);
		tuxtx_stop_subtitle();
		dvbsub_pause();
		dvbsub_start(pid);
	} else {
		char const * ptr = strchr(actionKey.c_str(), ':');
		ptr++;
		int pid = atoi(ptr);
		ptr = strchr(ptr, ':');
		ptr++;
		int page = strtol(ptr, NULL, 16);
		ptr = strchr(ptr, ':');
		ptr++;
printf("CSubtitleChangeExec::exec: TTX, pid %x page %x lang %s\n", pid, page, ptr);
		tuxtx_stop_subtitle();
		tuxtx_set_pid(pid, page, ptr);
		dvbsub_stop();
		tuxtx_main(g_RCInput->getFileHandle(), pid, page);
	}
        return menu_return::RETURN_EXIT;
}

int CNVODChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//    printf("CNVODChangeExec exec: %s\n", actionKey.c_str());
	unsigned sel= atoi(actionKey.c_str());
	g_RemoteControl->setSubChannel(sel);

	parent->hide();
	g_InfoViewer->showSubchan();
	return menu_return::RETURN_EXIT;
}

int CStreamFeaturesChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//printf("CStreamFeaturesChangeExec exec: %s\n", actionKey.c_str());
	int sel= atoi(actionKey.c_str());

	if(parent != NULL)
		parent->hide();
	// -- obsolete (rasc 2004-06-10)
	// if (sel==-1)
	// {
	// 	CStreamInfo StreamInfo;
	//	StreamInfo.exec(NULL, "");
	// } else
	if(actionKey == "teletext") {
		g_RCInput->postMsg(CRCInput::RC_timeout, 0);
		g_RCInput->postMsg(CRCInput::RC_text, 0);
#if 0
		g_RCInput->clearRCMsg();
		tuxtx_main(g_RCInput->getFileHandle(), frameBuffer->getFrameBufferPointer(), g_RemoteControl->current_PIDs.PIDs.vtxtpid);
		frameBuffer->paintBackground();
		if(!g_settings.cacheTXT)
			tuxtxt_stop();
		g_RCInput->clearRCMsg();
#endif
	}
	else if (sel>=0)
	{
		g_PluginList->startPlugin(sel,0);
	}

	return menu_return::RETURN_EXIT;
}

int CMoviePluginChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int sel= atoi(actionKey.c_str());
	parent->hide();
	if (sel>=0)
	{
			g_settings.movieplayer_plugin=g_PluginList->getName(sel);
	}
	return menu_return::RETURN_EXIT;
}
int COnekeyPluginChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int sel= atoi(actionKey.c_str());
	parent->hide();
	if (sel>=0)
	{
			g_settings.onekey_plugin=g_PluginList->getName(sel);
	}
	return menu_return::RETURN_EXIT;
}

long CNetAdapter::mac_addr_sys ( u_char *addr) //only for function getMacAddr()
{
	struct ifreq ifr;
	struct ifreq *IFR;
	struct ifconf ifc;
	char buf[1024];
	int s, i;
	int ok = 0;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s==-1) 
	{
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	ioctl(s, SIOCGIFCONF, &ifc);
	IFR = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++)
	{
		strcpy(ifr.ifr_name, IFR->ifr_name);
		if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) 
		{
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
			{
				if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) 
				{
					ok = 1;
					break;
				}
			}
		}
	}
	close(s);
	if (ok)
	{
		memmove(addr, ifr.ifr_hwaddr.sa_data, 6);
	}
	else
	{
		return -1;
	}
	return 0;
}

std::string CNetAdapter::getMacAddr(void)
{
	long stat;
	u_char addr[6];
	stat = mac_addr_sys( addr);
	if (0 == stat)
	{
		std::stringstream mac_tmp;
		for(int i=0;i<6;++i)
		mac_tmp<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)addr[i]<<':';
		return mac_tmp.str().substr(0,17);
	}
	else
	{
		printf("[neutrino] eth-id not detected...\n");
		return "";
	}
}

bool CTZChangeNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	bool found = false;
	std::string name, zone;
	printf("CTZChangeNotifier::changeNotify: %s\n", (char *) Data);

        xmlDocPtr parser = parseXmlFile("/etc/timezone.xml");
        if (parser != NULL) {
                xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
                while (search) {
                        if (!strcmp(xmlGetName(search), "zone")) {
				name = xmlGetAttribute(search, "name");
				if(!strcmp(g_settings.timezone, name.c_str())) {
					zone = xmlGetAttribute(search, "zone");
					if (!access(("/usr/share/zoneinfo/" + zone).c_str(), R_OK))
						found = true;
					break;
				}
                        }
                        search = search->xmlNextNode;
                }
                xmlFreeDoc(parser);
        }
	if(found) {
		printf("Timezone: %s -> %s\n", name.c_str(), zone.c_str());
		std::string cmd = "/usr/share/zoneinfo/" + zone;
		printf("symlink %s to /etc/localtime\n", cmd.c_str());
		if (unlink("/etc/localtime"))
			perror("unlink failed");
		if (symlink(cmd.c_str(), "/etc/localtime"))
			perror("symlink failed");
		cmd = ":" + zone;
		setenv("TZ", cmd.c_str(), 1);
	}

	return false;
}

extern Zapit_config zapitCfg;

int CDataResetNotifier::exec(CMenuTarget* /*parent*/, const std::string& actionKey)
{
	bool delete_all = (actionKey == "all");
	bool delete_chan = (actionKey == "channels") || delete_all;
	bool delete_set = (actionKey == "settings") || delete_all;
	neutrino_locale_t msg = delete_all ? LOCALE_RESET_ALL : delete_chan ? LOCALE_RESET_CHANNELS : LOCALE_RESET_SETTINGS;
	int ret = menu_return::RETURN_REPAINT;

	int result = ShowMsgUTF(msg, g_Locale->getText(LOCALE_RESET_CONFIRM), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo);
	if(result != CMessageBox::mbrYes)
		return true;

	if(delete_all) {
		system("rm -f /var/tuxbox/config/zapit/*.conf");
		CServiceManager::getInstance()->SatelliteList().clear();
		CZapit::getInstance()->LoadSettings();
		CZapit::getInstance()->GetConfig(zapitCfg);
		g_RCInput->postMsg( NeutrinoMessages::REBOOT, 0);
		ret = menu_return::RETURN_EXIT_ALL;
	}
	if(delete_set) {
		unlink(NEUTRINO_SETTINGS_FILE);
		//unlink(NEUTRINO_SCAN_SETTINGS_FILE);
		CNeutrinoApp::getInstance()->loadSetup(NEUTRINO_SETTINGS_FILE);
		CNeutrinoApp::getInstance()->saveSetup(NEUTRINO_SETTINGS_FILE);
		//CNeutrinoApp::getInstance()->loadColors(NEUTRINO_SETTINGS_FILE);
		CNeutrinoApp::getInstance()->SetupFonts();
		CNeutrinoApp::getInstance()->SetupTiming();
		CColorSetupNotifier colorSetupNotifier;
		colorSetupNotifier.changeNotify(NONEXISTANT_LOCALE, NULL);
		CVFD::getInstance()->setlcdparameter();
		CFrameBuffer::getInstance()->Clear();
	}
	if(delete_chan) {
		system("rm -f /var/tuxbox/config/zapit/*.xml");
		g_Zapit->reinitChannels();
	}
	return ret;
}

#if HAVE_COOL_HARDWARE
bool CFanControlNotifier::changeNotify(const neutrino_locale_t, void * data)
{
	int cfd, ret;
	//unsigned char speed = (unsigned char) g_settings.fan_speed;
	unsigned int speed = * (int *) data;

	printf("FAN Speed %d\n", speed);
	cfd = open("/dev/cs_control", O_RDONLY);
	if(cfd < 0) {
		perror("Cannot open /dev/cs_control");
		return false;
	}
	ret = ioctl(cfd, IOC_CONTROL_PWM_SPEED, speed);
	close(cfd);
	if(ret < 0) {
		perror("IOC_CONTROL_PWM_SPEED");
		return false;
	}
	return false;
}
#else
bool CFanControlNotifier::changeNotify(const neutrino_locale_t, void *)
{
	return false;
}
#endif

bool CCpuFreqNotifier::changeNotify(const neutrino_locale_t, void * data)
{
extern cCpuFreqManager * cpuFreq;
	int freq = * (int *) data;

	printf("CCpuFreqNotifier: %d Mhz\n", freq);
	freq *= 1000*1000;

	cpuFreq->SetCpuFreq(freq);
	return false;
}

extern CMenuOptionChooser::keyval_ext VIDEOMENU_VIDEOMODE_OPTIONS[];
bool CAutoModeNotifier::changeNotify(const neutrino_locale_t /*OptionName*/, void * /* data */)
{
	int i;
	int modes[VIDEO_STD_MAX+1];

	memset(modes, 0, sizeof(int)*VIDEO_STD_MAX+1);

	for(i = 0; i < VIDEOMENU_VIDEOMODE_OPTION_COUNT; i++) {
		modes[VIDEOMENU_VIDEOMODE_OPTIONS[i].key] = g_settings.enabled_video_modes[i];
	}
	videoDecoder->SetAutoModes(modes);
	return false;
}

int safe_mkdir(char * path)
{
	struct statfs s;
	int ret = 0;
	if(!strncmp(path, "/hdd", 4)) {
		ret = statfs("/hdd", &s);
		if((ret != 0) || (s.f_type == 0x72b6))
			ret = -1;
		else
			mkdir(path, 0755);
	} else
		mkdir(path, 0755);
	return ret;
}

int check_dir(const char * newdir)
{
  
  	struct statfs s;
	if (::statfs(newdir, &s) == 0) {
		switch (s.f_type)	/* f_type is long */
		{
			case 0xEF53L:		/*EXT2 & EXT3*/
			case 0x6969L:		/*NFS*/
			case 0xFF534D42L:	/*CIFS*/
			case 0x517BL:		/*SMB*/
			case 0x52654973L:	/*REISERFS*/
			case 0x65735546L:	/*fuse for ntfs*/
			case 0x58465342L:	/*xfs*/
			case 0x4d44L:		/*msdos*/
			case 0x0187:		/* AUTOFS_SUPER_MAGIC */
			case 0x858458f6: 	/*ramfs*/
				return 0;//ok
			default:
				fprintf(stderr, "%s(%s): Unknown File system type: 0x%lx\n",
						__func__, newdir, s.f_type);
				break;
		}
	}
	return 1;	// error
}
