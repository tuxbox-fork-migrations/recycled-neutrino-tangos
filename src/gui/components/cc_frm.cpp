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

using namespace std;

//-------------------------------------------------------------------------------------------------------
//sub class CComponentsForm from CComponentsItem
CComponentsForm::CComponentsForm()
{
	//CComponentsForm
	initVarForm();
}

CComponentsForm::CComponentsForm(const int x_pos, const int y_pos, const int w, const int h, bool has_shadow,
				fb_pixel_t color_frame, fb_pixel_t color_body, fb_pixel_t color_shadow)
{
	//CComponentsForm
	initVarForm();

	//CComponents
	x		= x_pos;
	y 		= y_pos;
	width 		= w;
	height	 	= h;

	shadow		= has_shadow;
	col_frame 	= color_frame;
	col_body	= color_body;
	col_shadow	= color_shadow;
}

CComponentsForm::~CComponentsForm()
{
	cleanCCForm();
}

void CComponentsForm::cleanCCForm()
{
#ifdef DEBUG_CC
	printf("[CComponents] calling %s...\n", __FUNCTION__);
#endif
	hide();
	clearSavedScreen();
	clearCCItems();
	clear();
}

void CComponentsForm::clearCCItems()
{
	if (v_cc_items.empty())
		return;
#ifdef DEBUG_CC
	printf("[CComponentsForm] %s... cleanup %d form cc-items\n", __FUNCTION__, v_cc_items.size());
#endif	
	for(size_t i=0; i<v_cc_items.size(); i++) {
		if (v_cc_items[i])
			delete v_cc_items[i];
		v_cc_items[i] = NULL;
	}
	v_cc_items.clear();
}


void CComponentsForm::initVarForm()
{
	//CComponentsItem
	initVarItem();
	  
	
	//simple default dimensions
	x 		= 0;
	y 		= 0;
	width 		= 150;
	height	 	= 150;
	shadow		= CC_SHADOW_OFF;
	shadow_w	= SHADOW_OFFSET;
	col_frame 	= COL_MENUCONTENT_PLUS_6;
	col_body	= COL_MENUCONTENT_PLUS_0;
	col_shadow	= COL_MENUCONTENTDARK_PLUS_0;
	corner_rad	= RADIUS_LARGE;
	corner_type 	= CORNER_ALL;
	
	//CComponentsForm
	v_cc_items.clear();
	
}

void CComponentsForm::addCCItem(CComponentsItem* cc_Item)
{
	v_cc_items.push_back(cc_Item);
}

int CComponentsForm::getCCItemId(CComponentsItem* cc_Item)
{
	for (size_t i= 0; i< v_cc_items.size(); i++)
		if (v_cc_items[i] == cc_Item)
			return i;
	return -1;
}

CComponentsItem* CComponentsForm::getCCItem(const uint& cc_item_id)
{
	if (v_cc_items[cc_item_id])
		return v_cc_items[cc_item_id];
	return NULL;
}

void CComponentsForm::replaceCCItem(const uint& cc_item_id, CComponentsItem* new_cc_Item)
{
	if (!v_cc_items.empty()){
		if (v_cc_items[cc_item_id]){
			delete v_cc_items[cc_item_id];
			v_cc_items[cc_item_id] = NULL;
			v_cc_items[cc_item_id] = new_cc_Item;
		}
	}
#ifdef DEBUG_CC
	else
		printf("[CComponentsForm]  %s replace cc_Item not possible, v_cc_items is empty\n", __FUNCTION__);
#endif

}

void CComponentsForm::replaceCCItem(CComponentsItem* old_cc_Item, CComponentsItem* new_cc_Item)
{
	replaceCCItem(getCCItemId(old_cc_Item), new_cc_Item);
}

void CComponentsForm::insertCCItem(const uint& cc_item_id, CComponentsItem* cc_Item)
{

	if (cc_Item == NULL){
#ifdef DEBUG_CC
		printf("[CComponentsForm]  %s parameter: cc_Item = %d...\n", __FUNCTION__, (int)cc_Item);
#endif
		return;
	}
	
	if (v_cc_items.empty()){
		v_cc_items.push_back(cc_Item);
#ifdef DEBUG_CC
		printf("[CComponentsForm]  %s insert cc_Item not possible, v_cc_items is empty, cc_Item added\n", __FUNCTION__);
#endif
	}else
		v_cc_items.insert(v_cc_items.begin()+cc_item_id, cc_Item);
}

void CComponentsForm::removeCCItem(const uint& cc_item_id)
{
	if (!v_cc_items.empty()){
		if (v_cc_items[cc_item_id]) {
			delete v_cc_items[cc_item_id];
			v_cc_items[cc_item_id] = NULL;
			v_cc_items.erase(v_cc_items.begin()+cc_item_id);
		}
	}
#ifdef DEBUG_CC
	else
		printf("[CComponentsForm]  %s removing cc_Item not possible, v_cc_items is empty...\n", __FUNCTION__);
#endif
}

void CComponentsForm::paint(bool do_save_bg)
{
	//paint body
	paintInit(do_save_bg);
	
	//paint items
	paintCCItems();	
}

void CComponentsForm::paintCCItems()
{	
	size_t items_count = v_cc_items.size();
	int x_frm_left 		= x+fr_thickness; //left form border
	int y_frm_top 		= y+fr_thickness; //top form border
	int x_frm_right		= x+width-fr_thickness; //right form border
	int y_frm_bottom	= y+height-fr_thickness; //bottom form border
		
	for(size_t i=0; i<items_count; i++) {
		//cache original item position and dimensions
		int x_item, y_item, w_item, h_item;
		v_cc_items[i]->getDimensions(&x_item, &y_item, &w_item, &h_item);
		

		int xy_ref = 0+fr_thickness; //allowed minimal x and y start position
		if (x_item < xy_ref){
#ifdef DEBUG_CC
			printf("[CComponentsForm] %s: item %d position is out of form dimensions\ndefinied x=%d\nallowed x>=%d\n", __FUNCTION__, i, x_item, xy_ref);
#endif
			x_item = xy_ref;
		}
		if (y_item < xy_ref){
#ifdef DEBUG_CC
			printf("[CComponentsForm] %s: item %d position is out of form dimensions\ndefinied y=%d\nallowed y>=%d\n", __FUNCTION__, i, y_item, xy_ref);
#endif
			y_item = xy_ref;
		}
		
		//set adapted position onto form
		v_cc_items[i]->setXPos(x_frm_left+x_item);
		v_cc_items[i]->setYPos(y_frm_top+y_item);

		//watch horizontal x dimensions of items
		int x_item_right = v_cc_items[i]->getXPos()+w_item; //right item border
		if (x_item_right > x_frm_right){
			v_cc_items[i]->setWidth(w_item-(x_item_right-x_frm_right));
#ifdef DEBUG_CC
			printf("[CComponentsForm] %s: item %d too large, definied width=%d, possible width=%d \n", __FUNCTION__, i, w_item, v_cc_items[i]->getWidth());
#endif
		}

		//watch vertical y dimensions
		int y_item_bottom = v_cc_items[i]->getYPos()+h_item; //bottom item border
		if (y_item_bottom > y_frm_bottom){
			v_cc_items[i]->setHeight(h_item-(y_item_bottom-y_frm_bottom));
#ifdef DEBUG_CC
			printf("[CComponentsForm] %s: item %d too large, definied height=%d, possible height=%d \n", __FUNCTION__, i, h_item, v_cc_items[i]->getHeight());
#endif
		}
		
		//paint element without saved screen!
		v_cc_items[i]->paint(CC_SAVE_SCREEN_NO);
		
		//restore dimensions and position
		v_cc_items[i]->setDimensionsAll(x_item, y_item, w_item, h_item);
	}
}

void CComponentsForm::hide(bool no_restore)
{
	//hide body
	hideCCItem(no_restore);
}
