/*
	Neutrino-GUI  -   DBoxII-Project

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

	Copyright (C) 2017 TangoCash

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <global.h>
#include <neutrino.h>
#include "widget/menue.h"
#include <system/helpers.h>
#include <system/setting_helpers.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/keyboard_input.h>
#include <gui/widget/msgbox.h>
#include <driver/screen_max.h>

#include <sys/stat.h>
#include <sys/time.h>

#include "skins.h"

#define USERDIR "/var" THEMESDIR
#define SKIN_SUFFIX "skin.conf"
static	SNeutrinoSkin &s = g_settings.skin;

CSkins::CSkins()
: skinfile('\t')
{
}

void CSkins::readFile(std::string filename)
{
	std::string skinname = filename + "/" + SKIN_SUFFIX;

	if(skinfile.loadConfig(skinname))
	{
		getSkin(skinfile);
		g_settings.skinfiles = filename + "/";
	}
	else
	{
		printf("[neutrino skin] %s not found\n", skinname.c_str());
		g_settings.skinfiles = "none";
	}
}

void CSkins::readSkinFile(std::string filename)
{
	if (filename.compare("none") == 0)
	{
		CConfigFile empty(':');
		getSkin(empty);
		return;
	}

	std::string skinname = filename + SKIN_SUFFIX;

	if(skinfile.loadConfig(skinname))
	{
		getSkin(skinfile);
		g_settings.skinfiles = filename;
	}
	else
	{
		printf("[neutrino readskin] %s not found\n", skinname.c_str());
		g_settings.skinfiles = "none";
	}
}

void CSkins::getSkin(CConfigFile &configfile)
{
	s.ReloadSkin = configfile.getBool( "ReloadSkin", true );
	s.skinEnabled = configfile.getBool( "skinEnabled", false );
	s.bgpic = configfile.getString("bgpic","infoviewer.png");
	s.bgX = configfile.getInt32( "bgX", 0 );
	s.bgY = configfile.getInt32( "bgY", 0 );
	s.bgW = configfile.getInt32( "bgW", 1280 );
	s.bgH = configfile.getInt32( "bgH", 333 );
	s.logoEnabled = configfile.getBool( "logoEnabled", true );
	s.logoX = configfile.getInt32( "logoX", 0 );
	s.logoY = configfile.getInt32( "logoY", 0 );
	s.logoW = configfile.getInt32( "logoW", 220 );
	s.logoH = configfile.getInt32( "logoH", 132 );
	s.clockEnabled = configfile.getBool( "clockEnabled", true );
	s.clockX = configfile.getInt32( "clockX", 0 );
	s.clockY = configfile.getInt32( "clockY", 0 );
	s.clockColor = configfile.getInt32( "clockColor", 0 );
	s.satInfoEnabled = configfile.getBool( "satInfoEnabled", false );
	s.satInfoX = configfile.getInt32( "satInfoX", 0 );
	s.satInfoY = configfile.getInt32( "satInfoY", 0 );
	s.satInfoColor = configfile.getInt32( "satInfoColor", 0 );
	s.displayWithLogo = configfile.getBool( "displayWithLogo", true );
	s.channelNameX = configfile.getInt32( "channelNameX", 0 );
	s.channelNameY = configfile.getInt32( "channelNameY", 0 );
	s.ChannelNameFontSize = configfile.getInt32( "ChannelNameFontSize", 20 );
	s.channelNameColor = configfile.getInt32( "channelNameColor", 0 );
	s.EventsX = configfile.getInt32( "EventsX", 0 );
	s.EventsY = configfile.getInt32( "EventsY", 0 );
	s.EventsW = configfile.getInt32( "EventsW", 1200 );
	s.EventsFontSize = configfile.getInt32( "EventsFontSize", 14 );
	s.EventsColor = configfile.getInt32( "EventsColor", 0 );
	s.IconsX = configfile.getInt32( "IconsX", 800 );
	s.IconsY = configfile.getInt32( "IconsY", 10 );
	s.BbarEnabled = configfile.getBool( "BbarEnabled", true );
	s.BbarOffset = configfile.getInt32( "BbarOffset", 5 );
	s.CabarEnabled = configfile.getBool( "CabarEnabled", true );
	s.CabarOffset = configfile.getInt32( "CabarOffset", 5 );
	s.header_bgpic = configfile.getString("header_bgpic","");
	s.header_h = configfile.getInt32( "header_h", 39 );
}

void CSkins::setSkin(CConfigFile &configfile)
{
	configfile.setBool( "ReloadSkin", s.ReloadSkin );
	configfile.setBool( "skinEnabled", s.skinEnabled );
	configfile.setString("bgpic", s.bgpic );
	configfile.setInt32( "bgX", s.bgX );
	configfile.setInt32( "bgY", s.bgY );
	configfile.setInt32( "bgW", s.bgW );
	configfile.setInt32( "bgX", s.bgH );
	configfile.setBool( "logoEnabled", s.logoEnabled );
	configfile.setInt32( "logoX", s.logoX );
	configfile.setInt32( "logoY", s.logoY );
	configfile.setInt32( "logoW", s.logoW );
	configfile.setInt32( "logoH", s.logoH );
	configfile.setBool( "clockEnabled", s.clockEnabled );
	configfile.setInt32( "clockX", s.clockX );
	configfile.setInt32( "clockY", s.clockY );
	configfile.setInt32( "clockColor", s.clockColor );
	configfile.setBool( "satInfoEnabled", s.satInfoEnabled );
	configfile.setInt32( "satInfoX", s.satInfoX );
	configfile.setInt32( "satInfoY", s.satInfoY );
	configfile.setInt32( "satInfoColor", s.satInfoColor );
	configfile.setBool( "displayWithLogo", s.displayWithLogo );
	configfile.setInt32( "channelNameX", s.channelNameX );
	configfile.setInt32( "channelNameY", s.channelNameY );
	configfile.setInt32( "ChannelNameFontSize", s.ChannelNameFontSize );
	configfile.setInt32( "channelNameColor", s.channelNameColor );
	configfile.setInt32( "EventsX", s.EventsX );
	configfile.setInt32( "EventsY", s.EventsY );
	configfile.setInt32( "EventsW", s.EventsW );
	configfile.setInt32( "EventsFontSize", s.EventsFontSize );
	configfile.setInt32( "EventsColor", s.EventsColor );
	configfile.setInt32( "IconsX", s.IconsX );
	configfile.setInt32( "IconsY", s.IconsY );
	configfile.setBool( "BbarEnabled", s.BbarEnabled );
	configfile.setInt32( "BbarOffset", s.BbarOffset );
	configfile.setBool( "CabarEnabled", s.CabarEnabled );
	configfile.setInt32( "CabarOffset", s.CabarOffset );
	configfile.setString("header_bgpic", s.header_bgpic );
	configfile.setInt32( "header_h", s.header_h );
}
