/*
	$Id: osdlang_setup.cpp,v 1.2 2010/09/30 20:13:59 dbt Exp $

	OSD-Language Setup  implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2010 T. Graf 'dbt'
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


#include "osdlang_setup.h"

#include <global.h>
#include <neutrino.h>
#include <mymenu.h>
#include <neutrino_menue.h>

#include <gui/widget/icons.h>

#include <driver/screen_max.h>

#include <xmlinterface.h>
#include <system/debug.h>

#include <algorithm>
#include <dirent.h>
#include <eitd/sectionsd.h>



COsdLangSetup::COsdLangSetup(bool wizard_mode)
{
	is_wizard = wizard_mode;

	width = w_max (45, 10);
}

COsdLangSetup::~COsdLangSetup()
{

}

int COsdLangSetup::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_DEBUG, "init international setup\n");
	if(parent != NULL)
		parent->hide();

	int res = showLocalSetup();

	return res;
}

//show international settings menu
int COsdLangSetup::showLocalSetup()
{
	//main local setup
	CMenuWidget *localSettings = new CMenuWidget(LOCALE_MAINSETTINGS_HEAD, NEUTRINO_ICON_LANGUAGE, width, MN_WIDGET_ID_LANGUAGESETUP);
	localSettings->setWizardMode(is_wizard);

	//add subhead and back button
	localSettings->addIntroItems(LOCALE_LANGUAGESETUP_HEAD);

	//language setup
	CMenuWidget osdl_setup(LOCALE_LANGUAGESETUP_OSD, NEUTRINO_ICON_LANGUAGE, width, MN_WIDGET_ID_LANGUAGESETUP_LOCALE);
	showLanguageSetup(&osdl_setup);

	CMenuForwarder * mf = new CMenuForwarder(LOCALE_LANGUAGESETUP_OSD, true, NULL, &osdl_setup, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
	mf->setHint("", LOCALE_MENU_HINT_OSD_LANGUAGE);
	localSettings->addItem(mf);

 	//timezone setup
	CMenuOptionStringChooser* tzSelect = getTzItems();
	if (tzSelect != NULL)
		localSettings->addItem(tzSelect);

	//prefered audio language
	CLangSelectNotifier *langNotifier = new CLangSelectNotifier();
	CMenuWidget prefMenu(LOCALE_AUDIOMENU_PREF_LANGUAGES, NEUTRINO_ICON_LANGUAGE, width, MN_WIDGET_ID_LANGUAGESETUP_PREFAUDIO_LANGUAGE);
	//call menue for prefered audio languages
	showPrefMenu(&prefMenu, langNotifier);

	mf = new CMenuForwarder(LOCALE_AUDIOMENU_PREF_LANGUAGES, true, NULL, &prefMenu, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
	mf->setHint("", LOCALE_MENU_HINT_LANG_PREF);
	localSettings->addItem(mf);
	//langNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	int res = localSettings->exec(NULL, "");
	delete localSettings;
	delete langNotifier;
	return res;
}


//returns items for selectable timezones
CMenuOptionStringChooser* COsdLangSetup::getTzItems()
{
	xmlDocPtr parser = parseXmlFile("/etc/timezone.xml");

	CMenuOptionStringChooser* tzSelect = NULL;
	if (parser != NULL)
	{
		tzSelect = new CMenuOptionStringChooser(LOCALE_MAINSETTINGS_TIMEZONE, g_settings.timezone, true, new CTZChangeNotifier(), CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true);
		tzSelect->setHint("", LOCALE_MENU_HINT_TIMEZONE);
		xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
		bool found = false;

		while (search)
		{
			if (!strcmp(xmlGetName(search), "zone"))
			{
				std::string name = xmlGetAttribute(search, "name");
//				std::string zone = xmlGetAttribute(search, "zone");
				//printf("Timezone: %s -> %s\n", name.c_str(), zone.c_str());
				tzSelect->addOption(name.c_str());
				found = true;
			}
			search = search->xmlNextNode;
		}

		if (!found)
		{
			delete tzSelect;
			tzSelect = NULL;
		}

		xmlFreeDoc(parser);
	}

	return tzSelect;
}


//shows locale setup for language selection
void COsdLangSetup::showLanguageSetup(CMenuWidget *osdl_setup)
{
	struct dirent **namelist;
	int n;
	//		printf("scanning locale dir now....(perhaps)\n");
	char *pfad[] = {(char *) DATADIR "/neutrino/locale",(char *) CONFIGDIR "/locale"};

	osdl_setup->addIntroItems();

	for (int p = 0; p < 2; p++)
	{
		n = scandir(pfad[p], &namelist, 0, alphasort);
		if (n < 0)
		{
			perror("loading locales: scandir");
		}
		else
		{
			for (int count=0; count<n; count++)
			{
				char * locale = namelist[count]->d_name;
				char * pos = strstr(locale, ".locale");
				if (pos != NULL)
				{
					*pos = '\0';
					CMenuOptionLanguageChooser* oj = new CMenuOptionLanguageChooser((char*)locale, this, locale);
					osdl_setup->addItem( oj );
				}
				free(namelist[count]);
			}
			free(namelist);
		}
	}
}

//shows menue for prefered audio/epg languages
void COsdLangSetup::showPrefMenu(CMenuWidget *prefMenu, CLangSelectNotifier *langNotifier)
{
	prefMenu->addItem(GenericMenuSeparator);
	prefMenu->addItem(GenericMenuBack);
	prefMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_LANG_HEAD));

	CMenuOptionChooser * mc = new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_LANG, &g_settings.auto_lang, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL);
	mc->setHint("", LOCALE_MENU_HINT_AUTO_LANG);
	prefMenu->addItem(mc);

	for(int i = 0; i < 3; i++)
	{
		CMenuOptionStringChooser * langSelect = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_LANG, g_settings.pref_lang[i], true, langNotifier, CRCInput::convertDigitToKey(i+1), "", true);
		langSelect->setHint("", LOCALE_MENU_HINT_PREF_LANG);
		langSelect->addOption("none");
		std::map<std::string, std::string>::const_iterator it;
		for(it = iso639rev.begin(); it != iso639rev.end(); ++it)
			langSelect->addOption(it->first.c_str());

		prefMenu->addItem(langSelect);
	}

	prefMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_SUBS_HEAD));
	mc = new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_SUBS, &g_settings.auto_subs, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL);
	mc->setHint("", LOCALE_MENU_HINT_AUTO_SUBS);
	prefMenu->addItem(mc);
	for(int i = 0; i < 3; i++)
	{
		CMenuOptionStringChooser * langSelect = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_SUBS, g_settings.pref_subs[i], true, NULL, CRCInput::convertDigitToKey(i+4), "", true);
		langSelect->setHint("", LOCALE_MENU_HINT_PREF_SUBS);
		std::map<std::string, std::string>::const_iterator it;
		langSelect->addOption("none");
		for(it = iso639rev.begin(); it != iso639rev.end(); ++it)
			langSelect->addOption(it->first.c_str());

		prefMenu->addItem(langSelect);
	}
}

bool COsdLangSetup::changeNotify(const neutrino_locale_t, void *)
{
	//apply osd language
	g_Locale->loadLocale(g_settings.language);

	return true;
}

bool CLangSelectNotifier::changeNotify(const neutrino_locale_t, void *)
{
	std::vector<std::string> v_languages;
	//bool found = false;
	std::map<std::string, std::string>::const_iterator it;

	//prefered audio languages
	for(int i = 0; i < 3; i++)
	{
		if(strlen(g_settings.pref_lang[i]) && strcmp(g_settings.pref_lang[i], "none"))
		{
			printf("setLanguages: %d: %s\n", i, g_settings.pref_lang[i]);

			std::string temp(g_settings.pref_lang[i]);
			for(it = iso639.begin(); it != iso639.end(); ++it)
			{
				if(temp == it->second)
				{
					v_languages.push_back(it->first);
					printf("setLanguages: adding %s\n", it->first.c_str());
					//found = true;
				}
			}
		}
	}
	CEitManager::getInstance()->setLanguages(v_languages);

	return false;
}
