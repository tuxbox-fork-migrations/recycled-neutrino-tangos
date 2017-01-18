/*
	Based up Neutrino-GUI - Tuxbox-Project
	Copyright (C) 2001 by Steffen Hehn 'McClean'

	Classes for generic GUI-related components.
	Copyright (C) 2012, 2013, 2014, Thilo Graf 'dbt'

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

#ifndef __CC_FORM_FOOTER_H__
#define __CC_FORM_FOOTER_H__

#include "cc_frm_header.h"
#include "cc_frm_button.h"
#include <global.h>
#include <gui/widget/buttons.h> //for compatibility with 'button_label' type

//for 'button_label' type with string
typedef struct button_label_s
{
	const char *		button;
	std::string 		text;
	neutrino_msg_t 		directKey;
	neutrino_msg_t 		directKeyAlt;
	int 			btn_result;
	int 			btn_alias;
} button_label_s_struct;

typedef struct button_label_l
{
	const char *      	button;
	neutrino_locale_t 	locale;
	neutrino_msg_t		directKey;
	neutrino_msg_t 		directKeyAlt;
	int 			btn_result;
	int 			btn_alias;
} button_label_l_struct;

/*!
CComponentsFooter, sub class of CComponentsHeader provides prepared container for footer
It's usable like a header but without caption, and context button icons as default.
Additional implemented is a button container (chain) and some methods which can add
buttons via struct, vector or text and icon parameters. Also a compatible but limited methode
to add button labels known by older button handler, to find in gui/widget/buttons.h/cpp.
functionality. Why limited ?: old parameter 'struct button_label' doesn't provide newer parameters. 
Missing parameters are filled with default values and must be assigned afterward, if required.
*/
class CComponentsFooter : public CComponentsHeader
{
	private:
		void initVarFooter(	const int& x_pos, const int& y_pos, const int& w, const int& h,
					const int& buttons,
					CComponentsForm *parent,
					int shadow_mode,
					fb_pixel_t color_frame,
					fb_pixel_t color_body,
					fb_pixel_t color_shadow );

		///show button with background, default false
		bool ccf_enable_button_bg;
		///enable button with shadow mode, default CC_SHADOW_OFF
		int ccf_enable_button_shadow;
		///set button shadow button width
		int ccf_button_shadow_width;
		///set button shadow button repaint mode
		bool ccf_button_shadow_force_paint;
		///enable/disable button frame in icon color, predefined for red, green, yellow and blue, default disabled
		bool btn_auto_frame_col;

		///property: set font for label caption, see also setButtonFont()
		Font* ccf_btn_font;

		///init default fonts for size modes
		virtual void initDefaultFonts();

		///container for button objects
		CComponentsFrmChain *chain;

	public:
		CComponentsFooter(CComponentsForm *parent = NULL);
		CComponentsFooter(	const int& x_pos, const int& y_pos, const int& w, const int& h = 0,
					const int& buttons = 0,
					CComponentsForm *parent = NULL,
					int shadow_mode = CC_SHADOW_OFF,
					fb_pixel_t color_frame = COL_FRAME_PLUS_0,
					fb_pixel_t color_body = COL_MENUFOOT_PLUS_0,
					fb_pixel_t color_shadow = COL_SHADOW_PLUS_0);

		///add button labels with string label type as content, count as size_t, chain_width as int, label width as int
		void setButtonLabels(const struct button_label_s * const content, const size_t& label_count, const int& chain_width = 0, const int& label_width = 0);
		///add button labels with locale label type as content, count as size_t, chain_width as int, label width as int
		void setButtonLabels(const struct button_label_l * const content, const size_t& label_count, const int& chain_width = 0, const int& label_width = 0);
		///add button labels with locale label type as content, parameter 1 as vector, chain_width as int, label width as int
		void setButtonLabels(const std::vector<button_label_l> &v_content, const int& chain_width, const int& label_width);
		///add button labels with string label type as content, parameter 1 as vector, chain_width as int, label width as int
		void setButtonLabels(const std::vector<button_label_s> &v_content, const int& chain_width, const int& label_width);

		///enable/disable button frame in icon color, predefined for red, green, yellow and blue
		inline void enableButtonFrameColor(bool enable = true){btn_auto_frame_col = enable;}

		///add button labels with old label type, count as size_t, chain_width as int, label width as int
		///NOTE: for compatibility with older button handler find in gui/widget/buttons.h, if possible, don't use this
		void setButtonLabels(const struct button_label * const content, const size_t& label_count, const int& chain_width = 0, const int& label_width = 0);

		///add single button label with string label type as content, chain_width as int, label width as int
		void setButtonLabel(	const char *button_icon,
					const std::string& text,
					const int& chain_width = 0,
					const int& label_width = 0,
					const neutrino_msg_t& msg = CRCInput::RC_nokey,
					const int& result_value = -1,
					const int& alias_value = -1,
					const neutrino_msg_t& directKeyAlt = CRCInput::RC_nokey);
		///add single button label with locale label type as content, chain_width as int, label width as int
		void setButtonLabel(	const char *button_icon,
					const neutrino_locale_t& locale,
					const int& chain_width = 0,
					const int& label_width = 0,
					const neutrino_msg_t& msg = CRCInput::RC_nokey,
					const int& result_value = -1,
					const int& alias_value = -1,
					const neutrino_msg_t& directKeyAlt = CRCInput::RC_nokey);
		
		///enables background of buttons, parameter bool show, default= true
		void enableButtonBg(bool enable = true);
		///disables background of buttons
		void disableButtonBg(){enableButtonBg(false);}

		/**Select a definied button inside button chain object
		* @param[in]	item_id
		* 	@li 	optional: exepts type size_t
		* @param[in]	fr_col
		* 	@li 	optional: exepts type fb_pixel_t, as default frame color
		* @param[in]	sel_fr_col
		* 	@li 	optional: exepts type fb_pixel_t, as selected frame color
		* @param[in]	bg_col
		* 	@li 	optional: exepts type fb_pixel_t, as default background color
		* @param[in]	sel_bg_col
		* 	@li 	optional: exepts type fb_pixel_t, as selected background color
		* @param[in]	text_col
		* 	@li 	optional: exepts type fb_pixel_t, as default text color
		* @param[in]	sel_text_col
		* 	@li 	optional: exepts type fb_pixel_t, as selected text color
		* @param[in]	frame_width
		* 	@li 	optional: exepts type int, default = 1
		* @param[in]	sel_frame_width
		* 	@li 	optional: exepts type int, default = 2
		*/
		void setSelectedButton(size_t item_id,
					const fb_pixel_t& fr_col 	= COL_MENUCONTENTSELECTED_PLUS_2,
					const fb_pixel_t& sel_fr_col 	= COL_MENUCONTENTSELECTED_PLUS_0,
					const fb_pixel_t& bg_col 	= COL_MENUCONTENT_PLUS_0,
					const fb_pixel_t& sel_bg_col 	= COL_MENUCONTENTSELECTED_PLUS_0,
					const fb_pixel_t& text_col 	= COL_MENUCONTENT_TEXT,
					const fb_pixel_t& sel_text_col 	= COL_MENUCONTENTSELECTED_TEXT,
					const int& frame_width 		= 1,
					const int& sel_frame_width 	= 1);
		///returns id of select button, return value as int, -1 = nothing is selected
		int getSelectedButton();
		///returns selected button object, return value as pointer to object, NULL means nothing is selected
		CComponentsButton* getSelectedButtonObject();

		/*!
		Sets a new text to an already predefined button.
		1st parameter 'btn_id' accepts current id of an already defined button. 2nd parameter sets the new text as std::string
		Usage:
		Buttons come with any text eg. 'Ok', 'No', 'Yes' ...whatever and this member allows to manipulate the text via button id.
		Button id means the showed button begins from the left position of button chain, starts with value=0, also to get via getButtonChainObject()->getCCItemId([CComponentsButton*])
		example: 1st buttons text is 'Hello', 2nd Button's text is 'You!',
		Now we want to change the text of 2nd button to 'World", so we must do this:
			setButtonText(1, "World");
		Wrong id's will be ignored.
		*/
		void setButtonText(const uint& btn_id, const std::string& text);

		///property: set font for label caption, parameter as font object, value NULL causes usage of dynamic font
		void setButtonFont(Font* font){ccf_btn_font = font;};

		///returns pointer to internal button container
		CComponentsFrmChain* getButtonChainObject(){return chain;};


		///this is a nearly methode similar with the older button handler find in gui/widget/buttons.h, some parameters are different, but require minimalized input
		///this member sets some basic parameters and will paint concurrently on execute, explicit call of paint() is not required
		void paintButtons(	const int& x_pos,
					const int& y_pos,
					const int& w,
					const int& h,
					const size_t& label_count,
					const struct button_label * const content,
					const int& label_width = 0,
					const int& context_buttons = 0,
					Font* font = g_Font[SNeutrinoSettings::FONT_TYPE_BUTTON_TEXT],
					bool do_save_bg = CC_SAVE_SCREEN_NO
				);

		enum
		{
			CC_FOOTER_SIZE_LARGE 	= 0,
			CC_FOOTER_SIZE_SMALL 	= 1
		};
		///set size of footer, possible values are CC_FOOTER_SIZE_LARGE, CC_FOOTER_SIZE_SMALL
		virtual void setSizeMode(const int& size_mode){cch_size_mode = size_mode; initCCItems();}

		///enable and sets shadow properties for embedded buttons
		void enableButtonShadow(int mode = CC_SHADOW_ON, const int& shadow_width = OFFSET_SHADOW/2, bool force_paint = false);
		///disable shadow for embedded buttons
		void disbaleButtonShadow(){enableButtonShadow(CC_SHADOW_OFF);}
};

#endif
