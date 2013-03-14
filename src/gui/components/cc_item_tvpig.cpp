/*
	Based up Neutrino-GUI - Tuxbox-Project 
	Copyright (C) 2001 by Steffen Hehn 'McClean'

	Classes for generic GUI-related components.
	Copyright (C) 2012, 2013, Thilo Graf 'dbt'
	Copyright (C) 2012, Michael Liebmann 'micha-bbg'

	License: GPL

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>
#include "cc.h"

#include <video.h>

extern cVideo * videoDecoder;

using namespace std;

//-------------------------------------------------------------------------------------------------------
//sub class CComponentsPIP from CComponentsItem
CComponentsPIP::CComponentsPIP(	const int x_pos, const int y_pos, const int percent, bool has_shadow)
{
	//CComponents, CComponentsItem
	initVarItem();
	cc_item_type 	= CC_ITEMTYPE_PIP;

	//CComponentsPIP
	screen_w = frameBuffer->getScreenWidth(true);
	screen_h = frameBuffer->getScreenHeight(true);

	//CComponents
	x 		= x_pos;
	y 		= y_pos;
	width 		= percent*screen_w/100;
	height	 	= percent*screen_h/100;
	shadow		= has_shadow;
	shadow_w	= SHADOW_OFFSET;
	col_frame 	= COL_BACKGROUND;
	col_body	= COL_BACKGROUND;
	col_shadow	= COL_MENUCONTENTDARK_PLUS_0;
}

CComponentsPIP::~CComponentsPIP()
{
 	hide();
 	videoDecoder->Pig(-1, -1, -1, -1);
 	clearSavedScreen();
 	clear();
}

void CComponentsPIP::paint(bool do_save_bg)
{
	paintInit(do_save_bg);
	videoDecoder->Pig(x+fr_thickness, y+fr_thickness, width-2*fr_thickness, height-2*fr_thickness, screen_w, screen_h);
}


void CComponentsPIP::hide(bool no_restore)
{
	videoDecoder->Pig(-1, -1, -1, -1);
	hideCCItem(no_restore);
}
