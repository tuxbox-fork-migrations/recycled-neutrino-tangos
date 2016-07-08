/*
	Neutrino-GUI   -  DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
							 and some other guys

	Copyright (C) 2006-2014 Stefan Seyfried

	Copyright (C) 2011 CoolStream International Ltd

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define __NFILE__ 1
#define NEUTRINO_CPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <dirent.h>

#include <fstream>

#include "global.h"
#include "neutrino.h"
#include "neutrino_menue.h"
#include "version_pseudo.h"

#include <daemonc/remotecontrol.h>

#include <driver/abstime.h>
#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>
#include <driver/neutrinofonts.h>
#include <driver/rcinput.h>
#include <driver/shutdown_count.h>
#include <driver/record.h>
#include <driver/screenshot.h>
#include <driver/volume.h>
#include <driver/streamts.h>
#include <driver/display.h>
#include <driver/radiotext.h>
#include <driver/scanepg.h>

#if !HAVE_GENERIC_HARDWARE
#include "gui/3dsetup.h"
#include "gui/psisetup.h"
#endif

#include "gui/adzap.h"
#include "gui/audiomute.h"
#include "gui/bouquetlist.h"
#include "gui/cam_menu.h"
#include "gui/cec_setup.h"
#include "gui/channellist.h"
#include "gui/epgview.h"
#include "gui/eventlist.h"
#include "gui/favorites.h"
#include "gui/filebrowser.h"
#include "gui/hdd_menu.h"
#include "gui/infoviewer.h"
#include "gui/mediaplayer.h"
#include "gui/movieplayer.h"
#include "gui/osd_setup.h"
#include "gui/osdlang_setup.h"
#include "gui/plugins.h"
#include "gui/rc_lock.h"
#include "gui/scan_setup.h"
#include "gui/screensaver.h"
#include "gui/sleeptimer.h"
#include "gui/update_ext.h"
#include "gui/update.h"
#include "gui/videosettings.h"
#include "gui/audio_select.h"

#include "gui/widget/hintbox.h"
#include "gui/widget/icons.h"
#include "gui/widget/menue.h"
#include "gui/widget/messagebox.h"
#include "gui/infoclock.h"
#include "gui/parentallock_setup.h"
#ifdef ENABLE_PIP
#include "gui/pipsetup.h"
#endif
#include "gui/themes.h"
#include "gui/timerlist.h"

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
#include "gui/screensetup.h"
#endif
#include <system/set_threadname.h>

#if 0
#include <system/ytcache.h>
#endif

#include <audio.h>
#include <ca_cs.h>
#include <cs_api.h>
#include <video.h>
#include <pwrmngr.h>

#include <system/debug.h>
#include <system/fsmounter.h>
#include <system/hddstat.h>
#include <system/setting_helpers.h>
#include <system/settings.h>
#include <system/helpers.h>
#include <system/sysload.h>
#ifdef ENABLE_GRAPHLCD
#include <driver/nglcd.h>
#endif

#include <timerdclient/timerdclient.h>
#include <timerd/timermanager.h>

#include <zapit/debug.h>
#include <zapit/zapit.h>
#include <zapit/getservices.h>
#include <zapit/satconfig.h>
#include <zapit/scan.h>
#include <zapit/capmt.h>
#include <zapit/client/zapitclient.h>

#include <linux/reboot.h>
#include <sys/reboot.h>

#include <lib/libdvbsub/dvbsub.h>
#include <lib/libtuxtxt/teletext.h>
#include <eitd/sectionsd.h>

#include <system/luaserver.h>

#include "gui/lcd4l.h"
CLCD4l  *LCD4l;

int old_b_id = -1;

CInfoClock      *InfoClock;
int allow_flash = 1;
Zapit_config zapitCfg;
char zapit_lat[20]="#";
char zapit_long[20]="#";
bool autoshift = false;
uint32_t scrambled_timer;
#if ENABLE_FASTSCAN
uint32_t fst_timer;
#endif
t_channel_id standby_channel_id = 0;

//NEW
static pthread_t timer_thread;
void * timerd_main_thread(void *data);
static bool timerd_thread_started = false;

#if ENABLE_WEBIF
void * nhttpd_main_thread(void *data);
#endif

//#define DISABLE_SECTIONSD

extern cVideo * videoDecoder;
#ifdef ENABLE_PIP
extern cVideo *pipDecoder;
#endif
extern cDemux *videoDemux;
extern cAudio * audioDecoder;
cPowerManager *powerManager;
cCpuFreqManager * cpuFreq;

void stop_daemons(bool stopall = true, bool for_flash = false);
void stop_video(void);
// uncomment if you want to have a "test" menue entry  (rasc)

//#define __EXPERIMENTAL_CODE__
#ifdef __EXPERIMENTAL_CODE__
#include "gui/ch_mosaic.h"
#endif

CAudioSetupNotifier	* audioSetupNotifier;
CBouquetList   * bouquetList; // current list

CBouquetList   * TVbouquetList;
CBouquetList   * TVsatList;
CBouquetList   * TVfavList;
CBouquetList   * TVallList;
CBouquetList   * TVwebList;

CBouquetList   * RADIObouquetList;
CBouquetList   * RADIOsatList;
CBouquetList   * RADIOfavList;
CBouquetList   * RADIOallList;
CBouquetList   * RADIOwebList;

CBouquetList   * AllFavBouquetList;

CPlugins       * g_PluginList;
CRemoteControl * g_RemoteControl;
CPictureViewer * g_PicViewer;
#if !HAVE_SPARK_HARDWARE
CCAMMenuHandler * g_CamHandler;
#endif
CVolume        * g_volume;
CAudioMute     * g_audioMute;
CNeutrinoFonts * neutrinoFonts = NULL;

// Globale Variablen - to use import global.h

// I don't like globals, I would have hidden them in classes,
// but if you wanna do it so... ;)
bool parentallocked = false;
static char **global_argv;

/* hack until we have real platform abstraction... */
static bool can_deepstandby = false;

extern const char * locale_real_names[]; /* #include <system/locals_intern.h> */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+          CNeutrinoApp - Constructor, initialize g_fontRenderer                      +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
CNeutrinoApp::CNeutrinoApp()
: configfile('\t')
{
	standby_pressed_at.tv_sec = 0;

	frameBuffer = CFrameBuffer::getInstance();
	frameBuffer->setIconBasePath(ICONSDIR);
#if USE_STB_HAL
	/* this needs to happen before the framebuffer is set up */
	init_td_api();
#endif
	SetupFrameBuffer();

	mode 			= mode_unknown;
	lastMode		= mode_unknown;
	channelList		= NULL;
	TVchannelList		= NULL;
	RADIOchannelList	= NULL;
	skipShutdownTimer	= false;
	skipSleepTimer		= false;
	lockStandbyCall         = false;
	current_muted		= 0;
	recordingstatus		= 0;
	channels_changed	= false;
	favorites_changed	= false;
	bouquets_changed	= false;
	channels_init		= false;
	channelList_allowed	= true;
	channelList_painted	= false;
}

/*-------------------------------------------------------------------------------------
-           CNeutrinoApp - Destructor                                                 -
-------------------------------------------------------------------------------------*/
CNeutrinoApp::~CNeutrinoApp()
{
	if (channelList)
		delete channelList;
	if (neutrinoFonts)
		delete neutrinoFonts;
	neutrinoFonts = NULL;
}

CNeutrinoApp* CNeutrinoApp::getInstance()
{
	static CNeutrinoApp* neutrinoApp = NULL;

	if(!neutrinoApp) {
		neutrinoApp = new CNeutrinoApp();
		dprintf(DEBUG_DEBUG, "NeutrinoApp Instance created\n");
	}
	return neutrinoApp;
}

typedef struct lcd_setting_t
{
	const char * const name;
	const unsigned int default_value;
} lcd_setting_struct_t;

const lcd_setting_struct_t lcd_setting[SNeutrinoSettings::LCD_SETTING_COUNT] =
{
	{"lcd_brightness"       , DEFAULT_VFD_BRIGHTNESS       },
	{"lcd_standbybrightness", DEFAULT_VFD_STANDBYBRIGHTNESS},
	{"lcd_contrast"         , DEFAULT_LCD_CONTRAST         },
	{"lcd_power"            , DEFAULT_LCD_POWER            },
	{"lcd_inverse"          , DEFAULT_LCD_INVERSE          },
	{"lcd_show_volume"      , DEFAULT_LCD_SHOW_VOLUME      },
	{"lcd_autodimm"         , DEFAULT_LCD_AUTODIMM         },
	{"lcd_deepbrightness"   , DEFAULT_VFD_STANDBYBRIGHTNESS }
#if USE_STB_HAL
	,{ "lcd_epgmode"        , 0 /*DEFAULT_LCD_EPGMODE*/ }
#endif
#if HAVE_SPARK_HARDWARE
	,{"lcd_displaymode"      , DEFAULT_LCD_DISPLAYMODE    }
	,{"lcd_standbydisplaymode", DEFAULT_LCD_DISPLAYMODE   }
#endif
};

static SNeutrinoSettings::usermenu_t usermenu_default[] = {
	{ CRCInput::RC_red,             "2,3,4,13",                             "",     "red"           },
	{ CRCInput::RC_green,           "6",                                    "",     "green"         },
	{ CRCInput::RC_yellow,          "7",                                    "",     "yellow"        },
	{ CRCInput::RC_blue,            "12,11,14,15,20,21,24,25,19",           "",     "blue"          },
	{ CRCInput::RC_play,            "9",                                    "",     "5"             },
	{ CRCInput::RC_audio,           "27",                                   "",     "6"             },
#if HAVE_SPARK_HARDWARE
	{ CRCInput::RC_timer,           "19",                                   "",     "7"             },
	{ CRCInput::RC_usb,             "31",                                   "",     "6"             },
	{ CRCInput::RC_archive,         "30",                                   "",     "4"             },
#endif
	{ CRCInput::RC_nokey,           "",                                     "",     ""              },
};

/**************************************************************************************
*          CNeutrinoApp -  loadSetup, load the application-settings                   *
**************************************************************************************/
#if HAVE_TRIPLEDRAGON || HAVE_SPARK_HARDWARE || HAVE_GENERIC_HARDWARE || HAVE_DUCKBOX_HARDWARE
#define DEFAULT_X_START_SD	32
#define DEFAULT_Y_START_SD	26
#define DEFAULT_X_END_SD	694
#define DEFAULT_Y_END_SD	570
#else
#define DEFAULT_X_START_SD	60
#define DEFAULT_Y_START_SD	20
#define DEFAULT_X_END_SD	1220
#define DEFAULT_Y_END_SD	560
#endif

#define DEFAULT_X_START_HD	40   //5
#define DEFAULT_Y_START_HD	25   //5
#define DEFAULT_X_END_HD	1235 //1275
#define DEFAULT_Y_END_HD	690  //715

std::string ttx_font_file = "";
std::string *sub_font_file;
int sub_font_size;

int CNeutrinoApp::loadSetup(const char * fname)
{
	char cfg_key[81];
	int erg = 0;

	configfile.clear();
	//settings laden - und dabei Defaults setzen!
	if(!configfile.loadConfig(fname)) {
		//file existiert nicht
		erg = 1;
	} else {
		/* try to detect bad / broken config file */
		if (!configfile.getInt32("screen_EndX_crt", 0) ||
				!configfile.getInt32("screen_EndY_crt", 0) ||
				!configfile.getInt32("screen_EndX_lcd", 0) ||
				!configfile.getInt32("screen_EndY_lcd", 0)) {
			printf("[neutrino] config file %s is broken, using defaults\n", fname);
			configfile.clear();
		} else {
			migrateConfig(fname);
		}
	}
	parentallocked = !access(NEUTRINO_PARENTALLOCKED_FILE, R_OK);

	g_settings.lcd4l_support = configfile.getInt32("lcd4l_support" , 0);
	g_settings.lcd4l_logodir = configfile.getString("lcd4l_logodir", LOGODIR);
	g_settings.lcd4l_skin = configfile.getInt32("lcd4l_skin" , 0);

	g_settings.show_ecm = configfile.getInt32("show_ecm" , 0);
	g_settings.show_ecm_pos = configfile.getInt32("show_ecm_pos" , 0);

	g_settings.softupdate_autocheck = configfile.getBool("softupdate_autocheck" , false);

	// video
#if HAVE_TRIPLEDRAGON || BOXMODEL_SPARK7162
	int vid_Mode_default = VIDEO_STD_PAL;
#else
	int vid_Mode_default = VIDEO_STD_720P50;
	if (getenv("NEUTRINO_DEFAULT_SCART") != NULL)
		vid_Mode_default = VIDEO_STD_PAL;
#endif
	g_settings.video_Mode = configfile.getInt32("video_Mode", vid_Mode_default);
#ifdef ANALOG_MODE
	g_settings.analog_mode1 = configfile.getInt32("analog_mode1", (int)ANALOG_MODE(BOTH,SD,RGB)); // default RGB
	g_settings.analog_mode2 = configfile.getInt32("analog_mode2", (int)ANALOG_MODE(CINCH,SD,YPRPB)); // default YPBPR
#else
	g_settings.analog_mode1 = configfile.getInt32("analog_mode1", (int)ANALOG_SD_RGB_SCART); // default RGB
	g_settings.analog_mode2 = configfile.getInt32("analog_mode2", (int)ANALOG_SD_YPRPB_CINCH); // default YPBPR
#endif
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.hdmi_mode = configfile.getInt32("hdmi_mode", (int)COLORFORMAT_HDMI_RGB);
#endif
	g_settings.hdmi_cec_mode = configfile.getInt32("hdmi_cec_mode", 0); // default off
	g_settings.hdmi_cec_view_on = configfile.getInt32("hdmi_cec_view_on", 0); // default off
	g_settings.hdmi_cec_standby = configfile.getInt32("hdmi_cec_standby", 0); // default off
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.hdmi_cec_broadcast = configfile.getInt32("hdmi_cec_broadcast", 0); // default off
	g_settings.psi_contrast = configfile.getInt32("video_psi_contrast", 128);
	g_settings.psi_saturation = configfile.getInt32("video_psi_saturation", 128);
	g_settings.psi_brightness = configfile.getInt32("video_psi_brightness", 128);
	g_settings.psi_tint = configfile.getInt32("video_psi_tint", 128);
	g_settings.psi_step = configfile.getInt32("video_psi_step", 2);
	g_settings.video_mixer_color = configfile.getInt32("video_mixer_color", 0xff000000);
#endif

	g_settings.video_Format = configfile.getInt32("video_Format", DISPLAY_AR_16_9);
	g_settings.video_43mode = configfile.getInt32("video_43mode", DISPLAY_AR_MODE_LETTERBOX);
	g_settings.current_volume = configfile.getInt32("current_volume", 50);
	g_settings.current_volume_step = configfile.getInt32("current_volume_step", 2);
	g_settings.start_volume = configfile.getInt32("start_volume", -1);
	if (g_settings.start_volume >= 0)
		g_settings.current_volume = g_settings.start_volume;

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.audio_mixer_volume_analog = configfile.getInt32("audio_mixer_volume_analog", 50);
	g_settings.audio_mixer_volume_hdmi = configfile.getInt32("audio_mixer_volume_hdmi", 75);
	g_settings.audio_mixer_volume_spdif = configfile.getInt32("audio_mixer_volume_spdif", 75);
#endif

	g_settings.audio_volume_percent_ac3 = configfile.getInt32("audio_volume_percent_ac3", 100);
	g_settings.audio_volume_percent_pcm = configfile.getInt32("audio_volume_percent_pcm", 100);

	g_settings.channel_mode = configfile.getInt32("channel_mode", LIST_MODE_FAV);
	g_settings.channel_mode_radio = configfile.getInt32("channel_mode_radio", LIST_MODE_FAV);
	g_settings.channel_mode_initial = configfile.getInt32("channel_mode_initial", -1);
	g_settings.channel_mode_initial_radio = configfile.getInt32("channel_mode_initial_radio", -1);
	if (g_settings.channel_mode_initial > -1)
		g_settings.channel_mode = g_settings.channel_mode_initial;
	if (g_settings.channel_mode_initial_radio > -1)
		g_settings.channel_mode_radio = g_settings.channel_mode_initial_radio;

	g_settings.fan_speed = configfile.getInt32( "fan_speed", 1);
	if(g_settings.fan_speed < 1) g_settings.fan_speed = 1;

	g_settings.srs_enable = configfile.getInt32( "srs_enable", 0);
	g_settings.srs_algo = configfile.getInt32( "srs_algo", 1);
	g_settings.srs_ref_volume = configfile.getInt32( "srs_ref_volume", 40);
	g_settings.srs_nmgr_enable = configfile.getInt32( "srs_nmgr_enable", 0);
	g_settings.hdmi_dd = configfile.getInt32( "hdmi_dd", 0);
	g_settings.spdif_dd = configfile.getInt32( "spdif_dd", 1);
	g_settings.analog_out = configfile.getInt32( "analog_out", 1);
	g_settings.avsync = configfile.getInt32( "avsync", 1);
	g_settings.clockrec = configfile.getInt32( "clockrec", 1);
	g_settings.video_dbdr = configfile.getInt32("video_dbdr", 0);

	for(int i = 0; i < VIDEOMENU_VIDEOMODE_OPTION_COUNT; i++) {
		sprintf(cfg_key, "enabled_video_mode_%d", i);
		g_settings.enabled_video_modes[i] = configfile.getInt32(cfg_key, 0);
	}
#if VIDEOMENU_VIDEOMODE_OPTION_COUNT > 3
	g_settings.enabled_video_modes[3] = 1; // 720p 50Hz
	g_settings.enabled_video_modes[4] = 1; // 1080i 50Hz
#endif

	for(int i = 0; i < VIDEOMENU_VIDEOMODE_OPTION_COUNT; i++) {
		sprintf(cfg_key, "enabled_auto_mode_%d", i);
		g_settings.enabled_auto_modes[i] = configfile.getInt32(cfg_key, 1);
	}

	g_settings.cpufreq = configfile.getInt32("cpufreq", 0);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.standby_cpufreq = configfile.getInt32("standby_cpufreq", 0);
#else
	g_settings.standby_cpufreq = configfile.getInt32("standby_cpufreq", 100);
#endif
	g_settings.rounded_corners = configfile.getInt32("rounded_corners", 1);
	g_settings.ci_standby_reset = configfile.getInt32("ci_standby_reset", 0);
	g_settings.ci_clock = configfile.getInt32("ci_clock", 9);
	g_settings.ci_ignore_messages = configfile.getInt32("ci_ignore_messages", 0);
	g_settings.ci_save_pincode = configfile.getInt32("ci_save_pincode", 0);
	g_settings.ci_check_live = configfile.getInt32("ci_check_live", 0);
	g_settings.ci_pincode = configfile.getString("ci_pincode", "");
	g_settings.ci_tuner = configfile.getInt32("ci_tuner", -1);

#ifndef CPU_FREQ
	g_settings.cpufreq = 0;
	g_settings.standby_cpufreq = 50;
#endif

	g_settings.make_hd_list = configfile.getInt32("make_hd_list", 0);
	g_settings.make_webtv_list = configfile.getInt32("make_webtv_list", 1);
	g_settings.make_new_list = configfile.getInt32("make_new_list", 1);
	g_settings.make_removed_list = configfile.getInt32("make_removed_list", 1);
	g_settings.keep_channel_numbers = configfile.getInt32("keep_channel_numbers", 0);
	g_settings.show_empty_favorites = configfile.getInt32("show_empty_favorites", 0);

	//misc
	g_settings.power_standby = configfile.getInt32( "power_standby", 0);

	//led
	g_settings.led_tv_mode = configfile.getInt32( "led_tv_mode", 2);
	g_settings.led_standby_mode = configfile.getInt32( "led_standby_mode", 3);
	g_settings.led_deep_mode = configfile.getInt32( "led_deep_mode", 3);
	g_settings.led_rec_mode = configfile.getInt32( "led_rec_mode", 1);
	g_settings.led_blink = configfile.getInt32( "led_blink", 1);
	g_settings.backlight_tv = configfile.getInt32( "backlight_tv", 1);
	g_settings.backlight_standby = configfile.getInt32( "backlight_standby", 0);
	g_settings.backlight_deepstandby = configfile.getInt32( "backlight_deepstandby", 0);
	g_settings.lcd_scroll = configfile.getInt32( "lcd_scroll", 1);
	g_settings.lcd_notify_rclock = configfile.getInt32("lcd_notify_rclock", 1);

	g_settings.hdd_fs = configfile.getInt32( "hdd_fs", 0);
	g_settings.hdd_sleep = configfile.getInt32( "hdd_sleep", 0);
	g_settings.hdd_noise = configfile.getInt32( "hdd_noise", 254);
	g_settings.hdd_statfs_mode = configfile.getInt32( "hdd_statfs_mode", SNeutrinoSettings::HDD_STATFS_RECORDING);

	g_settings.shutdown_real         = configfile.getBool("shutdown_real"        , false );
	g_settings.shutdown_real_rcdelay = configfile.getBool("shutdown_real_rcdelay", false );
	g_settings.shutdown_count = configfile.getInt32("shutdown_count", 0);

	g_settings.shutdown_min = 0;
	if (can_deepstandby || cs_get_revision() == 1 || cs_get_revision() > 7)
		g_settings.shutdown_min = configfile.getInt32("shutdown_min", 0);
	g_settings.sleeptimer_min = configfile.getInt32("sleeptimer_min", 0);

	g_settings.infobar_sat_display   = false; //configfile.getBool("infobar_sat_display"  , false );
	g_settings.infobar_show_channeldesc   = false; //configfile.getBool("infobar_show_channeldesc"  , false );
	g_settings.infobar_subchan_disp_pos = configfile.getInt32("infobar_subchan_disp_pos"  , 0 );
	g_settings.infobar_buttons_usertitle = configfile.getBool("infobar_buttons_usertitle", false );
	g_settings.progressbar_gradient = configfile.getBool("progressbar_gradient", true );
	g_settings.progressbar_design =  configfile.getInt32("progressbar_design", CProgressBar::PB_COLOR);
	bool pb_color = configfile.getBool("progressbar_color", true );
	if (!pb_color)
		g_settings.progressbar_design = CProgressBar::PB_MONO;
	g_settings.progressbar_timescale_red = configfile.getInt32("progressbar_timescale_red", 0);
	g_settings.progressbar_timescale_green = configfile.getInt32("progressbar_timescale_green", 100);
	g_settings.progressbar_timescale_yellow = configfile.getInt32("progressbar_timescale_yellow", 70);
	g_settings.progressbar_timescale_invert = configfile.getBool("progressbar_timescale_invert", false);
	g_settings.infobar_anaclock = configfile.getInt32("infobar_anaclock", 0);
	g_settings.infobar_show = configfile.getInt32("infobar_show", configfile.getInt32("infobar_cn", 1));
	g_settings.infobar_show_channellogo   = 2; //configfile.getInt32("infobar_show_channellogo"  , 2 );
	g_settings.infobar_progressbar   = 3; //configfile.getInt32("infobar_progressbar"  , 3 ); // between epg
	g_settings.infobar_casystem_display = configfile.getInt32("infobar_casystem_display", 1 );//discreet ca mode default
	g_settings.infobar_casystem_dotmatrix = configfile.getInt32("infobar_casystem_dotmatrix", 0 );
	g_settings.infobar_casystem_frame = configfile.getInt32("infobar_casystem_frame", 1 );
	g_settings.scrambled_message = configfile.getBool("scrambled_message", false );
	g_settings.volume_pos = configfile.getInt32("volume_pos", CVolumeBar::VOLUMEBAR_POS_TOP_RIGHT );
	g_settings.volume_digits = configfile.getBool("volume_digits", true);
	g_settings.volume_size = configfile.getInt32("volume_size", 26 );
	g_settings.menu_pos = configfile.getInt32("menu_pos", CMenuWidget::MENU_POS_CENTER);
	g_settings.show_menu_hints = configfile.getBool("show_menu_hints", false);
	g_settings.infobar_show_sysfs_hdd   = configfile.getBool("infobar_show_sysfs_hdd"  , false );
	g_settings.show_mute_icon = configfile.getInt32("show_mute_icon" ,0);
	g_settings.infobar_show_res = 1; //configfile.getInt32("infobar_show_res", 1 );
	g_settings.infobar_show_dd_available = 1; //configfile.getInt32("infobar_show_dd_available", 1 );

	g_settings.infobar_show_tuner = configfile.getInt32("infobar_show_tuner", 1 );
	g_settings.radiotext_enable = configfile.getBool("radiotext_enable"          , false);
	g_settings.radiotext_rass_dir = configfile.getString("radiotext_rass_dir", "/tmp/cache");
	//audio
	g_settings.audio_AnalogMode = configfile.getInt32( "audio_AnalogMode", 0 );
	g_settings.audio_DolbyDigital    = configfile.getBool("audio_DolbyDigital"   , false);

	g_settings.auto_lang = configfile.getInt32( "auto_lang", 0 );
	g_settings.auto_subs = configfile.getInt32( "auto_subs", 0 );

	for(int i = 0; i < 3; i++) {
		sprintf(cfg_key, "pref_lang_%d", i);
		g_settings.pref_lang[i] = configfile.getString(cfg_key, "none");
		sprintf(cfg_key, "pref_subs_%d", i);
		g_settings.pref_subs[i] = configfile.getString(cfg_key, "none");
	}
	g_settings.subs_charset = configfile.getString("subs_charset", "CP1252");
	g_settings.zap_cycle = configfile.getInt32( "zap_cycle", 0 );

	//screen saver
	g_settings.screensaver_delay = configfile.getInt32("screensaver_delay", 1);
	g_settings.screensaver_dir = configfile.getString("screensaver_dir", ICONSDIR);
	g_settings.screensaver_timeout = configfile.getInt32("screensaver_timeout", 10);
	g_settings.screensaver_random = configfile.getInt32("screensaver_random", 0);
	g_settings.screensaver_mode = configfile.getInt32("screensaver_mode", CScreenSaver::SCR_MODE_IMAGE);

	//vcr
	g_settings.vcr_AutoSwitch = configfile.getBool("vcr_AutoSwitch"       , true );

	//language
	g_settings.language = configfile.getString("language", "");
	g_settings.timezone = configfile.getString("timezone", "(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Vienna");
	//epg dir
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.epg_cache            = configfile.getInt32("epg_cache_time", 3);
	g_settings.epg_extendedcache    = configfile.getInt32("epg_extendedcache_time", 3);
	g_settings.epg_old_events       = configfile.getInt32("epg_old_events", 3);
	g_settings.epg_max_events       = configfile.getInt32("epg_max_events", 20000);
	g_settings.epg_dir              = configfile.getString("epg_dir", "/hdd/epg");
#else
	g_settings.epg_cache            = configfile.getInt32("epg_cache_time", 14);
	g_settings.epg_extendedcache    = configfile.getInt32("epg_extendedcache_time", 360);
	g_settings.epg_old_events       = configfile.getInt32("epg_old_events", 1);
	g_settings.epg_max_events       = configfile.getInt32("epg_max_events", 30000);
	g_settings.epg_dir              = configfile.getString("epg_dir", "/media/sda1/epg");
#endif
	// NTP-Server for sectionsd
	g_settings.network_ntpserver    = configfile.getString("network_ntpserver", "time.fu-berlin.de");
	g_settings.network_ntprefresh   = configfile.getString("network_ntprefresh", "30" );
	g_settings.network_ntpenable    = configfile.getBool("network_ntpenable", false);

	g_settings.ifname = configfile.getString("ifname", "eth0");

	g_settings.epg_save = configfile.getBool("epg_save", false);
	g_settings.epg_save_standby = configfile.getBool("epg_save_standby", true);
	g_settings.epg_save_frequently = configfile.getInt32("epg_save_frequently", false);
	g_settings.epg_read = configfile.getBool("epg_read", g_settings.epg_save);
	g_settings.epg_scan = configfile.getInt32("epg_scan", CEpgScan::SCAN_CURRENT);
	g_settings.epg_scan_mode = configfile.getInt32("epg_scan_mode", CEpgScan::MODE_OFF);
	// backward-compatible check
	if (g_settings.epg_scan == 0) {
		g_settings.epg_scan = CEpgScan::SCAN_CURRENT;
		g_settings.epg_scan_mode = CEpgScan::MODE_OFF;
	}
	g_settings.epg_save_mode = configfile.getInt32("epg_save_mode", 0);
	//widget settings
	g_settings.widget_fade = false;
	g_settings.widget_fade           = configfile.getBool("widget_fade"          , false );

	//theme/color options
	CThemes::getTheme(configfile);
	g_settings.osd_colorsettings_advanced_mode = configfile.getBool("osd_colorsettings_advanced_mode", false);

#ifdef ENABLE_GRAPHLCD
	g_settings.glcd_enable = configfile.getInt32("glcd_enable", 0);
	g_settings.glcd_color_fg = configfile.getInt32("glcd_color_fg", GLCD::cColor::White);
	g_settings.glcd_color_bg = configfile.getInt32("glcd_color_bg", GLCD::cColor::Black);
	g_settings.glcd_color_bar = configfile.getInt32("glcd_color_bar", GLCD::cColor::Blue);
	g_settings.glcd_percent_channel = configfile.getInt32("glcd_percent_channel", 22);
	g_settings.glcd_percent_epg = configfile.getInt32("glcd_percent_epg", 16);
	g_settings.glcd_percent_bar = configfile.getInt32("glcd_percent_bar", 8);
	g_settings.glcd_percent_time = configfile.getInt32("glcd_percent_time", 32);
	g_settings.glcd_percent_time_standby = configfile.getInt32("glcd_percent_time_standby", 50);
	g_settings.glcd_percent_logo = configfile.getInt32("glcd_percent_logo", 50);
	g_settings.glcd_mirror_osd = configfile.getInt32("glcd_mirror_osd", 0);
	g_settings.glcd_mirror_video = configfile.getInt32("glcd_mirror_video", 0);
	g_settings.glcd_time_in_standby = configfile.getInt32("glcd_time_in_standby", 1);
	g_settings.glcd_show_logo = configfile.getInt32("glcd_show_logo", 1);
	g_settings.glcd_font = configfile.getString("glcd_font", FONTDIR "/neutrino.ttf");
	g_settings.glcd_brightness = configfile.getInt32("glcd_brightness", 75);
	g_settings.glcd_brightness_standby = configfile.getInt32("glcd_brightness_standby", 45);
	g_settings.glcd_scroll_speed = configfile.getInt32("glcd_scroll_speed", 5);
#endif

	//personalize
	g_settings.personalize_pincode = configfile.getString( "personalize_pincode", "0000" );
	for (int i = 0; i < SNeutrinoSettings::P_SETTINGS_MAX; i++)//settings.h, settings.cpp
		g_settings.personalize[i] = configfile.getInt32( personalize_settings[i].personalize_settings_name, personalize_settings[i].personalize_default_val );

	g_settings.contrast_fonts = configfile.getInt32("contrast_fonts", 0);

	//network
	for(int i=0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) {
		std::string i_str(to_string(i));
		g_settings.network_nfs[i].ip = configfile.getString("network_nfs_ip_" + i_str, "");
		g_settings.network_nfs[i].dir = configfile.getString("network_nfs_dir_" + i_str, "");
		g_settings.network_nfs[i].local_dir = configfile.getString("network_nfs_local_dir_" + i_str, "");
		if (g_settings.network_nfs[i].local_dir.empty())
			g_settings.network_nfs[i].local_dir = "/mnt/mnt" + i_str;
		g_settings.network_nfs[i].automount = configfile.getInt32("network_nfs_automount_" + i_str, 0);
		g_settings.network_nfs[i].type = configfile.getInt32("network_nfs_type_" + i_str, 0);
		g_settings.network_nfs[i].username = configfile.getString("network_nfs_username_" + i_str, "" );
		g_settings.network_nfs[i].password = configfile.getString("network_nfs_password_" + i_str, "" );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		g_settings.network_nfs[i].mount_options1 = configfile.getString("network_nfs_mount_options1_" + i_str, "rw,soft,udp" );
		g_settings.network_nfs[i].mount_options2 = configfile.getString("network_nfs_mount_options2_" + i_str, "nolock,rsize=32768,wsize=32768" );
		g_settings.network_nfs[i].mac = configfile.getString("network_nfs_mac_" + i_str, "11:22:33:44:55:66");
	}
	g_settings.network_nfs_audioplayerdir = configfile.getString( "network_nfs_audioplayerdir", "/hdd/music" );
	g_settings.network_nfs_picturedir = configfile.getString( "network_nfs_picturedir", "/hdd/pictures" );
	g_settings.network_nfs_moviedir = configfile.getString( "network_nfs_moviedir", "/hdd/movie" );
	g_settings.network_nfs_recordingdir = configfile.getString( "network_nfs_recordingdir", "/hdd/movie" );
#else
		g_settings.network_nfs[i].mount_options1 = configfile.getString("network_nfs_mount_options1_" + i_str, "ro,soft,udp" );
		g_settings.network_nfs[i].mount_options2 = configfile.getString("network_nfs_mount_options2_" + i_str, "nolock,rsize=8192,wsize=8192" );
		g_settings.network_nfs[i].mac = configfile.getString("network_nfs_mac_" + i_str, "11:22:33:44:55:66");
	}
	g_settings.network_nfs_audioplayerdir = configfile.getString( "network_nfs_audioplayerdir", "/media/sda1/music" );
	g_settings.network_nfs_picturedir = configfile.getString( "network_nfs_picturedir", "/media/sda1/pictures" );
	g_settings.network_nfs_moviedir = configfile.getString( "network_nfs_moviedir", "/media/sda1/movie" );
	g_settings.network_nfs_recordingdir = configfile.getString( "network_nfs_recordingdir", "/media/sda1/movie" );
#endif
	g_settings.timeshiftdir = configfile.getString( "timeshiftdir", "" );
	g_settings.downloadcache_dir = configfile.getString( "downloadcache_dir", g_settings.network_nfs_recordingdir.c_str());
	g_settings.last_webtv_dir = configfile.getString( "last_webtv_dir", CONFIGDIR);

	g_settings.temp_timeshift = configfile.getInt32( "temp_timeshift", 0 );
	g_settings.auto_timeshift = configfile.getInt32( "auto_timeshift", 0 );
	g_settings.auto_delete = configfile.getInt32( "auto_delete", 1 );

	std::string timeshiftDir;
	if(g_settings.timeshiftdir.empty()) {
		timeshiftDir = g_settings.network_nfs_recordingdir + "/.timeshift";
		safe_mkdir(timeshiftDir.c_str());
	} else {
		if(g_settings.timeshiftdir != g_settings.network_nfs_recordingdir)
			timeshiftDir = g_settings.timeshiftdir;
		else
			timeshiftDir = g_settings.network_nfs_recordingdir + "/.timeshift";
	}
	dprintf(DEBUG_NORMAL, "recording dir: %s\n", g_settings.network_nfs_recordingdir.c_str());
	dprintf(DEBUG_NORMAL, "timeshift dir: %s\n", timeshiftDir.c_str());

	CRecordManager::getInstance()->SetTimeshiftDirectory(timeshiftDir.c_str());

	if(g_settings.auto_delete) {
		if(g_settings.timeshiftdir == g_settings.network_nfs_recordingdir) {
			DIR *d = opendir(timeshiftDir.c_str());
			if(d){
				while (struct dirent *e = readdir(d))
				{
					std::string filename = e->d_name;
					if ((filename.find("_temp.ts") == filename.size() - 8) || (filename.find("_temp.xml") == filename.size() - 9))
					{
						std::string timeshiftDir_filename= timeshiftDir;
						timeshiftDir_filename+= "/" + filename;
						remove(timeshiftDir_filename.c_str());
					}
				}
				closedir(d);
			}
		}
	}
	g_settings.record_hours = configfile.getInt32( "record_hours", 4 );
	g_settings.timeshift_hours = configfile.getInt32( "timeshift_hours", 4 );
	g_settings.filesystem_is_utf8              = configfile.getBool("filesystem_is_utf8"                 , true );

	//recording (server + vcr)
	g_settings.recording_type = configfile.getInt32("recording_type", RECORDING_FILE);
	g_settings.recording_stopsectionsd         = configfile.getBool("recording_stopsectionsd"            , false );
	g_settings.recording_audio_pids_default    = configfile.getInt32("recording_audio_pids_default", TIMERD_APIDS_STD | TIMERD_APIDS_AC3);
	g_settings.recording_zap_on_announce       = configfile.getBool("recording_zap_on_announce"      , false);
	g_settings.shutdown_timer_record_type      = configfile.getBool("shutdown_timer_record_type"      , false);

	g_settings.recording_stream_vtxt_pid       = configfile.getBool("recordingmenu.stream_vtxt_pid"      , true);
	g_settings.recording_stream_subtitle_pids  = configfile.getBool("recordingmenu.stream_subtitle_pids", true);
	g_settings.recording_stream_pmt_pid        = configfile.getBool("recordingmenu.stream_pmt_pid"      , false);
	g_settings.recording_filename_template     = configfile.getString("recordingmenu.filename_template" , "%C_%T_%d_%t");
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.recording_bufsize               = configfile.getInt32("recording_bufsize", 4);
	g_settings.recording_bufsize_dmx           = configfile.getInt32("recording_bufsize_dmx", 2);
#endif
	g_settings.recording_choose_direct_rec_dir = configfile.getInt32( "recording_choose_direct_rec_dir", 0 );
	g_settings.recording_epg_for_filename      = configfile.getBool("recording_epg_for_filename"         , true);
	g_settings.recording_epg_for_end           = configfile.getBool("recording_epg_for_end"              , true);
	g_settings.recording_save_in_channeldir    = configfile.getBool("recording_save_in_channeldir"         , false);
	g_settings.recording_slow_warning	   = configfile.getBool("recording_slow_warning"     , true);
	g_settings.recording_startstop_msg	   = configfile.getBool("recording_startstop_msg"     , true);
	g_settings.recording_already_found_check   = configfile.getBool("recording_already_found_check", false);

	// default plugin for movieplayer
	g_settings.movieplayer_plugin = configfile.getString( "movieplayer_plugin", "---" );

	g_settings.plugins_disabled = configfile.getString( "plugins_disabled", "" );
	g_settings.plugins_game = configfile.getString( "plugins_game", "" );
	g_settings.plugins_tool = configfile.getString( "plugins_tool", "" );
	g_settings.plugins_script = configfile.getString( "plugins_script", "" );
	g_settings.plugins_lua = configfile.getString( "plugins_lua", "" );

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.plugin_hdd_dir = configfile.getString( "plugin_hdd_dir", "/var/tuxbox/plugins" );
	g_settings.logo_hdd_dir = configfile.getString( "logo_hdd_dir", "/logos" );
#else
	g_settings.plugin_hdd_dir = configfile.getString( "plugin_hdd_dir", "/media/sda1/plugins" );
	g_settings.logo_hdd_dir = configfile.getString( "logo_hdd_dir", "/media/sda1/logos" );
#endif

	g_settings.webtv_xml.clear();
	int webtv_count = configfile.getInt32("webtv_xml_count", 0);
	if (webtv_count) {
		for (int i = 0; i < webtv_count; i++) {
			std::string k = "webtv_xml_" + to_string(i);
			std::string webtv_xml = configfile.getString(k, "");
			if (webtv_xml.empty())
				continue;
			g_settings.webtv_xml.push_back(webtv_xml);
		}
	} else {
		std::string webtv_xml = configfile.getString("webtv_xml", WEBTV_XML);
		if (file_size(webtv_xml.c_str()))
			g_settings.webtv_xml.push_back(webtv_xml);
	}

	loadKeys();

	g_settings.key_playbutton = configfile.getInt32("key_playbutton", 0);
	g_settings.timeshift_pause = configfile.getInt32( "timeshift_pause", 1 );

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.screenshot_png_compression = configfile.getInt32( "screenshot_png_compression", 1);
	g_settings.screenshot_backbuffer = configfile.getInt32( "screenshot_backbuffer", 1);
#endif
	g_settings.screenshot_count = configfile.getInt32( "screenshot_count",  1);
	g_settings.screenshot_format = configfile.getInt32( "screenshot_format",  1);
	g_settings.screenshot_cover = configfile.getInt32( "screenshot_cover",  0);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.screenshot_mode = configfile.getInt32( "screenshot_mode",  1 /* video */);
	g_settings.screenshot_res = configfile.getInt32( "screenshot_res",  2 /* = osd resolution */);
#else
	g_settings.screenshot_mode = configfile.getInt32( "screenshot_mode",  0);
	g_settings.screenshot_video = configfile.getInt32( "screenshot_video",  1);
	g_settings.screenshot_scale = configfile.getInt32( "screenshot_scale",  0);
#endif
	g_settings.auto_cover = configfile.getInt32( "auto_cover",  0);

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.screenshot_dir = configfile.getString( "screenshot_dir", "/hdd/movie" );
#else
	g_settings.screenshot_dir = configfile.getString( "screenshot_dir", "/media/sda1/movie" );
#endif
	g_settings.cacheTXT = configfile.getInt32( "cacheTXT",  0);
	g_settings.minimode = configfile.getInt32( "minimode",  0);
	g_settings.mode_clock = configfile.getInt32( "mode_clock",  0);
	g_settings.zapto_pre_time = configfile.getInt32( "zapto_pre_time",  0);
	g_settings.spectrum         = configfile.getBool("spectrum"          , false);
	g_settings.channellist_additional = configfile.getInt32("channellist_additional", 1);
	g_settings.eventlist_additional = configfile.getInt32("eventlist_additional", 1);
	g_settings.channellist_epgtext_align_right	= configfile.getBool("channellist_epgtext_align_right"          , false);
	g_settings.channellist_progressbar_design = configfile.getInt32("channellist_progressbar_design", g_settings.progressbar_design);
	g_settings.channellist_foot	= configfile.getInt32("channellist_foot"          , 2);//default next Event
	g_settings.channellist_new_zap_mode = configfile.getInt32("channellist_new_zap_mode", 1);
	g_settings.channellist_hdicon = configfile.getInt32("channellist_hdicon", 0); //default off
	g_settings.channellist_sort_mode  = configfile.getInt32("channellist_sort_mode", 0);//sort mode: alpha, freq, sat
	g_settings.channellist_numeric_adjust  = configfile.getInt32("channellist_numeric_adjust", 0);
	g_settings.channellist_show_channellogo = configfile.getInt32("channellist_show_channellogo", 1);
	g_settings.channellist_show_infobox = configfile.getInt32("channellist_show_infobox", 1);
	g_settings.channellist_show_numbers = configfile.getInt32("channellist_show_numbers", 1);

	//screen configuration
	g_settings.screen_xres = configfile.getInt32("screen_xres", 100);
	g_settings.screen_yres = configfile.getInt32("screen_yres", 100);
	g_settings.screen_StartX_crt = configfile.getInt32( "screen_StartX_crt", DEFAULT_X_START_SD);
	g_settings.screen_StartY_crt = configfile.getInt32( "screen_StartY_crt", DEFAULT_Y_START_SD );
	g_settings.screen_EndX_crt = configfile.getInt32( "screen_EndX_crt", DEFAULT_X_END_SD);
	g_settings.screen_EndY_crt = configfile.getInt32( "screen_EndY_crt", DEFAULT_Y_END_SD);
	g_settings.screen_StartX_lcd = configfile.getInt32( "screen_StartX_lcd", DEFAULT_X_START_HD);
	g_settings.screen_StartY_lcd = configfile.getInt32( "screen_StartY_lcd", DEFAULT_Y_START_HD );
	g_settings.screen_EndX_lcd = configfile.getInt32( "screen_EndX_lcd", DEFAULT_X_END_HD);
	g_settings.screen_EndY_lcd = configfile.getInt32( "screen_EndY_lcd", DEFAULT_Y_END_HD);
	g_settings.screen_preset = configfile.getInt32( "screen_preset", 1);

#if HAVE_TRIPLEDRAGON
	g_settings.screen_preset = 0; /* does not make sense to have two configurations for that... */
#elif ! HAVE_COOLSTREAM
	g_settings.screen_preset = 1; /* spark is now always using 1280x720 framebuffer */
#endif
	g_settings.screen_StartX = g_settings.screen_preset ? g_settings.screen_StartX_lcd : g_settings.screen_StartX_crt;
	g_settings.screen_StartY = g_settings.screen_preset ? g_settings.screen_StartY_lcd : g_settings.screen_StartY_crt;
	g_settings.screen_EndX = g_settings.screen_preset ? g_settings.screen_EndX_lcd : g_settings.screen_EndX_crt;
	g_settings.screen_EndY = g_settings.screen_preset ? g_settings.screen_EndY_lcd : g_settings.screen_EndY_crt;
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.screen_StartX_int = g_settings.screen_StartX;
	g_settings.screen_StartY_int = g_settings.screen_StartY;
	g_settings.screen_EndX_int = g_settings.screen_EndX;
	g_settings.screen_EndY_int = g_settings.screen_EndY;
	g_settings.screen_StartX = 0;
	g_settings.screen_StartY = 0;
	g_settings.screen_EndX = frameBuffer->getScreenWidth() - 1;
	g_settings.screen_EndY = frameBuffer->getScreenHeight() - 1;
#endif

	g_settings.screen_width = configfile.getInt32("screen_width", 0);
	g_settings.screen_height = configfile.getInt32("screen_height", 0);

#if 0
	g_settings.bigFonts = configfile.getInt32("bigFonts", 0);
#else
	g_settings.bigFonts = 1;
#endif
	g_settings.window_size = configfile.getInt32("window_size", 100);
	g_settings.window_width = configfile.getInt32("window_width", g_settings.window_size);
	g_settings.window_height = configfile.getInt32("window_height", g_settings.window_size);

	g_settings.remote_control_hardware = configfile.getInt32( "remote_control_hardware",  CRCInput::RC_HW_COOLSTREAM);
	g_settings.audiochannel_up_down_enable = configfile.getBool("audiochannel_up_down_enable", false);

	//Software-update
	g_settings.softupdate_mode = configfile.getInt32( "softupdate_mode", 1 );
	g_settings.apply_kernel = configfile.getBool("apply_kernel" , false);
	g_settings.apply_settings = configfile.getBool("apply_settings" , false);
#if ENABLE_EXTUPDATE
	g_settings.softupdate_name_mode_apply = configfile.getInt32( "softupdate_name_mode_apply", CExtUpdate::SOFTUPDATE_NAME_DEFAULT);
	g_settings.softupdate_name_mode_backup = configfile.getInt32( "softupdate_name_mode_backup", CExtUpdate::SOFTUPDATE_NAME_DEFAULT);
#else
	g_settings.softupdate_name_mode_apply = 0;
	g_settings.softupdate_name_mode_backup = 0; /* unused, but still initialize it */
#endif

	g_settings.flashupdate_createimage_add_var    = configfile.getInt32( "flashupdate_createimage_add_var",    1);
	g_settings.flashupdate_createimage_add_root1  = configfile.getInt32( "flashupdate_createimage_add_root1",  0);
	g_settings.flashupdate_createimage_add_uldr   = configfile.getInt32( "flashupdate_createimage_add_uldr",   1);
	g_settings.flashupdate_createimage_add_u_boot = configfile.getInt32( "flashupdate_createimage_add_u_boot", 0);
	g_settings.flashupdate_createimage_add_env    = configfile.getInt32( "flashupdate_createimage_add_env",    0);
	g_settings.flashupdate_createimage_add_spare  = configfile.getInt32( "flashupdate_createimage_add_spare",  0);
	g_settings.flashupdate_createimage_add_kernel = configfile.getInt32( "flashupdate_createimage_add_kernel", 1);

	g_settings.image_settings_backup_path = configfile.getString("image_settings_backup_path", "/swap/backup");
	g_settings.softupdate_url_file      = configfile.getString("softupdate_url_file", "/var/etc/update.urls");
	g_settings.softupdate_proxyserver   = configfile.getString("softupdate_proxyserver", "" );
	g_settings.softupdate_proxyusername = configfile.getString("softupdate_proxyusername", "" );
	g_settings.softupdate_proxypassword = configfile.getString("softupdate_proxypassword", "" );
	//
	if (g_settings.softupdate_proxyserver.empty())
		unsetenv("http_proxy");
	else {
		std::string proxy = "http://";
		if (!g_settings.softupdate_proxyusername.empty())
			proxy += g_settings.softupdate_proxyusername + ":" + g_settings.softupdate_proxypassword + "@";
		proxy += g_settings.softupdate_proxyserver;
		setenv("http_proxy", proxy.c_str(), 1);
	}

	g_settings.font_file = configfile.getString("font_file", FONTDIR"/neutrino.ttf");
	g_settings.ttx_font_file = configfile.getString( "ttx_font_file", FONTDIR"/DejaVuLGCSansMono-Bold.ttf");
	ttx_font_file = g_settings.ttx_font_file;
	g_settings.sub_font_file = configfile.getString("sub_font_file", FONTDIR"/neutrino.ttf");
	sub_font_file = &g_settings.sub_font_file;
	sub_font_size = configfile.getInt32("fontsize.subtitles", 24);

	g_settings.update_dir = configfile.getString("update_dir", "/tmp");
	g_settings.update_dir_opkg = configfile.getString("update_dir_opkg", g_settings.update_dir);

	// parentallock
	if (!parentallocked) {
		g_settings.parentallock_prompt = configfile.getInt32( "parentallock_prompt", 0 );
		g_settings.parentallock_lockage = configfile.getInt32( "parentallock_lockage", 12 );
	} else {
		g_settings.parentallock_prompt = 3;
		g_settings.parentallock_lockage = 18;
	}
	g_settings.parentallock_defaultlocked = configfile.getInt32("parentallock_defaultlocked", 0);
	g_settings.parentallock_pincode = configfile.getString( "parentallock_pincode", "0000" );
	g_settings.parentallock_zaptime = configfile.getInt32( "parentallock_zaptime", 60 );

	for (int i = 0; i < SNeutrinoSettings::TIMING_SETTING_COUNT; i++)
		g_settings.timing[i] = configfile.getInt32(locale_real_names[timing_setting[i].name], timing_setting[i].default_timing);

	for (int i = 0; i < SNeutrinoSettings::LCD_SETTING_COUNT; i++)
		g_settings.lcd_setting[i] = configfile.getInt32(lcd_setting[i].name, lcd_setting[i].default_value);
	g_settings.lcd_setting_dim_time = configfile.getString("lcd_dim_time","0");
	g_settings.lcd_setting_dim_brightness = configfile.getInt32("lcd_dim_brightness", 0);
	g_settings.lcd_info_line = configfile.getInt32("lcd_info_line", 0);//channel name or clock
#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
	g_settings.lcd_vfd_scroll = configfile.getInt32("lcd_vfd_scroll", 1);
#endif

	//Movie-Player
	g_settings.movieplayer_repeat_on = configfile.getInt32("movieplayer_repeat_on", CMoviePlayerGui::REPEAT_OFF);
	g_settings.youtube_dev_id = configfile.getString("youtube_dev_id","XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	g_settings.tmdb_api_key = configfile.getString("tmdb_api_key","XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

	//Filebrowser
	g_settings.filebrowser_showrights =  configfile.getInt32("filebrowser_showrights", 1);
	g_settings.filebrowser_sortmethod = configfile.getInt32("filebrowser_sortmethod", 0);
	if ((g_settings.filebrowser_sortmethod < 0) || (g_settings.filebrowser_sortmethod >= FILEBROWSER_NUMBER_OF_SORT_VARIANTS))
		g_settings.filebrowser_sortmethod = 0;
	g_settings.filebrowser_denydirectoryleave = configfile.getBool("filebrowser_denydirectoryleave", false);
	g_settings.filebrowser_use_filter = configfile.getBool("filebrowser_usefilter", true);

	//zapit setup
	g_settings.StartChannelTV = configfile.getString("startchanneltv","");
	g_settings.StartChannelRadio = configfile.getString("startchannelradio","");
	g_settings.startchanneltv_id =  configfile.getInt64("startchanneltv_id", 0);
	g_settings.startchannelradio_id =  configfile.getInt64("startchannelradio_id", 0);
	g_settings.uselastchannel         = configfile.getInt32("uselastchannel" , 1);
	//epg searsch
	g_settings.epg_search_history_max = configfile.getInt32("epg_search_history_max", 10);
	g_settings.epg_search_history_size = configfile.getInt32("epg_search_history_size", 0);
	if (g_settings.epg_search_history_size > g_settings.epg_search_history_max)
		g_settings.epg_search_history_size = g_settings.epg_search_history_max;
	g_settings.epg_search_history.clear();
	for(int i = 0; i < g_settings.epg_search_history_size; i++) {
		std::string s = configfile.getString("epg_search_history_" + to_string(i));
		if (!s.empty())
			g_settings.epg_search_history.push_back(configfile.getString("epg_search_history_" + to_string(i), ""));
	}
	g_settings.epg_search_history_size = g_settings.epg_search_history.size();

	g_settings.adzap_zapBackPeriod = configfile.getInt32("adzap_zapBackPeriod", 180);
	g_settings.adzap_writeData = configfile.getInt32("adzap_writeData", 0);

	// USERMENU -> in system/settings.h
	//-------------------------------------------

	g_settings.usermenu.clear();
	if (configfile.getString("usermenu_key_red", "").empty() ||
			configfile.getString("usermenu_key_green", "").empty() ||
			configfile.getString("usermenu_key_yellow", "").empty() ||
			configfile.getString("usermenu_key_blue", "").empty())
	{
		for(SNeutrinoSettings::usermenu_t *um = usermenu_default; um->key != CRCInput::RC_nokey; um++) {
			SNeutrinoSettings::usermenu_t *u = new SNeutrinoSettings::usermenu_t;
			*u = *um;
			g_settings.usermenu.push_back(u);
		}
	} else {
		bool unknown = configfile.getUnknownKeyQueryedFlag();
		for (unsigned int i = 0; ; i++) {
			std::string name = (i < 4) ? usermenu_default[i].name : to_string(i);
			std::string usermenu_key("usermenu_key_");
			usermenu_key += name;
			int uk = configfile.getInt32(usermenu_key, CRCInput::RC_nokey);
			if (!uk || uk == (int)CRCInput::RC_nokey) {
				if (i > 3) {
					configfile.setUnknownKeyQueryedFlag(unknown);
					break;
				}
				continue;
			}
			SNeutrinoSettings::usermenu_t *u = new SNeutrinoSettings::usermenu_t;
			u->key = uk;

			std::string txt1("usermenu_tv_");
			txt1 += name;
			u->items = configfile.getString(txt1, "");
			txt1 += "_text";
			u->title = configfile.getString(txt1, "");

			g_settings.usermenu.push_back(u);
		}
	}

	if(configfile.getUnknownKeyQueryedFlag() && (erg==0)) {
		erg = 2;
	}

	/* in case FB resolution changed */
	if((g_settings.screen_width && g_settings.screen_width != (int) frameBuffer->getScreenWidth(true))
			|| (g_settings.screen_height && g_settings.screen_height != (int) frameBuffer->getScreenHeight(true))) {
		g_settings.screen_StartX = g_settings.screen_preset ? DEFAULT_X_START_HD : DEFAULT_X_START_SD;
		g_settings.screen_StartY = g_settings.screen_preset ? DEFAULT_Y_START_HD : DEFAULT_Y_START_SD;
		g_settings.screen_EndX = g_settings.screen_preset ? DEFAULT_X_END_HD : DEFAULT_X_END_SD;
		g_settings.screen_EndY = g_settings.screen_preset ? DEFAULT_Y_END_HD : DEFAULT_Y_END_SD;

		g_settings.screen_width = frameBuffer->getScreenWidth(true);
		g_settings.screen_height = frameBuffer->getScreenHeight(true);
	}
#ifdef BOXMODEL_APOLLO
	g_settings.brightness = configfile.getInt32("brightness", 0);
	g_settings.contrast = configfile.getInt32("contrast", 0);
	g_settings.saturation = configfile.getInt32("saturation", 0);
	g_settings.enable_sd_osd = configfile.getInt32("enable_sd_osd", 1);
#endif
#ifdef ENABLE_PIP
	g_settings.pip_x = configfile.getInt32("pip_x", 50);
	g_settings.pip_y = configfile.getInt32("pip_y", 50);
	g_settings.pip_width = configfile.getInt32("pip_width", 365);
	g_settings.pip_height = configfile.getInt32("pip_height", 200);

	g_settings.pip_radio_x = configfile.getInt32("pip_radio_x", g_settings.pip_x);
	g_settings.pip_radio_y = configfile.getInt32("pip_radio_y", g_settings.pip_y);
	g_settings.pip_radio_width = configfile.getInt32("pip_radio_width", g_settings.pip_width);
	g_settings.pip_radio_height = configfile.getInt32("pip_radio_height", g_settings.pip_height);
#endif

	g_settings.infoClockFontSize = configfile.getInt32("infoClockFontSize", 30);
	g_settings.infoClockBackground = configfile.getInt32("infoClockBackground", 0);
	g_settings.infoClockSeconds = configfile.getInt32("infoClockSeconds", 1);

	g_settings.livestreamResolution = configfile.getInt32("livestreamResolution", 1920);
	g_settings.livestreamScriptPath = configfile.getString("livestreamScriptPath", PLUGINDIR_VAR "/webtv");

	g_settings.version_pseudo = configfile.getString("version_pseudo", "19700101000000");

	if (g_settings.version_pseudo < NEUTRINO_VERSION_PSEUDO)
		upgradeSetup(fname);

	if(erg)
		configfile.setModifiedFlag(true);
	return erg;
}

void CNeutrinoApp::upgradeSetup(const char * fname)
{
	if (g_settings.version_pseudo < "20160226110000")
	{
		if (g_settings.usermenu[SNeutrinoSettings::BUTTON_YELLOW]->items == "7")
		{
			g_settings.usermenu[SNeutrinoSettings::BUTTON_YELLOW]->items = "7,31";
			configfile.setString("usermenu_tv_yellow", g_settings.usermenu[SNeutrinoSettings::BUTTON_YELLOW]->items);
		}
	}

	g_settings.version_pseudo = NEUTRINO_VERSION_PSEUDO;
	configfile.setString("version_pseudo", g_settings.version_pseudo);

	if (configfile.getModifiedFlag())
		configfile.saveConfig(fname);
}

/**************************************************************************************
*          CNeutrinoApp -  saveSetup, save the application-settings                   *
**************************************************************************************/
void CNeutrinoApp::saveSetup(const char * fname)
{
	char cfg_key[81];
	//scansettings
	if(!scansettings.saveSettings(NEUTRINO_SCAN_SETTINGS_FILE)) {
		dprintf(DEBUG_NORMAL, "error while saving scan-settings!\n");
	}
	configfile.setInt32("lcd4l_support" , g_settings.lcd4l_support);
	configfile.setString("lcd4l_logodir" , g_settings.lcd4l_logodir);
	configfile.setInt32("lcd4l_skin" , g_settings.lcd4l_skin);

	configfile.setInt32("show_ecm" , g_settings.show_ecm);
	configfile.setInt32("show_ecm_pos" , g_settings.show_ecm_pos);

	//video
	configfile.setInt32( "video_Mode", g_settings.video_Mode );
	configfile.setInt32( "analog_mode1", g_settings.analog_mode1 );
	configfile.setInt32( "analog_mode2", g_settings.analog_mode2 );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32( "hdmi_mode", g_settings.hdmi_mode );
#endif
	configfile.setInt32( "video_Format", g_settings.video_Format );
	configfile.setInt32( "video_43mode", g_settings.video_43mode );
	configfile.setInt32( "hdmi_cec_mode", g_settings.hdmi_cec_mode );
	configfile.setInt32( "hdmi_cec_view_on", g_settings.hdmi_cec_view_on );
	configfile.setInt32( "hdmi_cec_standby", g_settings.hdmi_cec_standby );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32( "hdmi_cec_broadcast", g_settings.hdmi_cec_broadcast );
	configfile.setInt32( "video_psi_contrast", g_settings.psi_contrast );
	configfile.setInt32( "video_psi_saturation", g_settings.psi_saturation );
	configfile.setInt32( "video_psi_brightness", g_settings.psi_brightness );
	configfile.setInt32( "video_psi_tint", g_settings.psi_tint );
	configfile.setInt32( "video_psi_step", g_settings.psi_step );
	configfile.setInt32( "video_mixer_color", g_settings.video_mixer_color );
#endif

	configfile.setInt32( "current_volume", g_settings.current_volume );
	configfile.setInt32( "current_volume_step", g_settings.current_volume_step );
	configfile.setInt32( "start_volume", g_settings.start_volume );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32("audio_mixer_volume_analog", g_settings.audio_mixer_volume_analog);
	configfile.setInt32("audio_mixer_volume_hdmi", g_settings.audio_mixer_volume_hdmi);
	configfile.setInt32("audio_mixer_volume_spdif", g_settings.audio_mixer_volume_spdif);
#endif
	configfile.setInt32("audio_volume_percent_ac3", g_settings.audio_volume_percent_ac3);
	configfile.setInt32("audio_volume_percent_pcm", g_settings.audio_volume_percent_pcm);
	configfile.setInt32( "channel_mode", g_settings.channel_mode );
	configfile.setInt32( "channel_mode_radio", g_settings.channel_mode_radio );
	configfile.setInt32( "channel_mode_initial", g_settings.channel_mode_initial );
	configfile.setInt32( "channel_mode_initial_radio", g_settings.channel_mode_initial_radio );

	configfile.setInt32( "fan_speed", g_settings.fan_speed);

	configfile.setInt32( "srs_enable", g_settings.srs_enable);
	configfile.setInt32( "srs_algo", g_settings.srs_algo);
	configfile.setInt32( "srs_ref_volume", g_settings.srs_ref_volume);
	configfile.setInt32( "srs_nmgr_enable", g_settings.srs_nmgr_enable);
	configfile.setInt32( "hdmi_dd", g_settings.hdmi_dd);
	configfile.setInt32( "analog_out", g_settings.analog_out);
	configfile.setInt32( "spdif_dd", g_settings.spdif_dd);
	configfile.setInt32( "avsync", g_settings.avsync);
	configfile.setInt32( "clockrec", g_settings.clockrec);
	configfile.setInt32( "video_dbdr", g_settings.video_dbdr);
	for(int i = 0; i < VIDEOMENU_VIDEOMODE_OPTION_COUNT; i++) {
		sprintf(cfg_key, "enabled_video_mode_%d", i);
		configfile.setInt32(cfg_key, g_settings.enabled_video_modes[i]);
	}
	for(int i = 0; i < VIDEOMENU_VIDEOMODE_OPTION_COUNT; i++) {
		sprintf(cfg_key, "enabled_auto_mode_%d", i);
		configfile.setInt32(cfg_key, g_settings.enabled_auto_modes[i]);
	}
	configfile.setInt32( "cpufreq", g_settings.cpufreq);
	configfile.setInt32( "standby_cpufreq", g_settings.standby_cpufreq);
	configfile.setInt32("rounded_corners", g_settings.rounded_corners);
	configfile.setInt32("ci_standby_reset", g_settings.ci_standby_reset);
	configfile.setInt32("ci_clock", g_settings.ci_clock);
	configfile.setInt32("ci_ignore_messages", g_settings.ci_ignore_messages);
	configfile.setInt32("ci_save_pincode", g_settings.ci_save_pincode);
	configfile.setInt32("ci_check_live", g_settings.ci_check_live);
	configfile.setString("ci_pincode", g_settings.ci_pincode);
	configfile.setInt32("ci_tuner", g_settings.ci_tuner);

	configfile.setInt32( "make_hd_list", g_settings.make_hd_list);
	configfile.setInt32( "make_webtv_list", g_settings.make_webtv_list);
	configfile.setInt32( "make_new_list", g_settings.make_new_list);
	configfile.setInt32( "make_removed_list", g_settings.make_removed_list);
	configfile.setInt32( "keep_channel_numbers", g_settings.keep_channel_numbers);
	configfile.setInt32( "show_empty_favorites", g_settings.show_empty_favorites);
	//led
	configfile.setInt32( "led_tv_mode", g_settings.led_tv_mode);
	configfile.setInt32( "led_standby_mode", g_settings.led_standby_mode);
	configfile.setInt32( "led_deep_mode", g_settings.led_deep_mode);
	configfile.setInt32( "led_rec_mode", g_settings.led_rec_mode);
	configfile.setInt32( "led_blink", g_settings.led_blink);
	configfile.setInt32( "backlight_tv", g_settings.backlight_tv);
	configfile.setInt32( "backlight_standby", g_settings.backlight_standby);
	configfile.setInt32( "backlight_deepstandby", g_settings.backlight_deepstandby);
	configfile.setInt32( "lcd_scroll", g_settings.lcd_scroll);
	configfile.setInt32( "lcd_notify_rclock", g_settings.lcd_notify_rclock);

	//misc
	configfile.setInt32( "power_standby", g_settings.power_standby);
	configfile.setInt32( "zap_cycle", g_settings.zap_cycle );
	configfile.setInt32( "hdd_fs", g_settings.hdd_fs);
	configfile.setInt32( "hdd_sleep", g_settings.hdd_sleep);
	configfile.setInt32( "hdd_noise", g_settings.hdd_noise);
	configfile.setInt32( "hdd_statfs_mode", g_settings.hdd_statfs_mode);
	configfile.setBool("shutdown_real"        , g_settings.shutdown_real        );
	configfile.setBool("shutdown_real_rcdelay", g_settings.shutdown_real_rcdelay);
	configfile.setInt32("shutdown_count"           , g_settings.shutdown_count);
	configfile.setInt32("shutdown_min"  , g_settings.shutdown_min  );
	configfile.setInt32("sleeptimer_min", g_settings.sleeptimer_min);
	configfile.setBool("infobar_sat_display"  , g_settings.infobar_sat_display  );
	configfile.setBool("infobar_show_channeldesc"  , g_settings.infobar_show_channeldesc  );
	configfile.setInt32("infobar_subchan_disp_pos"  , g_settings.infobar_subchan_disp_pos  );
	configfile.setBool("infobar_buttons_usertitle", g_settings.infobar_buttons_usertitle);
	configfile.setBool("progressbar_gradient", g_settings.progressbar_gradient);
	configfile.setInt32("progressbar_design", g_settings.progressbar_design);
	configfile.setInt32("progressbar_timescale_red", g_settings.progressbar_timescale_red);
	configfile.setInt32("progressbar_timescale_green", g_settings.progressbar_timescale_green);
	configfile.setInt32("progressbar_timescale_yellow", g_settings.progressbar_timescale_yellow);
	configfile.setInt32("progressbar_timescale_invert", g_settings.progressbar_timescale_invert);
	configfile.setInt32("infobar_anaclock", g_settings.infobar_anaclock);
	configfile.setInt32("infobar_show", g_settings.infobar_show);
	configfile.setInt32("infobar_show_channellogo"  , g_settings.infobar_show_channellogo  );
	configfile.setInt32("infobar_progressbar"  , g_settings.infobar_progressbar  );
	configfile.setInt32("infobar_casystem_display"  , g_settings.infobar_casystem_display  );
	configfile.setInt32("infobar_casystem_dotmatrix"  , g_settings.infobar_casystem_dotmatrix  );
	configfile.setInt32("infobar_casystem_frame"  , g_settings.infobar_casystem_frame  );
	configfile.setBool("scrambled_message"  , g_settings.scrambled_message  );
	configfile.setInt32("volume_pos"  , g_settings.volume_pos  );
	configfile.setBool("volume_digits", g_settings.volume_digits);
	configfile.setInt32("volume_size"  , g_settings.volume_size);
	configfile.setInt32("menu_pos" , g_settings.menu_pos);
	configfile.setBool("show_menu_hints" , g_settings.show_menu_hints);
	configfile.setInt32("infobar_show_sysfs_hdd"  , g_settings.infobar_show_sysfs_hdd  );
	configfile.setInt32("show_mute_icon"   , g_settings.show_mute_icon);
	configfile.setInt32("infobar_show_res"  , g_settings.infobar_show_res  );
	configfile.setInt32("infobar_show_dd_available"  , g_settings.infobar_show_dd_available  );
	configfile.setInt32("infobar_show_tuner"  , g_settings.infobar_show_tuner  );
	configfile.setBool("radiotext_enable"          , g_settings.radiotext_enable);
	configfile.setString("radiotext_rass_dir", g_settings.radiotext_rass_dir);
	//audio
	configfile.setInt32( "audio_AnalogMode", g_settings.audio_AnalogMode );
	configfile.setBool("audio_DolbyDigital"   , g_settings.audio_DolbyDigital   );
	configfile.setInt32( "auto_lang", g_settings.auto_lang );
	configfile.setInt32( "auto_subs", g_settings.auto_subs );
	for(int i = 0; i < 3; i++) {
		sprintf(cfg_key, "pref_lang_%d", i);
		configfile.setString(cfg_key, g_settings.pref_lang[i]);
		sprintf(cfg_key, "pref_subs_%d", i);
		configfile.setString(cfg_key, g_settings.pref_subs[i]);
	}
	configfile.setString("subs_charset", g_settings.subs_charset);

	//screen saver
	configfile.setInt32("screensaver_delay", g_settings.screensaver_delay);
	configfile.setString("screensaver_dir", g_settings.screensaver_dir);
	configfile.setInt32("screensaver_timeout", g_settings.screensaver_timeout);
	configfile.setInt32("screensaver_random", g_settings.screensaver_random);
	configfile.setInt32("screensaver_mode", g_settings.screensaver_mode);

	//vcr
	configfile.setBool("vcr_AutoSwitch"       , g_settings.vcr_AutoSwitch       );

	//language
	configfile.setString("language", g_settings.language);
	configfile.setString("timezone", g_settings.timezone);
	// epg
	configfile.setBool("epg_save", g_settings.epg_save);
	configfile.setBool("epg_save_standby", g_settings.epg_save_standby);
	configfile.setInt32("epg_save_frequently", g_settings.epg_save_frequently);
	configfile.setBool("epg_read", g_settings.epg_read);
	configfile.setInt32("epg_scan", g_settings.epg_scan);
	configfile.setInt32("epg_scan_mode", g_settings.epg_scan_mode);
	configfile.setInt32("epg_save_mode", g_settings.epg_save_mode);
	configfile.setInt32("epg_cache_time"           ,g_settings.epg_cache );
	configfile.setInt32("epg_extendedcache_time"   ,g_settings.epg_extendedcache);
	configfile.setInt32("epg_old_events"           ,g_settings.epg_old_events );
	configfile.setInt32("epg_max_events"           ,g_settings.epg_max_events );
	configfile.setString("epg_dir"                  ,g_settings.epg_dir);

	// NTP-Server for sectionsd
	configfile.setString( "network_ntpserver", g_settings.network_ntpserver);
	configfile.setString( "network_ntprefresh", g_settings.network_ntprefresh);
	configfile.setBool( "network_ntpenable", g_settings.network_ntpenable);

	configfile.setString("ifname", g_settings.ifname);

	//widget settings
	configfile.setBool("widget_fade"          , g_settings.widget_fade          );

	//theme/color options
	CThemes::setTheme(configfile);
	configfile.setBool("osd_colorsettings_advanced_mode", g_settings.osd_colorsettings_advanced_mode);

#ifdef ENABLE_GRAPHLCD
	configfile.setInt32("glcd_enable", g_settings.glcd_enable);
	configfile.setInt32("glcd_color_fg", g_settings.glcd_color_fg);
	configfile.setInt32("glcd_color_bg", g_settings.glcd_color_bg);
	configfile.setInt32("glcd_color_bar", g_settings.glcd_color_bar);
	configfile.setInt32("glcd_percent_channel", g_settings.glcd_percent_channel);
	configfile.setInt32("glcd_percent_epg", g_settings.glcd_percent_epg);
	configfile.setInt32("glcd_percent_bar", g_settings.glcd_percent_bar);
	configfile.setInt32("glcd_percent_time", g_settings.glcd_percent_time);
	configfile.setInt32("glcd_percent_time_standby", g_settings.glcd_percent_time_standby);
	configfile.setInt32("glcd_percent_logo", g_settings.glcd_percent_logo);
	configfile.setInt32("glcd_mirror_osd", g_settings.glcd_mirror_osd);
	configfile.setInt32("glcd_mirror_video", g_settings.glcd_mirror_video);
	configfile.setInt32("glcd_time_in_standby", g_settings.glcd_time_in_standby);
	configfile.setInt32("glcd_show_logo", g_settings.glcd_show_logo);
	configfile.setString("glcd_font", g_settings.glcd_font);
	configfile.setInt32("glcd_brightness", g_settings.glcd_brightness);
	configfile.setInt32("glcd_brightness_standby", g_settings.glcd_brightness_standby);
	configfile.setInt32("glcd_scroll_speed", g_settings.glcd_scroll_speed);
#endif

	//personalize
	configfile.setString("personalize_pincode", g_settings.personalize_pincode);
	for (int i = 0; i < SNeutrinoSettings::P_SETTINGS_MAX; i++) //settings.h, settings.cpp
		configfile.setInt32(personalize_settings[i].personalize_settings_name, g_settings.personalize[i]);

	configfile.setInt32( "contrast_fonts", g_settings.contrast_fonts );
	//network
	for(int i=0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) {
		sprintf(cfg_key, "network_nfs_ip_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].ip);
		sprintf(cfg_key, "network_nfs_dir_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].dir);
		sprintf(cfg_key, "network_nfs_local_dir_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].local_dir);
		sprintf(cfg_key, "network_nfs_automount_%d", i);
		configfile.setInt32(cfg_key, g_settings.network_nfs[i].automount);
		sprintf(cfg_key, "network_nfs_type_%d", i);
		configfile.setInt32(cfg_key, g_settings.network_nfs[i].type);
		sprintf(cfg_key, "network_nfs_username_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].username);
		sprintf(cfg_key, "network_nfs_password_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].password);
		sprintf(cfg_key, "network_nfs_mount_options1_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].mount_options1);
		sprintf(cfg_key, "network_nfs_mount_options2_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].mount_options2);
		sprintf(cfg_key, "network_nfs_mac_%d", i);
		configfile.setString(cfg_key, g_settings.network_nfs[i].mac);
	}
	configfile.setString( "network_nfs_audioplayerdir", g_settings.network_nfs_audioplayerdir);
	configfile.setString( "network_nfs_picturedir", g_settings.network_nfs_picturedir);
	configfile.setString( "network_nfs_moviedir", g_settings.network_nfs_moviedir);
	configfile.setString( "network_nfs_recordingdir", g_settings.network_nfs_recordingdir);
	configfile.setString( "timeshiftdir", g_settings.timeshiftdir);
	configfile.setString( "downloadcache_dir", g_settings.downloadcache_dir);
	configfile.setString( "last_webtv_dir", g_settings.last_webtv_dir);
	configfile.setBool  ("filesystem_is_utf8"                 , g_settings.filesystem_is_utf8             );

	//recording (server + vcr)
	configfile.setInt32 ("recording_type",                      g_settings.recording_type);
	configfile.setBool  ("recording_stopsectionsd"            , g_settings.recording_stopsectionsd        );

	configfile.setInt32 ("recording_audio_pids_default"       , g_settings.recording_audio_pids_default   );
	configfile.setBool  ("recording_zap_on_announce"          , g_settings.recording_zap_on_announce      );
	configfile.setBool  ("shutdown_timer_record_type"          , g_settings.shutdown_timer_record_type    );

	configfile.setBool  ("recordingmenu.stream_vtxt_pid"      , g_settings.recording_stream_vtxt_pid      );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32 ("recording_bufsize"                  , g_settings.recording_bufsize);
	configfile.setInt32 ("recording_bufsize_dmx"              , g_settings.recording_bufsize_dmx);
#endif
	configfile.setBool  ("recordingmenu.stream_subtitle_pids" , g_settings.recording_stream_subtitle_pids );
	configfile.setBool  ("recordingmenu.stream_pmt_pid"       , g_settings.recording_stream_pmt_pid       );
	configfile.setString("recordingmenu.filename_template"    , g_settings.recording_filename_template    );
	configfile.setInt32 ("recording_choose_direct_rec_dir"    , g_settings.recording_choose_direct_rec_dir);
	configfile.setBool  ("recording_epg_for_filename"         , g_settings.recording_epg_for_filename     );
	configfile.setBool  ("recording_epg_for_end"              , g_settings.recording_epg_for_end          );
	configfile.setBool  ("recording_save_in_channeldir"       , g_settings.recording_save_in_channeldir   );
	configfile.setBool  ("recording_slow_warning"             , g_settings.recording_slow_warning         );
	configfile.setBool  ("recording_startstop_msg"             , g_settings.recording_startstop_msg       );
	configfile.setBool  ("recording_already_found_check"      , g_settings.recording_already_found_check  );

	// default plugin for movieplayer
	configfile.setString ( "movieplayer_plugin", g_settings.movieplayer_plugin );
	configfile.setString ( "plugin_hdd_dir", g_settings.plugin_hdd_dir );

	configfile.setString ( "plugins_disabled", g_settings.plugins_disabled );
	configfile.setString ( "plugins_game", g_settings.plugins_game );
	configfile.setString ( "plugins_tool", g_settings.plugins_tool );
	configfile.setString ( "plugins_script", g_settings.plugins_script );
	configfile.setString ( "plugins_lua", g_settings.plugins_lua );

	configfile.setString ( "logo_hdd_dir", g_settings.logo_hdd_dir );

	int webtv_count = 0;
	for (std::list<std::string>::iterator it = g_settings.webtv_xml.begin(); it != g_settings.webtv_xml.end(); ++it) {
		std::string k = "webtv_xml_" + to_string(webtv_count);
		configfile.setString(k, *it);
		webtv_count++;
	}
	configfile.setInt32 ( "webtv_xml_count", g_settings.webtv_xml.size());

	saveKeys();

	configfile.setInt32 ("key_playbutton", g_settings.key_playbutton );
	configfile.setInt32( "timeshift_pause", g_settings.timeshift_pause );
	configfile.setInt32( "temp_timeshift", g_settings.temp_timeshift );
	configfile.setInt32( "auto_timeshift", g_settings.auto_timeshift );
	configfile.setInt32( "auto_delete", g_settings.auto_delete );
	configfile.setInt32( "record_hours", g_settings.record_hours );
	configfile.setInt32( "timeshift_hours", g_settings.timeshift_hours );
	//printf("set: key_unlock =============== %d\n", g_settings.key_unlock);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32( "screenshot_png_compression", g_settings.screenshot_png_compression );
	configfile.setInt32( "screenshot_backbuffer", g_settings.screenshot_backbuffer);
#endif
	configfile.setInt32( "screenshot_count", g_settings.screenshot_count );
	configfile.setInt32( "screenshot_format", g_settings.screenshot_format );
	configfile.setInt32( "screenshot_cover", g_settings.screenshot_cover );
	configfile.setInt32( "screenshot_mode", g_settings.screenshot_mode );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	configfile.setInt32( "screenshot_res", g_settings.screenshot_res );
#else
	configfile.setInt32( "screenshot_video", g_settings.screenshot_video );
	configfile.setInt32( "screenshot_scale", g_settings.screenshot_scale );
#endif
	configfile.setInt32( "auto_cover", g_settings.auto_cover );

	configfile.setString( "screenshot_dir", g_settings.screenshot_dir);
	configfile.setInt32( "cacheTXT", g_settings.cacheTXT );
	configfile.setInt32( "minimode", g_settings.minimode );
	configfile.setInt32( "mode_clock", g_settings.mode_clock );
	configfile.setInt32( "zapto_pre_time", g_settings.zapto_pre_time );
	configfile.setBool("spectrum", g_settings.spectrum);
	configfile.setInt32("eventlist_additional", g_settings.eventlist_additional);
	configfile.setInt32("channellist_additional", g_settings.channellist_additional);
	configfile.setBool("channellist_epgtext_align_right", g_settings.channellist_epgtext_align_right);
	configfile.setInt32("channellist_progressbar_design", g_settings.channellist_progressbar_design);
	configfile.setInt32("channellist_foot", g_settings.channellist_foot);
	configfile.setInt32("channellist_new_zap_mode", g_settings.channellist_new_zap_mode);
	configfile.setInt32("channellist_hdicon", g_settings.channellist_hdicon);
	configfile.setInt32("remote_control_hardware", g_settings.remote_control_hardware);
	configfile.setBool  ( "audiochannel_up_down_enable", g_settings.audiochannel_up_down_enable );
	configfile.setInt32("channellist_sort_mode", g_settings.channellist_sort_mode);
	configfile.setInt32("channellist_numeric_adjust", g_settings.channellist_numeric_adjust);
	configfile.setInt32("channellist_show_channellogo", g_settings.channellist_show_channellogo);
	configfile.setInt32("channellist_show_infobox", g_settings.channellist_show_infobox);
	configfile.setInt32("channellist_show_numbers", g_settings.channellist_show_numbers);

	//screen configuration
	configfile.setInt32( "screen_xres", g_settings.screen_xres);
	configfile.setInt32( "screen_yres", g_settings.screen_yres);
	configfile.setInt32( "screen_StartX_lcd", g_settings.screen_StartX_lcd );
	configfile.setInt32( "screen_StartY_lcd", g_settings.screen_StartY_lcd );
	configfile.setInt32( "screen_EndX_lcd", g_settings.screen_EndX_lcd );
	configfile.setInt32( "screen_EndY_lcd", g_settings.screen_EndY_lcd );
	configfile.setInt32( "screen_StartX_crt", g_settings.screen_StartX_crt );
	configfile.setInt32( "screen_StartY_crt", g_settings.screen_StartY_crt );
	configfile.setInt32( "screen_EndX_crt", g_settings.screen_EndX_crt );
	configfile.setInt32( "screen_EndY_crt", g_settings.screen_EndY_crt );
	configfile.setInt32( "screen_preset", g_settings.screen_preset );
	configfile.setInt32( "screen_width", g_settings.screen_width);
	configfile.setInt32( "screen_height", g_settings.screen_height);

	//Software-update
	configfile.setInt32 ("softupdate_mode"          , g_settings.softupdate_mode          );
	configfile.setBool("apply_kernel", g_settings.apply_kernel);
	configfile.setBool("apply_settings", g_settings.apply_settings);
	configfile.setString("image_settings_backup_path", g_settings.image_settings_backup_path);
	configfile.setString("softupdate_url_file"      , g_settings.softupdate_url_file      );
	configfile.setInt32 ("softupdate_name_mode_apply", g_settings.softupdate_name_mode_apply);
	configfile.setInt32 ("softupdate_name_mode_backup", g_settings.softupdate_name_mode_backup);
	configfile.setBool("softupdate_autocheck", g_settings.softupdate_autocheck);

	configfile.setInt32("flashupdate_createimage_add_var",    g_settings.flashupdate_createimage_add_var);
	configfile.setInt32("flashupdate_createimage_add_root1",  g_settings.flashupdate_createimage_add_root1);
	configfile.setInt32("flashupdate_createimage_add_uldr",   g_settings.flashupdate_createimage_add_uldr);
	configfile.setInt32("flashupdate_createimage_add_u_boot", g_settings.flashupdate_createimage_add_u_boot);
	configfile.setInt32("flashupdate_createimage_add_env",    g_settings.flashupdate_createimage_add_env);
	configfile.setInt32("flashupdate_createimage_add_spare",  g_settings.flashupdate_createimage_add_spare);
	configfile.setInt32("flashupdate_createimage_add_kernel", g_settings.flashupdate_createimage_add_kernel);

	configfile.setString("softupdate_proxyserver"   , g_settings.softupdate_proxyserver   );
	configfile.setString("softupdate_proxyusername" , g_settings.softupdate_proxyusername );
	configfile.setString("softupdate_proxypassword" , g_settings.softupdate_proxypassword );

	configfile.setString("update_dir", g_settings.update_dir);
	configfile.setString("update_dir_opkg", g_settings.update_dir_opkg);

	configfile.setString("font_file", g_settings.font_file);
	configfile.setString("ttx_font_file", g_settings.ttx_font_file);
	configfile.setString("sub_font_file", g_settings.sub_font_file);

	//parentallock
	configfile.setInt32( "parentallock_prompt", g_settings.parentallock_prompt );
	configfile.setInt32( "parentallock_lockage", g_settings.parentallock_lockage );
	configfile.setString( "parentallock_pincode", g_settings.parentallock_pincode );
	configfile.setInt32("parentallock_zaptime", g_settings.parentallock_zaptime);
	configfile.setInt32("parentallock_defaultlocked", g_settings.parentallock_defaultlocked);

	//timing
	for (int i = 0; i < SNeutrinoSettings::TIMING_SETTING_COUNT; i++)
		configfile.setInt32(locale_real_names[timing_setting[i].name], g_settings.timing[i]);

	for (int i = 0; i < SNeutrinoSettings::LCD_SETTING_COUNT; i++)
		configfile.setInt32(lcd_setting[i].name, g_settings.lcd_setting[i]);
	configfile.setString("lcd_dim_time", g_settings.lcd_setting_dim_time);
	configfile.setInt32("lcd_dim_brightness", g_settings.lcd_setting_dim_brightness);
	configfile.setInt32("lcd_info_line", g_settings.lcd_info_line);//channel name or clock
#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
	configfile.setInt32("lcd_vfd_scroll", g_settings.lcd_vfd_scroll);
#endif

	//Movie-Player
	configfile.setInt32( "movieplayer_repeat_on", g_settings.movieplayer_repeat_on );
	configfile.setString( "youtube_dev_id", g_settings.youtube_dev_id );
	configfile.setString( "tmdb_api_key", g_settings.tmdb_api_key );

	//Filebrowser
	configfile.setInt32("filebrowser_showrights", g_settings.filebrowser_showrights);
	configfile.setInt32("filebrowser_sortmethod", g_settings.filebrowser_sortmethod);
	configfile.setBool("filebrowser_denydirectoryleave", g_settings.filebrowser_denydirectoryleave);
	configfile.setBool("filebrowser_usefilter", g_settings.filebrowser_use_filter);

	//zapit setup
	configfile.setString( "startchanneltv", g_settings.StartChannelTV );
	configfile.setString( "startchannelradio", g_settings.StartChannelRadio );
	configfile.setInt64("startchanneltv_id", g_settings.startchanneltv_id);
	configfile.setInt64("startchannelradio_id", g_settings.startchannelradio_id);
	configfile.setInt32("uselastchannel", g_settings.uselastchannel);
	configfile.setInt32("adzap_zapBackPeriod", g_settings.adzap_zapBackPeriod);
	configfile.setInt32("adzap_writeData", g_settings.adzap_writeData);
	//epg search
	g_settings.epg_search_history_size = g_settings.epg_search_history.size();
	if (g_settings.epg_search_history_size > g_settings.epg_search_history_max)
		g_settings.epg_search_history_size = g_settings.epg_search_history_max;
	configfile.setInt32("epg_search_history_max", g_settings.epg_search_history_max);
	configfile.setInt32("epg_search_history_size", g_settings.epg_search_history_size);
	std::list<std::string>:: iterator it = g_settings.epg_search_history.begin();
	for(int i = 0; i < g_settings.epg_search_history_size; i++, ++it)
		configfile.setString("epg_search_history_" + to_string(i), *it);

	// USERMENU
	//---------------------------------------
	for (unsigned int i = 0, count = 4; i < g_settings.usermenu.size(); i++) {
		if (g_settings.usermenu[i]->key != CRCInput::RC_nokey) {
			std::string name;
			if (i < 4)
				name = usermenu_default[i].name;
			else
				name = to_string(count++);
			std::string usermenu_key("usermenu_key_");
			usermenu_key += name;
			configfile.setInt32(usermenu_key, g_settings.usermenu[i]->key);
			std::string txt1("usermenu_tv_");
			txt1 += name;
			configfile.setString(txt1, g_settings.usermenu[i]->items);
			txt1 += "_text";
			configfile.setString(txt1, g_settings.usermenu[i]->title);
		}
	}

#if 0
	configfile.setInt32("bigFonts", g_settings.bigFonts);
#endif
	configfile.setInt32("window_size", g_settings.window_size);
	configfile.setInt32("window_width", g_settings.window_width);
	configfile.setInt32("window_height", g_settings.window_height);
#ifdef BOXMODEL_APOLLO
	configfile.setInt32("brightness", g_settings.brightness );
	configfile.setInt32("contrast", g_settings.contrast );
	configfile.setInt32("enable_sd_osd", g_settings.enable_sd_osd );
#endif
#ifdef ENABLE_PIP
	configfile.setInt32("pip_x", g_settings.pip_x);
	configfile.setInt32("pip_y", g_settings.pip_y);
	configfile.setInt32("pip_width", g_settings.pip_width);
	configfile.setInt32("pip_height", g_settings.pip_height);

	configfile.setInt32("pip_radio_x", g_settings.pip_radio_x);
	configfile.setInt32("pip_radio_y", g_settings.pip_radio_y);
	configfile.setInt32("pip_radio_width", g_settings.pip_radio_width);
	configfile.setInt32("pip_radio_height", g_settings.pip_radio_height);
#endif
	configfile.setInt32("infoClockFontSize", g_settings.infoClockFontSize);
	configfile.setInt32("infoClockBackground", g_settings.infoClockBackground);
	configfile.setInt32("infoClockSeconds", g_settings.infoClockSeconds);

	configfile.setInt32("livestreamResolution", g_settings.livestreamResolution);
	configfile.setString("livestreamScriptPath", g_settings.livestreamScriptPath);

	configfile.setString("version_pseudo", g_settings.version_pseudo);

	if(strcmp(fname, NEUTRINO_SETTINGS_FILE) || configfile.getModifiedFlag())
		configfile.saveConfig(fname);
}

/**************************************************************************************
*          CNeutrinoApp -  channelsInit, get the Channellist from daemon              *
**************************************************************************************/
extern CBouquetManager *g_bouquetManager;

void CNeutrinoApp::channelsInit(bool bOnly)
{
	CBouquet* tmp;

	printf("[neutrino] Creating channels lists...\n");
	TIMER_START();

	memset(tvsort, -1, sizeof(tvsort));
	memset(radiosort, -1, sizeof(tvsort));

	const char * fav_bouquetname = g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME);
	if(g_bouquetManager && g_bouquetManager->existsUBouquet(fav_bouquetname, true) == -1)
		g_bouquetManager->addBouquet(fav_bouquetname, true, true);

	if(TVbouquetList) delete TVbouquetList;
	if(RADIObouquetList) delete RADIObouquetList;

	if(TVfavList) delete TVfavList;
	if(RADIOfavList) delete RADIOfavList;

	if(TVchannelList) delete TVchannelList;
	if(RADIOchannelList) delete RADIOchannelList;

	if(TVwebList) delete TVwebList;
	if(RADIOwebList) delete RADIOwebList;

	TVchannelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD), false, true);
	RADIOchannelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD), false, true);

	TVbouquetList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_PROVS));
	TVfavList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_FAVS));
	TVwebList = new CBouquetList(g_Locale->getText(LOCALE_BOUQUETNAME_WEBTV));
	RADIOwebList = new CBouquetList(g_Locale->getText(LOCALE_BOUQUETNAME_WEBTV));

	RADIObouquetList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_PROVS));
	RADIOfavList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_FAVS));

	uint32_t i;
	i = 1;

	int tvi = 0, ri = 0;

	ZapitChannelList zapitList, webtvList;

	/* all TV channels */
	CServiceManager::getInstance()->GetAllTvChannels(zapitList);
	tvi = zapitList.size();
	TVchannelList->SetChannelList(&zapitList);

	/* all RADIO channels */
	CServiceManager::getInstance()->GetAllRadioChannels(zapitList);
	ri = zapitList.size();

	RADIOchannelList->SetChannelList(&zapitList);

	printf("[neutrino] got %d TV and %d RADIO channels\n", tvi, ri); fflush(stdout);
	TIMER_STOP("[neutrino] all channels took");

	/* unless we will do real channel delete from allchans, needed once ? */
	if(!bOnly) {
		if(TVallList) delete TVallList;
		if(RADIOallList) delete RADIOallList;

		TVallList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
		tmp = TVallList->addBouquet(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
		tmp->channelList->SetChannelList(&TVchannelList->getChannels());

		RADIOallList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
		tmp = RADIOallList->addBouquet(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
		tmp->channelList->SetChannelList(&RADIOchannelList->getChannels());

		if(TVsatList) delete TVsatList;
		TVsatList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_SATS));
		if(RADIOsatList) delete RADIOsatList;
		RADIOsatList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_SATS));

		/* all TV / RADIO channels per satellite */
		sat_iterator_t sit;
		satellite_map_t satlist = CServiceManager::getInstance()->SatelliteList();
		for(sit = satlist.begin(); sit != satlist.end(); sit++) {
			if (!CServiceManager::getInstance()->GetAllSatelliteChannels(zapitList, sit->first))
				continue;

			tvi = 0, ri = 0;
			CBouquet* tmp1 = TVsatList->addBouquet(sit->second.name.c_str());
			CBouquet* tmp2 = RADIOsatList->addBouquet(sit->second.name.c_str());
			tmp1->satellitePosition = tmp2->satellitePosition = sit->first;

			for(zapit_list_it_t it = zapitList.begin(); it != zapitList.end(); it++) {
				if ((*it)->getServiceType() == ST_DIGITAL_TELEVISION_SERVICE) {
					tmp1->channelList->addChannel(*it);
					tvi++;
				}
				else if ((*it)->getServiceType() == ST_DIGITAL_RADIO_SOUND_SERVICE) {
					tmp2->channelList->addChannel(*it);
					ri++;
				}
			}
			printf("[neutrino] created %s (%d) bouquet with %d TV and %d RADIO channels\n", sit->second.name.c_str(), sit->first, tvi, ri);
			if(!tvi)
				TVsatList->deleteBouquet(tmp1);
			if(!ri)
				RADIOsatList->deleteBouquet(tmp2);

			TIMER_STOP("[neutrino] sat took");
		}
		/* all WebTV channels */
		if (g_settings.make_webtv_list) {
			if (CServiceManager::getInstance()->GetAllWebTVChannels(webtvList)) {
				/* all channels */
				CBouquet* webtvBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_WEBTV), false, true);
				webtvBouquet->channelList->SetChannelList(&webtvList);
				TVallList->Bouquets.push_back(webtvBouquet);
				/* "satellite" */
				webtvBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_WEBTV), false, true);
				webtvBouquet->channelList->SetChannelList(&webtvList);
				TVsatList->Bouquets.push_back(webtvBouquet);
				printf("[neutrino] got %d WebTV channels\n", (int)webtvList.size()); fflush(stdout);
			}
		}
		/* all HD channels */
		if (g_settings.make_hd_list) {
			if (CServiceManager::getInstance()->GetAllHDChannels(zapitList)) {
				CBouquet* hdBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_HDTV), false, true);
				hdBouquet->channelList->SetChannelList(&zapitList);
				TVallList->Bouquets.push_back(hdBouquet);
				printf("[neutrino] got %d HD channels\n", (int)zapitList.size()); fflush(stdout);
			}
		}
		/* new channels */
		if (g_settings.make_new_list) {
			if (CServiceManager::getInstance()->GetAllTvChannels(zapitList, CZapitChannel::NEW)) {
				CBouquet* newBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_NEW), false, true);
				newBouquet->channelList->SetChannelList(&zapitList);
				TVallList->Bouquets.push_back(newBouquet);
				printf("[neutrino] got %d new TV channels\n", (int)zapitList.size()); fflush(stdout);
			}
			if (CServiceManager::getInstance()->GetAllRadioChannels(zapitList, CZapitChannel::NEW)) {
				CBouquet* newBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_NEW), false, true);
				newBouquet->channelList->SetChannelList(&zapitList);
				RADIOallList->Bouquets.push_back(newBouquet);
				printf("[neutrino] got %d new RADIO channels\n", (int)zapitList.size()); fflush(stdout);
			}
		}
		/* removed channels */
		if (g_settings.make_removed_list) {
			if (CServiceManager::getInstance()->GetAllTvChannels(zapitList, CZapitChannel::REMOVED)) {
				CBouquet* newBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_REMOVED), false, true);
				newBouquet->channelList->SetChannelList(&zapitList);
				TVallList->Bouquets.push_back(newBouquet);
				printf("[neutrino] got %d removed TV channels\n", (int)zapitList.size()); fflush(stdout);
			}
			if (CServiceManager::getInstance()->GetAllRadioChannels(zapitList, CZapitChannel::REMOVED)) {
				CBouquet* newBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_REMOVED), false, true);
				newBouquet->channelList->SetChannelList(&zapitList);
				RADIOallList->Bouquets.push_back(newBouquet);
				printf("[neutrino] got %d removed RADIO channels\n", (int)zapitList.size()); fflush(stdout);
			}
		}
		TIMER_STOP("[neutrino] sats took");
	}

	delete AllFavBouquetList;
	AllFavBouquetList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_FAVS));
	/* Favorites and providers bouquets */
	tvi = ri = 0;
	if(g_bouquetManager){
		for (i = 0; i < g_bouquetManager->Bouquets.size(); i++) {
			CZapitBouquet *b = g_bouquetManager->Bouquets[i];
			if (!b->bHidden) {
				if (b->getTvChannels(zapitList) || (g_settings.show_empty_favorites && b->bUser)) {
					if(b->bUser)
						tmp = TVfavList->addBouquet(b);
					else if(b->bWebtv)
						tmp = TVwebList->addBouquet(b);
					else
						tmp = TVbouquetList->addBouquet(b);

					tmp->channelList->SetChannelList(&zapitList);
					tvi++;
				}
				if (b->getRadioChannels(zapitList) || (g_settings.show_empty_favorites && b->bUser)) {
					if(b->bUser)
						tmp = RADIOfavList->addBouquet(b);
					else if(b->bWebtv)
						tmp = RADIOwebList->addBouquet(b);
					else
						tmp = RADIObouquetList->addBouquet(b);

					tmp->channelList->SetChannelList(&zapitList);
					ri++;
				}
				if(b->bUser)
					AllFavBouquetList->addBouquet(b);
			}
		}
	}
#if 0
	if (!webtvList.empty()) {
		/* provider */
		CBouquet* webtvBouquet = new CBouquet(0, g_Locale->getText(LOCALE_BOUQUETNAME_WEBTV), false, true);
		webtvBouquet->channelList->SetChannelList(&webtvList);
		TVbouquetList->Bouquets.push_back(webtvBouquet);
	}
#endif
	printf("[neutrino] got %d TV and %d RADIO bouquets\n", tvi, ri); fflush(stdout);
	TIMER_STOP("[neutrino] took");

	SetChannelMode(lastChannelMode);
	CEpgScan::getInstance()->ConfigureEIT();

	dprintf(DEBUG_DEBUG, "\nAll bouquets-channels received\n");
}

void CNeutrinoApp::SetChannelMode(int newmode)
{
	printf("CNeutrinoApp::SetChannelMode %d [%s]\n", newmode, mode == mode_radio ? "radio" : "tv");
	int *sortmode;

	if(mode == mode_radio) {
		channelList = RADIOchannelList;
		g_settings.channel_mode_radio = newmode;
		sortmode = radiosort;
	} else {
		channelList = TVchannelList;
		g_settings.channel_mode = newmode;
		sortmode = tvsort;
	}

	switch(newmode) {
		case LIST_MODE_FAV:
			if(mode == mode_radio)
				bouquetList = RADIOfavList;
			else
				bouquetList = TVfavList;
			break;
		case LIST_MODE_SAT:
			if(mode == mode_radio)
				bouquetList = RADIOsatList;
			else
				bouquetList = TVsatList;
			break;
		case LIST_MODE_WEBTV:
			if(mode == mode_radio)
				bouquetList = RADIOwebList;
			else
				bouquetList = TVwebList;
			break;
		case LIST_MODE_ALL:
			if(mode == mode_radio)
				bouquetList = RADIOallList;
			else
				bouquetList = TVallList;
			break;
		default:
			newmode = LIST_MODE_PROV;
		case LIST_MODE_PROV:
			if(mode == mode_radio)
				bouquetList = RADIObouquetList;
			else
				bouquetList = TVbouquetList;
			break;
	}
	INFO("newmode %d sort old %d new %d", newmode, sortmode[newmode], g_settings.channellist_sort_mode);
	if(newmode != LIST_MODE_FAV && sortmode[newmode] != g_settings.channellist_sort_mode && g_settings.channellist_sort_mode < CChannelList::SORT_MAX) {
		sortmode[newmode] = g_settings.channellist_sort_mode;
		INFO("sorting, mode %d, %d bouquets\n", g_settings.channellist_sort_mode, (int)bouquetList->Bouquets.size());
		for (uint32_t i = 0; i < bouquetList->Bouquets.size(); i++) {
			if(g_settings.channellist_sort_mode == CChannelList::SORT_ALPHA)
				bouquetList->Bouquets[i]->channelList->SortAlpha();
			if(g_settings.channellist_sort_mode == CChannelList::SORT_TP)
				bouquetList->Bouquets[i]->channelList->SortTP();
			if(g_settings.channellist_sort_mode == CChannelList::SORT_SAT)
				bouquetList->Bouquets[i]->channelList->SortSat();
			if(g_settings.channellist_sort_mode == CChannelList::SORT_CH_NUMBER)
				bouquetList->Bouquets[i]->channelList->SortChNumber();
		}
		adjustToChannelID(CZapit::getInstance()->GetCurrentChannelID());
	}
	lastChannelMode = newmode;
}

/**************************************************************************************
*          CNeutrinoApp -  run, the main runloop                                      *
**************************************************************************************/
extern int cnxt_debug;
extern bool sections_debug;
extern int zapit_debug;

void CNeutrinoApp::CmdParser(int argc, char **argv)
{
	global_argv = new char *[argc+1];
	for (int i = 0; i < argc; i++)
		global_argv[i] = argv[i];
	global_argv[argc] = NULL;

	sections_debug = false;
	softupdate = false;
	//fromflash = false;

	for(int x=1; x<argc; x++) {
		if ((!strcmp(argv[x], "-u")) || (!strcmp(argv[x], "--enable-update"))) {
			dprintf(DEBUG_NORMAL, "Software update enabled\n");
			softupdate = true;
			allow_flash = 1;
		}
		/*else if ((!strcmp(argv[x], "-f")) || (!strcmp(argv[x], "--enable-flash"))) {
			dprintf(DEBUG_NORMAL, "enable flash\n");
			fromflash = true;
		}*/
		else if (((!strcmp(argv[x], "-v")) || (!strcmp(argv[x], "--verbose"))) && (x+1 < argc)) {
			int dl = atoi(argv[x+ 1]);
			dprintf(DEBUG_NORMAL, "set debuglevel: %d\n", dl);
			setDebugLevel(dl);
			x++;
		}
		else if ((!strcmp(argv[x], "-xd"))) {
			cnxt_debug = 1;
		}
		else if ((!strcmp(argv[x], "-sd"))) {
			sections_debug = true;
		}
		else if ((!strcmp(argv[x], "-zd"))) {
			zapit_debug = 1;
		}
		else if (!strcmp(argv[x], "-r")) {
			printf("[neutrino] WARNING: parameter -r ignored\n");
			x++;
			if (x < argc)
				x++;
			if (x < argc)
				x++;
		}
		else {
			dprintf(DEBUG_NORMAL, "Usage: neutrino [-u | --enable-update] "
					      "[-v | --verbose 0..3]\n");
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
			exit(CNeutrinoApp::REBOOT);
#else
			exit(1);
#endif
		}
	}
}

/**************************************************************************************
*          CNeutrinoApp -  setup the framebuffer                                      *
**************************************************************************************/
void CNeutrinoApp::SetupFrameBuffer()
{
	frameBuffer->init();
	if(frameBuffer->setMode(720, 576, 8 * sizeof(fb_pixel_t))) {
		dprintf(DEBUG_NORMAL, "Error while setting framebuffer mode\n");
		exit(-1);
	}
	frameBuffer->Clear();
}

/**************************************************************************************
*          CNeutrinoApp -  setup fonts                                                *
**************************************************************************************/

void CNeutrinoApp::SetupFonts(int fmode)
{
	if (neutrinoFonts == NULL)
		neutrinoFonts = CNeutrinoFonts::getInstance();

	if ((fmode & CNeutrinoFonts::FONTSETUP_NEUTRINO_FONT) == CNeutrinoFonts::FONTSETUP_NEUTRINO_FONT)
		neutrinoFonts->SetupNeutrinoFonts(((fmode & CNeutrinoFonts::FONTSETUP_NEUTRINO_FONT_INST) == CNeutrinoFonts::FONTSETUP_NEUTRINO_FONT_INST));

	if ((fmode & CNeutrinoFonts::FONTSETUP_DYN_FONT) == CNeutrinoFonts::FONTSETUP_DYN_FONT) {
		neutrinoFonts->SetupDynamicFonts(((fmode & CNeutrinoFonts::FONTSETUP_DYN_FONT_INST) == CNeutrinoFonts::FONTSETUP_DYN_FONT_INST));
		neutrinoFonts->refreshDynFonts();
	}

	/* recalculate infobar position */
	if (g_InfoViewer)
		g_InfoViewer->start();
	OnAfterSetupFonts();
}

/**************************************************************************************
*          CNeutrinoApp -  setup the menu timouts                                     *
**************************************************************************************/

#define LCD_UPDATE_TIME_RADIO_MODE (6 * 1000 * 1000)
#define LCD_UPDATE_TIME_TV_MODE (60 * 1000 * 1000)

void CNeutrinoApp::MakeSectionsdConfig(CSectionsdClient::epg_config& config)
{
	config.epg_cache                = g_settings.epg_cache;
	config.epg_old_events           = g_settings.epg_old_events;
	config.epg_max_events           = g_settings.epg_max_events;
	config.epg_extendedcache        = g_settings.epg_extendedcache;
	config.epg_save_frequently      = g_settings.epg_save ? g_settings.epg_save_frequently : 0;
	config.epg_dir                  = g_settings.epg_dir;
	config.network_ntpserver        = g_settings.network_ntpserver;
	config.network_ntprefresh       = atoi(g_settings.network_ntprefresh.c_str());
	config.network_ntpenable        = g_settings.network_ntpenable;
}

void CNeutrinoApp::SendSectionsdConfig(void)
{
	CSectionsdClient::epg_config config;
	MakeSectionsdConfig(config);
	g_Sectionsd->setConfig(config);
}

void CNeutrinoApp::InitZapper()
{
	struct stat my_stat;

	g_InfoViewer->start();
	if (g_settings.epg_read) {
		if(stat(g_settings.epg_dir.c_str(), &my_stat) == 0)
			g_Sectionsd->readSIfromXML(g_settings.epg_dir.c_str());
	}
	int tvmode = CZapit::getInstance()->getMode() & CZapitClient::MODE_TV;
	lastChannelMode = tvmode ? g_settings.channel_mode : g_settings.channel_mode_radio;
	mode = tvmode ? mode_tv : mode_radio;
	lastMode = mode;

	SDTreloadChannels = false;
	channelsInit();

	if(tvmode)
		tvMode(true);
	else
		radioMode(true);

	if(g_settings.cacheTXT)
		tuxtxt_init();

	t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
	if(channelList->getSize() && live_channel_id  && !IS_WEBTV(live_channel_id))
		g_Sectionsd->setServiceChanged(live_channel_id, false);
}

void CNeutrinoApp::setupRecordingDevice(void)
{
	CRecordManager::getInstance()->SetDirectory(g_settings.network_nfs_recordingdir);
	CRecordManager::getInstance()->Config(g_settings.recording_stopsectionsd, g_settings.recording_stream_vtxt_pid, g_settings.recording_stream_pmt_pid, g_settings.recording_stream_subtitle_pids);
}

static void CSSendMessage(uint32_t msg, uint32_t data)
{
	if (g_RCInput)
		g_RCInput->postMsg(msg, data);
}

void CNeutrinoApp::InitTimerdClient()
{
	g_Timerd = new CTimerdClient;
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
#if 0
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
#endif
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_ON, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_OFF, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_RECORD, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_START, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_STOP, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_REMIND, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_EXEC_PLUGIN, 222, NEUTRINO_UDS_NAME);
}

void CNeutrinoApp::InitZapitClient()
{
	g_Zapit         = new CZapitClient;
#define ZAPIT_EVENT_COUNT 29
	const CZapitClient::events zapit_event[ZAPIT_EVENT_COUNT] =
	{
		CZapitClient::EVT_ZAP_COMPLETE,
		CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD,
		CZapitClient::EVT_ZAP_FAILED,
		CZapitClient::EVT_ZAP_SUB_COMPLETE,
		CZapitClient::EVT_ZAP_SUB_FAILED,
		CZapitClient::EVT_ZAP_MOTOR,
		CZapitClient::EVT_ZAP_CA_ID,
		CZapitClient::EVT_RECORDMODE_ACTIVATED,
		CZapitClient::EVT_RECORDMODE_DEACTIVATED,
		CZapitClient::EVT_SCAN_COMPLETE,
		CZapitClient::EVT_SCAN_FAILED,
		CZapitClient::EVT_SCAN_NUM_TRANSPONDERS,
		CZapitClient::EVT_SCAN_REPORT_NUM_SCANNED_TRANSPONDERS,
		CZapitClient::EVT_SCAN_REPORT_FREQUENCYP,
		CZapitClient::EVT_SCAN_SATELLITE,
		CZapitClient::EVT_SCAN_NUM_CHANNELS,
		CZapitClient::EVT_SCAN_PROVIDER,
		CZapitClient::EVT_BOUQUETS_CHANGED,
		CZapitClient::EVT_SERVICES_CHANGED,
		CZapitClient::EVT_SCAN_SERVICENAME,
		CZapitClient::EVT_SCAN_FOUND_TV_CHAN,
		CZapitClient::EVT_SCAN_FOUND_RADIO_CHAN,
		CZapitClient::EVT_SCAN_FOUND_DATA_CHAN,
		CZapitClient::EVT_SDT_CHANGED,
		CZapitClient::EVT_PMT_CHANGED,
		CZapitClient::EVT_TUNE_COMPLETE,
		CZapitClient::EVT_BACK_ZAP_COMPLETE,
		CZapitClient::EVT_WEBTV_ZAP_COMPLETE,
	};

	for (int i = 0; i < ZAPIT_EVENT_COUNT; i++)
		g_Zapit->registerEvent(zapit_event[i], 222, NEUTRINO_UDS_NAME);
}

void CNeutrinoApp::InitSectiondClient()
{
	g_Sectionsd = new CSectionsdClient;
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_TIMESET, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_GOT_CN_EPG, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_EIT_COMPLETE, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_WRITE_SI_FINISHED, 222, NEUTRINO_UDS_NAME);
}

#if HAVE_COOL_HARDWARE
#include <cs_frontpanel.h>
#endif

void wake_up(bool &wakeup)
{
#if HAVE_COOL_HARDWARE
#ifndef FP_IOCTL_CLEAR_WAKEUP_TIMER
#define FP_IOCTL_CLEAR_WAKEUP_TIMER 10
#endif

#define FP_IOCTL_SET_RTC         0x101
#define FP_IOCTL_GET_RTC         0x102

	int fd = open("/dev/display", O_RDONLY);
	if (fd < 0) {
		perror("/dev/display");
	} else {
		fp_wakeup_data_t wk;
		memset(&wk, 0, sizeof(wk));
		int ret = ioctl(fd, IOC_FP_GET_WAKEUP, &wk);
		if(ret >= 0)
			wakeup = ((wk.source == FP_WAKEUP_SOURCE_TIMER) /* || (wk.source == WAKEUP_SOURCE_PWLOST)*/);
		close(fd);
	}
	printf("[timerd] wakeup from standby: %s\n", wakeup ? "yes" : "no");
#endif
	/* not platform specific - this is created by the init process */
	if (access("/tmp/.timer_wakeup", F_OK) == 0) {
		wakeup = true;
#if !HAVE_SPARK_HARDWARE && !HAVE_DUCKBOX_HARDWARE
		unlink("/tmp/.timer_wakeup");
#endif
	}

	if(!wakeup){
		puts("[neutrino.cpp] executing " NEUTRINO_LEAVE_DEEPSTANDBY_SCRIPT ".");
		if (my_system(NEUTRINO_LEAVE_DEEPSTANDBY_SCRIPT) != 0)
			perror(NEUTRINO_LEAVE_DEEPSTANDBY_SCRIPT " failed");
	}
}

int CNeutrinoApp::run(int argc, char **argv)
{
	set_threadname("CNeutrinoApp::run");
	CmdParser(argc, argv);

TIMER_START();
	cs_api_init();
	cs_register_messenger(CSSendMessage);

	g_info.hw_caps  = get_hwcaps();
	can_deepstandby = g_info.hw_caps->can_shutdown;
	g_info.has_fan  = g_info.hw_caps->has_fan;

	g_Locale        = new CLocaleManager;

	int loadSettingsErg = loadSetup(NEUTRINO_SETTINGS_FILE);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	cpuFreq = new cCpuFreqManager();
	cpuFreq->SetCpuFreq(g_settings.cpufreq * 1000 * 1000);
#endif
	wake_up( timer_wakeup );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	CCECSetup cecsetup;
	cecsetup.setCECSettings(true);
#endif

	initialize_iso639_map();

	CLocaleManager::loadLocale_ret_t loadLocale_ret = g_Locale->loadLocale(g_settings.language.c_str());
	if (loadLocale_ret == CLocaleManager::NO_SUCH_LOCALE)
	{
		g_settings.language = "deutsch";
		loadLocale_ret = g_Locale->loadLocale(g_settings.language.c_str());
	}

	// default usermenu titles correspond to gui/user_menue_setup.h:struct usermenu_props_t usermenu
	if (g_settings.usermenu[0]->title.empty() && !g_settings.usermenu[0]->items.empty())
		g_settings.usermenu[0]->title = g_Locale->getText(LOCALE_USERMENU_TITLE_RED);
	if (g_settings.usermenu[1]->title.empty() && !g_settings.usermenu[1]->items.empty())
		g_settings.usermenu[1]->title = g_Locale->getText(LOCALE_USERMENU_TITLE_GREEN);
	if (g_settings.usermenu[2]->title.empty() && !g_settings.usermenu[2]->items.empty())
		g_settings.usermenu[2]->title = g_Locale->getText(LOCALE_USERMENU_TITLE_YELLOW);
	if (g_settings.usermenu[3]->title.empty() && !g_settings.usermenu[3]->items.empty())
		g_settings.usermenu[3]->title = g_Locale->getText(LOCALE_USERMENU_TITLE_BLUE);

	/* setup GUI */
	neutrinoFonts = CNeutrinoFonts::getInstance();
	SetupFonts();
	g_PicViewer = new CPictureViewer();
	CColorSetupNotifier::setPalette();

	CProgressWindow * bootstatus = new CProgressWindow(NULL,true);
	bootstatus->setTitle(LOCALE_NEUTRINO_STARTING);
	bootstatus->paint();
	bootstatus->showStatusMessageUTF("loading...");
	bootstatus->showGlobalStatus(20);
	CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_NEUTRINO_STARTING));
	//hintBox->paint();

	CVFD::getInstance()->init(neutrinoFonts->fontDescr.filename.c_str(), neutrinoFonts->fontDescr.name.c_str());
	CVFD::getInstance()->Clear();
	CVFD::getInstance()->ShowText(g_Locale->getText(LOCALE_NEUTRINO_STARTING));
	CVFD::getInstance()->setBacklight(g_settings.backlight_tv);

#if HAVE_DUCKBOX_HARDWARE
	CVFD::getInstance()->ClearIcons();
#endif
#ifdef ENABLE_GRAPHLCD
	nGLCD::getInstance();
#endif
	if (!scanSettings.loadSettings(NEUTRINO_SCAN_SETTINGS_FILE))
		dprintf(DEBUG_NORMAL, "Loading of scan settings failed. Using defaults.\n");

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+10);

	/* set service manager options before starting zapit */
	CServiceManager::getInstance()->KeepNumbers(g_settings.keep_channel_numbers);
	//zapit start parameters
	Z_start_arg ZapStart_arg;
	ZapStart_arg.startchanneltv_id = g_settings.startchanneltv_id;
	ZapStart_arg.startchannelradio_id = g_settings.startchannelradio_id;
	ZapStart_arg.uselastchannel = g_settings.uselastchannel;
	ZapStart_arg.video_mode = g_settings.video_Mode;
	ZapStart_arg.ci_clock = g_settings.ci_clock;
	ZapStart_arg.volume = g_settings.current_volume;
	ZapStart_arg.webtv_xml = &g_settings.webtv_xml;

	CCamManager::getInstance()->SetCITuner(g_settings.ci_tuner);
	/* create decoders, read channels */
	bool zapit_init = CZapit::getInstance()->Start(&ZapStart_arg);
	//get zapit config for writeChannelsNames
	CZapit::getInstance()->GetConfig(zapitCfg);

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+20);

	// init audio settings
	audioDecoder->SetSRS(g_settings.srs_enable, g_settings.srs_nmgr_enable, g_settings.srs_algo, g_settings.srs_ref_volume);
	//audioDecoder->setVolume(g_settings.current_volume, g_settings.current_volume);
	audioDecoder->SetHdmiDD((HDMI_ENCODED_MODE)g_settings.hdmi_dd);
	audioDecoder->SetSpdifDD(g_settings.spdif_dd ? true : false);
	audioDecoder->EnableAnalogOut(g_settings.analog_out ? true : false);
	audioSetupNotifier        = new CAudioSetupNotifier;
	// trigger a change
	if(g_settings.avsync != (AVSYNC_TYPE) AVSYNC_ENABLED)
		audioSetupNotifier->changeNotify(LOCALE_AUDIOMENU_AVSYNC, NULL);

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	//init video settings
	g_videoSettings = new CVideoSettings;
	g_videoSettings->setVideoSettings();

	g_RCInput = new CRCInput(timer_wakeup);

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	/* later on, we'll crash anyway, so tell about it. */
	if (! zapit_init)
		DisplayErrorMessage("Zapit initialization failed. This is a fatal error, sorry.");

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	InitZapitClient();
	g_Zapit->setStandby(false);

#if ENABLE_FASTSCAN
	CheckFastScan();
#endif

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	//timer start
#if !HAVE_SPARK_HARDWARE && !HAVE_DUCKBOX_HARDWARE
	timer_wakeup = false;//init
	wake_up( timer_wakeup );

	init_cec_setting = true;
	if(!(g_settings.shutdown_timer_record_type && timer_wakeup && g_settings.hdmi_cec_mode)){
		//init cec settings
		CCECSetup cecsetup;
		cecsetup.setCECSettings();
		init_cec_setting = false;
	}
#endif
	timer_wakeup = (timer_wakeup && g_settings.shutdown_timer_record_type);
	g_settings.shutdown_timer_record_type = false;

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	/* todo: check if this is necessary
	pthread_create (&timer_thread, NULL, timerd_main_thread, (void *) (timer_wakeup && g_settings.shutdown_timer_record_type));
	 */
	// The thread argument sets a pointer to Neutrinos timer_wakeup. *pointer is set to true
	// when timerd is ready, so save the real timer_wakeup value and restore it later. --martii
	bool timer_wakup_real = timer_wakeup;
	timer_wakeup = false;
	pthread_create (&timer_thread, NULL, timerd_main_thread, (void *)&timer_wakeup);
	timerd_thread_started = true;

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	audioSetupNotifier->changeNotify(LOCALE_AUDIOMENU_MIXER_VOLUME_ANALOG, &g_settings.audio_mixer_volume_analog);
	audioSetupNotifier->changeNotify(LOCALE_AUDIOMENU_MIXER_VOLUME_SPDIF, &g_settings.audio_mixer_volume_spdif);
	audioSetupNotifier->changeNotify(LOCALE_AUDIOMENU_MIXER_VOLUME_HDMI, &g_settings.audio_mixer_volume_hdmi);
#endif
	powerManager = new cPowerManager;
	powerManager->Open();

#if !HAVE_SPARK_HARDWARE && !HAVE_DUCKBOX_HARDWARE
	cpuFreq = new cCpuFreqManager();
	cpuFreq->SetCpuFreq(g_settings.cpufreq * 1000 * 1000);
#endif
#if HAVE_COOL_HARDWARE
	/* only SAT-hd1 before rev 8 has fan */
	g_info.has_fan = (cs_get_revision()  < 8 && CFEManager::getInstance()->getFE(0)->hasSat());
#endif
	dprintf(DEBUG_NORMAL, "g_info.has_fan: %d\n", g_info.has_fan);
	//fan speed
	if (g_info.has_fan)
		CFanControlNotifier::setSpeed(g_settings.fan_speed);

	dvbsub_init();

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

#if ENABLE_WEBIF
	pthread_t nhttpd_thread;
	if (!pthread_create (&nhttpd_thread, NULL, nhttpd_main_thread, (void *) NULL))
		pthread_detach (nhttpd_thread);
#endif

	CStreamManager::getInstance()->Start();

#ifndef DISABLE_SECTIONSD
	CSectionsdClient::epg_config config;
	MakeSectionsdConfig(config);
	CEitManager::getInstance()->SetConfig(config);
	CEitManager::getInstance()->Start();
#endif

	g_RemoteControl = new CRemoteControl;
	g_EpgData = new CEpgData;
	g_InfoViewer = new CInfoViewer;
	g_EventList = new CEventList;

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

#if !HAVE_SPARK_HARDWARE
	g_CamHandler = new CCAMMenuHandler();
	g_CamHandler->init();
#endif

#if !HAVE_SPARK_HARDWARE && !HAVE_DUCKBOX_HARDWARE
#ifndef ASSUME_MDEV
	mkdir("/media/sda1", 0755);
	mkdir("/media/sdb1", 0755);
	my_system(3, "mount", "/dev/sda1", "/media/sda1");
	my_system(3, "mount", "/dev/sdb1", "/media/sdb1");
#endif
#endif

	CFSMounter::automount();
	g_PluginList = new CPlugins;
	g_PluginList->setPluginDir(PLUGINDIR);
	//load Pluginlist before main menu (only show script menu if at least one script is available
	g_PluginList->loadPlugins();

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	// setup recording device
	setupRecordingDevice();

	dprintf( DEBUG_NORMAL, "menue setup\n");
	//init Menues
	InitMenu();

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	dprintf( DEBUG_NORMAL, "registering as event client\n");

	InitSectiondClient();

	/* wait until timerd is ready... */
	time_t timerd_wait = time_monotonic_ms();
	while (!timer_wakeup)
		usleep(100);
	dprintf(DEBUG_NORMAL, "had to wait %ld ms for timerd start...\n", time_monotonic_ms() - timerd_wait);
	timer_wakeup = timer_wakup_real;
	InitTimerdClient();

	// volume
	if (g_settings.show_mute_icon && g_settings.current_volume == 0)
		current_muted = true;

	g_volume = CVolume::getInstance();
	g_audioMute = CAudioMute::getInstance();

	g_audioMute->AudioMute(current_muted, true);
	CZapit::getInstance()->SetVolumePercent(g_settings.audio_volume_percent_ac3, g_settings.audio_volume_percent_pcm);
	CVFD::getInstance()->showVolume(g_settings.current_volume);
	CVFD::getInstance()->setMuted(current_muted);

	LCD4l = new CLCD4l();
	LCD4l->StartLCD4l();

	bootstatus->showGlobalStatus(bootstatus->getGlobalStatus()+5);

	InitZapper();

	bootstatus->showGlobalStatus(100);

	if(loadSettingsErg) {
		bootstatus->hide();
		//hintBox->hide();
		dprintf(DEBUG_INFO, "config file or options missing\n");
		ShowHint(LOCALE_MESSAGEBOX_INFO, loadSettingsErg ==  1 ? g_Locale->getText(LOCALE_SETTINGS_NOCONFFILE)
				: g_Locale->getText(LOCALE_SETTINGS_MISSINGOPTIONSCONFFILE));
		configfile.setModifiedFlag(true);
		saveSetup(NEUTRINO_SETTINGS_FILE);
	}

	CHDDDestExec * hdd = new CHDDDestExec();
	hdd->exec(NULL, "");
	delete hdd;

	bootstatus->hide();
	delete bootstatus;
	//hintBox->hide(); // InitZapper also displays a hintbox
	delete hintBox;

	cCA::GetInstance()->Ready(true);
	cCA::GetInstance()->setCheckLiveSlot(g_settings.ci_check_live);
	//InitZapper();

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	C3DSetup::getInstance()->exec(NULL, "zapped");
	CPSISetup::getInstance()->blankScreen(false);
#endif
	SHTDCNT::getInstance()->init();

	cSysLoad::getInstance();
	//cHddStat::getInstance();

TIMER_STOP("################################## after all ##################################");
	if (g_settings.softupdate_autocheck) {
		hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FLASHUPDATE_CHECKUPDATE_INTERNET));
		hintBox->paint();
		CFlashUpdate flash;
		if(flash.checkOnlineVersion()) {
			hintBox->hide();
			//flash.enableNotify(false);
			flash.exec(NULL, "inet");
		}
		delete hintBox;
	}
	RealRun();

	ExitRun(true, !can_deepstandby);

	return 0;
}

void CNeutrinoApp::quickZap(int msg)
{
	int res;

	StopSubtitles();
	bool ret;
	if(!bouquetList->Bouquets.empty())
		ret = bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->quickZap(msg, g_settings.zap_cycle);
	else
		ret = channelList->quickZap(msg);
	if (!ret) {
		res = channelList->numericZap(g_settings.key_zaphistory);
		StartSubtitles(res < 0);
	}
}

void CNeutrinoApp::numericZap(int msg)
{
	StopSubtitles();
	int res = channelList->numericZap( msg );
	StartSubtitles(res < 0);
	if (res >= 0 && CRCInput::isNumeric(msg)) {
		if (g_settings.channellist_numeric_adjust && first_mode_found >= 0) {
			SetChannelMode(first_mode_found);
			channelList->getLastChannels().set_mode(channelList->getActiveChannel_ChannelID());
		}
	}
}

void CNeutrinoApp::showInfo()
{
	StopSubtitles();
	g_InfoViewer->showTitle(channelList->getActiveChannel());
	StartSubtitles();
}

#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
static void check_timer()
{
	CTimerd::TimerList tmpTimerList;
	CTimerdClient tmpTimerdClient;
	tmpTimerList.clear();
	tmpTimerdClient.getTimerList(tmpTimerList);
	if(tmpTimerList.size() > 0) {
		CVFD::getInstance()->ShowIcon(FP_ICON_CLOCK, true);
	} else {
		CVFD::getInstance()->ShowIcon(FP_ICON_CLOCK, false);
	}
	tmpTimerList.clear();
}
#endif

void CNeutrinoApp::showMainMenu()
{
	StopSubtitles();
	InfoClock->enableInfoClock(false);
	int old_ttx = g_settings.cacheTXT;
	int old_epg = g_settings.epg_scan;
	int old_mode = g_settings.epg_scan_mode;
	int old_save_mode = g_settings.epg_save_mode;
	mainMenu->exec(NULL, "");
#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
	CVFD::getInstance()->UpdateIcons();
#endif
	InfoClock->enableInfoClock(true);
	StartSubtitles();
	saveSetup(NEUTRINO_SETTINGS_FILE);

	if (old_save_mode != g_settings.epg_save_mode)
		CEpgScan::getInstance()->ConfigureEIT();
	if (old_epg != g_settings.epg_scan || old_mode != g_settings.epg_scan_mode) {
		if (g_settings.epg_scan_mode != CEpgScan::MODE_OFF)
			CEpgScan::getInstance()->Start();
		else
			CEpgScan::getInstance()->Clear();
	}
	if (old_ttx != g_settings.cacheTXT) {
		if(g_settings.cacheTXT) {
			tuxtxt_init();
		} else
			tuxtxt_close();
	}
}

void CNeutrinoApp::screensaver(bool on)
{
	if (on)
	{
		m_screensaver = true;
		CInfoClock::getInstance()->block();
		CScreenSaver::getInstance()->Start();
	}
	else
	{
		CScreenSaver::getInstance()->Stop();
		m_screensaver = false;
		m_idletime = time(NULL);
	}
}

void CNeutrinoApp::RealRun()
{
	mainMenu = &personalize.getWidget(MENU_MAIN);

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	dprintf(DEBUG_NORMAL, "initialized everything\n");

	//activating infoclock
	InfoClock = CInfoClock::getInstance();

	if(g_settings.power_standby || init_cec_setting)
		standbyMode(true, true);

	//cCA::GetInstance()->Ready(true);

	CLuaServer *luaServer = CLuaServer::getInstance();

	g_PluginList->startPlugin("startup");
	if (!g_PluginList->getScriptOutput().empty()) {
		ShowMsg(LOCALE_PLUGINS_RESULT, g_PluginList->getScriptOutput(), CMessageBox::mbrBack,CMessageBox::mbBack,NEUTRINO_ICON_SHELL);
	}
	g_RCInput->clearRCMsg();

	m_idletime	= time(NULL);
	m_screensaver	= false;

	while( true ) {
		luaServer->UnBlock();
		g_RCInput->getMsg(&msg, &data, 100, ((g_settings.mode_left_right_key_tv == SNeutrinoSettings::VOLUME) && (g_RemoteControl->subChannels.size() < 1)) ? true : false);	// 10 secs..
		if (luaServer->Block(msg, data))
			continue;

#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
		check_timer();
#endif
		if (mode == mode_radio) {
			bool ignored_msg = (
				/* radio screensaver will ignore this msgs */
				   msg == NeutrinoMessages::EVT_CURRENTEPG
				|| msg == NeutrinoMessages::EVT_NEXTEPG
				|| msg == NeutrinoMessages::EVT_CURRENTNEXT_EPG
				|| msg == NeutrinoMessages::EVT_TIMESET
				|| msg == NeutrinoMessages::EVT_PROGRAMLOCKSTATUS
				|| msg == NeutrinoMessages::EVT_ZAP_GOT_SUBSERVICES
				|| msg == NeutrinoMessages::EVT_ZAP_GOTAPIDS
				|| msg == NeutrinoMessages::EVT_ZAP_GOTPIDS
			);
			if ( msg == CRCInput::RC_timeout  || msg == NeutrinoMessages::EVT_TIMER)
			{
				int delay = time(NULL) - m_idletime;
				int screensaver_delay = g_settings.screensaver_delay;
				if (screensaver_delay !=0 && delay > screensaver_delay*60 && !m_screensaver)
					screensaver(true);
			}
			else if (!ignored_msg)
			{
				m_idletime = time(NULL);
				if (m_screensaver)
				{
					printf("[neutrino] CSreenSaver stop; msg: %lX\n", msg);
					screensaver(false);

					frameBuffer->stopFrame();
					frameBuffer->showFrame("radiomode.jpg");

					if (msg <= CRCInput::RC_MaxRC) {
						// ignore first keypress - just quit the screensaver
						g_RCInput->clearRCMsg();
						continue;
					}
				}
			}
		}

		if( ( mode == mode_tv ) ||  ( mode == mode_radio )  || ( mode == mode_webtv ) ) {
			if( (msg == NeutrinoMessages::SHOW_EPG) /* || (msg == CRCInput::RC_info) */ ) {
				InfoClock->enableInfoClock(false);
				StopSubtitles();
				t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
				g_EpgData->show(live_channel_id);
				InfoClock->enableInfoClock(true);
				StartSubtitles();
			}
			/* the only hardcoded key to check before key bindings */
			else if( msg == CRCInput::RC_setup ) {
				if(!g_settings.minimode) {
					showMainMenu();
				}
			}
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
			else if( ( msg == (neutrino_msg_t) g_settings.key_quickzap_up ) || ( msg == (neutrino_msg_t) g_settings.key_quickzap_down ) || ( msg == CRCInput::RC_page_up ) || ( msg == CRCInput::RC_page_down ) )
#else
			else if( ( msg == (neutrino_msg_t) g_settings.key_quickzap_up ) || ( msg == (neutrino_msg_t) g_settings.key_quickzap_down ) )
#endif
			{
				quickZap(msg);
			}
			else if( msg == (neutrino_msg_t) g_settings.key_tvradio_mode ) {
				switchTvRadioMode();
			}
			else if( msg == (neutrino_msg_t) g_settings.key_subchannel_up || msg == (neutrino_msg_t) g_settings.key_subchannel_down) {
				if( !g_RemoteControl->subChannels.empty() ) {
					StopSubtitles();
					if( msg == (neutrino_msg_t) g_settings.key_subchannel_up )
						g_RemoteControl->subChannelUp();
					else if( msg == (neutrino_msg_t) g_settings.key_subchannel_down )
						g_RemoteControl->subChannelDown();
					g_InfoViewer->showSubchan();
				}
				else if ( msg == CRCInput::RC_left || msg == CRCInput::RC_right) {
					switch (g_settings.mode_left_right_key_tv)
					{
						case SNeutrinoSettings::INFOBAR:
						case SNeutrinoSettings::VZAP:
							if (channelList->getSize())
								showInfo();
							break;
						case SNeutrinoSettings::VOLUME:
							g_volume->setVolume(msg);
							break;
						default: /* SNeutrinoSettings::ZAP */
							quickZap(msg);
							break;
					}
				}
				else
					quickZap( msg );
			}
			else if( msg == (neutrino_msg_t) g_settings.key_zaphistory ) {
				// Zap-History "Bouquet"
				InfoClock->enableInfoClock(false);
				numericZap( msg );
				InfoClock->enableInfoClock(true);
			}
			else if (msg == (neutrino_msg_t) g_settings.key_screenshot) {
				for(int i = 0; i < g_settings.screenshot_count; i++) {
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
					CVFD::getInstance()->ShowText("SCREENSHOT");
					CHintBox *hintbox = NULL;
					if (g_settings.screenshot_mode == 1)
						hintbox = new CHintBox(LOCALE_SCREENSHOT_MENU, g_Locale->getText(LOCALE_SCREENSHOT_PLEASE_WAIT), 450, NEUTRINO_ICON_MOVIEPLAYER);
					if (hintbox)
						hintbox->paint();
#endif
					CScreenShot * sc = new CScreenShot("", (CScreenShot::screenshot_format_t)g_settings.screenshot_format);
					sc->MakeFileName(CZapit::getInstance()->GetCurrentChannelID());
					sc->Start();
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
					if (hintbox) {
						hintbox->hide();
						delete hintbox;
					}
#endif
				}
			}
			else if( msg == (neutrino_msg_t) g_settings.key_lastchannel ) {
				// Quick Zap
				numericZap( msg );
			}
			else if(msg == (neutrino_msg_t) g_settings.key_timeshift) {
#if 0
				if (mode == mode_webtv) {
					CMoviePlayerGui::getInstance().Pause();
				} else
#endif
					CRecordManager::getInstance()->StartTimeshift();
			}
			else if (msg == (neutrino_msg_t) g_settings.key_current_transponder) {
				InfoClock->enableInfoClock(false);
				numericZap( msg );
				InfoClock->enableInfoClock(true);
			}
#ifdef ENABLE_PIP
			else if (msg == (neutrino_msg_t) g_settings.key_pip_close) {
				t_channel_id pip_channel_id = CZapit::getInstance()->GetPipChannelID();
				if (pip_channel_id)
					g_Zapit->stopPip();
				else
					StartPip(CZapit::getInstance()->GetCurrentChannelID());
			}
			else if (msg == (neutrino_msg_t) g_settings.key_pip_setup) {
				CPipSetup pipsetup;
				pipsetup.exec(NULL, "");
			}
			else if (msg == (neutrino_msg_t) g_settings.key_pip_swap) {
				t_channel_id pip_channel_id = CZapit::getInstance()->GetPipChannelID();
				t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
				if (pip_channel_id && (pip_channel_id != live_channel_id)) {
					g_Zapit->stopPip();
					channelList->zapTo_ChannelID(pip_channel_id);
					StartPip(live_channel_id);
				}
			}
#endif
			else if( msg == (neutrino_msg_t) g_settings.key_record /* && (mode != mode_webtv) */) {
				if (g_settings.recording_type != CNeutrinoApp::RECORDING_OFF)
					CRecordManager::getInstance()->exec(NULL, "Record");
			}
#if 0
			else if ((mode == mode_webtv) && msg == (neutrino_msg_t) g_settings.mpkey_subtitle) {
				CMoviePlayerGui::getInstance(true).selectSubtitle();
			}
#endif
			/* after sensitive key bind, check user menu */
			else if (usermenu.showUserMenu(msg)) {
			}
			/* hardcoded key values, if not redefined in keybind or user menu */
			else if( msg == CRCInput::RC_text) {
				g_RCInput->clearRCMsg();
				InfoClock->enableInfoClock(false);
				StopSubtitles();
				tuxtx_stop_subtitle();

				tuxtx_main(g_RemoteControl->current_PIDs.PIDs.vtxtpid);
#if 1 // FIXME, needed? --martii
				//purge input queue
				do
					g_RCInput->getMsg(&msg, &data, 1);
				while (msg != CRCInput::RC_timeout);
#endif

				frameBuffer->paintBackground();
				//if(!g_settings.cacheTXT)
				//	tuxtxt_stop();
				g_RCInput->clearRCMsg();
				InfoClock->enableInfoClock(true);
				StartSubtitles();
			}
			else if (((msg == CRCInput::RC_tv) || (msg == CRCInput::RC_radio)) && (g_settings.key_tvradio_mode == (int)CRCInput::RC_nokey)) {
				switchTvRadioMode();//used with defined default tv/radio rc key
			}
			/* in case key_subchannel_up/down redefined */
			else if( msg == CRCInput::RC_left || msg == CRCInput::RC_right) {
				switch (g_settings.mode_left_right_key_tv)
				{
					case SNeutrinoSettings::INFOBAR:
					case SNeutrinoSettings::VZAP:
						if (channelList->getSize())
							showInfo();
						break;
					case SNeutrinoSettings::VOLUME:
						g_volume->setVolume(msg);
						break;
					default: /* SNeutrinoSettings::ZAP */
						quickZap(msg);
						break;
				}
			}
			else if( msg == CRCInput::RC_epg ) {
				InfoClock->enableInfoClock(false);
				StopSubtitles();
				t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
				g_EventList->exec(live_channel_id, channelList->getActiveChannelName());
				InfoClock->enableInfoClock(true);
				StartSubtitles();
			}
			else if (CRCInput::isNumeric(msg)) {
				numericZap( msg );

			}
			/* FIXME ??? */
			else if (CRCInput::isNumeric(msg) && g_RemoteControl->director_mode ) {
				g_RemoteControl->setSubChannel(CRCInput::getNumericValue(msg));
				g_InfoViewer->showSubchan();
			}
			else if( msg == CRCInput::RC_page_up || msg == CRCInput::RC_page_down) {
				quickZap(msg == CRCInput::RC_page_up ? CRCInput::RC_right : CRCInput::RC_left);
			}
			else if(msg == CRCInput::RC_rewind /* && (mode != mode_webtv) */) {
				if(g_RemoteControl->is_video_started) {
					t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
					if(CRecordManager::getInstance()->RecordingStatus(live_channel_id))
						CMoviePlayerGui::getInstance().exec(NULL, "rtimeshift");
				}
			}
			else if( msg == CRCInput::RC_stop) {
				StopSubtitles();
				CRecordManager::getInstance()->exec(NULL, "Stop_record");
				StartSubtitles();
			}
			else if ((msg == CRCInput::RC_audio) && !g_settings.audio_run_player)
			{
				StopSubtitles();
				CAudioSelectMenuHandler as;
				as.exec(NULL, "-1");
				StartSubtitles();
			}
			else if( (msg == CRCInput::RC_audio) && g_settings.audio_run_player) {
				//open mediaplayer menu in audio mode, user can select between audioplayer and internetradio
				CMediaPlayerMenu * media = CMediaPlayerMenu::getInstance();
				media->setMenuTitel(LOCALE_MAINMENU_AUDIOPLAYER);
				media->setUsageMode(CMediaPlayerMenu::MODE_AUDIO);
				media->exec(NULL, "");
			}
			else if( msg == CRCInput::RC_video ) {
				//open moviebrowser via media player menu object
				if (g_settings.recording_type != CNeutrinoApp::RECORDING_OFF)
					CMediaPlayerMenu::getInstance()->exec(NULL,"movieplayer");
			}
			else if( msg == CRCInput::RC_play ) {
				switch (g_settings.key_playbutton)
				{
				default:
				case 0:
					CMediaPlayerMenu::getInstance()->exec(NULL, "movieplayer");
					break;
				case 1:
					CMoviePlayerGui::getInstance().exec(NULL, "fileplayback");
					break;
				case 2:
					CMediaPlayerMenu::getInstance()->exec(NULL, "audioplayer");
					break;
				case 3:
					CMediaPlayerMenu::getInstance()->exec(NULL, "inetplayer");
					break;
				}
#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
				CVFD::getInstance()->UpdateIcons();
#endif
			}
			else if (CRCInput::isNumeric(msg) && g_RemoteControl->director_mode ) {
				g_RemoteControl->setSubChannel(CRCInput::getNumericValue(msg));
				g_InfoViewer->showSubchan();
			}
			else if( ( msg == CRCInput::RC_help ) || ( msg == CRCInput::RC_info) ||
						( msg == NeutrinoMessages::SHOW_INFOBAR ) )
			{
				bool show_info = ((msg != NeutrinoMessages::SHOW_INFOBAR) || (g_InfoViewer->is_visible || g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] != 0));
			         // turn on LCD display
				CVFD::getInstance()->wake_up();

				// show Infoviewer
				if(show_info && channelList->getSize()) {
					showInfo();
				}
#ifdef ENABLE_GRAPHLCD
				if (msg == NeutrinoMessages::EVT_CURRENTNEXT_EPG) {
					nGLCD::Update();
				}
#endif
			}
			else if (msg == CRCInput::RC_timer)
			{
				CTimerList Timerlist;
				Timerlist.exec(NULL, "");
			}
			else if (msg == CRCInput::RC_aux)
				scartMode(true);
			else {
				if (msg == CRCInput::RC_home)
					CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

				if (msg != CRCInput::RC_timeout)
					handleMsg(msg, data);
			}
		}
		else {
			// mode == mode_scart
			if (msg == CRCInput::RC_home || msg == CRCInput::RC_aux) {
				if( mode == mode_scart ) {
					// Scart-Mode verlassen
					scartMode( false );
				}
			}
			else {
				if (msg != CRCInput::RC_timeout)
					handleMsg(msg, data);
			}
		}
	}
}

int CNeutrinoApp::showChannelList(const neutrino_msg_t _msg, bool from_menu)
{
	/* Exit here if paint of channlellist is not allowed, disallow could be possible, eg: if
	 * RC_ok or other stuff is shared with other window handlers and
	 * it's easy here to disable channellist paint if required!
	*/
	if (!channelList_allowed){
		channelList_allowed = true;
		return menu_return::RETURN_NONE;
	}
	channelList_painted = true;

	neutrino_msg_t msg = _msg;
	InfoClock->enableInfoClock(false);//TODO: use callback in channel list class
	StopSubtitles();

//_show:
	int nNewChannel = -1;
	int old_b = bouquetList->getActiveBouquetNumber();
	t_channel_id old_id = 0;
	if(!bouquetList->Bouquets.empty())
		old_id = bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->getActiveChannel_ChannelID();

	int old_mode = GetChannelMode();
	printf("CNeutrinoApp::showChannelList: bouquetList %p size %d old_b %d\n", bouquetList, (int)bouquetList->Bouquets.size(), old_b);fflush(stdout);

	//_show:
	if(msg == CRCInput::RC_ok)
	{
		if( !bouquetList->Bouquets.empty() && bouquetList->Bouquets[old_b]->channelList->getSize() > 0)
			nNewChannel = bouquetList->Bouquets[old_b]->channelList->exec();//with ZAP!
		else
			nNewChannel = bouquetList->exec(true);
	} else if(msg == CRCInput::RC_sat) {
		SetChannelMode(LIST_MODE_SAT);
		nNewChannel = bouquetList->exec(true);
	} else if(msg == CRCInput::RC_favorites) {
		SetChannelMode(LIST_MODE_FAV);
		if (bouquetList->Bouquets.empty())
			SetChannelMode(LIST_MODE_PROV);
		nNewChannel = bouquetList->exec(true);
	} else if(msg == CRCInput::RC_www) {
		SetChannelMode(LIST_MODE_WEBTV);
		if (bouquetList->Bouquets.empty())
			SetChannelMode(LIST_MODE_PROV);
		nNewChannel = bouquetList->exec(true);
	}
_repeat:
	printf("CNeutrinoApp::showChannelList: nNewChannel %d\n", nNewChannel);fflush(stdout);
	//CVFD::getInstance ()->showServicename(channelList->getActiveChannelName());
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	if(nNewChannel == CHANLIST_CANCEL) { // restore orig. bouquet and selected channel on cancel
		/* FIXME if mode was changed while browsing,
		 * other modes selected bouquet not restored */
		SetChannelMode(old_mode);
		bouquetList->activateBouquet(old_b, false);

		if(!bouquetList->Bouquets.empty())
			bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->adjustToChannelID(old_id);

		StartSubtitles(mode == mode_tv);
	}
	else if(nNewChannel == CHANLIST_CHANGE_MODE) { // list mode changed
		printf("CNeutrinoApp::showChannelList: newmode: bouquetList %p size %d\n", bouquetList, (int)bouquetList->Bouquets.size());fflush(stdout);
		nNewChannel = bouquetList->exec(true);
		goto _repeat;
	}
	if (channels_changed || favorites_changed || bouquets_changed || channels_init) {
		neutrino_locale_t loc = channels_init ? LOCALE_SERVICEMENU_RELOAD_HINT : LOCALE_BOUQUETEDITOR_SAVINGCHANGES;
		CHintBox* hintBox= new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(loc));
		hintBox->paint();

		if (favorites_changed) {
			g_bouquetManager->saveUBouquets();
			if (!channels_init)
				CEpgScan::getInstance()->ConfigureEIT();
		}

		if (channels_changed)
			CServiceManager::getInstance()->SaveServices(true);

		if (bouquets_changed)
			g_bouquetManager->saveBouquets();

		if (channels_init) {
			g_bouquetManager->renumServices();
			channelsInit(/*true*/);
		}

		favorites_changed = false;
		channels_changed = false;
		bouquets_changed = false;
		channels_init = false;

		t_channel_id live_channel_id = channelList->getActiveChannel_ChannelID();
		if(!live_channel_id)
			live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		adjustToChannelID(live_channel_id);//FIXME what if deleted ?
		delete hintBox;
	}

	channelList_painted = false;

	if (!from_menu)
		InfoClock->enableInfoClock(true);

	return ((nNewChannel >= 0) ? menu_return::RETURN_EXIT_ALL : menu_return::RETURN_REPAINT);
}

void CNeutrinoApp::zapTo(t_channel_id channel_id)
{
	bool recordingStatus = CRecordManager::getInstance()->RecordingStatus(channel_id);
	if (!recordingStatus || (recordingStatus && CRecordManager::getInstance()->TimeshiftOnly()) ||
			(recordingStatus && channelList->SameTP(channel_id))) {

		dvbsub_stop();
		g_Zapit->zapTo_serviceID_NOWAIT(channel_id);
	}
}

bool CNeutrinoApp::wakeupFromStandby(void)
{
	bool alive = recordingstatus || CEpgScan::getInstance()->Running() ||
		CStreamManager::getInstance()->StreamStatus();

	if ((mode == mode_standby) && !alive) {
		cpuFreq->SetCpuFreq(g_settings.cpufreq * 1000 * 1000);
#if !HAVE_SPARK_HARDWARE
		if(g_settings.ci_standby_reset) {
			g_CamHandler->exec(NULL, "ca_ci_reset0");
			g_CamHandler->exec(NULL, "ca_ci_reset1");
		}
#endif
		g_Zapit->setStandby(false);
		g_Zapit->getMode();
		return true;
	}
	return false;
}

void CNeutrinoApp::standbyToStandby(void)
{
	bool alive = recordingstatus || CEpgScan::getInstance()->Running() ||
		CStreamManager::getInstance()->StreamStatus();

	if ((mode == mode_standby) && !alive) {
		// zap back to pre-recording channel if necessary
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		if (standby_channel_id && (live_channel_id != standby_channel_id)) {
			live_channel_id = standby_channel_id;
			channelList->zapTo_ChannelID(live_channel_id);
		}
		g_Zapit->setStandby(true);
		g_Sectionsd->setPauseScanning(true);
		cpuFreq->SetCpuFreq(g_settings.standby_cpufreq * 1000 * 1000);
#if defined (BOXMODEL_IPBOX9900) || defined (BOXMODEL_IPBOX99)
		system("echo 0 > /proc/stb/misc/fan");
#endif
	}
}

void CNeutrinoApp::stopPlayBack(bool lock)
{
	CMoviePlayerGui::getInstance().stopPlayBack();
	g_Zapit->stopPlayBack();
	if (lock)
		CZapit::getInstance()->EnablePlayback(false);
}

void CNeutrinoApp::lockPlayBack(bool blank)
{
	CMoviePlayerGui::getInstance().stopPlayBack();
	g_Zapit->lockPlayBack();
	if (blank)
		videoDecoder->setBlank(true);
}

int CNeutrinoApp::handleMsg(const neutrino_msg_t _msg, neutrino_msg_data_t data)
{
	int res = 0;
	neutrino_msg_t msg = _msg;

	if(msg == NeutrinoMessages::EVT_WEBTV_ZAP_COMPLETE) {
		t_channel_id chid = *(t_channel_id *) data;
		printf("EVT_WEBTV_ZAP_COMPLETE: %" PRIx64 "\n", chid);
		if (mode == mode_standby) {
			delete [] (unsigned char*) data;
		} else {
			CZapitChannel * cc = CZapit::getInstance()->GetCurrentChannel();
			if (cc && (chid == cc->getChannelID())) {
				CMoviePlayerGui::getInstance().stopPlayBack();
				if (CMoviePlayerGui::getInstance().PlayBackgroundStart(cc->getUrl(), cc->getName(), cc->getChannelID(), cc->getScriptName()))
					delete [] (unsigned char*) data;
				else
					g_RCInput->postMsg(NeutrinoMessages::EVT_ZAP_FAILED, data);
			} else
				delete [] (unsigned char*) data;
		}
		return messages_return::handled;
	}
#if 0
	if (mode == mode_webtv && msg == NeutrinoMessages::EVT_SUBT_MESSAGE) {
		CMoviePlayerGui::getInstance(true).showSubtitle(data);
		return messages_return::handled;
	}
#endif
	if(msg == NeutrinoMessages::EVT_ZAP_COMPLETE) {
		CZapit::getInstance()->GetAudioMode(g_settings.audio_AnalogMode);
		if(g_settings.audio_AnalogMode < 0 || g_settings.audio_AnalogMode > 2)
			g_settings.audio_AnalogMode = 0;

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		C3DSetup::getInstance()->exec(NULL, "zapped");
#endif
#if HAVE_DUCKBOX_HARDWARE || BOXMODEL_SPARK7162
		CVFD::getInstance()->UpdateIcons();
#endif
#ifdef ENABLE_GRAPHLCD
		nGLCD::Update();
#endif
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		{
			CScreenSetup cSS;
			cSS.showBorder(CZapit::getInstance()->GetCurrentChannelID());
		}
#endif
		g_RCInput->killTimer(scrambled_timer);
		if (mode != mode_webtv) {
			scrambled_timer = g_RCInput->addTimer(10*1000*1000, true);
			SelectSubtitles();
			//StartSubtitles(!g_InfoViewer->is_visible);

			/* update scan settings for manual scan to current channel */
			CScanSetup::getInstance()->updateManualSettings();
		}
	}
	if ((msg == NeutrinoMessages::EVT_TIMER)) {
		if(data == scrambled_timer) {
			scrambled_timer = 0;
#if BOXMODEL_UFS910
			if(g_settings.scrambled_message && videoDecoder->getPlayState()) {
#else
			if(g_settings.scrambled_message && videoDecoder->getBlank() && videoDecoder->getPlayState()) {
#endif
				const char * text = g_Locale->getText(LOCALE_SCRAMBLED_CHANNEL);
				ShowHint (LOCALE_MESSAGEBOX_INFO, text, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(text) + 10, 5);
			}
			return messages_return::handled;
		}
#if ENABLE_FASTSCAN
		if(data == fst_timer) {
			g_RCInput->killTimer(fst_timer);
			if (wakeupFromStandby()) {
				CheckFastScan(true);
				standbyToStandby();
			} else if (mode == mode_standby) {
				fst_timer = g_RCInput->addTimer(30*1000*1000, true);
			}
			return messages_return::handled;
		}
#endif
	}
	if (msg == NeutrinoMessages::SHOW_MAINMENU) {
		showMainMenu();
		return messages_return::handled;
	}


	res = res | g_RemoteControl->handleMsg(msg, data);
	res = res | g_InfoViewer->handleMsg(msg, data);
	if (channelList) /* can be not yet ready during startup */
		res = res | channelList->handleMsg(msg, data);
	res = res | CRecordManager::getInstance()->handleMsg(msg, data);
	res = res | CEpgScan::getInstance()->handleMsg(msg, data);
	res = res | CHDDMenuHandler::getInstance()->handleMsg(msg, data);

	if( res != messages_return::unhandled ) {
		if( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData+ 0x10000000 ) ) {
			delete[] (unsigned char*) data;
		}
		return( res & ( 0xFFFFFFFF - messages_return::unhandled ) );
	}

#if !HAVE_SPARK_HARDWARE
	/* we assume g_CamHandler free/delete data if needed */
	res = g_CamHandler->handleMsg(msg, data);
	if( res != messages_return::unhandled ) {
		return(res & (0xFFFFFFFF - messages_return::unhandled));
	}
#endif

	/* ================================== KEYS ================================================ */
	if( msg == CRCInput::RC_ok || (!g_InfoViewer->getSwitchMode() && (msg == CRCInput::RC_sat || msg == CRCInput::RC_favorites || msg == CRCInput::RC_www))) {
		if( (mode == mode_tv) || (mode == mode_radio) || (mode == mode_ts) || (mode == mode_webtv)) {
			showChannelList(msg);
			return messages_return::handled;
		}
	}
	else if (msg == CRCInput::RC_standby_on) {
		if (data == 0)
			g_RCInput->postMsg(NeutrinoMessages::STANDBY_ON, 0);
		return messages_return::cancel_all | messages_return::handled;
	}
	else if ((msg == CRCInput::RC_standby_off) || (msg == CRCInput::RC_power_on)) {
		if (data == 0)
			g_RCInput->postMsg(NeutrinoMessages::STANDBY_OFF, 0);
		return messages_return::handled;
	}
	else if (msg == CRCInput::RC_power_off) {
		g_RCInput->postMsg(NeutrinoMessages::SHUTDOWN, 0);
		return messages_return::cancel_all | messages_return::handled;
	}
	else if ((msg == CRCInput::RC_tv) || (msg == CRCInput::RC_radio)) {
		if (data == 0)
			g_RCInput->postMsg(NeutrinoMessages::LEAVE_ALL, 0);
		return messages_return::cancel_all | messages_return::handled;
	}
	else if (msg == (neutrino_msg_t) g_settings.key_power_off /*CRCInput::RC_standby*/) {
		if (data == 0) {
			neutrino_msg_t new_msg;

			/* Note: pressing the power button on the dbox (not the remote control) over 1 second */
			/*       shuts down the system even if !g_settings.shutdown_real_rcdelay (see below)  */
			gettimeofday(&standby_pressed_at, NULL);

			if ((mode != mode_standby) && (g_settings.shutdown_real)) {
				CRecordManager::getInstance()->StopAutoRecord();
				if(CRecordManager::getInstance()->RecordingStatus()) {
					new_msg = NeutrinoMessages::STANDBY_ON;
					CTimerManager::getInstance()->wakeup = true;
					g_RCInput->firstKey = false;
				} else
					new_msg = NeutrinoMessages::SHUTDOWN;
			}
			else {
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
				if((mode != mode_standby) && (g_settings.shutdown_real) && recordingstatus)
					timer_wakeup = true;
#endif
				new_msg = (mode == mode_standby) ? NeutrinoMessages::STANDBY_OFF : NeutrinoMessages::STANDBY_ON;
				//printf("standby: new msg %X\n", new_msg);
				if ((g_settings.shutdown_real_rcdelay)) {
					neutrino_msg_t      _msg_;
					neutrino_msg_data_t mdata;
					struct timeval      endtime;
					time_t              seconds;

					int timeout = g_settings.repeat_blocker;
					int timeout1 = g_settings.repeat_genericblocker;

					if (timeout1 > timeout)
						timeout = timeout1;

					timeout += 500;
					//printf("standby: timeout %d\n", timeout);

					while(true) {
						g_RCInput->getMsg_ms(&_msg_, &mdata, timeout);

						//printf("standby: input msg %X\n", msg);
						if (_msg_ == CRCInput::RC_timeout)
							break;

						gettimeofday(&endtime, NULL);
						seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
						if (endtime.tv_usec < standby_pressed_at.tv_usec)
							seconds--;
						//printf("standby: input seconds %d\n", seconds);
						if (seconds >= 1) {
							if (_msg_ == CRCInput::RC_standby)
								new_msg = NeutrinoMessages::SHUTDOWN;
							break;
						}
					}
				}
			}
			g_RCInput->postMsg(new_msg, 0);
			return messages_return::cancel_all | messages_return::handled;
		}
		return messages_return::handled;
#if 0
		else  /* data == 1: KEY_POWER released                         */
			if (standby_pressed_at.tv_sec != 0) /* check if we received a KEY_POWER pressed event before */
			{                                   /* required for correct handling of KEY_POWER events of  */
				/* the power button on the dbox (not the remote control) */
				struct timeval endtime;
				gettimeofday(&endtime, NULL);
				time_t seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
				if (endtime.tv_usec < standby_pressed_at.tv_usec)
					seconds--;
				if (seconds >= 1) {
					g_RCInput->postMsg(NeutrinoMessages::SHUTDOWN, 0);
					return messages_return::cancel_all | messages_return::handled;
				}
			}
#endif
	}
	else if ((msg == CRCInput::RC_plus) || (msg == CRCInput::RC_minus))
	{
		g_volume->setVolume(msg);
#if HAVE_DUCKBOX_HARDWARE
		if((mode == mode_tv) || (mode == mode_radio)) {
			CVFD::getInstance()->showServicename(channelList->getActiveChannelName());
		}
#endif
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_spkr ) {
		if( mode == mode_standby ) {
			//switch lcd off/on
			CVFD::getInstance()->togglePower();
		}
		else {
			//mute
			g_audioMute->AudioMute(!current_muted, true);
		}
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_mute_on ) {
		g_audioMute->AudioMute(true, true);
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_mute_off ) {
		g_audioMute->AudioMute(false, true);
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_analog_on ) {
		g_settings.analog_out = 1;
		audioDecoder->EnableAnalogOut(true);
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_analog_off ) {
		g_settings.analog_out = 0;
		audioDecoder->EnableAnalogOut(false);
		return messages_return::handled;
	}
	else if(( msg == CRCInput::RC_mode ) && g_settings.key_format_mode_active ) {
		g_videoSettings->nextMode();
		return messages_return::handled;
	}
	else if(( msg == (neutrino_msg_t) g_settings.key_next43mode ) && g_settings.key_pic_size_active ) {
		g_videoSettings->next43Mode();
		return messages_return::handled;
	}
	else if(( msg == (neutrino_msg_t) g_settings.key_switchformat) && g_settings.key_pic_mode_active ) {
		g_videoSettings->SwitchFormat();
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_sleep ) {
		CSleepTimerWidget *sleepTimer = new CSleepTimerWidget;
		sleepTimer->exec(NULL, "");
		delete sleepTimer;
		return messages_return::handled;
	}
#if 0
	else if (msg == (neutrino_msg_t) g_settings.key_screenshot) {
		char shotname[80];
		time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		strftime(shotname, sizeof(shotname), "/screenshot-%Y%m%d%H%M%S.png", tm);
		CVFD::getInstance()->ShowText("screenshot");
		frameBuffer->OSDShot(g_settings.screenshot_dir + string(shotname));
		CVFD::getInstance()->ShowText("done");
		return messages_return::handled;
	}
#endif
	else if (msg == (neutrino_msg_t) g_settings.key_screenshot) {
		//video+osd scaled to osd size
		CScreenShot * sc = new CScreenShot("", (CScreenShot::screenshot_format_t)g_settings.screenshot_format);
		sc->EnableOSD(true);
		sc->MakeFileName(CZapit::getInstance()->GetCurrentChannelID());
		sc->Start();
	}

	/* ================================== MESSAGES ================================================ */
	else if (msg == NeutrinoMessages::EVT_VOLCHANGED) {
		//setVolume(msg, false, true);
		return messages_return::handled;
	}
#ifdef HAVE_CONTROLD
	else if( msg == NeutrinoMessages::EVT_VCRCHANGED ) {
		if (g_settings.vcr_AutoSwitch) {
			if( data != VCR_STATUS_OFF )
				g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
			else
				g_RCInput->postMsg( NeutrinoMessages::VCR_OFF, 0 );
		}
		return messages_return::handled | messages_return::cancel_info;
	}
#endif
	else if( msg == NeutrinoMessages::EVT_MUTECHANGED ) {
		//FIXME unused ?
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_SERVICESCHANGED ) {
		printf("NeutrinoMessages::EVT_SERVICESCHANGED\n");fflush(stdout);
		channelsInit();
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		adjustToChannelID(live_channel_id);//FIXME what if deleted ?
		if(old_b_id >= 0) {
			bouquetList->activateBouquet(old_b_id, false);
			old_b_id = -1;
			g_RCInput->postMsg(CRCInput::RC_ok, 0);
		}
	}
	else if( msg == NeutrinoMessages::EVT_BOUQUETSCHANGED ) {
		printf("NeutrinoMessages::EVT_BOUQUETSCHANGED\n");fflush(stdout);
		channelsInit();
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		adjustToChannelID(live_channel_id);//FIXME what if deleted ?
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_RECORDMODE ) {
		/* sent by rcinput, when got msg from zapit about record activated/deactivated */
		/* should be sent when no record running */
		printf("NeutrinoMessages::EVT_RECORDMODE: %s\n", ( data ) ? "on" : "off");
		recordingstatus = data;
		CEpgScan::getInstance()->Next();
		standbyToStandby();
		autoshift = CRecordManager::getInstance()->TimeshiftOnly();
		CVFD::getInstance()->ShowIcon(FP_ICON_CAM1, recordingstatus != 0);

		if( ( !g_InfoViewer->is_visible ) && data && !autoshift)
			g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::RECORD_START) {
		//FIXME better at announce ?
		wakeupFromStandby();
#if HAVE_DUCKBOX_HARDWARE
		CVFD::getInstance()->ShowIcon(FP_ICON_REC, true);
#endif
#if 0
		//zap to rec channel if box start from deepstandby
		if(timer_wakeup){
			timer_wakeup=false;
			dvbsub_stop();
			CTimerd::RecordingInfo * eventinfo = (CTimerd::RecordingInfo *) data;
			t_channel_id channel_id=eventinfo->channel_id;
			g_Zapit->zapTo_serviceID_NOWAIT(channel_id);
		}
#endif
		//zap to rec channel in standby-mode
		CTimerd::RecordingInfo * eventinfo = (CTimerd::RecordingInfo *) data;
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		/* special case for nhttpd: start direct record, if no eventID */
		if (eventinfo->eventID == 0) {
			int rec_mode = CRecordManager::getInstance()->GetRecordMode(live_channel_id);
			/* start only if not recorded yet */
			if (rec_mode == CRecordManager::RECMODE_OFF || rec_mode == CRecordManager::RECMODE_TSHIFT)
				CRecordManager::getInstance()->Record(live_channel_id);
			delete[] (unsigned char*) data;
			return messages_return::handled | messages_return::cancel_all;
		}
		if(mode == mode_standby){
			if((eventinfo->channel_id != live_channel_id) && !(SAME_TRANSPONDER(live_channel_id, eventinfo->channel_id)))
				zapTo(eventinfo->channel_id);
		}

		if (g_settings.recording_type != CNeutrinoApp::RECORDING_OFF) {
			CRecordManager::getInstance()->Record(eventinfo);
			autoshift = CRecordManager::getInstance()->TimeshiftOnly();
		}

		delete[] (unsigned char*) data;
		return messages_return::handled | messages_return::cancel_all;
	}
	else if( msg == NeutrinoMessages::RECORD_STOP) {
#if HAVE_DUCKBOX_HARDWARE
		CVFD::getInstance()->ShowIcon(FP_ICON_REC, false);
#endif
		CTimerd::RecordingStopInfo* recinfo = (CTimerd::RecordingStopInfo*)data;
		printf("NeutrinoMessages::RECORD_STOP: eventID %d channel_id %" PRIx64 "\n", recinfo->eventID, recinfo->channel_id);
		CRecordManager::getInstance()->Stop(recinfo);
		autoshift = CRecordManager::getInstance()->TimeshiftOnly();

		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_STREAM_START) {
		int fd = (int) data;
		printf("NeutrinoMessages::EVT_STREAM_START: fd %d\n", fd);
		wakeupFromStandby();
		if (g_Radiotext)
			g_Radiotext->setPid(0);

		if (!CStreamManager::getInstance()->AddClient(fd)) {
			close(fd);
			g_RCInput->postMsg(NeutrinoMessages::EVT_STREAM_STOP, 0);
		}
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_STREAM_STOP) {
		printf("NeutrinoMessages::EVT_STREAM_STOP\n");
		CEpgScan::getInstance()->Next();
		standbyToStandby();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_PMT_CHANGED) {
		t_channel_id channel_id = *(t_channel_id*) data;
		CRecordManager::getInstance()->Update(channel_id);
		return messages_return::handled;
	}

	else if( msg == NeutrinoMessages::ZAPTO) {
		CTimerd::EventInfo * eventinfo = (CTimerd::EventInfo *) data;
		if (eventinfo->channel_id != CZapit::getInstance()->GetCurrentChannelID()){
			if( (recordingstatus == 0) || (recordingstatus && CRecordManager::getInstance()->TimeshiftOnly()) ||
					(recordingstatus && channelList->SameTP(eventinfo->channel_id)) ) {
				bool isTVMode = CServiceManager::getInstance()->IsChannelTVChannel(eventinfo->channel_id);

				dvbsub_stop();

				if ((!isTVMode) && (mode != mode_radio)) {
					radioMode(true);
				}
				else if (isTVMode && (mode != mode_tv) && (mode != mode_webtv)) {
					tvMode(true);
				}
				channelList->zapTo_ChannelID(eventinfo->channel_id);
			}
		}
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_ZAPTO) {
		if( mode == mode_standby ) {
			standbyMode( false );
		}
		if( mode != mode_scart ) {
			CTimerd::RecordingInfo * eventinfo = (CTimerd::RecordingInfo *) data;
			std::string name = g_Locale->getText(LOCALE_ZAPTOTIMER_ANNOUNCE);
			getAnnounceEpgName( eventinfo, name);
			ShowHint( LOCALE_MESSAGEBOX_INFO, name.c_str() );
		}
		delete [] (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_RECORD) {
		my_system(NEUTRINO_RECORDING_TIMER_SCRIPT);
		CTimerd::RecordingInfo * eventinfo = (CTimerd::RecordingInfo *) data;
		if (g_settings.recording_type == RECORDING_FILE) {
			char * recordingDir = eventinfo->recordingDir;
			for(int i=0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) {
				if (g_settings.network_nfs[i].local_dir == recordingDir) {
					printf("[neutrino] waking up %s (%s)\n", g_settings.network_nfs[i].ip.c_str(), recordingDir);
					if (my_system(2, "ether-wake", g_settings.network_nfs[i].mac.c_str()) != 0)
						perror("ether-wake failed");
					break;
				}
			}
			wakeup_hdd(recordingDir);
			if (my_system(2, "wakeup_hdd", recordingDir) != 0)
					perror("wakeup_hdd failed");
		}

		if( g_settings.recording_zap_on_announce && (mode != mode_standby) && (eventinfo->channel_id != CZapit::getInstance()->GetCurrentChannelID())) {
			CRecordManager::getInstance()->StopAutoRecord();
			zapTo(eventinfo->channel_id);
		}
		if(( mode != mode_scart ) && ( mode != mode_standby ) && g_settings.recording_startstop_msg) {
			std::string name = g_Locale->getText(LOCALE_RECORDTIMER_ANNOUNCE);
			getAnnounceEpgName(eventinfo, name);
			ShowHint(LOCALE_MESSAGEBOX_INFO, name.c_str());
		}
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SLEEPTIMER) {
		if( mode != mode_scart && mode != mode_standby)
			skipSleepTimer = (ShowMsg(LOCALE_MESSAGEBOX_INFO, g_settings.shutdown_real ? LOCALE_SHUTDOWNTIMER_ANNOUNCE:LOCALE_SLEEPTIMERBOX_ANNOUNCE,CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes);
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::SLEEPTIMER) {
		if(data) {//INACTIVITY SLEEPTIMER
			skipShutdownTimer =
				(ShowMsg(LOCALE_MESSAGEBOX_INFO, g_settings.shutdown_real ? LOCALE_SHUTDOWNTIMER_ANNOUNCE:LOCALE_SLEEPTIMERBOX_ANNOUNCE,
				      CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes);//FIXME
			if(skipShutdownTimer) {
				printf("NeutrinoMessages::INACTIVITY SLEEPTIMER: skiping\n");
				skipShutdownTimer = false;
				return messages_return::handled;
			}
		}else{ //MAIN-MENU SLEEPTIMER
			if(skipSleepTimer) {
				printf("NeutrinoMessages::SLEEPTIMER: skiping\n");
				skipSleepTimer = false;
				return messages_return::handled;
			}
		}
		if (g_settings.shutdown_real && can_deepstandby)
			g_RCInput->postMsg(NeutrinoMessages::SHUTDOWN, 0);
		else
			g_RCInput->postMsg(NeutrinoMessages::STANDBY_ON, 0);
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::RELOAD_SETUP ) {
		bool tmp1 = g_settings.make_hd_list;
		bool tmp2 = g_settings.make_webtv_list;
		loadSetup(NEUTRINO_SETTINGS_FILE);
		if(tmp1 != g_settings.make_hd_list || tmp2 != g_settings.make_webtv_list)
			g_Zapit->reinitChannels();

		SendSectionsdConfig();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_TOGGLE ) {
		standbyMode( !(mode & mode_standby) );
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::LEAVE_ALL ) {
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_ON ) {
		if( mode != mode_standby ) {
			standbyMode( true );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_OFF ) {
		if( mode == mode_standby ) {
			standbyMode( false );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SHUTDOWN) {
		if( mode != mode_scart )
			skipShutdownTimer = (ShowMsg(LOCALE_MESSAGEBOX_INFO, LOCALE_SHUTDOWNTIMER_ANNOUNCE, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 5) == CMessageBox::mbrYes);
	}
	else if( msg == NeutrinoMessages::SHUTDOWN ) {
		if(!skipShutdownTimer) {
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
			timer_wakeup = true;
			ExitRun(true, !can_deepstandby);
#else
			ExitRun(true, (cs_get_revision() > 7));
#endif
		}
		else {
			skipShutdownTimer=false;
		}
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::REBOOT ) {
		FILE *f = fopen("/tmp/.reboot", "w");
		fclose(f);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		ExitRun(true, CNeutrinoApp::REBOOT);
#else
		ExitRun(true);
#endif
	}
	else if (msg == NeutrinoMessages::EVT_POPUP || msg == NeutrinoMessages::EVT_EXTMSG) {
		if (mode != mode_scart && mode != mode_standby) {
			std::string timeout="-1";
			std::string text = (char*)data;
			std::string::size_type pos;

			pos = text.find("&timeout=", 0);
			if (pos != std::string::npos) {
				timeout = text.substr( pos+9, text.length()+1 );
				text[pos] = '\0';
			}

			if (msg == NeutrinoMessages::EVT_POPUP)
				ShowHint(LOCALE_MESSAGEBOX_INFO, text.c_str(), 0, atoi(timeout.c_str()));
			else if (msg == NeutrinoMessages::EVT_EXTMSG)
				ShowMsg(LOCALE_MESSAGEBOX_INFO, text, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO, 0, atoi(timeout.c_str()));

		}
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_RECORDING_ENDED) {
		/* FIXME TODO, when/if needed, monitor record status somewhere
		 * and report possible error to user if any with this message ?
		 * not used/not supported for now */
		//delete[] (unsigned char*) data;

		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::REMIND) {
		std::string text = (char*)data;
		std::string::size_type pos;
		while((pos=text.find('/'))!= std::string::npos)
		{
			text[pos] = '\n';
		}
		if( mode != mode_scart )
			ShowMsg(LOCALE_TIMERLIST_TYPE_REMIND, text, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO); // UTF-8
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::LOCK_RC)
	{
		CRCLock::getInstance()->exec(NULL, CRCLock::NO_USER_INPUT);
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::CHANGEMODE ) {
		printf("CNeutrinoApp::handleMsg: CHANGEMODE to %d rezap %d\n", (int)(data & mode_mask), (data & norezap) != norezap);
		if((data & mode_mask)== mode_radio) {
			if( mode != mode_radio ) {
				radioMode((data & norezap) != norezap);
			}
		}
		if((data & mode_mask)== mode_tv) {
			if( mode != mode_tv ) {
				tvMode((data & norezap) != norezap);
			}
		}
		if((data & mode_mask)== mode_standby) {
			if(mode != mode_standby)
				standbyMode( true );
		}
		if((data & mode_mask)== mode_upnp) {
			lastMode=mode;
			mode=mode_upnp;
		}
		if((data & mode_mask)== mode_audio) {
			lastMode=mode;
			mode=mode_audio;
		}
		if((data & mode_mask)== mode_pic) {
			lastMode=mode;
			mode=mode_pic;
		}
		if((data & mode_mask)== mode_ts) {
			if(mode == mode_radio)
				frameBuffer->stopFrame();
			lastMode=mode;
			mode=mode_ts;
		}
		if((data & mode_mask)== mode_webtv) {
			lastMode=mode;
			mode=mode_webtv;
			if ((data & norezap) != norezap) {
				CZapitChannel * cc = CZapit::getInstance()->GetCurrentChannel();
				if (cc && IS_WEBTV(cc->getChannelID())) {
					CMoviePlayerGui::getInstance().stopPlayBack();
					if (!CMoviePlayerGui::getInstance().PlayBackgroundStart(cc->getUrl(), cc->getName(), cc->getChannelID(), cc->getScriptName()))
						g_RCInput->postMsg(NeutrinoMessages::EVT_ZAP_FAILED, data);
				}
			}
		}
	}
	else if( msg == NeutrinoMessages::VCR_ON ) {
		if( mode != mode_scart ) {
			scartMode( true );
		}
		else
			CVFD::getInstance()->setMode(CVFD::MODE_SCART);
	}

	else if( msg == NeutrinoMessages::VCR_OFF ) {
		if( mode == mode_scart ) {
			scartMode( false );
		}
	}
	else if (msg == NeutrinoMessages::EVT_START_PLUGIN) {
		g_PluginList->startPlugin((const char *)data);
		if (!g_PluginList->getScriptOutput().empty()) {
			ShowMsg(LOCALE_PLUGINS_RESULT, g_PluginList->getScriptOutput(), CMessageBox::mbrBack,CMessageBox::mbBack,NEUTRINO_ICON_SHELL);
		}

		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_SERVICES_UPD) {
		SDTreloadChannels = true;
		g_InfoViewer->SDT_freq_update = true;
		if( !g_InfoViewer->is_visible && !autoshift){
			g_RCInput->postMsg(NeutrinoMessages::SHOW_INFOBAR , 0);
		}
		return messages_return::handled;
//		ShowHint(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_EXTRA_ZAPIT_SDT_CHANGED),
//				CMessageBox::mbrBack,CMessageBox::mbBack, NEUTRINO_ICON_INFO);
	}
#if !HAVE_SPARK_HARDWARE && !HAVE_DUCKBOX_HARDWARE
	else if (msg == NeutrinoMessages::EVT_HDMI_CEC_VIEW_ON) {
		if(g_settings.hdmi_cec_view_on)
			videoDecoder->SetCECAutoView(g_settings.hdmi_cec_view_on);

		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_HDMI_CEC_STANDBY) {
		if(g_settings.hdmi_cec_standby)
			  videoDecoder->SetCECAutoStandby(g_settings.hdmi_cec_standby);

		return messages_return::handled;
	}
#endif
	else if (msg == NeutrinoMessages::EVT_SET_MUTE) {
		g_audioMute->AudioMute((int)data, true);
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_SET_VOLUME) {
		g_volume->setVolumeExt((int)data);
		return messages_return::handled;
	}
	if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000)) {
		INFO("###################################### DELETED msg %lx data %lx\n", msg, data);
		delete [] (unsigned char*) data;
		return messages_return::handled;
	}
	return messages_return::unhandled;
}

extern time_t timer_minutes;//timermanager.cpp
extern bool timer_is_rec;//timermanager.cpp

void CNeutrinoApp::ExitRun(const bool /*write_si*/, int retcode)
{
	printf("[neutrino] %s retcode: %d can_deep: %d\n", __func__, retcode, can_deepstandby);
	bool do_shutdown = true;

	CRecordManager::getInstance()->StopAutoRecord();
	if(CRecordManager::getInstance()->RecordingStatus() /*|| cYTCache::getInstance()->isActive()*/) {
		do_shutdown =
			(ShowMsg(LOCALE_MESSAGEBOX_INFO, LOCALE_SHUTDOWN_RECORDING_QUERY, CMessageBox::mbrNo,
					CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes);
	}

	if(do_shutdown) {
		if(SDTreloadChannels){
			SDT_ReloadChannels();
			//SDTreloadChannels = false;
		}


		dprintf(DEBUG_INFO, "exit\n");
		StopSubtitles();
		stopPlayBack();

		frameBuffer->paintBackground();
		frameBuffer->showFrame("shutdown.jpg");

		//delete cHddStat::getInstance();
		delete CRecordManager::getInstance();

		CEpgScan::getInstance()->Stop();
		if(g_settings.epg_save /* && timeset && g_Sectionsd->getIsTimeSet ()*/) {
			g_Sectionsd->setPauseScanning(true);
			saveEpg(true);// true CVFD::MODE_SHUTDOWN
		}

#if ENABLE_FASTSCAN
		/* on shutdown force load new fst */
		if (retcode)
			CheckFastScan(true, false);
#endif

		CVFD::getInstance()->setMode(CVFD::MODE_SHUTDOWN);

		stop_daemons(true /*retcode*/);//need here for timer_is_rec before saveSetup
		g_settings.shutdown_timer_record_type = timer_is_rec;
		saveSetup(NEUTRINO_SETTINGS_FILE);

#if HAVE_COOL_HARDWARE
		if(retcode) {
#endif
			puts("[neutrino.cpp] executing " NEUTRINO_ENTER_DEEPSTANDBY_SCRIPT ".");
			if (my_system(NEUTRINO_ENTER_DEEPSTANDBY_SCRIPT) != 0)
				perror(NEUTRINO_ENTER_DEEPSTANDBY_SCRIPT " failed");

			printf("entering off state\n");
			printf("timer_minutes: %ld\n", timer_minutes);
			mode = mode_off;
			//CVFD::getInstance()->ShowText(g_Locale->getText(LOCALE_MAINMENU_SHUTDOWN));

#if HAVE_COOL_HARDWARE
			{
				fp_standby_data_t standby;
				time_t mtime = time(NULL);
				struct tm *tmtime = localtime(&mtime);
				time_t fp_timer = 0;

				if(timer_minutes) {
					fp_timer = timer_minutes - mtime/60;
					if(fp_timer < 1)
						fp_timer = 1;
				}
				printf("now: %ld, timer %ld, FP timer %ldmin\n", mtime/60, timer_minutes, fp_timer);fflush(stdout);
				int leds = 0x40;
				switch(g_settings.led_deep_mode){
					case 0:
						leds = 0x0;//off  leds
						break;
					case 1:
						leds = 0x60;//on led1 & 2
						break;
					case 2:
						leds = 0x20;//led1 on , 2 off
						break;
					case 3:
						leds = 0x40;//led2 off, 2 on
						break;
					default:
						break;
				}
				if(leds && g_settings.led_blink && fp_timer)
					leds |= 0x80;

				standby.brightness          = cs_get_revision() == 10 ? 0 : g_settings.lcd_setting[SNeutrinoSettings::LCD_DEEPSTANDBY_BRIGHTNESS];
				standby.flags               = leds;
				standby.current_hour        = tmtime->tm_hour;
				standby.current_minute      = tmtime->tm_min;
				standby.timer_minutes_hi    = fp_timer >> 8;;
				standby.timer_minutes_lo    = fp_timer & 0xFF;

				my_system("/etc/init.d/rcK");
				sync();
				my_system(2,"/bin/umount", "-a");
				sleep(1);

				stop_video();

				int fd = open("/dev/display", O_RDONLY);
				if (fd < 0) {
					perror("/dev/display");
					reboot(LINUX_REBOOT_CMD_RESTART);
				} else {

					if (ioctl(fd, IOC_FP_STANDBY, (fp_standby_data_t *)  &standby)) {
						perror("IOC_FP_STANDBY");
						reboot(LINUX_REBOOT_CMD_RESTART);
					} else {
						while(true) sleep(1);
					}
				}
			}
		} else {
#endif
			if (timer_minutes)
			{
				FILE *f = fopen("/tmp/.timer", "w");
				if (f)
				{
					fprintf(stderr, "timer_wakeup: %ld\n", timer_minutes * 60);
					fprintf(f, "%ld\n", timer_minutes * 60);
					fclose(f);
				}
				else
					perror("fopen /tmp/.timer");
			}

			delete g_RCInput;
			g_RCInput = NULL;
			
			if(LCD4l) {
				LCD4l->StopLCD4l();
				delete LCD4l;
			}
			LCD4l = NULL;
			//fan speed
			if (g_info.has_fan) {
				CFanControlNotifier::setSpeed(0);
			}
			//CVFD::getInstance()->ShowText(g_Locale->getText(LOCALE_MAINMENU_REBOOT));
			delete CVFD::getInstance();
			delete SHTDCNT::getInstance();
			stop_video();

#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
			if (retcode == SHUTDOWN) {
				CCECSetup cecsetup;
				cecsetup.setCECSettings(false);
			}
#endif
#ifdef ENABLE_GRAPHLCD
			if (retcode == SHUTDOWN)
				nGLCD::SetBrightness(0);
#endif

			printf("[neutrino] This is the end. exiting with code %d\n", retcode);
			Cleanup();
#ifdef __sh__
			/* the sh4 gcc seems to dislike someting about openthreads... */
			_exit(retcode);
#else
			exit(retcode);
#endif
#if HAVE_COOL_HARDWARE
		}
#endif
	}
}

void CNeutrinoApp::saveEpg(bool cvfd_mode)
{
	struct stat my_stat;
	if(stat(g_settings.epg_dir.c_str(), &my_stat) == 0){
		if(!cvfd_mode){//skip saveepg in standby mode, if last saveepg time < 15 Min.
			std::string index_xml = g_settings.epg_dir.c_str();
			index_xml += "/index.xml";
			time_t t=0;
			if(stat(index_xml.c_str(), &my_stat) == 0){
				if(difftime(time(&t), my_stat.st_ctime) < 900){
					return;
				}
			}
		}
		printf("[neutrino] Saving EPG to %s...\n", g_settings.epg_dir.c_str());

		CVFD::getInstance()->Clear();
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		CVFD::getInstance()->ShowText(g_Locale->getText(LOCALE_EPG_SAVING));

		g_Sectionsd->writeSI2XML(g_settings.epg_dir.c_str());

		neutrino_msg_t      msg;
		neutrino_msg_data_t data;
		while( true ) {
			g_RCInput->getMsg(&msg, &data, 1200); // 120 secs..
			if (( msg == CRCInput::RC_timeout ) || (msg == NeutrinoMessages::EVT_SI_FINISHED)) {
				//printf("Msg %x timeout %d EVT_SI_FINISHED %x\n", msg, CRCInput::RC_timeout, NeutrinoMessages::EVT_SI_FINISHED);
				CVFD::getInstance()->Clear();
				CVFD::getInstance()->setMode(cvfd_mode ? CVFD::MODE_SHUTDOWN : CVFD::MODE_STANDBY);// true CVFD::MODE_SHUTDOWN  , false CVFD::MODE_STANDBY
				break;
			} else if (!cvfd_mode){
				printf("wait for epg saving, Msg %x \n", (int) msg);
				handleMsg(msg, data);
			}
		}
	}
}

void CNeutrinoApp::tvMode( bool rezap )
{
	INFO("rezap %d current mode %d", rezap, mode);
	if (mode == mode_radio) {
		if (g_settings.radiotext_enable && g_Radiotext) {
			delete g_Radiotext;
			g_Radiotext = NULL;
		}

		frameBuffer->stopFrame();
		CVFD::getInstance()->ShowIcon(FP_ICON_RADIO, false);
		StartSubtitles(!rezap);
	}
	g_InfoViewer->setUpdateTimer(LCD_UPDATE_TIME_TV_MODE);

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	CVFD::getInstance()->ShowIcon(FP_ICON_TV, true);

	if( mode == mode_standby ) {
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		videoDecoder->Standby(false);
	}

#ifdef ENABLE_PIP
	pipDecoder->Pig(g_settings.pip_x, g_settings.pip_y,
			g_settings.pip_width, g_settings.pip_height,
			frameBuffer->getScreenWidth(true), frameBuffer->getScreenHeight(true));
#endif
#if 0
	if(mode != mode_ts /*&& autoshift*/) {
		//printf("standby on: autoshift ! stopping ...\n");
		CRecordManager::getInstance()->StopAutoRecord();
	}
#endif
	if (mode != mode_webtv) {
		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();
	}
	mode = mode_tv;

	g_RemoteControl->tvMode();
	SetChannelMode(g_settings.channel_mode);
	if( rezap )
		channelRezap();
#ifdef USEACTIONLOG
	g_ActionLog->println("mode: tv");
#endif
}

void CNeutrinoApp::scartMode( bool bOnOff )
{
	//printf( ( bOnOff ) ? "mode: scart on\n" : "mode: scart off\n" );

	if( bOnOff ) {
		// SCART AN
		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();

		//g_Controld->setScartMode( 1 );
#if HAVE_TRIPLEDRAGON
		/* would this hurt on Coolstream? */
		videoDecoder->Stop(true);
		videoDecoder->Standby(true);
#endif
		CVFD::getInstance()->setMode(CVFD::MODE_SCART);
		lastMode = mode;
		mode = mode_scart;
	} else {
		// SCART AUS
		//g_Controld->setScartMode( 0 );
#if HAVE_TRIPLEDRAGON
		/* could actually go into radioMode() and tvMode()? */
		videoDecoder->Standby(false);
		videoDecoder->Start();
#endif

		mode = mode_unknown;
		//re-set mode
		if( lastMode == mode_radio ) {
			radioMode( false );
		}
		else if( lastMode == mode_tv || lastMode == mode_webtv) {
			tvMode( false );
		}
		else if( lastMode == mode_standby ) {
			standbyMode( true );
		}
	}
}

void CNeutrinoApp::standbyMode( bool bOnOff, bool fromDeepStandby )
{
	//static bool wasshift = false;
	INFO("%s", bOnOff ? "ON" : "OFF" );

	if(lockStandbyCall)
		return;

	lockStandbyCall = true;

	if( bOnOff ) {
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		CCECSetup cecsetup;
		cecsetup.setCECSettings(false);
#endif
#ifdef ENABLE_GRAPHLCD
		nGLCD::StandbyMode(true);
#endif
		CVFD::getInstance()->ShowText("standby...");
		if( mode == mode_scart ) {
			//g_Controld->setScartMode( 0 );
		}
		g_InfoViewer->setUpdateTimer(0); // delete timer
		StopSubtitles();
		if(SDTreloadChannels && !CRecordManager::getInstance()->RecordingStatus()){
			SDT_ReloadChannels();
			//SDTreloadChannels = false;
		}
		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();

		/* wasshift = */ CRecordManager::getInstance()->StopAutoRecord();

		if(mode == mode_radio && g_Radiotext)
			g_Radiotext->radiotext_stop();

#ifdef ENABLE_PIP
		g_Zapit->stopPip();
#endif
		CMoviePlayerGui::getInstance().stopPlayBack();
		bool stream_status = CStreamManager::getInstance()->StreamStatus();
		if((g_settings.epg_scan_mode == CEpgScan::MODE_OFF) && !fromDeepStandby &&
				!CRecordManager::getInstance()->RecordingStatus() && !stream_status) {
			g_Zapit->setStandby(true);
		} else {
			//g_Zapit->stopPlayBack();
			g_Zapit->lockPlayBack();
		}

		videoDecoder->Standby(true);

		g_Sectionsd->setServiceChanged(0, false);
		g_Sectionsd->setPauseScanning(!fromDeepStandby);

		lastMode = mode;
		mode = mode_standby;

		if(!CRecordManager::getInstance()->RecordingStatus() ) {
			//only save epg when not recording
			if(g_settings.epg_save && !fromDeepStandby && g_settings.epg_save_standby) {
				saveEpg(false);//false CVFD::MODE_STANDBY
			}
		}

		if(CVFD::getInstance()->getMode() != CVFD::MODE_STANDBY){
			CVFD::getInstance()->Clear();
			CVFD::getInstance()->setMode(CVFD::MODE_STANDBY);
		}
		CVFD::getInstance()->setBacklight(g_settings.backlight_standby);

		InfoClock->enableInfoClock(false);

		//remember tuned channel-id
		standby_channel_id = CZapit::getInstance()->GetCurrentChannelID();

		puts("[neutrino.cpp] executing " NEUTRINO_ENTER_STANDBY_SCRIPT ".");
		if (my_system(NEUTRINO_ENTER_STANDBY_SCRIPT) != 0)
			perror(NEUTRINO_ENTER_STANDBY_SCRIPT " failed");
		bool alive = recordingstatus || CEpgScan::getInstance()->Running() ||
			CStreamManager::getInstance()->StreamStatus();
		if(!alive)
			cpuFreq->SetCpuFreq(g_settings.standby_cpufreq * 1000 * 1000);

		//fan speed
		if (g_info.has_fan)
			CFanControlNotifier::setSpeed(1);

		frameBuffer->setActive(false);
		// Active standby on
		powerManager->SetStandby(false, false);
		CEpgScan::getInstance()->Start(true);
#if ENABLE_FASTSCAN
		if (scansettings.fst_update)
			fst_timer = g_RCInput->addTimer(30*1000*1000, true);
#endif
	} else {
		// Active standby off
		powerManager->SetStandby(false, false);
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		CVFD::getInstance()->ShowText("resume");
		cpuFreq->SetCpuFreq(g_settings.cpufreq * 1000 * 1000);
		videoDecoder->Standby(false);
		CEpgScan::getInstance()->Stop();
#if ENABLE_FASTSCAN
		g_RCInput->killTimer(fst_timer);
#endif

#ifdef ENABLE_GRAPHLCD
		nGLCD::StandbyMode(false);
#endif
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		if (!timer_wakeup) {
			CCECSetup cecsetup;
			cecsetup.setCECSettings(true);
		}
#else
		if(init_cec_setting){
			//init cec settings
			CCECSetup cecsetup;
			cecsetup.setCECSettings();
			init_cec_setting = false;
		}
#endif

#if !HAVE_SPARK_HARDWARE
		if(!recordingstatus && g_settings.ci_standby_reset) {
			g_CamHandler->exec(NULL, "ca_ci_reset0");
			g_CamHandler->exec(NULL, "ca_ci_reset1");
		}
#endif
		frameBuffer->setActive(true);
		//fan speed
		if (g_info.has_fan)
			CFanControlNotifier::setSpeed(g_settings.fan_speed);

		puts("[neutrino.cpp] executing " NEUTRINO_LEAVE_STANDBY_SCRIPT ".");
		if (my_system(NEUTRINO_LEAVE_STANDBY_SCRIPT) != 0)
			perror(NEUTRINO_LEAVE_STANDBY_SCRIPT " failed");

		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		CVFD::getInstance()->setBacklight(g_settings.backlight_tv);
		CVFD::getInstance()->showVolume(g_settings.current_volume, true);

		CZapit::getInstance()->EnablePlayback(true);
		g_Zapit->setStandby(false);
		/* the old code did:
		   if(was_record) g_Zapit->startPlayBack()
		   unfortunately this bypasses the parental PIN code check if a record timer
		   was set on a locked channel, then the box put in standby and after the
		   recording started, the box was woken up.
		   The channelList->setSelected(); channelList->zapTo_ChannelID() sequence
		   does trigger the PIN check
		   If the channel is the same (as during a recording), then it will only
		   check PIN and not zap, so we should be fine here
		 */
		mode = mode_unknown;
		if( lastMode == mode_radio ) {
			radioMode( false );
		} else {
			/* for standby -> tv mode from radio mode in case of record */
			frameBuffer->stopFrame();
			tvMode( false );
		}
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		if(!recordingstatus) { //only switch to standby_channel_id when not recording
			live_channel_id = standby_channel_id;
		}
		channelList->zapTo_ChannelID(live_channel_id, true); /* force re-zap */

		g_Sectionsd->setPauseScanning(false);

		InfoClock->enableInfoClock(true);

		g_audioMute->AudioMute(current_muted, true);
		StartSubtitles();
	}
	lockStandbyCall = false;
}

void CNeutrinoApp::radioMode( bool rezap)
{
	//printf("radioMode: rezap %s\n", rezap ? "yes" : "no");
	INFO("rezap %d current mode %d", rezap, mode);
	if (mode == mode_webtv) {
		CMoviePlayerGui::getInstance().setLastMode(mode_unknown);
		CMoviePlayerGui::getInstance().stopPlayBack();
		CVFD::getInstance()->ShowIcon(FP_ICON_TV, false);
	}
	if (mode == mode_tv) {
		CVFD::getInstance()->ShowIcon(FP_ICON_TV, false);
		StopSubtitles();
	}
	g_InfoViewer->setUpdateTimer(LCD_UPDATE_TIME_RADIO_MODE);
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	CVFD::getInstance()->ShowIcon(FP_ICON_RADIO, true);

	if( mode == mode_standby ) {
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		videoDecoder->Standby(false);
	}
	mode = mode_radio;
#ifdef ENABLE_PIP
	pipDecoder->Pig(g_settings.pip_radio_x, g_settings.pip_radio_y,
			g_settings.pip_radio_width, g_settings.pip_radio_height,
			frameBuffer->getScreenWidth(true), frameBuffer->getScreenHeight(true));
#endif
	CRecordManager::getInstance()->StopAutoRecord();

	g_RemoteControl->radioMode();
	SetChannelMode(g_settings.channel_mode_radio);

	if (g_settings.radiotext_enable && !g_Radiotext)
		g_Radiotext = new CRadioText;

	if( rezap )
		channelRezap();
	frameBuffer->showFrame("radiomode.jpg");
}

void CNeutrinoApp::channelRezap()
{
	t_channel_id last_chid = 0;
	if (mode == mode_tv)
		last_chid = CZapit::getInstance()->GetLastTVChannel();
	else if (mode == mode_radio)
		last_chid = CZapit::getInstance()->GetLastRADIOChannel();
	else
		return;

	if(CServiceManager::getInstance()->FindChannel(last_chid))
		channelList->zapTo_ChannelID(last_chid, true);
	else
		channelList->zapTo(0, true);
}

//switching from current mode to tv or radio mode or to optional parameter prev_mode
void CNeutrinoApp::switchTvRadioMode(const int prev_mode)
{
	if (prev_mode != mode_unknown){
		if (prev_mode == mode_tv && mode != mode_tv )
			tvMode();
		else if(prev_mode == mode_radio && mode != mode_radio)
			radioMode();
	} else {
		if (mode == mode_radio )
			tvMode();
		else if(mode == mode_tv || mode == mode_webtv)
			radioMode();
	}
}


/**************************************************************************************
*          CNeutrinoApp -  exec, menuitem callback (shutdown)                         *
**************************************************************************************/

int CNeutrinoApp::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//	printf("ac: %s\n", actionKey.c_str());
	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey == "help_recording") {
		ShowMsg(LOCALE_SETTINGS_HELP, LOCALE_RECORDINGMENU_HELP, CMessageBox::mbrBack, CMessageBox::mbBack);
	}
	else if(actionKey=="shutdown") {
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		ExitRun(true, CNeutrinoApp::SHUTDOWN);
#else
		ExitRun(true, 1);
#endif
	}
	else if(actionKey=="reboot")
	{
		FILE *f = fopen("/tmp/.reboot", "w");
		if (f)
			fclose(f);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		ExitRun(true, CNeutrinoApp::REBOOT);
#else
		ExitRun(true);
#endif
		unlink("/tmp/.reboot");
		returnval = menu_return::RETURN_NONE;
	}
	else if(actionKey=="changegui")
	{
		FILE *f = fopen("/var/etc/.e2", "w");
		if (f)
			fclose(f);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		ExitRun(true, CNeutrinoApp::CHANGEGUI);
#else
		ExitRun(true);
#endif
		returnval = menu_return::RETURN_NONE;
	}
	else if (actionKey=="clock_switch")
	{
		InfoClock->switchClockOnOff();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if (actionKey=="tv_radio_switch")//used in mainmenu
	{
		switchTvRadioMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if (actionKey=="tv")//used in mainmenu
	{
		switchTvRadioMode(mode_tv);
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if (actionKey=="radio")//used in mainmenu
	{
		switchTvRadioMode(mode_radio);
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="scart") {
		g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="savesettings") {
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();

		saveSetup(NEUTRINO_SETTINGS_FILE);

		if(g_settings.cacheTXT) {
			tuxtxt_init();
		} else
			tuxtxt_close();

		//g_Sectionsd->setEventsAreOldInMinutes((unsigned short) (g_settings.epg_old_hours*60));
		//g_Sectionsd->setHoursToCache((unsigned short) (g_settings.epg_cache_days*24));

		hintBox->hide();
		delete hintBox;
	}
	else if(actionKey=="recording") {
		setupRecordingDevice();
	}
	else if(actionKey=="reloadplugins") {
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SERVICEMENU_GETPLUGINS_HINT));
		hintBox->paint();

		g_PluginList->loadPlugins();

		hintBox->hide();
		delete hintBox;
	}
	else if(actionKey=="restarttuner") {
		CHintBox * hintBox = new CHintBox(LOCALE_SERVICEMENU_RESTART_TUNER,
			g_Locale->getText(LOCALE_SERVICEMENU_RESTARTING_TUNER));
		hintBox->paint();

		g_Zapit->setStandby(true);
		sleep(2);
		g_Zapit->setStandby(false);
		sleep(2);
		g_Zapit->Rezap();

		hintBox->hide();
		delete hintBox;
	}
	else if(actionKey=="nkplayback" /*|| actionKey=="ytplayback"*/ || actionKey=="tsmoviebrowser" || actionKey=="fileplayback") {
		frameBuffer->Clear();
		if(mode == NeutrinoMessages::mode_radio )
			frameBuffer->stopFrame();
		int _mode = mode;
		// FIXME CMediaPlayerMenu::getInstance()->exec(NULL, actionKey); ??
		CMoviePlayerGui::getInstance().exec(NULL, actionKey);
		if(_mode == NeutrinoMessages::mode_radio )
			frameBuffer->showFrame("radiomode.jpg");
#if 0
		else if (_mode == mode_webtv)
			tvMode(true);
#endif
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="audioplayer" || actionKey == "inetplayer") {
		frameBuffer->Clear();
		CMediaPlayerMenu * media = CMediaPlayerMenu::getInstance();
		media->exec(NULL, actionKey);
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="rass") {
		frameBuffer->Clear();
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		if (g_Radiotext)
			g_Radiotext->RASS_interactive_mode();
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey=="restart") {
		//usage of slots from any classes
		OnBeforeRestart();

		if (recordingstatus)
			DisplayErrorMessage(g_Locale->getText(LOCALE_SERVICEMENU_RESTART_REFUSED_RECORDING));
		else {
			CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SERVICEMENU_RESTART_HINT));
			hintBox->paint();

			saveSetup(NEUTRINO_SETTINGS_FILE);

			/* this is an ugly mess :-( */
			delete g_RCInput;
			delete g_Sectionsd;
			delete g_RemoteControl;
			delete g_fontRenderer;
			delete g_dynFontRenderer;

			delete hintBox;

			stop_daemons(true);
			stop_video();
			/* g_Timerd, g_Zapit and CVFD are used in stop_daemons */
			delete g_Timerd;
			delete g_Zapit; //do we really need this?
			delete CVFD::getInstance();
			delete SHTDCNT::getInstance();

			for(int i = 3; i < 256; i++)
				close(i);
			execvp(global_argv[0], global_argv); // no return if successful
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
			exit(CNeutrinoApp::REBOOT);
#else
			exit(1);
#endif
		}
	}
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	else if(actionKey == "3dmode") {
		C3DSetup::getInstance()->exec(parent, "");
		return menu_return::RETURN_EXIT_ALL;
	}
#endif
	else if(actionKey == "moviedir") {
		parent->hide();

		chooserDir(g_settings.network_nfs_moviedir, false, NULL);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "clearSectionsd")
	{
		g_Sectionsd->freeMemory();
	}
	else if(actionKey == "channels")
		return showChannelList(CRCInput::RC_ok, true);
	else if(actionKey == "standby")
	{
		g_RCInput->postMsg(NeutrinoMessages::STANDBY_ON, 0);
		return menu_return::RETURN_EXIT_ALL;
	}

	return returnval;
}

/**************************************************************************************
*          changeNotify - features menu recording start / stop                        *
**************************************************************************************/
bool CNeutrinoApp::changeNotify(const neutrino_locale_t OptionName, void * /*data*/)
{
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_LANGUAGESETUP_SELECT))
	{
		g_Locale->loadLocale(g_settings.language.c_str());
		return true;
	}
	return false;
}

void CNeutrinoApp::stopDaemonsForFlash()
{
	stop_daemons(false, true);
}

/**************************************************************************************
*          Main programm - no function here                                           *
**************************************************************************************/
void stop_daemons(bool stopall, bool for_flash)
{
	CMoviePlayerGui::getInstance().stopPlayBack();
	if (for_flash) {
		CVFD::getInstance()->Clear();
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		CVFD::getInstance()->ShowText("Stop daemons...");
		g_settings.epg_scan_mode = CEpgScan::MODE_OFF;
		my_system(NEUTRINO_ENTER_FLASH_SCRIPT);
	}

	InfoClock->enableInfoClock(false);
	dvbsub_close();
	tuxtxt_stop();
	tuxtxt_close();

#ifdef ENABLE_GRAPHLCD
	nGLCD::Exit();
#endif
	if (g_Radiotext) {
		delete g_Radiotext;
		g_Radiotext = NULL;
	}
	printf("streaming shutdown\n");
	CStreamManager::getInstance()->Stop();
	printf("streaming shutdown done\n");
	if(stopall || for_flash) {
		printf("timerd shutdown\n");
		if (g_Timerd)
			g_Timerd->shutdown();
		if (timerd_thread_started)
			pthread_join(timer_thread, NULL);
		printf("timerd shutdown done\n");
	}
#ifndef DISABLE_SECTIONSD
	printf("sectionsd shutdown\n");
	CEitManager::getInstance()->Stop();
	printf("sectionsd shutdown done\n");
#endif
	tuxtx_stop_subtitle();
	printf("zapit shutdown\n");
	if(!for_flash && !stopall && g_settings.hdmi_cec_mode && g_settings.hdmi_cec_standby){
	  	videoDecoder->SetCECMode((VIDEO_HDMI_CEC_MODE)0);
	}

	delete &CMoviePlayerGui::getInstance();

	CZapit::getInstance()->Stop();
	printf("zapit shutdown done\n");
	if (!for_flash) {
		CVFD::getInstance()->Clear();
		CVFD::getInstance()->setBacklight(g_settings.backlight_deepstandby);
	}
	if(stopall && !for_flash) {
		if (cpuFreq) {
			cpuFreq->SetCpuFreq(g_settings.cpufreq * 1000 * 1000);
			delete cpuFreq;
		}

		if (powerManager) {
			/* if we were in standby, leave it otherwise, the next
			   start of neutrino will fail in "_write_gxa" in
			   framebuffer.cpp
			   => this is needed because the drivers are crap :( */
			powerManager->SetStandby(false, false);
			powerManager->Close();
			delete powerManager;
		}
		cs_deregister_messenger();
	}

	if (for_flash) {
		//delete cHddStat::getInstance();
		delete CRecordManager::getInstance();
		delete videoDemux;
		int ret = my_system(4, "mount", "-no", "remount,ro", "/");
		printf("remount rootfs readonly %s.\n", (ret == 0)?"successful":"failed"); fflush(stdout);
	}
}

void stop_video()
{
	delete videoDecoder;
	delete videoDemux;
	delete CFrameBuffer::getInstance();
	cs_api_exit();
}

void sighandler (int signum)
{
	signal (signum, SIG_IGN);
	switch (signum) {
	case SIGTERM:
	case SIGINT:
		//delete cHddStat::getInstance();
		delete CRecordManager::getInstance();
		//CNeutrinoApp::getInstance()->saveSetup(NEUTRINO_SETTINGS_FILE);
		stop_daemons();
		delete CVFD::getInstance();
		delete SHTDCNT::getInstance();
		stop_video();
		//_exit(0);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
		exit(CNeutrinoApp::SHUTDOWN);
#else
		exit(0);
#endif
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	/* build date */
	printf(">>> Neutrino (compiled %s %s) <<<\n", __DATE__, __TIME__);
	g_Timerd = NULL;
	g_Radiotext = NULL;
	g_Zapit = NULL;
	setDebugLevel(DEBUG_NORMAL);
	signal(SIGTERM, sighandler);	// TODO: consider the following
	signal(SIGINT, sighandler);	// NOTES: The effects of signal() in a multithreaded
	signal(SIGHUP, SIG_IGN);	//        process are unspecified (signal(2))
	/* don't die in streamts.cpp from a SIGPIPE if client disconnects */
	signal(SIGPIPE, SIG_IGN);

	tzset();

	return CNeutrinoApp::getInstance()->run(argc, argv);
}

void CNeutrinoApp::loadKeys(const char * fname)
{
	bool res;
	CConfigFile & tconfig = configfile;

	if(fname) {
		CConfigFile newconfig(',', true);

		res = newconfig.loadConfig(fname);
		if(!res) return;
		tconfig = newconfig;
	}

	//rc-key configuration
	g_settings.key_tvradio_mode = tconfig.getInt32( "key_tvradio_mode", (unsigned int)CRCInput::RC_nokey );
	g_settings.key_power_off = tconfig.getInt32( "key_power_off", CRCInput::RC_standby );

	g_settings.key_pageup = tconfig.getInt32( "key_channelList_pageup",  CRCInput::RC_page_up );
	g_settings.key_pagedown = tconfig.getInt32( "key_channelList_pagedown", CRCInput::RC_page_down );
	g_settings.key_channelList_cancel = tconfig.getInt32( "key_channelList_cancel",  CRCInput::RC_home );
	g_settings.key_channelList_sort = tconfig.getInt32( "key_channelList_sort",  CRCInput::RC_blue );
	g_settings.key_channelList_addrecord = tconfig.getInt32( "key_channelList_addrecord",  CRCInput::RC_red );
	g_settings.key_channelList_addremind = tconfig.getInt32( "key_channelList_addremind",  CRCInput::RC_yellow );

	g_settings.key_list_start = tconfig.getInt32( "key_list_start", (unsigned int)CRCInput::RC_nokey );
	g_settings.key_list_end = tconfig.getInt32( "key_list_end", (unsigned int)CRCInput::RC_nokey );
	g_settings.key_timeshift = tconfig.getInt32( "key_timeshift", CRCInput::RC_pause );
	g_settings.key_unlock = tconfig.getInt32( "key_unlock", CRCInput::RC_setup );
	g_settings.key_screenshot = tconfig.getInt32( "key_screenshot", (unsigned int)CRCInput::RC_nokey );
#ifdef ENABLE_PIP
	g_settings.key_pip_close = tconfig.getInt32( "key_pip_close", CRCInput::RC_help );
	g_settings.key_pip_setup = tconfig.getInt32( "key_pip_setup", CRCInput::RC_pos );
	g_settings.key_pip_swap = tconfig.getInt32( "key_pip_swap", CRCInput::RC_recall );
#endif
	g_settings.key_current_transponder = tconfig.getInt32( "key_current_transponder", CRCInput::RC_games );

	g_settings.key_quickzap_up = tconfig.getInt32( "key_quickzap_up",  CRCInput::RC_down );
	g_settings.key_quickzap_down = tconfig.getInt32( "key_quickzap_down",  CRCInput::RC_up );
	g_settings.key_subchannel_up = tconfig.getInt32( "key_subchannel_up",  CRCInput::RC_right );
	g_settings.key_subchannel_down = tconfig.getInt32( "key_subchannel_down",  CRCInput::RC_left );
	g_settings.key_zaphistory = tconfig.getInt32( "key_zaphistory",  CRCInput::RC_nokey );
	g_settings.key_lastchannel = tconfig.getInt32( "key_lastchannel",  CRCInput::RC_0 );

	g_settings.key_bouquet_up = tconfig.getInt32( "key_bouquet_up",  CRCInput::RC_right);
	g_settings.key_bouquet_down = tconfig.getInt32( "key_bouquet_down",  CRCInput::RC_left);

	g_settings.mbkey_copy_onefile = tconfig.getInt32( "mbkey.copy_onefile", CRCInput::RC_radio );
	g_settings.mbkey_copy_several = tconfig.getInt32( "mbkey.copy_several", CRCInput::RC_text );
	g_settings.mbkey_cut = tconfig.getInt32( "mbkey.cut", CRCInput::RC_audio );
	g_settings.mbkey_truncate = tconfig.getInt32( "mbkey.truncate", CRCInput::RC_games );
	g_settings.mbkey_cover = tconfig.getInt32( "mbkey.cover", CRCInput::RC_favorites );

	g_settings.mpkey_rewind = tconfig.getInt32( "mpkey.rewind", CRCInput::RC_rewind );
	g_settings.mpkey_forward = tconfig.getInt32( "mpkey.forward", CRCInput::RC_forward );
	g_settings.mpkey_pause = tconfig.getInt32( "mpkey.pause", CRCInput::RC_pause );
	g_settings.mpkey_stop = tconfig.getInt32( "mpkey.stop", CRCInput::RC_stop );
	g_settings.mpkey_play = tconfig.getInt32( "mpkey.play", CRCInput::RC_play );
	g_settings.mpkey_audio = tconfig.getInt32( "mpkey.audio", CRCInput::RC_green );
	g_settings.mpkey_time = tconfig.getInt32( "mpkey.time", CRCInput::RC_timeshift );
	g_settings.mpkey_bookmark = tconfig.getInt32( "mpkey.bookmark", CRCInput::RC_yellow );
	g_settings.mpkey_next3dmode = tconfig.getInt32( "mpkey.next3dmode", CRCInput::RC_nokey );
	g_settings.mpkey_plugin = tconfig.getInt32( "mpkey.plugin", (unsigned int)CRCInput::RC_nokey );
	g_settings.mpkey_subtitle = tconfig.getInt32( "mpkey.subtitle", CRCInput::RC_sub );

	g_settings.mpkey_goto = tconfig.getInt32( "mpkey.goto", CRCInput::RC_text );
	g_settings.mpkey_next_repeat_mode = tconfig.getInt32( "mpkey.next_repeat_mode", CRCInput::RC_nokey);

	g_settings.key_format_mode_active = tconfig.getInt32( "key_format_mode_active", 1 );
	g_settings.key_pic_mode_active = tconfig.getInt32( "key_pic_mode_active", 1 );
	g_settings.key_pic_size_active = tconfig.getInt32( "key_pic_size_active", 1 );

	/* options */
	g_settings.menu_left_exit = tconfig.getInt32( "menu_left_exit", 0 );
	g_settings.key_click = tconfig.getInt32( "key_click", 1 );
	g_settings.repeat_blocker = tconfig.getInt32("repeat_blocker", 450);
	g_settings.repeat_genericblocker = tconfig.getInt32("repeat_genericblocker", 100);
	g_settings.longkeypress_duration = tconfig.getInt32("longkeypress_duration", LONGKEYPRESS_OFF);
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	g_settings.accept_other_remotes = tconfig.getInt32( "accept_other_remotes", 1);
#endif

	g_settings.bouquetlist_mode = tconfig.getInt32( "bouquetlist_mode", 0 );
	g_settings.sms_channel = tconfig.getInt32( "sms_channel", 0 );
	g_settings.sms_movie = tconfig.getInt32( "sms_movie", 0 );
	g_settings.mode_left_right_key_tv = tconfig.getInt32( "mode_left_right_key_tv",  SNeutrinoSettings::ZAP);

	g_settings.key_help = tconfig.getInt32( "key_help", CRCInput::RC_help );
	g_settings.key_record = tconfig.getInt32( "key_record", CRCInput::RC_record );
	g_settings.key_switchformat = tconfig.getInt32("key_switchformat", CRCInput::RC_prev);
	g_settings.key_next43mode = tconfig.getInt32("key_next43mode", CRCInput::RC_next);
	g_settings.key_volumeup = tconfig.getInt32( "key_volumeup",  CRCInput::RC_plus );
	g_settings.key_volumedown = tconfig.getInt32( "key_volumedown", CRCInput::RC_minus );
}

void CNeutrinoApp::saveKeys(const char * fname)
{
	CConfigFile & tconfig = configfile;
	if(fname) {
		CConfigFile newconfig(',', true);
		tconfig = newconfig;
	}
	//rc-key configuration
	tconfig.setInt32( "key_tvradio_mode", g_settings.key_tvradio_mode );
	tconfig.setInt32( "key_power_off", g_settings.key_power_off );

	tconfig.setInt32( "key_channelList_pageup", g_settings.key_pageup );
	tconfig.setInt32( "key_channelList_pagedown", g_settings.key_pagedown );
	tconfig.setInt32( "key_channelList_cancel", g_settings.key_channelList_cancel );
	tconfig.setInt32( "key_channelList_sort", g_settings.key_channelList_sort );
	tconfig.setInt32( "key_channelList_addrecord", g_settings.key_channelList_addrecord );
	tconfig.setInt32( "key_channelList_addremind", g_settings.key_channelList_addremind );

	tconfig.setInt32( "key_list_start", g_settings.key_list_start );
	tconfig.setInt32( "key_list_end", g_settings.key_list_end );
	tconfig.setInt32( "key_timeshift", g_settings.key_timeshift );
	tconfig.setInt32( "key_unlock", g_settings.key_unlock );
	tconfig.setInt32( "key_screenshot", g_settings.key_screenshot );
#ifdef ENABLE_PIP
	tconfig.setInt32( "key_pip_close", g_settings.key_pip_close );
	tconfig.setInt32( "key_pip_setup", g_settings.key_pip_setup );
	tconfig.setInt32( "key_pip_swap", g_settings.key_pip_swap );
#endif
	tconfig.setInt32( "key_current_transponder", g_settings.key_current_transponder );

	tconfig.setInt32( "key_quickzap_up", g_settings.key_quickzap_up );
	tconfig.setInt32( "key_quickzap_down", g_settings.key_quickzap_down );
	tconfig.setInt32( "key_subchannel_up", g_settings.key_subchannel_up );
	tconfig.setInt32( "key_subchannel_down", g_settings.key_subchannel_down );
	tconfig.setInt32( "key_zaphistory", g_settings.key_zaphistory );
	tconfig.setInt32( "key_lastchannel", g_settings.key_lastchannel );

	tconfig.setInt32( "key_bouquet_up", g_settings.key_bouquet_up );
	tconfig.setInt32( "key_bouquet_down", g_settings.key_bouquet_down );

	tconfig.setInt32( "mbkey.copy_onefile", g_settings.mbkey_copy_onefile );
	tconfig.setInt32( "mbkey.copy_several", g_settings.mbkey_copy_several );
	tconfig.setInt32( "mbkey.cut", g_settings.mbkey_cut );
	tconfig.setInt32( "mbkey.truncate", g_settings.mbkey_truncate );
	tconfig.setInt32( "mbkey.cover", g_settings.mbkey_cover );

	tconfig.setInt32( "mpkey.rewind", g_settings.mpkey_rewind );
	tconfig.setInt32( "mpkey.forward", g_settings.mpkey_forward );
	tconfig.setInt32( "mpkey.pause", g_settings.mpkey_pause );
	tconfig.setInt32( "mpkey.stop", g_settings.mpkey_stop );
	tconfig.setInt32( "mpkey.play", g_settings.mpkey_play );
	tconfig.setInt32( "mpkey.audio", g_settings.mpkey_audio );
	tconfig.setInt32( "mpkey.time", g_settings.mpkey_time );
	tconfig.setInt32( "mpkey.bookmark", g_settings.mpkey_bookmark );
	tconfig.setInt32( "mpkey.plugin", g_settings.mpkey_plugin );
	tconfig.setInt32( "mpkey.next3dmode", g_settings.mpkey_next3dmode );
	tconfig.setInt32( "mpkey.subtitle", g_settings.mpkey_subtitle );

	tconfig.setInt32( "mpkey.goto", g_settings.mpkey_goto );
	tconfig.setInt32( "mpkey.next_repeat_mode", g_settings.mpkey_next_repeat_mode );

	tconfig.setInt32( "key_format_mode_active", g_settings.key_format_mode_active );
	tconfig.setInt32( "key_pic_mode_active", g_settings.key_pic_mode_active );
	tconfig.setInt32( "key_pic_size_active", g_settings.key_pic_size_active );

	tconfig.setInt32( "menu_left_exit", g_settings.menu_left_exit );
	tconfig.setInt32( "key_click", g_settings.key_click );
	tconfig.setInt32( "repeat_blocker", g_settings.repeat_blocker );
	tconfig.setInt32( "repeat_genericblocker", g_settings.repeat_genericblocker );
	tconfig.setInt32( "longkeypress_duration", g_settings.longkeypress_duration );
#if HAVE_SPARK_HARDWARE || HAVE_DUCKBOX_HARDWARE
	tconfig.setInt32("accept_other_remotes", g_settings.accept_other_remotes);
#endif

	tconfig.setInt32( "bouquetlist_mode", g_settings.bouquetlist_mode );
	tconfig.setInt32( "sms_channel", g_settings.sms_channel );
	tconfig.setInt32( "sms_movie", g_settings.sms_movie );
	tconfig.setInt32( "mode_left_right_key_tv", g_settings.mode_left_right_key_tv );

	tconfig.setInt32( "key_help", g_settings.key_help );
	tconfig.setInt32( "key_record", g_settings.key_record );
	tconfig.setInt32( "key_switchformat", g_settings.key_switchformat );
	tconfig.setInt32( "key_next43mode", g_settings.key_next43mode );
	tconfig.setInt32( "key_volumeup", g_settings.key_volumeup );
	tconfig.setInt32( "key_volumedown", g_settings.key_volumedown );

	if(fname)
		tconfig.saveConfig(fname);
}

void CNeutrinoApp::StopSubtitles(bool enable_glcd_mirroring)
{
	//printf("[neutrino] %s\n", __FUNCTION__);
	if (CMoviePlayerGui::getInstance().Playing()) {
		CMoviePlayerGui::getInstance().StopSubtitles(enable_glcd_mirroring);
		return;
	}

	int ttx, dvbpid, ttxpid, ttxpage;

	dvbpid = dvbsub_getpid();
	tuxtx_subtitle_running(&ttxpid, &ttxpage, &ttx);

	if(dvbpid)
		dvbsub_pause();
	if(ttx) {
		tuxtx_pause_subtitle(true);
		frameBuffer->paintBackground();
	}
#ifdef ENABLE_GRAPHLCD
	if (enable_glcd_mirroring)
		nGLCD::MirrorOSD(g_settings.glcd_mirror_osd);
#endif
#if 0
	if (mode == mode_webtv)
		CMoviePlayerGui::getInstance(true).clearSubtitle(true);
#endif
}

void CNeutrinoApp::StartSubtitles(bool show)
{
	//printf("%s: %s\n", __FUNCTION__, show ? "Show" : "Not show");

	if (CMoviePlayerGui::getInstance().Playing()) {
		CMoviePlayerGui::getInstance().StartSubtitles(show);
		return;
	}

#ifdef ENABLE_GRAPHLCD
	nGLCD::MirrorOSD(false);
#endif
	if(!show)
		return;
	dvbsub_start(0);
	tuxtx_pause_subtitle(false);
#if 0
	if (mode == mode_webtv)
		CMoviePlayerGui::getInstance(true).clearSubtitle(false);
#endif 
}

void CNeutrinoApp::SelectSubtitles()
{
	/* called on NeutrinoMessages::EVT_ZAP_COMPLETE, should be safe to use zapit current channel */
	CZapitChannel * cc = CZapit::getInstance()->GetCurrentChannel();

	if(!g_settings.auto_subs || cc == NULL)
		return;

	for(int i = 0; i < 3; i++) {
		if(g_settings.pref_subs[i].empty() || g_settings.pref_subs[i] == "none")
			continue;

		std::string temp(g_settings.pref_subs[i]);

		for(int j = 0 ; j < (int)cc->getSubtitleCount() ; j++) {
			CZapitAbsSub* s = cc->getChannelSub(j);
			if (s->thisSubType == CZapitAbsSub::DVB) {
				CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); ++it) {
					if(temp == it->second && sd->ISO639_language_code == it->first) {
						printf("CNeutrinoApp::SelectSubtitles: found DVB %s, pid %x\n", sd->ISO639_language_code.c_str(), sd->pId);
						dvbsub_stop();
						dvbsub_setpid(sd->pId);
						return;
					}
				}
			}
		}
		for(int j = 0 ; j < (int)cc->getSubtitleCount() ; j++) {
			CZapitAbsSub* s = cc->getChannelSub(j);
			if (s->thisSubType == CZapitAbsSub::TTX) {
				CZapitTTXSub* sd = reinterpret_cast<CZapitTTXSub*>(s);
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); ++it) {
					if(temp == it->second && sd->ISO639_language_code == it->first) {
						int page = ((sd->teletext_magazine_number & 0xFF) << 8) | sd->teletext_page_number;
						printf("CNeutrinoApp::SelectSubtitles: found TTX %s, pid %x page %03X\n", sd->ISO639_language_code.c_str(), sd->pId, page);
						tuxtx_stop_subtitle();
						tuxtx_set_pid(sd->pId, page, (char *) sd->ISO639_language_code.c_str());
						return;
					}
				}
			}
		}
	}
}

void CNeutrinoApp::SDT_ReloadChannels()
{
	SDTreloadChannels = false;
	//g_Zapit->reinitChannels();
	channelsInit();
	t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
	adjustToChannelID(live_channel_id);//FIXME what if deleted ?
	if(old_b_id >= 0) {
		bouquetList->activateBouquet(old_b_id, false);
		old_b_id = -1;
		g_RCInput->postMsg(CRCInput::RC_ok, 0);
	}
}

void CNeutrinoApp::getAnnounceEpgName(CTimerd::RecordingInfo * eventinfo, std::string &name)
{

	name += "\n";

	std::string zAddData = CServiceManager::getInstance()->GetServiceName(eventinfo->channel_id);
	if( zAddData.empty()) {
		zAddData = g_Locale->getText(LOCALE_TIMERLIST_PROGRAM_UNKNOWN);
	}

	if(eventinfo->epgID!=0) {
		CEPGData epgdata;
		zAddData += " :\n";
		if (CEitManager::getInstance()->getEPGid(eventinfo->epgID, eventinfo->epg_starttime, &epgdata)) {
			zAddData += epgdata.title;
		}
		else if(strlen(eventinfo->epgTitle)!=0) {
			zAddData += eventinfo->epgTitle;
		}
	}
	else if(strlen(eventinfo->epgTitle)!=0) {
		zAddData += " :\n";
		zAddData += eventinfo->epgTitle;
	}

	name += zAddData;
}

#ifdef ENABLE_PIP
bool CNeutrinoApp::StartPip(const t_channel_id channel_id)
{
	bool ret = false;
	CZapitChannel * channel = CServiceManager::getInstance()->FindChannel(channel_id);
	if (!channel)
		return ret;

	if (channel->getRecordDemux() == channel->getPipDemux())
		CStreamManager::getInstance()->StopStream(channel_id);

	int recmode = CRecordManager::getInstance()->GetRecordMode(channel_id);
	if ((recmode == CRecordManager::RECMODE_OFF) || (channel->getRecordDemux() != channel->getPipDemux())) {
		if (!g_Zapit->zapTo_pip(channel_id))
			DisplayErrorMessage(g_Locale->getText(LOCALE_VIDEOMENU_PIP_ERROR));
		else
			ret = true;
	}
	return ret;
}
#endif

void CNeutrinoApp::Cleanup()
{
//	CLuaServer::destroyInstance();
#ifdef EXIT_CLEANUP
	INFO("cleanup...");
	printf("cleanup 10\n");fflush(stdout);
	delete g_Sectionsd; g_Sectionsd = NULL;
	delete g_Timerd; g_Timerd = NULL;
	delete g_Zapit; g_Zapit = NULL;
	delete g_RemoteControl; g_RemoteControl = NULL;

	printf("cleanup 11\n");fflush(stdout);
	delete g_fontRenderer; g_fontRenderer = NULL;
	delete g_dynFontRenderer; g_dynFontRenderer = NULL;
	printf("cleanup 12\n");fflush(stdout);
	delete g_PicViewer; g_PicViewer = NULL;
	printf("cleanup 13\n");fflush(stdout);
	delete g_PluginList; g_PluginList = NULL;
	printf("cleanup 16\n");fflush(stdout);
#if !HAVE_SPARK_HARDWARE
	delete g_CamHandler; g_CamHandler = NULL;
	printf("cleanup 17\n");fflush(stdout);
#endif
	delete g_volume; g_volume = NULL;
	printf("cleanup 17a\n");fflush(stdout);
	delete g_audioMute; g_audioMute = NULL;
	printf("cleanup 18\n");fflush(stdout);
	delete g_EpgData; g_EpgData = NULL;
	printf("cleanup 19\n");fflush(stdout);
#if 0
	/* crashes in destructor???? very strange */
	delete g_InfoViewer; g_InfoViewer = NULL;
	printf("cleanup 11\n");fflush(stdout);
#endif
	delete g_EventList; g_EventList = NULL;
	printf("cleanup 12\n");fflush(stdout);
	delete g_Locale; g_Locale = NULL;
	delete g_videoSettings; g_videoSettings = NULL;
	delete g_Radiotext; g_Radiotext = NULL;

	printf("cleanup 13\n");fflush(stdout);
	delete audioSetupNotifier; audioSetupNotifier = NULL;
	printf("cleanup 14\n");fflush(stdout);

	delete TVbouquetList; TVbouquetList = NULL;
	delete RADIObouquetList; RADIObouquetList = NULL;

	delete TVfavList; TVfavList = NULL;
	delete RADIOfavList; RADIOfavList = NULL;

	delete TVchannelList; TVchannelList = NULL;
	delete RADIOchannelList; RADIOchannelList = NULL;
	delete TVallList; TVallList = NULL;
	delete RADIOallList; RADIOallList = NULL;
	delete TVsatList; TVsatList = NULL;
	delete RADIOsatList; RADIOsatList = NULL;

	printf("cleanup 1\n");fflush(stdout);
	for (int i = 0; i < SNeutrinoSettings::FONT_TYPE_COUNT; i++) {
		delete g_Font[i];
		g_Font[i] = NULL;
	}
	printf("cleanup 2\n");fflush(stdout);
	delete g_SignalFont; g_SignalFont = NULL;
	printf("cleanup 3\n");fflush(stdout);
	configfile.clear();

	printf("cleanup 4\n");fflush(stdout);
	delete CZapit::getInstance();
	printf("cleanup 5\n");fflush(stdout);
	delete CEitManager::getInstance();
	printf("cleanup 6\n");fflush(stdout);
#if HAVE_COOL_HARDWARE
	delete CVFD::getInstance();
#ifdef __UCLIBC__
	malloc_stats(NULL);
#else
	malloc_stats();
#endif
#endif
#endif
}

#if ENABLE_FASTSCAN
void CNeutrinoApp::CheckFastScan(bool standby, bool reload)
{
	if (scansettings.fst_update) {
		g_Zapit->getMode();
		INFO("fst version %02x (%s)", scansettings.fst_version, standby ? "force" : "check");
		CServiceScan::getInstance()->QuietFastScan(true);
		int new_fst = scansettings.fst_version;
		if (!standby) {
			if (CServiceScan::getInstance()->ReadFstVersion(scansettings.fast_op))
				new_fst = CServiceScan::getInstance()->GetFstVersion();
		}
		if (standby || (new_fst != scansettings.fst_version)) {
			CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
			CVFD::getInstance()->ShowText(g_Locale->getText(LOCALE_SATSETUP_FASTSCAN_HEAD));
			CHintBox * fhintbox = NULL;
			if (!standby) {
				fhintbox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SATSETUP_FASTSCAN_HEAD));
				fhintbox->paint();
			}
			if (CServiceScan::getInstance()->ScanFast(scansettings.fast_op, reload)) {
				scanSettings.fst_version = CServiceScan::getInstance()->GetFstVersion();
				scanSettings.saveSettings(NEUTRINO_SCAN_SETTINGS_FILE);
			}
			delete fhintbox;
			if (standby)
				CVFD::getInstance()->setMode(CVFD::MODE_STANDBY);
		}
	}
}
#endif

bool CNeutrinoApp::adjustToChannelID(const t_channel_id channel_id)
{
	int old_mode = lastChannelMode;
	int new_mode = old_mode;
	bool has_channel = false;
	first_mode_found = -1;

	if (!channelList->adjustToChannelID(channel_id))
		return false;

	channelList->getLastChannels().store (channel_id);
	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_tv
			|| CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv) {
		has_channel = TVfavList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_FAV;
		if(!has_channel && old_mode == LIST_MODE_FAV)
			new_mode = LIST_MODE_PROV;

		has_channel = TVbouquetList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_PROV;
		if(!has_channel && old_mode == LIST_MODE_PROV)
			new_mode = LIST_MODE_WEBTV;

		has_channel = TVwebList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_WEBTV;
		if(!has_channel && old_mode == LIST_MODE_WEBTV)
			new_mode = LIST_MODE_SAT;

		has_channel = TVsatList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_SAT;
		if(!has_channel && old_mode == LIST_MODE_SAT)
			new_mode = LIST_MODE_ALL;

		has_channel = TVallList->adjustToChannelID(channel_id);
	}
	else if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio) {
		has_channel = RADIOfavList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_FAV;
		if(!has_channel && old_mode == LIST_MODE_FAV)
			new_mode = LIST_MODE_PROV;

		has_channel = RADIObouquetList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_PROV;
		if(!has_channel && old_mode == LIST_MODE_PROV)
			new_mode = LIST_MODE_WEBTV;

		has_channel = RADIOwebList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_WEBTV;
		if(!has_channel && old_mode == LIST_MODE_WEBTV)
			new_mode = LIST_MODE_SAT;

		has_channel = RADIOsatList->adjustToChannelID(channel_id);
		if (has_channel && first_mode_found < 0)
			first_mode_found = LIST_MODE_SAT;
		if(!has_channel && old_mode == LIST_MODE_SAT)
			new_mode = LIST_MODE_ALL;

		has_channel = RADIOallList->adjustToChannelID(channel_id);
	}
	if(old_mode != new_mode)
		CNeutrinoApp::getInstance()->SetChannelMode(new_mode);

	return true;
}

/*
 * commit 523b273a changed the names of config file entries:
 *	casystem_display	=> infobar_casystem_display
 *	casystem_dotmatrix	=> infobar_casystem_dotmatrix
 *	casystem_frame		=> infobar_casystem_frame
 * convert these, so that users do not need to set up their system again
*/
struct __key_rename {
	const char *from;
	const char *to;
};

static struct __key_rename key_rename[] = {
	{ "casystem_display",	"infobar_casystem_display" },
	{ "casystem_dotmatrix",	"infobar_casystem_dotmatrix"},
	{ "casystem_frame",	"infobar_casystem_frame" },
	{ NULL, NULL }
};

/* actually do the migration of the config entries */
void CNeutrinoApp::migrateConfig(const char *fname)
{
	/* we need a second configfile to not create new entries and trigger the
	 * "new entry created" flag */
	CConfigFile migconf('\t', false);
	migconf.loadConfig(fname);
	/* here we do a simple rename of config file keys */
	int magic = -424242; /* obviously a value that does not appear in real cases */
	int i;
	for (i = 0; key_rename[i].from != NULL; i++) {
		const char *from = key_rename[i].from;
		const char *to   = key_rename[i].to;
		int tmp = migconf.getInt32(from, magic);
		if (tmp == magic)	/* old key does not exist */
			continue;
		/* only set new key to old value if the new key does not yet exist */
		if (configfile.getInt32(to, magic) == magic)
			configfile.setInt32(to, tmp);
		/* always remove old key*/
		configfile.deleteKey(from);
	}
	/* more complex migration, including converting values etc. could be done here */
}

