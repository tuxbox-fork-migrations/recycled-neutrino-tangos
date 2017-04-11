/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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


#ifndef __infoview__
#define __infoview__

#include <sectionsdclient/sectionsdclient.h>

#include <driver/rcinput.h>
#include <system/settings.h>
#include <string>
#include <zapit/channel.h>
#include <gui/components/cc.h>
#include <driver/fade.h>
#include "widget/menue.h"

class CFrameBuffer;
class COSDFader;
class CInfoViewer
{
public:
    enum
    {
        // The order of icons from left to right. Here nothing changes!
        BUTTON_RED	= 0,
        BUTTON_GREEN	= 1,
        BUTTON_YELLOW	= 2,
        BUTTON_BLUE	= 3,
        BUTTON_MAX	= 4
    };
private:

    CFrameBuffer *frameBuffer;
    CComponentsFrmClock *clock;
    CComponentsShapeSquare *header, *body, *rec, *foot, *ca_bar;
    CComponentsTextTransp *txt_cur_start, *txt_cur_event, *txt_cur_event_rest, *txt_next_start, *txt_next_event, *txt_next_in;

    bool           gotTime;
    bool           recordModeActive;

#ifndef SKIP_CA_STATUS
    bool           CA_Status;
#endif

    int            InfoHeightY;
    bool	       fileplay;

    int            ButtonWidth;
    int            spacer;

    // dimensions of radiotext window
    int             rt_dx;
    int             rt_dy;
    int             rt_x;
    int             rt_y;
    int             rt_h;
    int             rt_w;

    std::string ChannelName;

    int            ChanNameX;
    int            ChanNumWidth;
    int            ChanWidth;
    int            numbox_offset;

    int            ana_clock_size;
    char		   strChanNum[10];
    void           PaintChanNumber();

    CSectionsdClient::CurrentNextInfo info_CurrentNext;
    CSectionsdClient::CurrentNextInfo oldinfo;
    t_channel_id   current_channel_id;
    t_channel_id   current_epg_id;

    //uint32_t           fadeTimer;
    COSDFader	fader;

    int time_height;
    int info_time_width;
    int header_height;
    bool newfreq ;
    static const short bar_width = 72;
    static event_id_t last_curr_id, last_next_id;
    uint64_t timeoutEnd;
    void setInfobarTimeout(int timeout_ext = 0);

    CChannelEventList               evtlist;
    CChannelEventList::iterator     eli;

    int lastsnr, lastsig, lasttime;
    CProgressBar *timescale;
    CSignalBox *sigbox;

    bool casysChange;
    bool channellogoChange;
    uint32_t lcdUpdateTimer;
    int	 zap_mode;
    std::string _livestreamInfo1;
    std::string _livestreamInfo2;

    void paintBackground(int col_Numbox);
    void paintHead();
    void paintBody();
    void show_Data( bool calledFromEvent = false );
    void display_Info(const char *current, const char *next,
                      bool starttimes = true, const int pb_pos = -1,
                      const char *runningStart = NULL, const char *runningRest = NULL,
                      const char *nextStart = NULL, const char *nextDuration = NULL,
                      bool update_current = true, bool update_next = true);
    void initClock();
    void showRecordIcon(const bool show);
    void showIcon_Tuner() const;

    void showFailure();
    void showMotorMoving(int duration);
    void showLcdPercentOver();
    int showChannelLogo(const t_channel_id logo_channel_id, const int channel_number_width);
    void showRadiotext();
    void killRadiotext();

    //small infobox, shows a small textbox with a short message text,
    //text must be located in a file named /tmp/infobar.txt
    CComponentsInfoBox *infobar_txt;
    void showInfoFile();
    void killInfobarText();

    //void loop(int fadeValue, bool show_dot ,bool fadeIn);
    void loop(bool show_dot);
    std::string eventname;
    void show_current_next(bool new_chan, int  epgpos);
    void reset_allScala();
    void check_channellogo_ca_SettingsChange();
    void sendNoEpg(const t_channel_id channel_id);
    bool showLivestreamInfo();

    CComponentsWindowMax *ecmInfoBox;

    enum
    {
        // The order of icons from right to left. Here nothing changes!
        ICON_SUBT	= 0,
        ICON_VTXT	= 1,
        ICON_RT		= 2,
        ICON_DD		= 3,
        ICON_16_9	= 4,
        ICON_RES	= 5,
        ICON_CA		= 6,
        ICON_TUNER	= 7,
        ICON_UPDATE	= 8,
        ICON_LOGO	= 9,
        ICON_MAX	= 10
    };

    typedef struct
    {
        bool paint;
        int w;
        int x;
        int cx;
        int h;
        std::string icon;
        std::string text;
        bool active;
    } bbButtonInfoStruct;

    enum CAM_DECODE_NUM {UNKNOWN, NA, FTA, LOCAL, CARD, REMOTE};
    void 		paint_cam_icons();
    void*		Thread_paint_cam_icons(void);
    unsigned short int decode;
    int		DecEndx;
    int 		parse_ecmInfo(const char * file);
    int		check_ecmInfo();
    bool	camCI;
    bool	useCI;

    typedef struct
    {
        int x;
        int h;
    } bbIconInfoStruct;

    bbIconInfoStruct bbIconInfo[ICON_MAX];
    bbButtonInfoStruct bbButtonInfo[BUTTON_MAX];
    std::string tmp_bbButtonInfoText[BUTTON_MAX];
    int bbIconMinX, bbButtonMaxX, bbIconMaxH, bbButtonMaxH;

    int BBarY, BBarFontY;
    int hddwidth;
    //int lasthdd, lastsys;
    bool fta;
    int minX;

    bool scrambledErr, scrambledErrSave;
    bool scrambledNoSig, scrambledNoSigSave;
    pthread_t scrambledT;

    CProgressBar *hddscale, *sysscale;
    void paintFoot(int w = 0);
    void showBBIcons(const int modus, const std::string & icon);
    void getBBIconInfo(void);
    bool checkBBIcon(const char * const icon, int *w, int *h);

    void paint_ca_icons(int, const char*, int&);
    void paint_ca_bar();
    void showOne_CAIcon();

    static void* scrambledThread(void *arg);
    void scrambledCheck(bool force=false);

    void showBarSys(int percent = 0);
    void showBarHdd(int percent = 0);

public:
    bool     chanready;
    bool	 is_visible;

    char     aspectRatio;
    uint32_t sec_timer_id;

    int      BoxStartX;
    int      BoxStartY;
    int      ChanHeight;
    int      BoxEndX;
    int      BoxEndY;
    int      ChanInfoX;
    bool     showButtonBar;
    bool     isVolscale;
    int      ChanNameY;
    int      time_width;

    std::string	md5_ecmInfo;
    bool ecminfo_toggle;
    void ecmInfoBox_hide();
    void ecmInfoBox_show(const char * txt, int w, int h, Font * font);

    CInfoViewer();
    ~CInfoViewer();

    void	showMovieTitle(const int playState, const t_channel_id &channel_id, const std::string &title,
                           const std::string &g_file_epg, const std::string &g_file_epg1,
                           const int duration, const int curr_pos, const int repeat_mode, const int _zap_mode = IV_MODE_DEFAULT);

    void	start();
    void	showEpgInfo();
    void	showTitle(CZapitChannel * channel, const bool calledFromNumZap = false, int epgpos = 0);
    void	showTitle(t_channel_id channel_id, const bool calledFromNumZap = false, int epgpos = 0);
    void lookAheadEPG(const int ChanNum, const std::string & Channel, const t_channel_id new_channel_id = 0, const bool calledFromNumZap = false); //alpha: fix for nvod subchannel update
    void	killTitle();
    void	getEPG(const t_channel_id for_channel_id, CSectionsdClient::CurrentNextInfo &info);

    void	showSubchan();
    //void	Set_CA_Status(int Status);

    int     handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data);

    enum
    {
        IV_MODE_DEFAULT		= 0,
        IV_MODE_VIRTUAL_ZAP 	= 1,
        IV_MODE_NUMBER_ZAP	= 2
    };/*iv_switch_mode_t*/
    /**sets mode for infoviewer.
    * @param[in]  mode
    * 	@li IV_MODE_DEFAULT
    * 	@li IV_MODE_VIRTUAL_ZAP 	means the virtual zap mode, user is typing keys for virtual channel switch
    * 	@li IV_MODE_NUMBER_ZAP 		means number mode, user is typing number keys into screen
    * @return
    *	void
    * @see
    * 	resetSwitchMode()
    * 	getSwitchMode()
    */
    void	setSwitchMode(const int& mode)
    {
        zap_mode = mode;
    }
    int	getSwitchMode()
    {
        return zap_mode;
    }
    void    resetSwitchMode()
    {
        setSwitchMode(IV_MODE_DEFAULT);
    }

    void    changePB();
    void 	ResetPB();
    void    showSNR();
    void    Init(void);
    bool    SDT_freq_update;
    void	setUpdateTimer(uint64_t interval);
    uint32_t getUpdateTimer(void)
    {
        return lcdUpdateTimer;
    }
    inline t_channel_id get_current_channel_id(void)
    {
        return current_channel_id;
    }
    void 	ResetModules(bool kill = false);
    void	KillModules()
    {
        ResetModules(true);
    };

    int bottom_bar_offset, InfoHeightY_Info, showBBIcons_width;

    void showSysfsHdd(void);
    void showIcon_CA_Status(int);
    void showIcon_16_9();
    void showIcon_RadioText(bool rt_available);
    void showIcon_VTXT();
    void showIcon_SubT();
    void showIcon_Resolution();
    void showIcon_Tuner(void);
    void showIcon_Update(bool);
    void showIcon_Logo();
    void show_clock(int posx,int posy,int dia);
    void ShowRecDirScale();
    void showIcon_DD(void);
    void showBBButtons(bool paintFooter = false);
    void paintshowButtonBar();
    void getBBButtonInfo(void);
    void initBBOffset(void);
    // modules
    CComponentsShapeSquare* getFooter(void)
    {
        return foot;
    }
    CComponentsShapeSquare* getCABar(void)
    {
        return ca_bar;
    }
};
#endif
