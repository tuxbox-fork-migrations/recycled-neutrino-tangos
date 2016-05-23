/*
	lcd4l_setup

	(C) 2009-2016 NG-Team
	(C) 2016 NI-Team
	(C) 2016 TangoCash

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
#include <iostream>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <unistd.h>

#include <global.h>
#include <neutrino.h>
#include <mymenu.h>
#include <neutrino_menue.h>

#include <gui/filebrowser.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>

#include <gui/lcd4l_setup.h>

#include <system/helpers.h>

#include <driver/screen_max.h>

// lcd4l-support
#include "gui/lcd4l.h"
extern CLCD4l *LCD4l;

const CMenuOptionChooser::keyval LCD4L_SUPPORT_OPTIONS[] =
{
	{ 0, LOCALE_LCD4L_SUPPORT_OFF },
	{ 1, LOCALE_LCD4L_SUPPORT_AUTO },
	{ 2, LOCALE_LCD4L_SUPPORT_ON }
};
#define LCD4L_SUPPORT_OPTION_COUNT (sizeof(LCD4L_SUPPORT_OPTIONS)/sizeof(CMenuOptionChooser::keyval))

const CMenuOptionChooser::keyval LCD4L_SKIN_OPTIONS[] =
{
	{ 0, LOCALE_LCD4L_SKIN_0 },
	{ 1, LOCALE_LCD4L_SKIN_1 },
	{ 2, LOCALE_LCD4L_SKIN_2 }
};
#define LCD4L_SKIN_OPTION_COUNT (sizeof(LCD4L_SKIN_OPTIONS)/sizeof(CMenuOptionChooser::keyval))

CLCD4Linux_Setup::CLCD4Linux_Setup()
{
	width = 40;
}

CLCD4Linux_Setup::~CLCD4Linux_Setup()
{
}

CLCD4Linux_Setup* CLCD4Linux_Setup::getInstance()
{
	static CLCD4Linux_Setup* LCD4Linux_Setup = NULL;
	if (!LCD4Linux_Setup)
		LCD4Linux_Setup = new CLCD4Linux_Setup();
	return LCD4Linux_Setup;
}

int CLCD4Linux_Setup::exec(CMenuTarget* parent, const std::string &actionkey)
{
	printf("CLCD4Linux_Setup::exec: actionkey %s\n", actionkey.c_str());
	int res = menu_return::RETURN_REPAINT;
	char *buffer;
	ssize_t read;
	size_t len;
	FILE *fh;
	std::ostringstream buf;

        if (parent)
                parent->hide();

	if (actionkey=="lcd4l_logodir") {
		const char *action_str = "lcd4l_logodir";
		chooserDir(g_settings.lcd4l_logodir, false, action_str);
		return menu_return::RETURN_REPAINT;
	}

	res = show();

	return res;
}

bool CLCD4Linux_Setup::changeNotify(const neutrino_locale_t OptionName, void * /*data*/)
{
#if 0
	int val = 0;

	if (data)
		val = (*(int *)data);
#endif

	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_LCD4L_SUPPORT))
	{
		LCD4l->StopLCD4l();
		if (g_settings.lcd4l_support)
			LCD4l->StartLCD4l();
	}

	return false;
}

int CLCD4Linux_Setup::show()
{
	int shortcut = 1;
	int lcd4l_shortcut = 1;

	std::ostringstream buf;
	char *buffer;
	ssize_t read;
	size_t len;
	FILE *fh;

	// lcd4l menu
	CMenuWidget* lcd4lMenu = new CMenuWidget(LOCALE_LCD4L_SUPPORT, NEUTRINO_ICON_SETTINGS, width, MN_WIDGET_ID_LCD4L_SETUP);
	lcd4lMenu->addIntroItems();

	int temp_lcd4l_skin	= g_settings.lcd4l_skin;
	int flag_lcd4l_weather	= file_exists(FLAG_DIR ".lcd-weather");
	int flag_lcd4l_clock_a	= file_exists(FLAG_DIR ".lcd-clock_a");

	mc = new CMenuOptionChooser(LOCALE_LCD4L_SUPPORT, &g_settings.lcd4l_support, LCD4L_SUPPORT_OPTIONS, LCD4L_SUPPORT_OPTION_COUNT, true, this, CRCInput::RC_red);
	mc->setHint("", LOCALE_MENU_HINT_LCD4L_SUPPORT);
	lcd4lMenu->addItem(mc);
	lcd4lMenu->addItem(GenericMenuSeparatorLine);

	mf = new CMenuForwarder(LOCALE_LCD4L_LOGODIR, true, g_settings.lcd4l_logodir, this, "lcd4l_logodir", CRCInput::convertDigitToKey(lcd4l_shortcut++));
	mf->setHint("", LOCALE_MENU_HINT_LCD4L_LOGODIR);
	lcd4lMenu->addItem(mf);

	mc = new CMenuOptionChooser(LOCALE_LCD4L_SKIN, &temp_lcd4l_skin, LCD4L_SKIN_OPTIONS, LCD4L_SKIN_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(lcd4l_shortcut++));
	mc->setHint("", LOCALE_MENU_HINT_LCD4L_SKIN);
	lcd4lMenu->addItem(mc);

	CNITouchFileNotifier * lcd_weatherFileNotifier = new CNITouchFileNotifier("lcd-weather");
	mc = new CMenuOptionChooser(LOCALE_LCD4L_WEATHER, &flag_lcd4l_weather, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, (file_exists("/share/lcd/scripts/weather")), lcd_weatherFileNotifier, CRCInput::convertDigitToKey(lcd4l_shortcut++));
	mc->setHint("", LOCALE_MENU_HINT_LCD4L_WEATHER);
	lcd4lMenu->addItem(mc);

	CNITouchFileNotifier * lcd_clock_aFileNotifier = new CNITouchFileNotifier("lcd-clock_a");
	mc = new CMenuOptionChooser(LOCALE_LCD4L_CLOCK_A, &flag_lcd4l_clock_a, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcd_clock_aFileNotifier, CRCInput::convertDigitToKey(lcd4l_shortcut++));
	mc->setHint("", LOCALE_MENU_HINT_LCD4L_CLOCK_A);
	lcd4lMenu->addItem(mc);

	int res = lcd4lMenu->exec(NULL, "");

	lcd4lMenu->hide();
	delete lcd4lMenu;

	// the things to do on exit

	if (g_settings.lcd4l_skin != temp_lcd4l_skin)
	{
		g_settings.lcd4l_skin = temp_lcd4l_skin;
		LCD4l->InitLCD4l();
	}

	return res;
}

bool CNITouchFileNotifier::changeNotify(const neutrino_locale_t, void * data)
{
	std::ostringstream buf;

	buf << FLAG_DIR << "." << filename;
	std::string flag = buf.str();

	if ((*(int *)data) != 0)
	{
		FILE * fd = fopen(flag.c_str(), "w");
		if (fd)
		{
			fclose(fd);
			if (strstr(filename, "lcd-weather"))
			{
				// do nothing
			}
			else if (strstr(filename, "lcd-clock_a"))
			{
				// do nothing
			}
			else
			{
				// do nothing
			}
		}
	}
	else
	{
		buf.str("");
		if (strstr(filename, "lcd-weather"))
		{
			// do nothing
		}
		else if (strstr(filename, "lcd-clock_a"))
		{
			// do nothing
		}
		else
		{
			// do nothing
		}
	remove(flag.c_str());
	}
	return menu_return::RETURN_REPAINT;
}
