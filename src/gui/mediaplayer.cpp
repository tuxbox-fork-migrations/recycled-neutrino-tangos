/*
	Mediaplayer selection menu - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2011 T. Graf 'dbt'
	Homepage: http://www.dbox2-tuning.net/

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


#include "mediaplayer.h"

#include <global.h>
#include <neutrino.h>
#include <neutrino_menue.h>
#include <neutrinoMessages.h>

#include <gui/infoclock.h>
#include <gui/movieplayer.h>
#if ENABLE_UPNP
#include <gui/upnpbrowser.h>
#endif
#include <gui/nfs.h>

#include <gui/widget/icons.h>

#include <driver/screen_max.h>

#include <system/debug.h>
#include <video.h>
extern cVideo * videoDecoder;


CMediaPlayerMenu::CMediaPlayerMenu()
{
	setMenuTitel();
	setUsageMode();

	width = 40;
	
}

CMediaPlayerMenu* CMediaPlayerMenu::getInstance()
{
	static CMediaPlayerMenu* mpm = NULL;

	if(!mpm) {
		mpm = new CMediaPlayerMenu();
		printf("[neutrino] mediaplayer menu instance created\n");
	}
	return mpm;
}

CMediaPlayerMenu::~CMediaPlayerMenu()
{
}

int CMediaPlayerMenu::exec(CMenuTarget* parent, const std::string &actionKey)
{
	printf("init mediaplayer menu in usage mode %d\n", usage_mode);

	if (parent)
		parent->hide();
	
	if (actionKey == "movieplayer")
	{
		CInfoClock::getInstance()->enableInfoClock(false);
		int mode = CNeutrinoApp::getInstance()->getMode();
		if( mode == NeutrinoMessages::mode_radio )
			CFrameBuffer::getInstance()->stopFrame();
		int res = CMoviePlayerGui::getInstance().exec(NULL, "tsmoviebrowser");
		if( mode == NeutrinoMessages::mode_radio )
			CFrameBuffer::getInstance()->showFrame("radiomode.jpg");
		CInfoClock::getInstance()->enableInfoClock(true);
		return res;
	}
	
	int res = initMenuMedia();
	
	return res;
}


//show selectable mediaplayer items
int CMediaPlayerMenu::initMenuMedia(CMenuWidget *m, CPersonalizeGui *p)
{	
	CPersonalizeGui *personalize = p;
	CMenuWidget 	*media = m;
	
	bool show = (personalize == NULL || media == NULL);

	if (personalize == NULL)
		 personalize = new CPersonalizeGui();
	
	if (media == NULL)
		 media = new CMenuWidget(menu_title, NEUTRINO_ICON_MULTIMEDIA, width, MN_WIDGET_ID_MEDIA);

	personalize->addWidget(media);
	personalize->addIntroItems(media);
	
#if ENABLE_UPNP
	static CUpnpBrowserGui *upnpbrowsergui = NULL;
	CMenuForwarder *fw_upnp = NULL;
#endif

	bool enabled = !CMoviePlayerGui::getInstance().Playing();

	if (usage_mode == MODE_DEFAULT)
	{
#if ENABLE_UPNP
		//upnp browser
		if (!upnpbrowsergui)
			upnpbrowsergui = new CUpnpBrowserGui();
		fw_upnp = new CMenuForwarder(LOCALE_UPNPBROWSER_HEAD, enabled, NULL, upnpbrowsergui, NULL, CRCInput::RC_yellow);
		fw_upnp->setHint(NEUTRINO_ICON_HINT_A_PIC, LOCALE_MENU_HINT_UPNP);
#endif
//		media->addIntroItems(NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, usage_mode == MODE_AUDIO ? CMenuWidget::BTN_TYPE_CANCEL : CMenuWidget::BTN_TYPE_BACK);
	}

	if (usage_mode == MODE_VIDEO)
	{
		showMoviePlayer(media, personalize);

		showNetworkNFSMounts(media, personalize);
	}
	else
	{
#if ENABLE_UPNP
		//upnp browser
		personalize->addItem(media, fw_upnp, &g_settings.personalize[SNeutrinoSettings::P_MEDIA_UPNP]);
#endif
		if (g_settings.recording_type != CNeutrinoApp::RECORDING_OFF)
			showMoviePlayer(media, personalize);
			showNetworkNFSMounts(media, personalize);

	}
	
	int res = menu_return::RETURN_NONE;
	
	if (show)
	{
 		//adding personalized items
		personalize->addPersonalizedItems();
		
		//add I_TYPE_MULTIMEDIA plugins
		unsigned int nextShortcut = (unsigned int)media->getNextShortcut();
		media->integratePlugins(CPlugins::I_TYPE_MULTIMEDIA, nextShortcut, enabled);

		res = media->exec(NULL, "");
		delete media;
		delete personalize;
#if ENABLE_UPNP
		//delete upnpbrowsergui;
#endif

		setUsageMode();//set default usage_mode
	}
	return res;
}

//show movieplayer submenu with selectable items for moviebrowser or filebrowser
void CMediaPlayerMenu::showMoviePlayer(CMenuWidget *moviePlayer, CPersonalizeGui *p)
{ 
	p->addSeparator(*moviePlayer, LOCALE_MAINMENU_MOVIEPLAYER, true);

	//moviebrowser
	CMenuForwarder *fw_mbrowser = new CMenuForwarder(LOCALE_MOVIEBROWSER_HEAD, true, NULL, this, "movieplayer");
	fw_mbrowser->setHint(NEUTRINO_ICON_HINT_MB, LOCALE_MENU_HINT_MB);
	p->addItem(moviePlayer, fw_mbrowser, &g_settings.personalize[SNeutrinoSettings::P_MPLAYER_MBROWSER]);
	
	//fileplayback
	CMenuForwarder *fw_file = new CMenuForwarder(LOCALE_MOVIEPLAYER_FILEPLAYBACK, true, NULL, &CMoviePlayerGui::getInstance(), "fileplayback");
	fw_file->setHint(NEUTRINO_ICON_HINT_FILEPLAY, LOCALE_MENU_HINT_FILEPLAY);
	p->addItem(moviePlayer, fw_file, &g_settings.personalize[SNeutrinoSettings::P_MPLAYER_FILEPLAY]);

#if 0
	//ytplayback
	CMenuForwarder *fw_yt = new CMenuForwarder(LOCALE_MOVIEPLAYER_YTPLAYBACK, g_settings.youtube_enabled, NULL, &CMoviePlayerGui::getInstance(), "ytplayback");
	fw_yt->setHint(NEUTRINO_ICON_HINT_YTPLAY, LOCALE_MENU_HINT_YTPLAY);
	p->addItem(moviePlayer, fw_yt, &g_settings.personalize[SNeutrinoSettings::P_MPLAYER_YTPLAY]);
#endif

}

void CMediaPlayerMenu::showNetworkNFSMounts(CMenuWidget *menu_nfs, CPersonalizeGui *p)
{
	p->addSeparator(*menu_nfs, LOCALE_NETWORKMENU_MOUNT, true);
	
	CMenuForwarder * mf_mount = new CMenuForwarder(LOCALE_NFS_MOUNT , true, NULL, new CNFSMountGui(), NULL);
	mf_mount->setHint("", LOCALE_MENU_HINT_NET_NFS_MOUNT);
	p->addItem(menu_nfs, mf_mount, &g_settings.personalize[SNeutrinoSettings::P_MPLAYER_FILEPLAY]);

	CMenuForwarder * mf_umount = new CMenuForwarder(LOCALE_NFS_UMOUNT, true, NULL, new CNFSUmountGui(), NULL);
	mf_umount->setHint("", LOCALE_MENU_HINT_NET_NFS_UMOUNT);
	p->addItem(menu_nfs, mf_umount, &g_settings.personalize[SNeutrinoSettings::P_MPLAYER_FILEPLAY]);
}
