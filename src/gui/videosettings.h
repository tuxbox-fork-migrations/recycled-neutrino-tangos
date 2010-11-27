/*
	$port: video_setup.cpp,v 1.6 2010/08/28 23:06:59 tuxbox-cvs Exp $

	video setup implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2009 T. Graf 'dbt'
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
#ifndef __video_setup__
#define __video_setup__

#include <gui/widget/menue.h>
#include <driver/framebuffer.h>

#include <string>

class CVideoSettings : public CMenuWidget, CChangeObserver
{
	private:
		CFrameBuffer 		*frameBuffer;
		CMenuForwarder 		*SyncControlerForwarder;
		CMenuOptionChooser 	*cec1, *cec2;
		CMenuOptionChooser 	*VcrVideoOutSignalOptionChooser;

		int			vcr_video_out_signal;
		int			prev_video_mode;
		
		bool is_wizard;
		
		int x, y, width, height, hheight, mheight;
		void showVideoSetup();

public:
		enum VIDEO_SETUP_MODE
		{
			V_SETUP_MODE_WIZARD_NO   = 0,
			V_SETUP_MODE_WIZARD   = 1
		};
		
		CVideoSettings(bool wizard_mode = V_SETUP_MODE_WIZARD_NO);
		~CVideoSettings();
		
		virtual bool changeNotify(const neutrino_locale_t OptionName, void *data);
		//virtual void paint();
		void nextMode();
		void next43Mode();
		void SwitchFormat();
		
		void setVideoCECSettings();
		void setVideoSettings();
		void setCECSettings();
		void setupVideoSystem(bool do_ask);
		
		bool getWizardMode() {return is_wizard;};
		void setWizardMode(bool mode);
		
		void hide();
		
		int exec(CMenuTarget* parent, const std::string & actionKey);
};
#endif

