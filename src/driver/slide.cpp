/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2022 TangoCash

	License: GPLv2

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

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

#include <global.h>
#include <neutrino.h>
#include <driver/rcinput.h>
#include <driver/slide.h>
#include <system/proc_tools.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

COSDSlider::COSDSlider(int percent, int mode)
{
	slideTimer = 0;
	slideIn = false;
	slideOut = false;
	slideMode = mode;
	if (mode == MID2SIDE)
		slideMax = MAX_WIDTH;
	else
		slideMax = MAX_HEIGHT * percent / 100;
	slideValue = slideMax;
	setValue(0);
}

COSDSlider::~COSDSlider()
{
	StopSlide();
}

void COSDSlider::setValue(int val)
{
	//printf("setvalue: %d\n",val);

	if (slideMode == MID2SIDE)
	{
		int w = MAX_WIDTH - val;
		int l = (MAX_WIDTH - w) / 2;
		proc_put_hex("/proc/stb/fb/dst_width", w);
		proc_put_hex("/proc/stb/fb/dst_apply", 1);
		proc_put_hex("/proc/stb/fb/dst_left", l);
	}
	else
	{
		proc_put_hex("/proc/stb/fb/dst_top", val);
	}
	proc_put_hex("/proc/stb/fb/dst_apply", 1);
}

void COSDSlider::StartSlideIn()
{
	setValue(slideMax);
	usleep(5000);
	slideIn = true;
	slideOut = false;
	slideValue = slideMax;
	slideTimer = g_RCInput->addTimer(SLIDETIME, false, true);
}

/* return true if slide out started */
bool COSDSlider::StartSlideOut()
{
	bool ret = false;

	if (slideIn)
	{
		g_RCInput->killTimer(slideTimer);
		slideTimer = 0;
		slideIn = false;
		g_RCInput->postMsg(NeutrinoMessages::EVT_SLIDER, AFTER_SLIDEIN);
	}

	if (!slideOut)
	{
		slideOut = true;
		g_RCInput->postMsg(NeutrinoMessages::EVT_SLIDER, BEFORE_SLIDEOUT);
		setValue(0);
		slideTimer = g_RCInput->addTimer(SLIDETIME, false, true);
		ret = true;
	}

	return ret;
}

void COSDSlider::StopSlide()
{
	if (slideIn || slideOut)
	{
		g_RCInput->killTimer(slideTimer);
		slideTimer = 0;
		//usleep(SLIDETIME * 3);
		setValue(0);
		slideIn = slideOut = false;
	}
}

/* return true, if slide out done */
bool COSDSlider::SlideDone()
{
	bool ret = false;

	if (slideOut) // disappear
	{
		slideValue += STEPSIZE;

		if (slideValue >= slideMax)
		{
			slideValue = slideMax;
			g_RCInput->killTimer(slideTimer);
			slideTimer = 0;
			ret = true;
		}
		else
			setValue(slideValue);
	}
	else // appear
	{
		slideValue -= STEPSIZE;

		if (slideValue <= 0)
		{
			slideValue = 0;
			g_RCInput->killTimer(slideTimer);
			slideTimer = 0;
			slideIn = false;
			g_RCInput->postMsg(NeutrinoMessages::EVT_SLIDER, AFTER_SLIDEIN);
		}
		else
			setValue(slideValue);
	}

	return ret;
}
