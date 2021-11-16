/*
	daemon_control

	(C) 2017 NI-Team
	(C) 2020 port & mod by TangoCash

	License: GPL

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sstream>

#include <gui/daemon_control.h>

#include <global.h>
#include <neutrino.h>
#include <mymenu.h>

#include <system/helpers.h>

CDaemonControlMenu::CDaemonControlMenu()
{
	width = 40;
}

CDaemonControlMenu::~CDaemonControlMenu()
{
}

int CDaemonControlMenu::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{
	if (parent)
		parent->hide();

	return show();
}

typedef struct daemons_data_t
{
	neutrino_locale_t name;
	neutrino_locale_t desc;
	const char * icon;
	const char * flag;
	int daemon_exist; // currently unused
	int flag_exist;
}
daemons_data_struct;

daemons_data_t daemons_data[]=
{
	{LOCALE_DAEMON_ITEM_BOOTLOGO_NAME,		LOCALE_DAEMON_ITEM_BOOTLOGO_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"bootlogo",		0 ,0},
	{LOCALE_DAEMON_ITEM_NFSSERVER_NAME,		LOCALE_DAEMON_ITEM_NFSSERVER_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"nfsd",			0 ,0},
	{LOCALE_DAEMON_ITEM_SAMBASERVER_NAME,	LOCALE_DAEMON_ITEM_SAMBASERVER_DESC,NEUTRINO_ICON_HINT_SETTINGS,	"samba",		0 ,0},
	{LOCALE_DAEMON_ITEM_XUPNPD_NAME,		LOCALE_DAEMON_ITEM_XUPNPD_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"xupnpd",		0 ,0},
	{LOCALE_DAEMON_ITEM_TUXCALD_NAME,		LOCALE_DAEMON_ITEM_TUXCALD_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"tuxcald",		0 ,0},
	{LOCALE_DAEMON_ITEM_TUXMAILD_NAME,		LOCALE_DAEMON_ITEM_TUXMAILD_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"tuxmaild",		0 ,0},
	{LOCALE_DAEMON_ITEM_DROPBEAR_NAME,		LOCALE_DAEMON_ITEM_DROPBEAR_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"dropbear",		0 ,0},
	{LOCALE_DAEMON_ITEM_DJMOUNT_NAME,		LOCALE_DAEMON_ITEM_DJMOUNT_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"djmount",		0 ,0},
	{LOCALE_DAEMON_ITEM_CROND_NAME,			LOCALE_DAEMON_ITEM_CROND_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"crond",		0 ,0},
	{LOCALE_DAEMON_ITEM_FCM_NAME,			LOCALE_DAEMON_ITEM_FCM_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"fritzcall",	0 ,0},
	{LOCALE_DAEMON_ITEM_MOUNTBEFORE_NAME,	LOCALE_DAEMON_ITEM_MOUNTBEFORE_DESC,NEUTRINO_ICON_HINT_SETTINGS,	"mbefore",		0 ,0},
	{LOCALE_DAEMON_ITEM_ETIMER_NAME,		LOCALE_DAEMON_ITEM_ETIMER_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"adjust",		0 ,0}
//	{LOCALE_DAEMON_ITEM_EMMREMIND_NAME,		LOCALE_DAEMON_ITEM_EMMREMIND_DESC,	NEUTRINO_ICON_HINT_SETTINGS,	"emmrd",		0 ,0},
//	{LOCALE_DAEMON_ITEM_INADYN_NAME,		LOCALE_DAEMON_ITEM_INADYN_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"inadyn",		0 ,0},
//	{LOCALE_DAEMON_ITEM_USHARE_NAME,		LOCALE_DAEMON_ITEM_USHARE_DESC,		NEUTRINO_ICON_HINT_SETTINGS,	"ushare",		0 ,0},
};
#define DAEMONS_COUNT (sizeof(daemons_data)/sizeof(struct daemons_data_t))

int CDaemonControlMenu::show()
{
	int daemon_shortcut = 0;

	CMenuWidget* daemonControlMenu = new CMenuWidget(LOCALE_DAEMON_CONTROL, NEUTRINO_ICON_SETTINGS, width, MN_WIDGET_ID_DAEMON_CONTROL);
	daemonControlMenu->addIntroItems();

	CMenuOptionChooser *mc;
	CFlagFileNotifier * flagFileNotifier[DAEMONS_COUNT];
	for (unsigned int i = 0; i < DAEMONS_COUNT; i++)
	{
		std::string flagfile = CONFIGDIR;
		flagfile += "/.";
		flagfile += daemons_data[i].flag;

		daemons_data[i].flag_exist = file_exists(flagfile.c_str());

		flagFileNotifier[i] = new CFlagFileNotifier(daemons_data[i].flag);

		mc = new CMenuOptionChooser(daemons_data[i].name, &daemons_data[i].flag_exist, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, flagFileNotifier[i], CRCInput::convertDigitToKey(daemon_shortcut++));
		mc->setHint(daemons_data[i].icon, daemons_data[i].desc);
		daemonControlMenu->addItem(mc);
	}

	int res = daemonControlMenu->exec(NULL,"");
	daemonControlMenu->hide();
	for (unsigned int i = 0; i < DAEMONS_COUNT; i++)
		delete flagFileNotifier[i];
	delete daemonControlMenu;
	return res;
}

// ----------------------------------------------------------------------------

CCamdControlMenu::CCamdControlMenu()
{
	width = 40;
}

CCamdControlMenu::~CCamdControlMenu()
{
}

int CCamdControlMenu::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{
	if (parent)
		parent->hide();

	return show();
}

typedef struct camds_data_t
{
	neutrino_locale_t name;
	neutrino_locale_t desc;
	const char * camd_name;
	const char * camd_file;
	int camd_exist;
	int camd_runs;
}
camds_data_struct;

camds_data_t camds_data[]=
{
	{LOCALE_CAMD_ITEM_MGCAMD_NAME,		LOCALE_CAMD_ITEM_MGCAMD_HINT,	"MGCAMD",		"mgcamd",		0, 0},
	{LOCALE_CAMD_ITEM_DOSCAM_NAME,		LOCALE_CAMD_ITEM_DOSCAM_HINT,	"DOSCAM",		"doscam",		0, 0},
	{LOCALE_CAMD_ITEM_NCAM_NAME,		LOCALE_CAMD_ITEM_NCAM_HINT,		"NCAM",			"ncam",			0, 0},
	{LOCALE_CAMD_ITEM_OSMOD_NAME,		LOCALE_CAMD_ITEM_OSMOD_HINT,	"OSMOD",		"osmod",		0, 0},
	{LOCALE_CAMD_ITEM_OSCAM_NAME,		LOCALE_CAMD_ITEM_OSCAM_HINT,	"OSCAM",		"oscam",		0, 0},
	{LOCALE_CAMD_ITEM_CCCAM_NAME,		LOCALE_CAMD_ITEM_CCCAM_HINT,	"CCCAM",		"cccam",		0, 0},
	{LOCALE_CAMD_ITEM_GBOX_NAME,		LOCALE_CAMD_ITEM_GBOX_HINT,		"GBOX.NET",		"gbox",			0, 0},
	{LOCALE_CAMD_ITEM_CIPHELPER_NAME,	LOCALE_CAMD_ITEM_CIPHELPER_HINT,"CI+ Helper",	"ciplushelper",	0, 0}
};
#define CAMDS_COUNT (sizeof(camds_data)/sizeof(struct camds_data_t))

int CCamdControlMenu::show()
{
	int camd_shortcut = 0;

	char *buffer;
	ssize_t read;
	size_t len;
	FILE *fh;

	CMenuWidget* camdControlMenu = new CMenuWidget(LOCALE_CAMD_CONTROL, NEUTRINO_ICON_SETTINGS, width, MN_WIDGET_ID_CAMD_CONTROL);
	camdControlMenu->addIntroItems();

	// camd reset
	CMenuForwarder *mf = new CMenuForwarder(LOCALE_CAMD_RESET, true, NULL, CNeutrinoApp::getInstance(), "camd_reset", CRCInput::RC_red);
	mf->setHint(NEUTRINO_ICON_HINT_SETTINGS, LOCALE_MENU_HINT_CAMD_RESET);
	camdControlMenu->addItem(mf);

	camdControlMenu->addItem(GenericMenuSeparatorLine);

	CMenuOptionChooser *mc;
	CFlagFileNotifier * flagFileNotifier[CAMDS_COUNT];
	for (unsigned int i = 0; i < CAMDS_COUNT; i++)
	{
		std::string camd_binary = "/var/emu/";
		camd_binary += camds_data[i].camd_file;

		camds_data[i].camd_exist = file_exists(camd_binary.c_str());

		if (getpidof(camds_data[i].camd_file))
			camds_data[i].camd_runs = 1;
		else
			camds_data[i].camd_runs = 0;

		std::string hint(g_Locale->getText(camds_data[i].desc));

		flagFileNotifier[i] = new CFlagFileNotifier(camds_data[i].camd_file);

		mc = new CMenuOptionChooser(camds_data[i].name, &camds_data[i].camd_runs, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, camds_data[i].camd_exist, flagFileNotifier[i], CRCInput::convertDigitToKey(camd_shortcut++));
		mc->setHint(NEUTRINO_ICON_HINT_SETTINGS, hint);
		camdControlMenu->addItem(mc);
	}

	int res = camdControlMenu->exec(NULL,"");
	camdControlMenu->hide();
	for (unsigned int i = 0; i < CAMDS_COUNT; i++)
		delete flagFileNotifier[i];
	delete camdControlMenu;
	return res;
}
