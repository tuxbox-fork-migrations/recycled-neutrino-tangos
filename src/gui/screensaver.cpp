/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2012-2013 defans@bluepeercrew.us

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
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <global.h>
#include <neutrino.h>
#include <pthread.h>
#include <algorithm>    // std::sort
#include <ctype.h>
#include "audiomute.h"
#include "screensaver.h"

#include <video.h>
extern cVideo * videoDecoder;


CScreenSaver::CScreenSaver()
{
	thrScreenSaver 	= 0;
	m_frameBuffer 	= CFrameBuffer::getInstance();
	index 		= 0;
}

CScreenSaver::~CScreenSaver()
{
	if(thrScreenSaver)
		pthread_cancel(thrScreenSaver);
	thrScreenSaver = 0;
}


CScreenSaver* CScreenSaver::getInstance()
{
	static CScreenSaver * screenSaver = NULL;
	if (!screenSaver)
		screenSaver = new CScreenSaver();

	return screenSaver;
}


void CScreenSaver::Start()
{
	CAudioMute::getInstance()->enableMuteIcon(false);

	if(!thrScreenSaver)
	{
		//printf("[%s] %s: starting thread\n", __FILE__, __FUNCTION__);
		pthread_create(&thrScreenSaver, NULL, ScreenSaverPrg, (void*) this);
		pthread_detach(thrScreenSaver);
	}

}

void CScreenSaver::Stop()
{
	if(thrScreenSaver)
	{
		pthread_cancel(thrScreenSaver);
		thrScreenSaver = 0;
	}

	if(thrScreenSaver)
		pthread_cancel(thrScreenSaver);
	thrScreenSaver = 0;

	CAudioMute::getInstance()->enableMuteIcon(true);
}

void* CScreenSaver::ScreenSaverPrg(void* arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	CScreenSaver * PScreenSaver = static_cast<CScreenSaver*>(arg);

	PScreenSaver->ReadDir(); //TODO kill Screensaver if false
	PScreenSaver->m_frameBuffer->Clear();

	while(1)
	{
		PScreenSaver->PaintPicture();
		sleep(10);
	}
	return 0;
}

bool CScreenSaver::ReadDir()
{
	std::string d = g_settings.screensaver_dir;
	if (d.length() > 1)
	{
		//remove trailing slash
		std::string::iterator it = d.end() - 1;
		if (*it == '/')
			d.erase(it);
	}

	char *dir_name = (char *) d.c_str();
	struct dirent *dirpointer;
	DIR *dir;
	char curr_ext[5];
	int curr_lenght;
	char *p;
	bool ret = false;

	v_bg_files.clear();

	/* open dir */
	if((dir=opendir(dir_name)) == NULL) {
		fprintf(stderr,"[CScreenSaver] Error opendir ...\n");
		return ret;
	}

	/* read complete dir */
	while((dirpointer=readdir(dir)) != NULL)
	{
		curr_lenght = strlen((*dirpointer).d_name);
		string str = dir_name;
//		printf("%d\n",curr_lenght);
		if(curr_lenght > 4)
		{
			strncpy(curr_ext,(*dirpointer).d_name+(curr_lenght-4),5);

			for (p = curr_ext; *p; ++p)
				*p = (char)tolower(*p);
//			printf("%s\n",curr_ext);

			if((strcmp(".jpg",curr_ext))
				//|| (strcmp(".png",curr_ext))
				//|| (strcmp(".bmp",curr_ext))
			)
			continue;

			str += "/";
			str += (*dirpointer).d_name;
			v_bg_files.push_back(str);
		}
	}

	sort(v_bg_files.begin(), v_bg_files.end());

	/* close pointer */
	if(closedir(dir) == -1)
		printf("[CScreenSaver] Error no closed %s\n", dir_name);

	if(!v_bg_files.empty())
		ret = true;
	else
		printf("[CScreenSaver] no picture found\n");

	return ret;
}


void CScreenSaver::PaintPicture()
{
	if(v_bg_files.empty())
		return;

	if( (index >= v_bg_files.size()) || (access(v_bg_files.at(index).c_str(), F_OK)) )
	{
		ReadDir();
		index = 0;
		return;
	}

	printf("[CScreenSaver] ShowPicture: %s\n", v_bg_files.at(index).c_str());
	videoDecoder->StopPicture();
	videoDecoder->ShowPicture(v_bg_files.at(index).c_str());
	index++;

	if(index ==  v_bg_files.size())
		index = 0;
}
