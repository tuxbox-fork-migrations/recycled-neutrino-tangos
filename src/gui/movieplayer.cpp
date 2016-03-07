/*
  Neutrino-GUI  -   DBoxII-Project

  Movieplayer (c) 2003, 2004 by gagga
  Based on code by Dirch, obi and the Metzler Bros. Thanks.

  Copyright (C) 2011 CoolStream International Ltd

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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <global.h>
#include <neutrino.h>

#include <gui/audiomute.h>
#include <gui/audio_select.h>
#include <gui/epgview.h>
#include <gui/eventlist.h>
#include <gui/movieplayer.h>
#include <gui/infoviewer.h>
#include <gui/timeosd.h>
#include <gui/widget/helpbox.h>
#include <gui/infoclock.h>
#include <gui/plugins.h>
#include <gui/videosettings.h>
#include <gui/streaminfo2.h>
#include <gui/lua/luainstance.h>
#include <gui/lua/lua_video.h>
#include <gui/screensaver.h>
#include <driver/screenshot.h>
#include <driver/volume.h>
#include <driver/display.h>
#include <driver/abstime.h>
#include <driver/record.h>
#include <driver/volume.h>
#include <eitd/edvbstring.h>
#include <system/helpers.h>

#include <src/mymenu.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <sys/mount.h>
#include <json/json.h>

#include <video.h>
#include <libtuxtxt/teletext.h>
#include <zapit/zapit.h>
#include <system/set_threadname.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iconv.h>
#include <system/stacktrace.h>

extern cVideo * videoDecoder;
extern CRemoteControl *g_RemoteControl;	/* neutrino.cpp */

extern CVolume* g_volume;

#define TIMESHIFT_SECONDS 3
#define ISO_MOUNT_POINT "/media/iso"

CMoviePlayerGui* CMoviePlayerGui::instance_mp = NULL;
CMoviePlayerGui* CMoviePlayerGui::instance_bg = NULL;
OpenThreads::Mutex CMoviePlayerGui::mutex;
OpenThreads::Mutex CMoviePlayerGui::bgmutex;
OpenThreads::Condition CMoviePlayerGui::cond;
pthread_t CMoviePlayerGui::bgThread;
cPlayback *CMoviePlayerGui::playback;
bool CMoviePlayerGui::webtv_started;
CMovieBrowser* CMoviePlayerGui::moviebrowser;
CBookmarkManager * CMoviePlayerGui::bookmarkmanager;

CMoviePlayerGui& CMoviePlayerGui::getInstance(bool background)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> m_lock(mutex);
	if (!instance_mp )
	{
		instance_mp = new CMoviePlayerGui();
		instance_bg = new CMoviePlayerGui();
		printf("[neutrino CMoviePlayerGui] Instance created...\n");
	}
	return background ? *instance_bg : *instance_mp;
}

CMoviePlayerGui::CMoviePlayerGui()
{
	Init();
}

CMoviePlayerGui::~CMoviePlayerGui()
{
	//playback->Close();
	if (this == instance_mp)
		stopPlayBack();
	delete moviebrowser;
	moviebrowser = NULL;
	delete filebrowser;
	filebrowser = NULL;
	delete bookmarkmanager;
	bookmarkmanager = NULL;
	delete playback;
	playback = NULL;
	if (this == instance_mp) {
		delete instance_bg;
		instance_bg = NULL;
	}
	instance_mp = NULL;
}

void CMoviePlayerGui::Init(void)
{
	playing = false;
	stopped = true;

	frameBuffer = CFrameBuffer::getInstance();

	if (playback == NULL)
		playback = new cPlayback(0);
	if (moviebrowser == NULL)
		moviebrowser = new CMovieBrowser();
	if (bookmarkmanager == NULL)
		bookmarkmanager = new CBookmarkManager();

	tsfilefilter.addFilter("ts");
	tsfilefilter.addFilter("avi");
	tsfilefilter.addFilter("mkv");
	tsfilefilter.addFilter("wav");
	tsfilefilter.addFilter("asf");
	tsfilefilter.addFilter("aiff");
	tsfilefilter.addFilter("mpg");
	tsfilefilter.addFilter("mpeg");
	tsfilefilter.addFilter("m2p");
	tsfilefilter.addFilter("mpv");
	tsfilefilter.addFilter("vob");
	tsfilefilter.addFilter("m2ts");
	tsfilefilter.addFilter("mp4");
	tsfilefilter.addFilter("mov");
	tsfilefilter.addFilter("m3u");
	tsfilefilter.addFilter("m3u8");
	tsfilefilter.addFilter("pls");
	tsfilefilter.addFilter("iso");

	if (g_settings.network_nfs_moviedir.empty())
		Path_local = "/";
	else
		Path_local = g_settings.network_nfs_moviedir;

	if (g_settings.filebrowser_denydirectoryleave)
		filebrowser = new CFileBrowser(Path_local.c_str());
	else
		filebrowser = new CFileBrowser();

	filebrowser->Filter = &tsfilefilter;
	filebrowser->Hide_records = true;
	filebrowser->Multi_Select = true;
	filebrowser->Dirs_Selectable = true;

	speed = 1;
	timeshift = TSHIFT_MODE_OFF;
	numpida = 0;
	showStartingHint = false;

	min_x = 0;
	max_x = 0;
	min_y = 0;
	max_y = 0;
	ext_subs = false;
	iso_file = false;
	lock_subs = false;
	bgThread = 0;
	info_1 = "";
	info_2 = "";
	filelist_it = filelist.end();
	vzap_it = filelist_it;
	fromInfoviewer = false;
	keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_NORMAL;
	isLuaPlay = false;
	haveLuaInfoFunc = false;
	blockedFromPlugin = false;
	m_screensaver = false;
	m_idletime = time(NULL);
}

void CMoviePlayerGui::cutNeutrino()
{
	printf("%s: playing %d isUPNP %d\n", __func__, playing, isUPNP);
	if (playing)
		return;

	playing = true;
	/* set g_InfoViewer update timer to 1 sec, should be reset to default from restoreNeutrino->set neutrino mode  */
	if (!isWebTV)
		g_InfoViewer->setUpdateTimer(1000 * 1000);

	if (isUPNP)
		return;

	g_Zapit->lockPlayBack();
	if (!isWebTV)
		g_Sectionsd->setPauseScanning(true);

	m_LastMode = (CNeutrinoApp::getInstance()->getMode() /*| NeutrinoMessages::norezap*/);
	if (isWebTV)
		m_LastMode |= NeutrinoMessages::norezap;
	printf("%s: save mode %x\n", __func__, m_LastMode);fflush(stdout);
	int new_mode = NeutrinoMessages::norezap | (isWebTV ? NeutrinoMessages::mode_webtv : NeutrinoMessages::mode_ts);
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, new_mode);
}

void CMoviePlayerGui::restoreNeutrino()
{
	printf("%s: playing %d isUPNP %d\n", __func__, playing, isUPNP);
	if (!playing)
		return;

	playing = false;

	if (isUPNP)
		return;

	//g_Zapit->unlockPlayBack();
	CZapit::getInstance()->EnablePlayback(true);
	g_Sectionsd->setPauseScanning(false);

	printf("%s: restore mode %x\n", __func__, m_LastMode);fflush(stdout);
#if 0
	if (m_LastMode == NeutrinoMessages::mode_tv)
		g_RCInput->postMsg(NeutrinoMessages::EVT_PROGRAMLOCKSTATUS, 0x200, false);
#endif
	if (m_LastMode != NeutrinoMessages::mode_unknown)
		CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, m_LastMode);

#if 0
	if (m_LastMode == NeutrinoMessages::mode_tv) {
		CZapitChannel *channel = CZapit::getInstance()->GetCurrentChannel();
		if (channel && channel->scrambled)
			CZapit::getInstance()->Rezap();
	}
#endif
	printf("%s: restoring done.\n", __func__);fflush(stdout);
}

int CMoviePlayerGui::exec(CMenuTarget * parent, const std::string & actionKey)
{
	printf("[movieplayer] actionKey=%s\n", actionKey.c_str());

	if (parent)
		parent->hide();

	puts("[movieplayer.cpp] executing " MOVIEPLAYER_START_SCRIPT ".");
	if (my_system(MOVIEPLAYER_START_SCRIPT) != 0)
		perror(MOVIEPLAYER_START_SCRIPT " failed");

	Cleanup();
	ClearFlags();
	ClearQueue();

	if (actionKey == "tsmoviebrowser") {
		isMovieBrowser = true;
		moviebrowser->setMode(MB_SHOW_RECORDS);
	}
	else if (actionKey == "ytplayback") {
		isMovieBrowser = true;
		moviebrowser->setMode(MB_SHOW_YT);
		isYT = true;
	}
	else if (actionKey == "fileplayback") {
	}
	else if (actionKey == "timeshift") {
		timeshift = TSHIFT_MODE_ON;
	}
	else if (actionKey == "ptimeshift") {
		timeshift = TSHIFT_MODE_PAUSE;
	}
	else if (actionKey == "rtimeshift") {
		timeshift = TSHIFT_MODE_REWIND;
	}
#if 0 // TODO ?
	else if (actionKey == "bookmarkplayback") {
		isBookmark = true;
	}
#endif
	else if (actionKey == "upnp") {
		isUPNP = true;
		is_file_player = true;
		PlayFile();
	}
	else if (actionKey == "http") {
		isHTTP = true;
		is_file_player = true;
		PlayFile();
	}
	else if (actionKey == "http_lua") {
		isHTTP = true;
		isLuaPlay = true;
		is_file_player = true;
		PlayFile();
		haveLuaInfoFunc = false;
	}
	else {
		return menu_return::RETURN_REPAINT;
	}

	while(!isHTTP && !isUPNP && SelectFile()) {
		if (timeshift != TSHIFT_MODE_OFF) {
			PlayFile();
			break;
		}
		do {
			PlayFile();
		}
		while (repeat_mode || filelist_it != filelist.end());
	}

	bookmarkmanager->flush();

	puts("[movieplayer.cpp] executing " MOVIEPLAYER_END_SCRIPT ".");
	if (my_system(MOVIEPLAYER_END_SCRIPT) != 0)
		perror(MOVIEPLAYER_END_SCRIPT " failed");

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	if (timeshift != TSHIFT_MODE_OFF){
		timeshift = TSHIFT_MODE_OFF;
		return menu_return::RETURN_EXIT_ALL;
	}
	return menu_ret;
}

void CMoviePlayerGui::updateLcd()
{
	char tmp[20];
	std::string lcd;
	std::string name;

	if (isMovieBrowser && p_movie_info && !p_movie_info->epgTitle.empty() && p_movie_info->epgTitle.size() && strncmp(p_movie_info->epgTitle.c_str(), "not", 3))
		name = p_movie_info->epgTitle;
	else
		name = pretty_name;

	switch (playstate) {
		case CMoviePlayerGui::PAUSE:
			if (speed < 0) {
				sprintf(tmp, "%dx<| ", abs(speed));
				lcd = tmp;
			} else if (speed > 0) {
				sprintf(tmp, "%dx|> ", abs(speed));
				lcd = tmp;
			} else
				lcd = "|| ";
			break;
		case CMoviePlayerGui::REW:
			sprintf(tmp, "%dx<< ", abs(speed));
			lcd = tmp;
			break;
		case CMoviePlayerGui::FF:
			sprintf(tmp, "%dx>> ", abs(speed));
			lcd = tmp;
			break;
		case CMoviePlayerGui::PLAY:
			lcd = "> ";
			break;
		default:
			break;
	}
	lcd += name;
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);
	CVFD::getInstance()->showMenuText(0, lcd.c_str(), -1, true);
}

void CMoviePlayerGui::fillPids()
{
	if (p_movie_info == NULL)
		return;

	vpid = p_movie_info->epgVideoPid;
	vtype = p_movie_info->VideoType;
	numpida = 0; currentapid = 0;
	/* FIXME: better way to detect TS recording */
	if (!p_movie_info->audioPids.empty()) {
		currentapid = p_movie_info->audioPids[0].epgAudioPid;
		currentac3 = p_movie_info->audioPids[0].atype;
	} else if (!vpid) {
		is_file_player = true;
		return;
	}
	for (int i = 0; i < (int)p_movie_info->audioPids.size(); i++) {
		apids[i] = p_movie_info->audioPids[i].epgAudioPid;
		ac3flags[i] = p_movie_info->audioPids[i].atype;
		numpida++;
		if (p_movie_info->audioPids[i].selected) {
			currentapid = p_movie_info->audioPids[i].epgAudioPid;
			currentac3 = p_movie_info->audioPids[i].atype;
		}
	}
}

void CMoviePlayerGui::Cleanup()
{
	for (int i = 0; i < numpida; i++) {
		apids[i] = 0;
		ac3flags[i] = 0;
		language[i].clear();
	}
	numpida = 0; currentapid = 0;
	currentspid = -1;
	numsubs = 0;
	vpid = 0;
	vtype = 0;

	startposition = -1;
	is_file_player = false;
	p_movie_info = NULL;
	autoshot_done = false;
	currentaudioname = "Unk";
}

void CMoviePlayerGui::ClearFlags()
{
	isMovieBrowser = false;
	isBookmark = false;
	isHTTP = false;
	isLuaPlay = false;
	isUPNP = false;
	isWebTV = false;
	isYT = false;
	is_file_player = false;
	timeshift = TSHIFT_MODE_OFF;
}

void CMoviePlayerGui::ClearQueue()
{
	repeat_mode = REPEAT_OFF;
	filelist.clear();
	filelist_it = filelist.end();
	milist.clear();
}

void CMoviePlayerGui::EnableClockAndMute(bool enable)
{
	CAudioMute::getInstance()->enableMuteIcon(enable);
	CInfoClock::getInstance()->enableInfoClock(enable);
}

void CMoviePlayerGui::makeFilename()
{
	if (pretty_name.empty()) {
		std::string::size_type pos = file_name.find_last_of('/');
		if (pos != std::string::npos) {
			pretty_name = file_name.substr(pos+1);
			std::replace(pretty_name.begin(), pretty_name.end(), '_', ' ');
		} else
			pretty_name = file_name;

		if (pretty_name.substr(0,14)=="videoplayback?") {//youtube name
			if (!p_movie_info->epgTitle.empty())
				pretty_name = p_movie_info->epgTitle;
			else
				pretty_name = "";
		}
		printf("CMoviePlayerGui::makeFilename: file_name [%s] pretty_name [%s]\n", file_name.c_str(), pretty_name.c_str());
	}
}

bool CMoviePlayerGui::prepareFile(CFile *file)
{
	bool ret = true;

	numpida = 0; currentapid = 0;
	currentspid = -1;
	numsubs = 0;
	autoshot_done = 0;
	if (file->Url.empty())
		file_name = file->Name;
	else {
		file_name = file->Url;
		pretty_name = file->Name;
	}
	if (isMovieBrowser) {
		if (filelist_it != filelist.end()) {
			unsigned idx = filelist_it - filelist.begin();
			if(milist.size() > idx){
				p_movie_info = milist[idx];
				startposition = p_movie_info->bookmarks.start > 0 ? p_movie_info->bookmarks.start*1000 : -1;
				printf("CMoviePlayerGui::prepareFile: file %s start %d\n", file_name.c_str(), startposition);
			}
		}
		if (isYT) {
			file_name = file->Url;
			is_file_player = true;
		}
		fillPids();
	}
	if (file->getType() == CFile::FILE_ISO)
		ret = mountIso(file);
	//else if (file->getType() == CFile::FILE_PLAYLIST)
		//parsePlaylist(file);

	if (ret)
		makeFilename();
	return ret;
}

bool CMoviePlayerGui::SelectFile()
{
	bool ret = false;
	menu_ret = menu_return::RETURN_REPAINT;

	Cleanup();
	info_1 = "";
	info_2 = "";
	pretty_name.clear();
	file_name.clear();
	cookie_header.clear();
	//reinit Path_local for webif reloadsetup
	if (g_settings.network_nfs_moviedir.empty())
		Path_local = "/";
	else
		Path_local = g_settings.network_nfs_moviedir;

	printf("CMoviePlayerGui::SelectFile: isBookmark %d timeshift %d isMovieBrowser %d\n", isBookmark, timeshift, isMovieBrowser);
	wakeup_hdd(g_settings.network_nfs_recordingdir.c_str());

	if (timeshift != TSHIFT_MODE_OFF) {
		t_channel_id live_channel_id = CZapit::getInstance()->GetCurrentChannelID();
		p_movie_info = CRecordManager::getInstance()->GetMovieInfo(live_channel_id);
		file_name = CRecordManager::getInstance()->GetFileName(live_channel_id) + ".ts";
		fillPids();
		makeFilename();
		ret = true;
	}
#if 0 // TODO
	else if (isBookmark) {
		const CBookmark * theBookmark = bookmarkmanager->getBookmark(NULL);
		if (theBookmark == NULL) {
			bookmarkmanager->flush();
			return false;
		}
		file_name = theBookmark->getUrl();
		sscanf(theBookmark->getTime(), "%lld", &startposition);
		startposition *= 1000;
		ret = true;
	}
#endif
	else if (isMovieBrowser) {
		EnableClockAndMute(false);
		if (moviebrowser->exec(Path_local.c_str())) {
			Path_local = moviebrowser->getCurrentDir();
			CFile *file =  NULL;
			filelist_it = filelist.end();
			if (moviebrowser->getSelectedFiles(filelist, milist)) {
				filelist_it = filelist.begin();
				p_movie_info = *(milist.begin());
// 				file = &(*filelist_it);
			}
			else if ((file = moviebrowser->getSelectedFile()) != NULL) {
				p_movie_info = moviebrowser->getCurrentMovieInfo();
				startposition = 1000 * moviebrowser->getCurrentStartPos();
				printf("CMoviePlayerGui::SelectFile: file %s start %d apid %X atype %d vpid %x vtype %d\n", file_name.c_str(), startposition, currentapid, currentac3, vpid, vtype);

			}
			if (p_movie_info)
				ret = prepareFile(&p_movie_info->file);
		} else
			menu_ret = moviebrowser->getMenuRet();
		EnableClockAndMute(true);
	} else { // filebrowser
		EnableClockAndMute(false);
		while (ret == false && filebrowser->exec(Path_local.c_str()) == true) {
			Path_local = filebrowser->getCurrentDir();
			CFile *file = NULL;
			filelist = filebrowser->getSelectedFiles();
			filelist_it = filelist.end();
			if (!filelist.empty()) {
				filelist_it = filelist.begin();
				file = &(*filelist_it);
			}
			if (file) {
				is_file_player = true;
				if (file->getType() == CFile::FILE_PLAYLIST)
					parsePlaylist(file);
				if (!filelist.empty()) {
					filelist_it = filelist.begin();
					file = &(*filelist_it);
				}
				ret = prepareFile(file);
			}
		}
		menu_ret = filebrowser->getMenuRet();
		EnableClockAndMute(true);
	}
	g_settings.network_nfs_moviedir = Path_local;

	return ret;
}

void *CMoviePlayerGui::ShowStartHint(void *arg)
{
	set_threadname(__func__);
	CMoviePlayerGui *caller = (CMoviePlayerGui *)arg;
	CHintBox *hintbox = NULL;
	if (!caller->pretty_name.empty()) {
		hintbox = new CHintBox(LOCALE_MOVIEPLAYER_STARTING, caller->pretty_name.c_str(), 450, NEUTRINO_ICON_MOVIEPLAYER);
		hintbox->paint();
	}
	while (caller->showStartingHint) {
		neutrino_msg_t msg;
		neutrino_msg_data_t data;
		g_RCInput->getMsg(&msg, &data, 1);
		if (msg == CRCInput::RC_home || msg == CRCInput::RC_stop) {
			caller->playback->RequestAbort();
		}
#if 0
		else if (caller->isWebTV) {
			CNeutrinoApp::getInstance()->handleMsg(msg, data);
		}
#endif
		else if (caller->isWebTV && ((msg == (neutrino_msg_t) g_settings.key_quickzap_up ) || (msg == (neutrino_msg_t) g_settings.key_quickzap_down))) {
			caller->playback->RequestAbort();
			g_RCInput->postMsg(msg, data);
		}
		else if (msg != NeutrinoMessages::EVT_WEBTV_ZAP_COMPLETE && msg != CRCInput::RC_timeout && msg > CRCInput::RC_MaxRC) {
			CNeutrinoApp::getInstance()->handleMsg(msg, data);
		}
		else if ((msg>= CRCInput::RC_WithData) && (msg< CRCInput::RC_WithData+ 0x10000000))
                        delete[] (unsigned char*) data;

	}
	if (hintbox != NULL) {
		hintbox->hide();
		delete hintbox;
	}
	return NULL;
}

bool CMoviePlayerGui::StartWebtv(void)
{
	printf("%s: starting...\n", __func__);fflush(stdout);
	last_read = position = duration = 0;

	cutNeutrino();
	clearSubtitle();

	playback->Open(is_file_player ? PLAYMODE_FILE : PLAYMODE_TS);

	bool res = playback->Start((char *) file_name.c_str(), cookie_header);//url with cookies

	playback->SetSpeed(1);
	if (!res) {
		playback->Close();
	} else {
		getCurrentAudioName(is_file_player, currentaudioname);
		if (is_file_player)
			selectAutoLang();
	}

	return res;
}

void* CMoviePlayerGui::bgPlayThread(void *arg)
{
	set_threadname(__func__);
	CMoviePlayerGui *mp = (CMoviePlayerGui *) arg;
	printf("%s: starting... instance %p\n", __func__, mp);fflush(stdout);

	int eof = 0, pos = 0;
	unsigned char *chid = new unsigned char[sizeof(t_channel_id)];
	*(t_channel_id*)chid = mp->movie_info.epgId;

	bool started = mp->StartWebtv();
	printf("%s: started: %d\n", __func__, started);fflush(stdout);
	bool chidused = false;

	mutex.lock();
	if (!webtv_started)
		started = false;
	else if (!started){
		g_RCInput->postMsg(NeutrinoMessages::EVT_ZAP_FAILED, (neutrino_msg_data_t) chid);
		chidused = true;
	}
	webtv_started = started;
	mutex.unlock();

	while(webtv_started) {
		if (mp->playback->GetPosition(mp->position, mp->duration)) {
#if 0
			printf("CMoviePlayerGui::bgPlayThread: position %d duration %d (%d)\n", mp->position, mp->duration, mp->duration-mp->position);
#endif
			if (pos == mp->position)
				eof++;
			else
				eof = 0;
			if (eof > 5) {
				printf("CMoviePlayerGui::bgPlayThread: playback stopped, try to rezap...\n");
				g_RCInput->postMsg(NeutrinoMessages::EVT_WEBTV_ZAP_COMPLETE, (neutrino_msg_data_t) chid);
				chidused = true;
				break;
			}
			pos = mp->position;
		}
		bgmutex.lock();
		int res = cond.wait(&bgmutex, 1000);
		bgmutex.unlock();
		if (res == 0)
			break;
		mp->showSubtitle(0);
	}
	printf("%s: play end...\n", __func__);fflush(stdout);
	mp->PlayFileEnd();
	if(!chidused)
		delete [] chid;

	pthread_exit(NULL);
}

bool CMoviePlayerGui::sortStreamList(livestream_info_t info1, livestream_info_t info2)
{
	return (info1.res1 < info2.res1);
}

bool CMoviePlayerGui::luaGetUrl(const std::string &script, const std::string &file, std::vector<livestream_info_t> &streamList)
{
	CHintBox* box = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_LIVESTREAM_READ_DATA));
	box->paint();

	std::string result_code = "";
	std::string result_string = "";

	std::vector<std::string> args;
	args.push_back(file);

	CLuaInstance *lua = new CLuaInstance();
	lua->runScript(script.c_str(), &args, &result_code, &result_string);
	delete lua;

	if ((result_code != "0") || result_string.empty()) {
		if (box != NULL) {
			box->hide();
			delete box;
		}
		return false;
	}

	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(result_string, root, false);
	if (!parsedSuccess) {
		printf("Failed to parse JSON\n");
		printf("%s\n", reader.getFormattedErrorMessages().c_str());
		if (box != NULL) {
			box->hide();
			delete box;
		}
		return false;
	}

	livestream_info_t info;
	std::string tmp;
	bool haveurl = false;
	if ( !root.isObject() ) {
		for (Json::Value::iterator it = root.begin(); it != root.end(); ++it) {
			info.url=""; info.name=""; info.header=""; info.bandwidth = 1; info.resolution=""; info.res1 = 1;
			tmp = "0";
			Json::Value object_it = *it;
			for (Json::Value::iterator iti = object_it.begin(); iti != object_it.end(); iti++) {
				std::string name = iti.name();
				if (name=="url") {
					info.url = (*iti).asString();
					haveurl = true;
				} else if (name=="name") {
					info.name = (*iti).asString();
				}
				else if (name=="header") {
					info.header = (*iti).asString();
				}
				else if (name=="band") {
					info.bandwidth  = atoi((*iti).asString().c_str());
				} else if (name=="res1") {
					tmp = (*iti).asString();
					info.res1 = atoi(tmp.c_str());
				} else if (name=="res2") {
					info.resolution = tmp + "x" + (*iti).asString();
				}
			}
			if (haveurl) {
				streamList.push_back(info);
			}
			haveurl = false;
		}
	}
	if (root.isObject()) {
		for (Json::Value::iterator it = root.begin(); it != root.end(); ++it) {
			info.url=""; info.name=""; info.header=""; info.bandwidth = 1; info.resolution=""; info.res1 = 1;
			tmp = "0";
			std::string name = it.name();
			if (name=="url") {
				info.url = (*it).asString();
				haveurl = true;
			} else if (name=="name") {
				info.name = (*it).asString();
			} else if (name=="header") {
					info.header = (*it).asString();
			} else if (name=="band") {
				info.bandwidth  = atoi((*it).asString().c_str());
			} else if (name=="res1") {
				tmp = (*it).asString();
				info.res1 = atoi(tmp.c_str());
			} else if (name=="res2") {
				info.resolution = tmp + "x" + (*it).asString();
			}
		}
		if (haveurl) {
			streamList.push_back(info);
		}
	}
	/* sort streamlist */
	std::sort(streamList.begin(), streamList.end(), sortStreamList);

	/* remove duplicate resolutions */
	livestream_info_t *_info;
	int res_old = 0;
	for (size_t i = 0; i < streamList.size(); ++i) {
		_info = &(streamList[i]);
		if (res_old == _info->res1)
			streamList.erase(streamList.begin()+i);
		res_old = _info->res1;
	}

	if (box != NULL) {
		box->hide();
		delete box;
	}

	return true;
}

bool CMoviePlayerGui::selectLivestream(std::vector<livestream_info_t> &streamList, int res, livestream_info_t* info)
{
	livestream_info_t* _info;
	int _res = res;

#if 0
	printf("\n");
	for (size_t i = 0; i < streamList.size(); ++i) {
		_info = &(streamList[i]);
		printf("%d - _info->res1: %4d, _info->res: %9s, _info->bandwidth: %d\n", i, _info->res1, (_info->resolution).c_str(), _info->bandwidth);
	}
	printf("\n");
#endif

	bool resIO = false;
	while (1) {
		size_t i;
		for (i = 0; i < streamList.size(); ++i) {
			_info = &(streamList[i]);
			if (_info->res1 == _res) {
				info->url        = _info->url;
				info->name       = _info->name;
				info->header     = _info->header;
				info->resolution = _info->resolution;
				info->res1       = _info->res1;
				info->bandwidth  = _info->bandwidth;
				return true;
			}
		}
		/* Required resolution not found, decreasing resolution */
		for (i = streamList.size(); i > 0; --i) {
			_info = &(streamList[i-1]);
			if (_info->res1 < _res) {
				_res = _info->res1;
				resIO = true;
				break;
			}
		}
		/* Required resolution not found, increasing resolution */
		if (resIO == false) {
			for (i = 0; i < streamList.size(); ++i) {
				_info = &(streamList[i]);
				if (_info->res1 > _res) {
					_res = _info->res1;
					break;
				}
			}
		}
	}
	return false;
}

bool CMoviePlayerGui::getLiveUrl(const t_channel_id chan, const std::string &url, const std::string &script, std::string &realUrl, std::string &_pretty_name, std::string &info1, std::string &info2, std::string &header)
{
	static t_channel_id oldChan = 0;
	static std::vector<livestream_info_t> liveStreamList;
	livestream_info_t info;

	if (script.empty()) {
		realUrl = url;
		return true;
	}
	std::string _script = script;

	if (_script.find("/") == std::string::npos)
		_script = g_settings.livestreamScriptPath + "/" + _script;

	size_t pos = _script.find(".lua");
	if (!file_exists(_script.c_str()) || (pos == std::string::npos) || (_script.length()-pos != 4)) {
		liveStreamList.clear();
		printf(">>>>> [%s:%s:%d] script error\n", __file__, __func__, __LINE__);
		return false;
	}
	if ((oldChan != chan) || liveStreamList.empty()) {
		liveStreamList.clear();
		if (!luaGetUrl(_script, url, liveStreamList)) {
			liveStreamList.clear();
			printf(">>>>> [%s:%s:%d] lua script error\n", __file__, __func__, __LINE__);
			return false;
		}
		oldChan = chan;
	}

	if (!selectLivestream(liveStreamList, g_settings.livestreamResolution, &info)) {
		liveStreamList.clear();
		printf(">>>>> [%s:%s:%d] error selectLivestream\n", __file__, __func__, __LINE__);
		return false;
	}

	realUrl = info.url;
	if (!info.name.empty()) {
		info1 = info.name;
		_pretty_name = info.name;
	}
	if (!info.header.empty()) {
		header = info.header;
	}

#if 0
	if (!info.resolution.empty())
		info2 = info.resolution;
	if (info.bandwidth > 0) {
		char buf[32];
		memset(buf, '\0', sizeof(buf));
		snprintf(buf, sizeof(buf), "%.02f kbps", (float)((float)info.bandwidth/(float)1000));
		info2 += (std::string)", " + (std::string)buf;
	}
#else
	if (info.bandwidth > 0) {
		char buf[32];
		memset(buf, '\0', sizeof(buf));
		snprintf(buf, sizeof(buf), "%.02f kbps", (float)((float)info.bandwidth/(float)1000));
		info2 = (std::string)buf;
	}
#endif
	return true;
}

bool CMoviePlayerGui::PlayBackgroundStart(const std::string &file, const std::string &name, t_channel_id chan, const std::string &script)
{
	printf("%s: starting...\n", __func__);fflush(stdout);
	static CZapProtection *zp = NULL;
	if (zp)
		return true;

	if (g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_NEVER) {
		int age = -1;
		const char *ages[] = { "18+", "16+", "12+", "6+", "0+", NULL };
		int agen[] = { 18, 16, 12, 6, 0 };
		for (int i = 0; ages[i] && age < 0; i++) {
			const char *n = name.c_str();
			const char *h = n;
			while ((age < 0) && (h = strstr(h, ages[i])))
				if ((h == n) || !isdigit(*(h - 1)))
					age = agen[i];
		}
		if (age > -1 && age >= g_settings.parentallock_lockage && g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_NEVER) {
			zp = new CZapProtection(g_settings.parentallock_pincode, age);
			bool unlocked = zp->check();
			delete zp;
			zp = NULL;
			if (!unlocked)
				return false;
		} else {
			CZapitChannel * channel = CServiceManager::getInstance()->FindChannel(chan);
			if (channel && channel->Locked() != g_settings.parentallock_defaultlocked && !CNeutrinoApp::getInstance()->channelList->checkLockStatus(0x100))
				return false;
		}
	}

	std::string realUrl;
	std::string _pretty_name = name;
	cookie_header.clear();
	if (!getLiveUrl(chan, file, script, realUrl, _pretty_name, livestreamInfo1, livestreamInfo2, cookie_header)) {
		return false;
	}

	instance_bg->Cleanup();
	instance_bg->ClearFlags();
	instance_bg->ClearQueue();

	instance_bg->isWebTV = true;
	instance_bg->is_file_player = true;
	instance_bg->isHTTP = true;
	instance_bg->file_name = realUrl;
	instance_bg->pretty_name = _pretty_name;
	instance_bg->cookie_header = cookie_header;

	instance_bg->movie_info.epgTitle = name;
	instance_bg->movie_info.epgChannel = realUrl;
	instance_bg->movie_info.epgId = chan;
	instance_bg->p_movie_info = &movie_info;

	stopPlayBack();
	webtv_started = true;
	if (pthread_create (&bgThread, 0, CMoviePlayerGui::bgPlayThread, instance_bg)) {
		printf("ERROR: pthread_create(%s)\n", __func__);
		webtv_started = false;
		return false;
	}

	printf("%s: this %p started, thread %lx\n", __func__, this, bgThread);fflush(stdout);
	return true;
}

void CMoviePlayerGui::stopPlayBack(void)
{
	printf("%s: stopping...\n", __func__);
	//playback->RequestAbort();

	repeat_mode = REPEAT_OFF;
	if (bgThread) {
		printf("%s: this %p join background thread %lx\n", __func__, this, bgThread);fflush(stdout);
		mutex.lock();
		webtv_started = false;
		if(playback)
			playback->RequestAbort();
		mutex.unlock();
		cond.broadcast();
		pthread_join(bgThread, NULL);
		bgThread = 0;
	}
	livestreamInfo1.clear();
	livestreamInfo2.clear();
	printf("%s: stopped\n", __func__);
}

void CMoviePlayerGui::Pause(bool b)
{
	if (b && (playstate == CMoviePlayerGui::PAUSE))
		b = !b;
	if (b) {
		playback->SetSpeed(0);
		playstate = CMoviePlayerGui::PAUSE;
	} else {
		playback->SetSpeed(1);
		playstate = CMoviePlayerGui::PLAY;
	}
}

void CMoviePlayerGui::PlayFile(void)
{
	PlayFileStart();
	PlayFileLoop();
	bool repeat = (repeat_mode == REPEAT_OFF);
	if (isLuaPlay)
		repeat = (!blockedFromPlugin);
	PlayFileEnd(repeat);
}

bool CMoviePlayerGui::PlayFileStart(void)
{
	menu_ret = menu_return::RETURN_REPAINT;

	time_forced = false;

	position = 0, duration = 0;
	speed = 1;
	last_read = 0;

	printf("%s: starting...\n", __func__);
	stopPlayBack();

	playstate = CMoviePlayerGui::STOPPED;
	printf("Startplay at %d seconds\n", startposition/1000);
	handleMovieBrowser(CRCInput::RC_nokey, position);

	cutNeutrino();
	if (isWebTV)
		videoDecoder->setBlank(true);

	clearSubtitle();

	printf("IS FILE PLAYER: %s\n", is_file_player ?  "true": "false" );
	playback->Open(is_file_player ? PLAYMODE_FILE : PLAYMODE_TS);

	if (p_movie_info) {
		if (timeshift != TSHIFT_MODE_OFF) {
		// p_movie_info may be invalidated by CRecordManager while we're still using it. Create and use a copy.
			movie_info = *p_movie_info;
			p_movie_info = &movie_info;
		}

		duration = p_movie_info->length * 60 * 1000;
		int percent = CZapit::getInstance()->GetPidVolume(p_movie_info->epgId, currentapid, currentac3 == 1);
		CZapit::getInstance()->SetVolumePercent(percent);
	}

	file_prozent = 0;
	pthread_t thrStartHint = 0;
	if (is_file_player) {
		showStartingHint = true;
		pthread_create(&thrStartHint, NULL, CMoviePlayerGui::ShowStartHint, this);
	}
	bool res = playback->Start((char *) file_name.c_str(), vpid, vtype, currentapid, currentac3, duration);

	if (thrStartHint) {
		showStartingHint = false;
		pthread_join(thrStartHint, NULL);
	}

	if (!res) {
		playback->Close();
		repeat_mode = REPEAT_OFF;
		return false;
	} else {
		repeat_mode = (repeat_mode_enum) g_settings.movieplayer_repeat_on;
		playstate = CMoviePlayerGui::PLAY;
		CVFD::getInstance()->ShowIcon(FP_ICON_PLAY, true);
		if(timeshift != TSHIFT_MODE_OFF) {
			startposition = -1;
			int i;
			int towait = (timeshift == TSHIFT_MODE_ON) ? TIMESHIFT_SECONDS+1 : TIMESHIFT_SECONDS;
			int cnt = 500;
			if (IS_WEBTV(movie_info.epgId)) {
				videoDecoder->setBlank(false);
				cnt = 200;
				towait = 20;
			}
			for(i = 0; i < cnt; i++) {
				playback->GetPosition(position, duration);
				startposition = (duration - position);

				//printf("CMoviePlayerGui::PlayFile: waiting for data, position %d duration %d (%d), start %d\n", position, duration, towait, startposition);
				if (startposition > towait*1000)
					break;

				usleep(20000);
			}
			printf("CMoviePlayerGui::PlayFile: waiting for data: i=%d position %d duration %d (%d), start %d\n", i, position, duration, towait, startposition);
			if (timeshift == TSHIFT_MODE_REWIND) {
				startposition = duration;
			} else {
				if (g_settings.timeshift_pause)
					playstate = CMoviePlayerGui::PAUSE;
				if (timeshift == TSHIFT_MODE_ON)
					startposition = 0;
				else
					startposition = std::max(0, duration - towait*1000);
			}
			printf("******************* Timeshift %d, position %d, seek to %d seconds\n", timeshift, position, startposition/1000);
		}
		if (/* !is_file_player && */ startposition >= 0)
			playback->SetPosition(startposition, true);


		/* playback->Start() starts paused */
		if (timeshift == TSHIFT_MODE_REWIND) {
			speed = -1;
			playback->SetSpeed(-1);
			playstate = CMoviePlayerGui::REW;
			if (!FileTime.IsVisible() && !time_forced) {
				FileTime.switchMode(position, duration);
				time_forced = true;
			}
		} else if (timeshift == TSHIFT_MODE_OFF || !g_settings.timeshift_pause) {
			playback->SetSpeed(1);
		}
	}
	getCurrentAudioName(is_file_player, currentaudioname);
	if (is_file_player)
		selectAutoLang();

	EnableClockAndMute(true);
	return res;
}

bool CMoviePlayerGui::SetPosition(int pos, bool absolute)
{
	clearSubtitle();
	bool res = playback->SetPosition(pos, absolute);
	return res;
}

void CMoviePlayerGui::quickZap(neutrino_msg_t msg)
{
	if ((msg == CRCInput::RC_right) || msg == (neutrino_msg_t) g_settings.key_quickzap_up)
	{
		//printf("CMoviePlayerGui::%s: CRCInput::RC_right or g_settings.key_quickzap_up\n", __func__);
		if (isLuaPlay)
		{
			playstate = CMoviePlayerGui::STOPPED;
			keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_NEXT;
			ClearQueue();
		}
		else if (!filelist.empty())
		{
			if (filelist_it < (filelist.end() - 1))
			{
				playstate = CMoviePlayerGui::STOPPED;
				++filelist_it;
			}
			else if (repeat_mode == REPEAT_ALL)
			{
				playstate = CMoviePlayerGui::STOPPED;
				++filelist_it;
				if (filelist_it == filelist.end())
				{
					filelist_it = filelist.begin();
				}
			}
		}
	}
	else if ((msg == CRCInput::RC_left) || msg == (neutrino_msg_t) g_settings.key_quickzap_down)
	{
		//printf("CMoviePlayerGui::%s: CRCInput::RC_left or g_settings.key_quickzap_down\n", __func__);
		if (isLuaPlay)
		{
			playstate = CMoviePlayerGui::STOPPED;
			keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_PREV;
			ClearQueue();
		}
		else if (filelist.size() > 1)
		{
			if (filelist_it != filelist.begin())
			{
				playstate = CMoviePlayerGui::STOPPED;
				--filelist_it;
			}
		}
	}
}

void CMoviePlayerGui::PlayFileLoop(void)
{
	bool first_start = true;
	bool update_lcd = true;
	int eof = 0;
	bool at_eof = !(playstate >= CMoviePlayerGui::PLAY);;
	keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_NORMAL;
	while (playstate >= CMoviePlayerGui::PLAY)
	{
		if (update_lcd) {
			update_lcd = false;
			updateLcd();
		}
		if (first_start) {
			callInfoViewer();
			first_start = false;
		}

		neutrino_msg_t msg;
		neutrino_msg_data_t data;
		g_RCInput->getMsg(&msg, &data, 10);	// 1 secs..

		if ((playstate >= CMoviePlayerGui::PLAY) && (timeshift != TSHIFT_MODE_OFF || (playstate != CMoviePlayerGui::PAUSE))) {
			if (playback->GetPosition(position, duration)) {
				FileTime.update(position, duration);
				if (duration > 100)
					file_prozent = (unsigned char) (position / (duration / 100));

				CVFD::getInstance()->showPercentOver(file_prozent);

				playback->GetSpeed(speed);
				/* at BOF lib set speed 1, check it */
				if ((playstate != CMoviePlayerGui::PLAY) && (speed == 1)) {
					playstate = CMoviePlayerGui::PLAY;
					update_lcd = true;
				}
#ifdef DEBUG
				printf("CMoviePlayerGui::%s: spd %d pos %d/%d (%d, %d%%)\n", __func__, speed, position, duration, duration-position, file_prozent);
#endif
				/* in case ffmpeg report incorrect values */
				int posdiff = duration - position;
				if ((posdiff >= 0) && (posdiff < 2000) && timeshift == TSHIFT_MODE_OFF)
				{
					int delay = (filelist_it != filelist.end() || repeat_mode != REPEAT_OFF) ? 5 : 10;
					if (++eof > delay) {
						at_eof = true;
						break;
					}
				}
				else
					eof = 0;
			}
			handleMovieBrowser(0, position);
			if (playstate == CMoviePlayerGui::STOPPED)
				at_eof = true;

			FileTime.update(position, duration);
		}
		showSubtitle(0);

		if (playstate == CMoviePlayerGui::PAUSE && (msg == CRCInput::RC_timeout || msg == NeutrinoMessages::EVT_TIMER))
		{
			int delay = time(NULL) - m_idletime;
			int screensaver_delay = g_settings.screensaver_delay;
			if (screensaver_delay != 0 && delay > screensaver_delay*60 && !m_screensaver) {
				videoDecoder->setBlank(true);
				screensaver(true);
			}
		}
		else
		{
			m_idletime = time(NULL);
			if (m_screensaver)
			{
				videoDecoder->setBlank(false);
				screensaver(false);
				//ignore first keypress stop - just quit the screensaver and call infoviewer
				if (msg == CRCInput::RC_stop) {
					g_RCInput->clearRCMsg();
					callInfoViewer();
					continue;
				}

			}
		}

		if (msg == (neutrino_msg_t) g_settings.mpkey_plugin) {
			g_PluginList->startPlugin_by_name(g_settings.movieplayer_plugin.c_str ());
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_stop) {
			playstate = CMoviePlayerGui::STOPPED;
			keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_STOP;
			ClearQueue();
		} else if (msg == CRCInput::RC_left || msg == CRCInput::RC_right) {
			bool reset_vzap_it = true;
			switch (g_settings.mode_left_right_key_tv)
			{
				case SNeutrinoSettings::INFOBAR:
					callInfoViewer();
					break;
				case SNeutrinoSettings::VZAP:
					if (fromInfoviewer)
					{
						set_vzap_it(msg == CRCInput::RC_right);
						reset_vzap_it = false;
						fromInfoviewer = false;
					}
					callInfoViewer(reset_vzap_it);
					break;
				case SNeutrinoSettings::VOLUME:
					g_volume->setVolume(msg);
					break;
				default: /* SNeutrinoSettings::ZAP */
					quickZap(msg);
					break;
			}
		} else if (msg == (neutrino_msg_t) g_settings.key_quickzap_up || msg == (neutrino_msg_t) g_settings.key_quickzap_down) {
			quickZap(msg);
		} else if (fromInfoviewer && msg == CRCInput::RC_ok && !filelist.empty()) {
			printf("CMoviePlayerGui::%s: start playlist movie #%d\n", __func__, vzap_it - filelist.begin());
			fromInfoviewer = false;
			playstate = CMoviePlayerGui::STOPPED;
			filelist_it = vzap_it;
		} else if (timeshift == TSHIFT_MODE_OFF && !isWebTV /* && !isYT */ && (msg == (neutrino_msg_t) g_settings.mpkey_next_repeat_mode)) {
			repeat_mode = (repeat_mode_enum)((int)repeat_mode + 1);
			if (repeat_mode > (int) REPEAT_ALL)
				repeat_mode = REPEAT_OFF;
			g_settings.movieplayer_repeat_on = repeat_mode;
			callInfoViewer();
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_play) {
			if (time_forced) {
				time_forced = false;
				FileTime.kill();
			}
			if (playstate > CMoviePlayerGui::PLAY) {
				playstate = CMoviePlayerGui::PLAY;
				speed = 1;
				playback->SetSpeed(speed);
				updateLcd();
				if (timeshift == TSHIFT_MODE_OFF)
					callInfoViewer();
			} else if (!filelist.empty()) {
				EnableClockAndMute(false);
				CFileBrowser *playlist = new CFileBrowser();
				CFile *pfile = NULL;
				pfile = &(*filelist_it);
				int selected = std::distance( filelist.begin(), filelist_it );
				filelist_it = filelist.end();
				if (playlist->playlist_manager(filelist, selected))
				{
					playstate = CMoviePlayerGui::STOPPED;
					CFile *sfile = NULL;
					for (filelist_it = filelist.begin(); filelist_it != filelist.end(); ++filelist_it)
					{
						pfile = &(*filelist_it);
						sfile = playlist->getSelectedFile();
						if ( (sfile->getFileName() == pfile->getFileName()) && (sfile->getPath() == pfile->getPath()))
							break;
					}
				}
				else {
					if (!filelist.empty())
						filelist_it = filelist.begin() + selected;
				}
				delete playlist;
				EnableClockAndMute(true);
			}
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_pause) {
			if (playstate == CMoviePlayerGui::PAUSE) {
				playstate = CMoviePlayerGui::PLAY;
				//CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);
				speed = 1;
				playback->SetSpeed(speed);
			} else {
				playstate = CMoviePlayerGui::PAUSE;
				//CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, true);
				speed = 0;
				playback->SetSpeed(speed);
			}
			updateLcd();
			if (timeshift == TSHIFT_MODE_OFF)
				callInfoViewer();
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_bookmark) {
			if (is_file_player)
				selectChapter();
			else
				handleMovieBrowser((neutrino_msg_t) g_settings.mpkey_bookmark, position);
			update_lcd = true;
			clearSubtitle();
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_audio) {
			selectAudioPid();
			update_lcd = true;
			clearSubtitle();
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_subtitle) {
			selectSubtitle();
			clearSubtitle();
			update_lcd = true;
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_time) {
			FileTime.switchMode(position, duration);
		} else if (msg == (neutrino_msg_t) g_settings.mbkey_cover) {
			makeScreenShot(false, true);
		} else if (msg == (neutrino_msg_t) g_settings.key_screenshot) {
			makeScreenShot();
		} else if ((msg == (neutrino_msg_t) g_settings.mpkey_rewind) ||
				(msg == (neutrino_msg_t) g_settings.mpkey_forward)) {
			int newspeed;
			if (msg == (neutrino_msg_t) g_settings.mpkey_rewind) {
				newspeed = (speed >= 0) ? -1 : speed - 1;
			} else {
				newspeed = (speed <= 0) ? 2 : speed + 1;
			}
			/* if paused, playback->SetSpeed() start slow motion */
			if (playback->SetSpeed(newspeed)) {
				printf("SetSpeed: update speed\n");
				speed = newspeed;
				if (playstate != CMoviePlayerGui::PAUSE)
					playstate = msg == (neutrino_msg_t) g_settings.mpkey_rewind ? CMoviePlayerGui::REW : CMoviePlayerGui::FF;
				updateLcd();
			}

			if (!FileTime.IsVisible() && !time_forced) {
				FileTime.switchMode(position, duration);
				time_forced = true;
			}
			if (timeshift == TSHIFT_MODE_OFF)
				callInfoViewer();
		} else if (msg == CRCInput::RC_1) {	// Jump Backwards 1 minute
			SetPosition(-60 * 1000);
		} else if (msg == CRCInput::RC_3) {	// Jump Forward 1 minute
			SetPosition(60 * 1000);
		} else if (msg == CRCInput::RC_4) {	// Jump Backwards 5 minutes
			SetPosition(-5 * 60 * 1000);
		} else if (msg == CRCInput::RC_6) {	// Jump Forward 5 minutes
			SetPosition(5 * 60 * 1000);
		} else if (msg == CRCInput::RC_7) {	// Jump Backwards 10 minutes
			SetPosition(-10 * 60 * 1000);
		} else if (msg == CRCInput::RC_9) {	// Jump Forward 10 minutes
			SetPosition(10 * 60 * 1000);
		} else if (msg == CRCInput::RC_2) {	// goto start
			SetPosition(0, true);
		} else if (msg == CRCInput::RC_5) {	// goto middle
			SetPosition(duration/2, true);
		} else if (msg == CRCInput::RC_8) {	// goto end
			SetPosition(duration - 60 * 1000, true);
		} else if (msg == CRCInput::RC_page_up) {
			SetPosition(10 * 1000);
		} else if (msg == CRCInput::RC_page_down) {
			SetPosition(-10 * 1000);
		} else if (msg == CRCInput::RC_0) {	// cancel bookmark jump
			handleMovieBrowser(CRCInput::RC_0, position);
		} else if (msg == (neutrino_msg_t) g_settings.mpkey_goto) {
			bool cancel = true;
			playback->GetPosition(position, duration);
			int ss = position/1000;
			int hh = ss/3600;
			ss -= hh * 3600;
			int mm = ss/60;
			ss -= mm * 60;
			std::string Value = to_string(hh/10) + to_string(hh%10) + ":" + to_string(mm/10) + to_string(mm%10) + ":" + to_string(ss/10) + to_string(ss%10);
			CTimeInput jumpTime (LOCALE_MPKEY_GOTO, &Value, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, NULL, &cancel);
			jumpTime.exec(NULL, "");
			jumpTime.hide();
			if (!cancel && (3 == sscanf(Value.c_str(), "%d:%d:%d", &hh, &mm, &ss)))
				SetPosition(1000 * (hh * 3600 + mm * 60 + ss), true);

		} else if (msg == CRCInput::RC_help || msg == CRCInput::RC_info) {
			if (fromInfoviewer)
			{
				g_EpgData->show_mp(p_movie_info,GetPosition(),GetDuration());
				fromInfoviewer = false;
			}
			else
			callInfoViewer();
			update_lcd = true;
			clearSubtitle();
		} else if (timeshift != TSHIFT_MODE_OFF && (msg == CRCInput::RC_text || msg == CRCInput::RC_epg || msg == NeutrinoMessages::SHOW_EPG)) {
			bool restore = FileTime.IsVisible();
			FileTime.kill();

			if (msg == CRCInput::RC_epg )
				g_EventList->exec(CNeutrinoApp::getInstance()->channelList->getActiveChannel_ChannelID(), CNeutrinoApp::getInstance()->channelList->getActiveChannelName());
			else if (msg == NeutrinoMessages::SHOW_EPG)
				g_EpgData->show(CNeutrinoApp::getInstance()->channelList->getActiveChannel_ChannelID());
			else {
				if (g_settings.cacheTXT)
					tuxtxt_stop();
				tuxtx_main(g_RCInput->getFileHandle(), g_RemoteControl->current_PIDs.PIDs.vtxtpid, 0, 2);
				frameBuffer->paintBackground();
			}
			if (restore)
				FileTime.show(position);
#if 0
		} else if (msg == CRCInput::RC_red) {
			bool restore = FileTime.IsVisible();
			FileTime.kill();
			CStreamInfo2 streaminfo;
			streaminfo.exec(NULL, "");
			if (restore)
				FileTime.show(position);
			update_lcd = true;
#endif
		} else if (msg == NeutrinoMessages::SHOW_EPG) {
			handleMovieBrowser(NeutrinoMessages::SHOW_EPG, position);
		} else if (msg == NeutrinoMessages::EVT_SUBT_MESSAGE) {
			showSubtitle(data);
		} else if (msg == NeutrinoMessages::ANNOUNCE_RECORD ||
				msg == NeutrinoMessages::RECORD_START) {
			CNeutrinoApp::getInstance()->handleMsg(msg, data);
		} else if (msg == NeutrinoMessages::ZAPTO ||
				msg == NeutrinoMessages::STANDBY_ON ||
				msg == NeutrinoMessages::LEAVE_ALL ||
				msg == NeutrinoMessages::SHUTDOWN ||
				((msg == NeutrinoMessages::SLEEPTIMER) && !data) ) {	// Exit for Record/Zapto Timers
			printf("CMoviePlayerGui::PlayFile: ZAPTO etc..\n");
			if (msg != NeutrinoMessages::ZAPTO)
				menu_ret = menu_return::RETURN_EXIT_ALL;

			playstate = CMoviePlayerGui::STOPPED;
			keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_LEAVE_ALL;
			ClearQueue();
			g_RCInput->postMsg(msg, data);
		} else if (msg == CRCInput::RC_timeout || msg == NeutrinoMessages::EVT_TIMER) {
			if (playstate == CMoviePlayerGui::PLAY && (position >= 300000 || (duration < 300000 && (position > (duration /2)))))
				makeScreenShot(true);
		} else if (msg == CRCInput::RC_sat || msg == CRCInput::RC_favorites || msg == CRCInput::RC_www) {
			//FIXME do nothing ?
		} else if (msg == (neutrino_msg_t) CRCInput::RC_setup) {
			CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::SHOW_MAINMENU, 0);
		} else if (msg == CRCInput::RC_red || msg == CRCInput::RC_green || msg == CRCInput::RC_yellow || msg == CRCInput::RC_blue ) {
			//maybe move FileTime.kill to Usermenu to simplify this call
			bool restore = FileTime.IsVisible();
			FileTime.kill();
			CNeutrinoApp::getInstance()->usermenu.showUserMenu(msg);
			if (restore)
				FileTime.show(position);
			update_lcd = true;
		} else {
			if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all) {
				printf("CMoviePlayerGui::PlayFile: neutrino handleMsg messages_return::cancel_all\n");
				menu_ret = menu_return::RETURN_EXIT_ALL;
				playstate = CMoviePlayerGui::STOPPED;
				keyPressed = CMoviePlayerGui::PLUGIN_PLAYSTATE_LEAVE_ALL;
				ClearQueue();
			}
			else if (msg <= CRCInput::RC_MaxRC ) {
				update_lcd = true;
				clearSubtitle();
			}
		}
	}
	printf("CMoviePlayerGui::PlayFile: exit, isMovieBrowser %d p_movie_info %p\n", isMovieBrowser, p_movie_info);
	playstate = CMoviePlayerGui::STOPPED;
	handleMovieBrowser((neutrino_msg_t) g_settings.mpkey_stop, position);
	if (position >= 300000 || (duration < 300000 && (position > (duration /2))))
		makeScreenShot(true);

	if (at_eof && !filelist.empty()) {
		if (filelist_it != filelist.end() && repeat_mode != REPEAT_TRACK)
			++filelist_it;

		if (filelist_it == filelist.end() && repeat_mode == REPEAT_ALL)
			filelist_it = filelist.begin();
	}
}

void CMoviePlayerGui::PlayFileEnd(bool restore)
{
	printf("%s: stopping, this %p thread %p\n", __func__, this, CMoviePlayerGui::bgPlayThread);fflush(stdout);
	if (filelist_it == filelist.end())
		FileTime.kill();
	clearSubtitle();

	playback->SetSpeed(1);
	playback->Close();
	if (iso_file) {
		iso_file = false;
		if (umount2(ISO_MOUNT_POINT ,MNT_FORCE))
			perror(ISO_MOUNT_POINT);
	}

	CVFD::getInstance()->ShowIcon(FP_ICON_PLAY, false);
	CVFD::getInstance()->ShowIcon(FP_ICON_PAUSE, false);

	if (restore)
		restoreNeutrino();

	stopped = true;
	printf("%s: stopped\n", __func__);
	if (!filelist.empty() && filelist_it != filelist.end()) {
		pretty_name.clear();
		prepareFile(&(*filelist_it));
	}
}

void CMoviePlayerGui::set_vzap_it(bool up)
{
	//printf("CMoviePlayerGui::%s: vzap_it: %d count %s\n", __func__, vzap_it - filelist.begin(), up ? "up" : "down");
	if (up)
	{
		if (vzap_it < (filelist.end() - 1))
			++vzap_it;
	}
	else
	{
		if (vzap_it > filelist.begin())
			--vzap_it;
	}
	//printf("CMoviePlayerGui::%s: vzap_it: %d\n", __func__, vzap_it - filelist.begin());
}

void CMoviePlayerGui::callInfoViewer(bool init_vzap_it)
{
	if (init_vzap_it)
	{
		//printf("CMoviePlayerGui::%s: init_vzap_it\n", __func__);
		vzap_it = filelist_it;
	}

	if (timeshift != TSHIFT_MODE_OFF) {
		g_InfoViewer->showTitle(CNeutrinoApp::getInstance()->channelList->getActiveChannel());
		return;
	}

	if(duration == 0)
		UpdatePosition();

	if (isMovieBrowser && p_movie_info)
	{
		MI_MOVIE_INFO *mi;
		mi = p_movie_info;
		if (!filelist.empty() && g_settings.mode_left_right_key_tv == SNeutrinoSettings::VZAP)
		{
			if (vzap_it <= filelist.end()) {
				unsigned idx = vzap_it - filelist.begin();
				//printf("CMoviePlayerGui::%s: idx: %d\n", __func__, idx);
				if(milist.size() > idx)
					mi = milist[idx];
			}
		}
		g_InfoViewer->showMovieTitle(playstate, mi->epgEpgId >>16, mi->epgChannel, mi->epgTitle, mi->epgInfo1,
			duration, position, repeat_mode, init_vzap_it ? 0 /*IV_MODE_DEFAULT*/ : 1 /*IV_MODE_VIRTUAL_ZAP*/);
		return;
	}

	/* not moviebrowser => use the filename as title */
	g_InfoViewer->showMovieTitle(playstate, 0, pretty_name, info_1, info_2, duration, position, repeat_mode);
}

bool CMoviePlayerGui::getAudioName(int apid, std::string &apidtitle)
{
	if (p_movie_info == NULL)
		return false;

	for (int i = 0; i < (int)p_movie_info->audioPids.size(); i++) {
		if (p_movie_info->audioPids[i].epgAudioPid == apid && !p_movie_info->audioPids[i].epgAudioPidName.empty()) {
			apidtitle = p_movie_info->audioPids[i].epgAudioPidName;
			return true;
		}
	}
	return false;
}

void CMoviePlayerGui::addAudioFormat(int count, std::string &apidtitle, bool& enabled)
{
	enabled = true;
	switch(ac3flags[count])
	{
		case 1: /*AC3,EAC3*/
			if (apidtitle.find("AC3") == std::string::npos)
				apidtitle.append(" (AC3)");
			break;
		case 2: /*teletext*/
			apidtitle.append(" (Teletext)");
			enabled = false;
			break;
		case 3: /*MP2*/
			apidtitle.append("( MP2)");
			break;
		case 4: /*MP3*/
			apidtitle.append(" (MP3)");
			break;
		case 5: /*AAC*/
			apidtitle.append(" (AAC)");
			break;
		case 6: /*DTS*/
			if (apidtitle.find("DTS") == std::string::npos)
				apidtitle.append(" (DTS)");
#ifndef BOXMODEL_APOLLO
			enabled = false;
#endif
			break;
		case 7: /*MLP*/
			apidtitle.append(" (MLP)");
			break;
		default:
			break;
	}
}

void CMoviePlayerGui::getCurrentAudioName(bool file_player, std::string &audioname)
{
	if (file_player && !numpida) {
		playback->FindAllPids(apids, ac3flags, &numpida, language);
		if (numpida)
			currentapid = apids[0];
	}
	bool dumm = true;
	for (unsigned int count = 0; count < numpida; count++) {
		if (currentapid == apids[count]) {
			if (!file_player) {
				getAudioName(apids[count], audioname);
				return ;
			} else if (!language[count].empty()) {
				audioname = language[count];
				addAudioFormat(count, audioname, dumm);
				if (!dumm && (count < numpida)) {
					currentapid = apids[count+1];
					continue;
				}
				return ;
			}
			char apidnumber[20];
			sprintf(apidnumber, "Stream %d %X", count + 1, apids[count]);
			audioname = apidnumber;
			addAudioFormat(count, audioname, dumm);
			if (!dumm && (count < numpida)) {
				currentapid = apids[count+1];
				continue;
			}
			return ;
		}
	}
}

void CMoviePlayerGui::selectAudioPid()
{
	CMenuWidget APIDSelector(LOCALE_APIDSELECTOR_HEAD, NEUTRINO_ICON_AUDIO);
	APIDSelector.addIntroItems();

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	if (is_file_player && !numpida) {
		playback->FindAllPids(apids, ac3flags, &numpida, language);
		if (numpida)
			currentapid = apids[0];
	}
	for (unsigned int count = 0; count < numpida; count++) {
		bool name_ok = false;
		bool enabled = true;
		bool defpid = currentapid ? (currentapid == apids[count]) : (count == 0);
		std::string apidtitle;

		if (!is_file_player) {
			name_ok = getAudioName(apids[count], apidtitle);
		}
		else if (!language[count].empty()) {
			apidtitle = language[count];
			name_ok = true;
		}
		if (!name_ok) {
			char apidnumber[20];
			sprintf(apidnumber, "Stream %d %X", count + 1, apids[count]);
			apidtitle = apidnumber;
		}
		addAudioFormat(count, apidtitle, enabled);
		if (defpid && !enabled && (count < numpida)) {
			currentapid = apids[count+1];
			defpid = false;
		}

		char cnt[5];
		sprintf(cnt, "%d", count);
		CMenuForwarder * item = new CMenuForwarder(apidtitle.c_str(), enabled, NULL, selector, cnt, CRCInput::convertDigitToKey(count + 1));
		APIDSelector.addItem(item, defpid);
	}

	int percent[numpida+1];
	if (p_movie_info && numpida <= p_movie_info->audioPids.size()) {
		APIDSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_VOLUME_ADJUST));

		CVolume::getInstance()->SetCurrentChannel(p_movie_info->epgId);
		CVolume::getInstance()->SetCurrentPid(currentapid);
		for (uint i=0; i < numpida; i++) {
			percent[i] = CZapit::getInstance()->GetPidVolume(p_movie_info->epgId, apids[i], ac3flags[i]);
			APIDSelector.addItem(new CMenuOptionNumberChooser(p_movie_info->audioPids[i].epgAudioPidName,
						&percent[i], currentapid == apids[i],
						0, 999, CVolume::getInstance()));
		}
	}
	if (!g_settings.easymenu) {
		APIDSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE));
		extern CAudioSetupNotifier     * audioSetupNotifier;
		APIDSelector.addItem( new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOG_OUT, &g_settings.analog_out,
					OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT,
					true, audioSetupNotifier, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN) );
	}
	APIDSelector.exec(NULL, "");
	delete selector;
	printf("CMoviePlayerGui::selectAudioPid: selected %d (%x) current %x\n", select, (select >= 0) ? apids[select] : -1, currentapid);
	if ((select >= 0) && (currentapid != apids[select])) {
		currentapid = apids[select];
		currentac3 = ac3flags[select];
		playback->SetAPid(currentapid, currentac3);
		getCurrentAudioName(is_file_player, currentaudioname);
		printf("[movieplayer] apid changed to %d type %d\n", currentapid, currentac3);
	}
}

void CMoviePlayerGui::handleMovieBrowser(neutrino_msg_t msg, int /*position*/)
{
	CMovieInfo cMovieInfo;	// funktions to save and load movie info

	static int jump_not_until = 0;	// any jump shall be avoided until this time (in seconds from moviestart)
	static MI_BOOKMARK new_bookmark;	// used for new movie info bookmarks created from the movieplayer

	std::string key_bookmark = CRCInput::getKeyName((neutrino_msg_t) g_settings.mpkey_bookmark);
	char txt[1024];

	//TODO: width and height could be more flexible
	static int width = frameBuffer->getScreenWidth() / 2;
	static int height = 65;

	static int x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
	static int y = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - height - 20;

	static CBox boxposition(x, y, width, height);	// window position for the hint boxes

	static CTextBox endHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_MOVIEEND), NULL, CTextBox::CENTER /*CTextBox::AUTO_WIDTH | CTextBox::AUTO_HIGH */ , &boxposition);
	static CTextBox comHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_JUMPFORWARD), NULL, CTextBox::CENTER /*CTextBox::AUTO_WIDTH | CTextBox::AUTO_HIGH */ , &boxposition);
	static CTextBox loopHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_JUMPBACKWARD), NULL, CTextBox::CENTER /*CTextBox::AUTO_WIDTH | CTextBox::AUTO_HIGH */ , &boxposition);

	snprintf(txt, sizeof(txt)-1, g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_NEWBOOK_BACKWARD), key_bookmark.c_str());
	static CTextBox newLoopHintBox(txt, NULL, CTextBox::CENTER /*CTextBox::AUTO_WIDTH | CTextBox::AUTO_HIGH */ , &boxposition);

	snprintf(txt, sizeof(txt)-1, g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_NEWBOOK_FORWARD), key_bookmark.c_str());
	static CTextBox newComHintBox(txt, NULL, CTextBox::CENTER /*CTextBox::AUTO_WIDTH | CTextBox::AUTO_HIGH */ , &boxposition);

	static bool showEndHintBox = false;	// flag to check whether the box shall be painted
	static bool showComHintBox = false;	// flag to check whether the box shall be painted
	static bool showLoopHintBox = false;	// flag to check whether the box shall be painted

	int play_sec = position / 1000;	// get current seconds from moviestart
	if (msg == CRCInput::RC_nokey) {
		printf("CMoviePlayerGui::handleMovieBrowser: reset vars\n");
		// reset statics
		jump_not_until = 0;
		showEndHintBox = showComHintBox = showLoopHintBox = false;
		new_bookmark.pos = 0;
		// move in case osd position changed
		int newx = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
		int newy = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - height - 20;
		endHintBox.movePosition(newx, newy);
		comHintBox.movePosition(newx, newy);
		loopHintBox.movePosition(newx, newy);
		newLoopHintBox.movePosition(newx, newy);
		newComHintBox.movePosition(newx, newy);
		return;
	}
	else if (msg == (neutrino_msg_t) g_settings.mpkey_stop) {
		// if we have a movie information, try to save the stop position
		printf("CMoviePlayerGui::handleMovieBrowser: stop, isMovieBrowser %d p_movie_info %x\n", isMovieBrowser, (int) p_movie_info);
		if (isMovieBrowser && p_movie_info) {
			timeb current_time;
			ftime(&current_time);
			p_movie_info->dateOfLastPlay = current_time.time;
			current_time.time = time(NULL);
			p_movie_info->bookmarks.lastPlayStop = position / 1000;
			cMovieInfo.saveMovieInfo(*p_movie_info);
			//p_movie_info->fileInfoStale(); //TODO: we might to tell the Moviebrowser that the movie info has changed, but this could cause long reload times  when reentering the Moviebrowser
		}
	}
	else if ((msg == 0) && isMovieBrowser && (playstate == CMoviePlayerGui::PLAY) && p_movie_info) {
		if (play_sec + 10 < jump_not_until || play_sec > jump_not_until + 10)
			jump_not_until = 0;	// check if !jump is stale (e.g. if user jumped forward or backward)

		// do bookmark activities only, if there is no new bookmark started
		if (new_bookmark.pos != 0)
			return;
#ifdef DEBUG
		//printf("CMoviePlayerGui::handleMovieBrowser: process bookmarks\n");
#endif
		if (p_movie_info->bookmarks.end != 0) {
			// *********** Check for stop position *******************************
			if (play_sec >= p_movie_info->bookmarks.end - MOVIE_HINT_BOX_TIMER && play_sec < p_movie_info->bookmarks.end && play_sec > jump_not_until) {
				if (showEndHintBox == false) {
					endHintBox.paint();	// we are 5 sec before the end postition, show warning
					showEndHintBox = true;
					TRACE("[mp]  user stop in 5 sec...\r\n");
				}
			} else {
				if (showEndHintBox == true) {
					endHintBox.hide();	// if we showed the warning before, hide the box again
					showEndHintBox = false;
				}
			}

			if (play_sec >= p_movie_info->bookmarks.end && play_sec <= p_movie_info->bookmarks.end + 2 && play_sec > jump_not_until)	// stop playing
			{
				// *********** we ARE close behind the stop position, stop playing *******************************
				TRACE("[mp]  user stop: play_sec %d bookmarks.end %d jump_not_until %d\n", play_sec, p_movie_info->bookmarks.end, jump_not_until);
				playstate = CMoviePlayerGui::STOPPED;
				return;
			}
		}
		// *************  Check for bookmark jumps *******************************
		showLoopHintBox = false;
		showComHintBox = false;
		for (int book_nr = 0; book_nr < MI_MOVIE_BOOK_USER_MAX; book_nr++) {
			if (p_movie_info->bookmarks.user[book_nr].pos != 0 && p_movie_info->bookmarks.user[book_nr].length != 0) {
				// valid bookmark found, now check if we are close before or after it
				if (play_sec >= p_movie_info->bookmarks.user[book_nr].pos - MOVIE_HINT_BOX_TIMER && play_sec < p_movie_info->bookmarks.user[book_nr].pos && play_sec > jump_not_until) {
					if (p_movie_info->bookmarks.user[book_nr].length < 0)
						showLoopHintBox = true;	// we are 5 sec before , show warning
					else if (p_movie_info->bookmarks.user[book_nr].length > 0)
						showComHintBox = true;	// we are 5 sec before, show warning
					//else  // TODO should we show a plain bookmark infomation as well?
				}

				if (play_sec >= p_movie_info->bookmarks.user[book_nr].pos && play_sec <= p_movie_info->bookmarks.user[book_nr].pos + 2 && play_sec > jump_not_until)	//
				{
					//for plain bookmark, the following calc shall result in 0 (no jump)
					int jumpseconds = p_movie_info->bookmarks.user[book_nr].length;

					// we are close behind the bookmark, do bookmark activity (if any)
					if (p_movie_info->bookmarks.user[book_nr].length < 0) {
						// if the jump back time is to less, it does sometimes cause problems (it does probably jump only 5 sec which will cause the next jump, and so on)
						if (jumpseconds > -15)
							jumpseconds = -15;

						playback->SetPosition(jumpseconds * 1000);
					} else if (p_movie_info->bookmarks.user[book_nr].length > 0) {
						// jump at least 15 seconds
						if (jumpseconds < 15)
							jumpseconds = 15;

						playback->SetPosition(jumpseconds * 1000);
					}
					TRACE("[mp]  do jump %d sec\r\n", jumpseconds);
					break;	// do no further bookmark checks
				}
			}
		}
		// check if we shall show the commercial warning
		if (showComHintBox == true) {
			comHintBox.paint();
			TRACE("[mp]  com jump in 5 sec...\r\n");
		} else
			comHintBox.hide();

		// check if we shall show the loop warning
		if (showLoopHintBox == true) {
			loopHintBox.paint();
			TRACE("[mp]  loop jump in 5 sec...\r\n");
		} else
			loopHintBox.hide();

		return;
	} else if (msg == CRCInput::RC_0) {	// cancel bookmark jump
		printf("CMoviePlayerGui::handleMovieBrowser: CRCInput::RC_0\n");
		if (isMovieBrowser == true) {
			if (new_bookmark.pos != 0) {
				new_bookmark.pos = 0;	// stop current bookmark activity, TODO:  might bemoved to another key
				newLoopHintBox.hide();	// hide hint box if any
				newComHintBox.hide();
			}
			comHintBox.hide();
			loopHintBox.hide();
			jump_not_until = (position / 1000) + 10; // avoid bookmark jumping for the next 10 seconds, , TODO:  might be moved to another key
		}
		return;
	}
	else if (msg == (neutrino_msg_t) g_settings.mpkey_bookmark) {
		if (newComHintBox.isPainted() == true) {
			// yes, let's get the end pos of the jump forward
			new_bookmark.length = play_sec - new_bookmark.pos;
			TRACE("[mp] commercial length: %d\r\n", new_bookmark.length);
			if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true) {
				cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
			}
			new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
			newComHintBox.hide();
		} else if (newLoopHintBox.isPainted() == true) {
			// yes, let's get the end pos of the jump backward
			new_bookmark.length = new_bookmark.pos - play_sec;
			new_bookmark.pos = play_sec;
			TRACE("[mp] loop length: %d\r\n", new_bookmark.length);
			if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true) {
				cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
				jump_not_until = play_sec + 5;	// avoid jumping for this time
			}
			new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
			newLoopHintBox.hide();
		} else {
			// very dirty usage of the menue, but it works and I already spent to much time with it, feel free to make it better ;-)
#define BOOKMARK_START_MENU_MAX_ITEMS 7
			CSelectedMenu cSelectedMenuBookStart[BOOKMARK_START_MENU_MAX_ITEMS];

			CMenuWidget bookStartMenu(LOCALE_MOVIEBROWSER_MENU_MAIN_BOOKMARKS, NEUTRINO_ICON_STREAMING);
			bookStartMenu.addIntroItems();
#if 0 // not supported, TODO
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_HEAD, !isMovieBrowser, NULL, &cSelectedMenuBookStart[0]));
			bookStartMenu.addItem(GenericMenuSeparatorLine);
#endif
			const char *unit_short_minute = g_Locale->getText(LOCALE_UNIT_SHORT_MINUTE);
			char play_pos[32];
			int lastplaystop = p_movie_info ? p_movie_info->bookmarks.lastPlayStop/60:0;
			snprintf(play_pos, sizeof(play_pos), "%3d %s", lastplaystop, unit_short_minute);
			char start_pos[32] = {0};
			if (p_movie_info && p_movie_info->bookmarks.start != 0)
				snprintf(start_pos, sizeof(start_pos), "%3d %s", p_movie_info->bookmarks.start/60, unit_short_minute);
			char end_pos[32] = {0};
			if (p_movie_info && p_movie_info->bookmarks.end != 0)
				snprintf(end_pos, sizeof(end_pos), "%3d %s", p_movie_info->bookmarks.end/60, unit_short_minute);

			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_LASTMOVIESTOP, isMovieBrowser, play_pos, &cSelectedMenuBookStart[1]));
			bookStartMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEBROWSER_BOOK_ADD));
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_NEW, isMovieBrowser, NULL, &cSelectedMenuBookStart[2]));
			bookStartMenu.addItem(new CMenuSeparator());
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_TYPE_FORWARD, isMovieBrowser, NULL, &cSelectedMenuBookStart[3], NULL, CRCInput::RC_red));
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_TYPE_BACKWARD, isMovieBrowser, NULL, &cSelectedMenuBookStart[4], NULL, CRCInput::RC_green));
			bookStartMenu.addItem(new CMenuSeparator());
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIESTART, isMovieBrowser, start_pos, &cSelectedMenuBookStart[5], NULL, CRCInput::RC_yellow));
			bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIEEND, isMovieBrowser, end_pos, &cSelectedMenuBookStart[6], NULL, CRCInput::RC_blue));

			// no, nothing else to do, we open a new bookmark menu
			new_bookmark.name = "";	// use default name
			new_bookmark.pos = 0;
			new_bookmark.length = 0;

			// next seems return menu_return::RETURN_EXIT, if something selected
			bookStartMenu.exec(NULL, "none");
#if 0 // not supported, TODO
			if (cSelectedMenuBookStart[0].selected == true) {
				/* Movieplayer bookmark */
				if (bookmarkmanager->getBookmarkCount() < bookmarkmanager->getMaxBookmarkCount()) {
					char timerstring[200];
					sprintf(timerstring, "%lld", play_sec);
					std::string bookmarktime = timerstring;
					fprintf(stderr, "fileposition: %lld timerstring: %s bookmarktime: %s\n", play_sec, timerstring, bookmarktime.c_str());
					bookmarkmanager->createBookmark(file_name, bookmarktime);
				} else {
					fprintf(stderr, "too many bookmarks\n");
					DisplayErrorMessage(g_Locale->getText(LOCALE_MOVIEPLAYER_TOOMANYBOOKMARKS));	// UTF-8
				}
				cSelectedMenuBookStart[0].selected = false;	// clear for next bookmark menu
			} else
#endif
			if (cSelectedMenuBookStart[1].selected == true) {
				int pos = p_movie_info->bookmarks.lastPlayStop;
				printf("[mb] last play stop: %d\n", pos);
				SetPosition(pos*1000, true);
			} else if (cSelectedMenuBookStart[2].selected == true) {
				/* Moviebrowser plain bookmark */
				new_bookmark.pos = play_sec;
				new_bookmark.length = 0;
				if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true)
					cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
				new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
			} else if (cSelectedMenuBookStart[3].selected == true) {
				/* Moviebrowser jump forward bookmark */
				new_bookmark.pos = play_sec;
				TRACE("[mp] new bookmark 1. pos: %d\r\n", new_bookmark.pos);
				newComHintBox.paint();
			} else if (cSelectedMenuBookStart[4].selected == true) {
				/* Moviebrowser jump backward bookmark */
				new_bookmark.pos = play_sec;
				TRACE("[mp] new bookmark 1. pos: %d\r\n", new_bookmark.pos);
				newLoopHintBox.paint();
			} else if (cSelectedMenuBookStart[5].selected == true) {
				/* Moviebrowser movie start bookmark */
				p_movie_info->bookmarks.start = play_sec;
				TRACE("[mp] New movie start pos: %d\r\n", p_movie_info->bookmarks.start);
				cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
			} else if (cSelectedMenuBookStart[6].selected == true) {
				/* Moviebrowser movie end bookmark */
				p_movie_info->bookmarks.end = play_sec;
				TRACE("[mp]  New movie end pos: %d\r\n", p_movie_info->bookmarks.end);
				cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
			}
		}
	} else if (msg == NeutrinoMessages::SHOW_EPG && (p_movie_info || (isLuaPlay && haveLuaInfoFunc))) {
		CTimeOSD::mode m_mode = FileTime.getMode();
		bool restore = FileTime.IsVisible();
		if (restore)
			FileTime.kill();
		CInfoClock::getInstance()->enableInfoClock(false);

		if (isLuaPlay && haveLuaInfoFunc) {
			int xres = 0, yres = 0, aspectRatio = 0, framerate = -1;
			if (!videoDecoder->getBlank()) {
				videoDecoder->getPictureInfo(xres, yres, framerate);
				if (yres == 1088)
					yres = 1080;
				aspectRatio = videoDecoder->getAspectRatio();
			}
			CLuaInstVideo::getInstance()->execLuaInfoFunc(luaState, xres, yres, aspectRatio, framerate);
		}
		else if (p_movie_info)
			cMovieInfo.showMovieInfo(*p_movie_info);

		CInfoClock::getInstance()->enableInfoClock(true);
		if (restore) {
			FileTime.setMode(m_mode);
			FileTime.update(position, duration);
		}
	}
	return;
}

void CMoviePlayerGui::UpdatePosition()
{
	if (playback->GetPosition(position, duration)) {
		if (duration > 100)
			file_prozent = (unsigned char) (position / (duration / 100));
		FileTime.update(position, duration);
#ifdef DEBUG
		printf("CMoviePlayerGui::%s: spd %d pos %d/%d (%d, %d%%)\n", __func__, speed, position, duration, duration-position, file_prozent);
#endif
	}
}

void CMoviePlayerGui::showHelpTS()
{
	Helpbox helpbox;
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP1));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP2));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_YELLOW, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP3));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP4));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_MENU, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP5));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_1, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP6));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_3, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP7));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_4, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP8));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_6, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP9));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_7, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP10));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_9, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP11));
	helpbox.addLine(g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP12));
	helpbox.show(LOCALE_MESSAGEBOX_INFO);
}

void CMoviePlayerGui::selectChapter()
{
	if (!is_file_player)
		return;

	std::vector<int> positions; std::vector<std::string> titles;
	playback->GetChapters(positions, titles);

	std::vector<int> playlists; std::vector<std::string> ptitles;
	int current;
	playback->GetTitles(playlists, ptitles, current);

	if (positions.empty() && playlists.empty())
		return;

	CMenuWidget ChSelector(LOCALE_MOVIEBROWSER_MENU_MAIN_BOOKMARKS, NEUTRINO_ICON_AUDIO);
	//ChSelector.addIntroItems();
	ChSelector.addItem(GenericMenuCancel);

	int pselect = -1;
	CMenuSelectorTarget * pselector = new CMenuSelectorTarget(&pselect);

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	char cnt[5];
	if (!playlists.empty()) {
		ChSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEPLAYER_TITLES));
		for (unsigned i = 0; i < playlists.size(); i++) {
			sprintf(cnt, "%d", i);
			CMenuForwarder * item = new CMenuForwarder(ptitles[i].c_str(), current != playlists[i], NULL, pselector, cnt);
			ChSelector.addItem(item);
		}
	}

	if (!positions.empty()) {
		ChSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEPLAYER_CHAPTERS));
		for (unsigned i = 0; i < positions.size(); i++) {
			sprintf(cnt, "%d", i);
			CMenuForwarder * item = new CMenuForwarder(titles[i].c_str(), true, NULL, selector, cnt, CRCInput::convertDigitToKey(i + 1));
			ChSelector.addItem(item, position > positions[i]);
		}
	}
	ChSelector.exec(NULL, "");
	delete selector;
	delete pselector;
	printf("CMoviePlayerGui::selectChapter: selected %d (%d)\n", select, (select >= 0) ? positions[select] : -1);
	printf("CMoviePlayerGui::selectChapter: pselected %d (%d)\n", pselect, (pselect >= 0) ? playlists[pselect] : -1);
	if (select >= 0) {
		playback->SetPosition(positions[select], true);
	} else if (pselect >= 0) {
		numsubs = numpida = 0;
		currentspid = -1;
		currentapid = 0;
		playback->SetTitle(playlists[pselect]);
	}
}

void CMoviePlayerGui::selectSubtitle()
{
	if (!is_file_player)
		return;

	CMenuWidget APIDSelector(LOCALE_SUBTITLES_HEAD, NEUTRINO_ICON_AUDIO);
	APIDSelector.addIntroItems();

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);
	if (!numsubs)
		playback->FindAllSubs(spids, sub_supported, &numsubs, slanguage);

	CMenuOptionStringChooser * sc = new CMenuOptionStringChooser(LOCALE_SUBTITLES_CHARSET, &g_settings.subs_charset, currentspid == -1, NULL, CRCInput::RC_red, NULL, true);
	sc->addOption("UTF-8");
	sc->addOption("UCS-2");
	sc->addOption("CP1250");
	sc->addOption("CP1251");
	sc->addOption("CP1252");
	sc->addOption("CP1253");
	sc->addOption("KOI8-R");

	APIDSelector.addItem(sc);
	APIDSelector.addItem(GenericMenuSeparatorLine);

	char cnt[5];
	unsigned int count;
	for (count = 0; count < numsubs; count++) {
		bool enabled = sub_supported[count] && (currentspid != spids[count]);

		std::string title = slanguage[count];
		if (title.empty()) {
			char pidnumber[20];
			sprintf(pidnumber, "Stream %d %X", count + 1, spids[count]);
			title = pidnumber;
		}
		sprintf(cnt, "%d", count);
		CMenuForwarder * item = new CMenuForwarder(title.c_str(), enabled, NULL, selector, cnt, CRCInput::convertDigitToKey(count + 1));
		APIDSelector.addItem(item);
	}
	sprintf(cnt, "%d", count);
	APIDSelector.addItem(new CMenuForwarder(LOCALE_SUBTITLES_STOP, currentspid != -1, NULL, selector, cnt, CRCInput::RC_stop), currentspid > 0);

	APIDSelector.exec(NULL, "");
	delete selector;
	printf("CMoviePlayerGui::selectSubtitle: selected %d (%x) current %x\n", select, (select >= 0) ? spids[select] : -1, currentspid);
	if ((select >= 0) && (select < numsubs) && (currentspid != spids[select])) {
		currentspid = spids[select];
		/* external subtitles pid is 0x1FFF */
		ext_subs = (currentspid == 0x1FFF);
		playback->SelectSubtitles(currentspid, g_settings.subs_charset);
		printf("[movieplayer] spid changed to %d\n", currentspid);
	} else if (select > 0) {
		ext_subs = false;
		currentspid = -1;
		playback->SelectSubtitles(currentspid, g_settings.subs_charset);
		printf("[movieplayer] spid changed to %d\n", currentspid);
	}
}

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

void CMoviePlayerGui::clearSubtitle(bool lock)
{
	if ((max_x-min_x > 0) && (max_y-min_y > 0))
		frameBuffer->paintBackgroundBoxRel(min_x, min_y, max_x-min_x, max_y-min_y);

	min_x = CFrameBuffer::getInstance()->getScreenWidth();
	min_y = CFrameBuffer::getInstance()->getScreenHeight();
	max_x = CFrameBuffer::getInstance()->getScreenX();
	max_y = CFrameBuffer::getInstance()->getScreenY();
	end_time = 0;
	lock_subs = lock;
}

fb_pixel_t * simple_resize32(uint8_t * orgin, uint32_t * colors, int nb_colors, int ox, int oy, int dx, int dy);

bool CMoviePlayerGui::convertSubtitle(std::string &text)
{
	bool ret = false;
	iconv_t cd = iconv_open("UTF-8", g_settings.subs_charset.c_str());
	if (cd == (iconv_t)-1) {
		perror("iconv_open");
		return ret;
	}
	size_t ilen = text.length();
	size_t olen = ilen*4;
	size_t len = olen;
	char * buf = (char *) calloc(olen + 1, 1);
	if (buf == NULL) {
		iconv_close(cd);
		return ret;
	}
	char * out = buf;
	char * in = (char *) text.c_str();
	if (iconv(cd, &in, &ilen, &out, &olen) == (size_t)-1) {
		printf("CMoviePlayerGui::convertSubtitle: iconv error\n");
	}
	else {
		memset(buf + (len - olen), 0, olen);
		text = buf;
		ret = true;
	}

	free(buf);
	iconv_close(cd);
	return true;
}

void CMoviePlayerGui::showSubtitle(neutrino_msg_data_t data)
{
	AVSubtitle * sub = (AVSubtitle *) data;
	if (lock_subs) {
		if (sub) {
			avsubtitle_free(sub);
			delete sub;
		}
		return;
	}

	if (!data) {
		if (end_time && time_monotonic_ms() > end_time) {
			printf("************************* hide subs *************************\n");
			clearSubtitle();
		}
		return;
	}

	printf("************************* EVT_SUBT_MESSAGE: num_rects %d fmt %d *************************\n",  sub->num_rects, sub->format);
#if 0
	if (!sub->num_rects)
		return;
#endif
	if (sub->format == 0) {
		int xres = 0, yres = 0, framerate;
		videoDecoder->getPictureInfo(xres, yres, framerate);

		double xc, yc;
		if (sub->num_rects && (sub->rects[0]->x + sub->rects[0]->w) <= 720 &&
				sub->rects[0]->y + sub->rects[0]->h <= 576) {
			xc = (double) CFrameBuffer::getInstance()->getScreenWidth(/*true*/)/(double) 720;
			yc = (double) CFrameBuffer::getInstance()->getScreenHeight(/*true*/)/(double) 576;
		} else {
			xc = (double) CFrameBuffer::getInstance()->getScreenWidth(/*true*/)/(double) xres;
			yc = (double) CFrameBuffer::getInstance()->getScreenHeight(/*true*/)/(double) yres;
		}

		clearSubtitle();

		for (unsigned i = 0; i < sub->num_rects; i++) {
			uint32_t * colors = (uint32_t *) sub->rects[i]->pict.data[1];

			int xoff = (double) sub->rects[i]->x * xc;
			int yoff = (double) sub->rects[i]->y * yc;
			int nw = GetWidth4FB_HW_ACC(xoff, (double) sub->rects[i]->w * xc);
			int nh = (double) sub->rects[i]->h * yc;

			printf("Draw: #%d at %d,%d size %dx%d colors %d (x=%d y=%d w=%d h=%d) \n", i+1,
					sub->rects[i]->x, sub->rects[i]->y, sub->rects[i]->w, sub->rects[i]->h,
					sub->rects[i]->nb_colors, xoff, yoff, nw, nh);

			fb_pixel_t * newdata = simple_resize32 (sub->rects[i]->pict.data[0], colors,
					sub->rects[i]->nb_colors, sub->rects[i]->w, sub->rects[i]->h, nw, nh);
			frameBuffer->blit2FB(newdata, nw, nh, xoff, yoff);
			free(newdata);

			min_x = std::min(min_x, xoff);
			max_x = std::max(max_x, xoff + nw);
			min_y = std::min(min_y, yoff);
			max_y = std::max(max_y, yoff + nh);
		}

		//end_time = sub->end_display_time + time_monotonic_ms();
		end_time = 10*1000;
		if (sub->end_display_time != UINT32_MAX)
			end_time = sub->end_display_time;
		end_time += time_monotonic_ms();

		avsubtitle_free(sub);
		delete sub;
		return;
	}
	std::vector<std::string> subtext;
	for (unsigned i = 0; i < sub->num_rects; i++) {
		char * txt = NULL;
		if (sub->rects[i]->type == SUBTITLE_TEXT)
			txt = sub->rects[i]->text;
		else if (sub->rects[i]->type == SUBTITLE_ASS)
			txt = sub->rects[i]->ass;
		printf("subt[%d] type %d [%s]\n", i, sub->rects[i]->type, txt ? txt : "");
		if (txt) {
			int len = strlen(txt);
			if (len > 10 && memcmp(txt, "Dialogue: ", 10) == 0) {
				char* p = txt;
#if LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(55, 69, 100)
				int skip_commas = 4;
#else
				int skip_commas = 9;
#endif
				/* skip ass times */
				for (int j = 0; j < skip_commas && *p != '\0'; p++)
					if (*p == ',')
						j++;
				/* skip ass tags */
				if (*p == '{') {
					char * d = strchr(p, '}');
					if (d)
						p += d - p + 1;
				}
				char * d = strchr(p, '{');
				if (d && strchr(d, '}'))
					*d = 0;

				len = strlen(p);
				/* remove newline */
				for (int j = len-1; j > 0; j--) {
					if (p[j] == '\n' || p[j] == '\r')
						p[j] = 0;
					else
						break;
				}
				if (*p == '\0')
					continue;
				txt = p;
			}
			//printf("title: [%s]\n", txt);
			std::string str(txt);
			size_t start = 0, end = 0;
			/* split string with \N as newline */
			std::string delim("\\N");
			while ((end = str.find(delim, start)) != string::npos) {
				subtext.push_back(str.substr(start, end - start));
				start = end + 2;
			}
			subtext.push_back(str.substr(start));
		}
	}
	for (unsigned i = 0; i < subtext.size(); i++) {
		if (!isUTF8(subtext[i])) {
			if (g_settings.subs_charset != "UTF-8")
				convertSubtitle(subtext[i]);
			else
				subtext[i] = convertLatin1UTF8(subtext[i]);
		}
		printf("subtext %d: [%s]\n", i, subtext[i].c_str());
	}
	printf("********************************************************************\n");

	if (!subtext.empty()) {
		int sh = frameBuffer->getScreenHeight();
		int sw = frameBuffer->getScreenWidth();
		int h = g_Font[SNeutrinoSettings::FONT_TYPE_SUBTITLES]->getHeight();
		int height = h*subtext.size();

		clearSubtitle();

		int x[subtext.size()];
		int y[subtext.size()];
		for (unsigned i = 0; i < subtext.size(); i++) {
			int w = g_Font[SNeutrinoSettings::FONT_TYPE_SUBTITLES]->getRenderWidth (subtext[i]);
			x[i] = (sw - w) / 2;
			y[i] = sh - height + h*(i + 1);
			min_x = std::min(min_x, x[i]);
			max_x = std::max(max_x, x[i]+w);
			min_y = std::min(min_y, y[i]-h);
			max_y = std::max(max_y, y[i]);
		}

		frameBuffer->paintBoxRel(min_x, min_y, max_x - min_x, max_y-min_y, COL_MENUCONTENT_PLUS_0);

		for (unsigned i = 0; i < subtext.size(); i++)
			g_Font[SNeutrinoSettings::FONT_TYPE_SUBTITLES]->RenderString(x[i], y[i], sw, subtext[i].c_str(), COL_MENUCONTENT_TEXT);

		end_time = sub->end_display_time + time_monotonic_ms();
	}
	avsubtitle_free(sub);
	delete sub;
}

void CMoviePlayerGui::selectAutoLang()
{
	if (!numsubs)
		playback->FindAllSubs(spids, sub_supported, &numsubs, slanguage);

	if (ext_subs) {
		for (unsigned count = 0; count < numsubs; count++) {
			if (spids[count] == 0x1FFF) {
				currentspid = spids[count];
				playback->SelectSubtitles(currentspid, g_settings.subs_charset);
			}
		}
	}
	if (g_settings.auto_lang &&  (numpida > 1)) {
		int pref_idx = -1;

		playback->FindAllPids(apids, ac3flags, &numpida, language);
		for(int i = 0; i < 3; i++) {
			for (unsigned j = 0; j < numpida; j++) {
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); ++it) {
					if (g_settings.pref_lang[i] == it->second && strncasecmp(language[j].c_str(), it->first.c_str(), 3) == 0) {
						bool enabled = true;
						// TODO: better check of supported
						std::string audioname;
						addAudioFormat(j, audioname, enabled);
						if (enabled)
							pref_idx = j;
						break;
					}
				}
				if (pref_idx >= 0)
					break;
			}
			if (pref_idx >= 0)
				break;
		}
		if (pref_idx >= 0) {
			currentapid = apids[pref_idx];
			currentac3 = ac3flags[pref_idx];
			playback->SetAPid(currentapid, currentac3);
			getCurrentAudioName(is_file_player, currentaudioname);
		}
	}
	if (isWebTV && g_settings.auto_subs && numsubs > 0) {
		for(int i = 0; i < 3; i++) {
			if(g_settings.pref_subs[i].empty() || g_settings.pref_subs[i] == "none")
				continue;

			std::string temp(g_settings.pref_subs[i]);
			std::string slang;
			for (int j = 0 ; j < numsubs; j++) {
				if (!sub_supported[j])
					continue;
				slang = slanguage[j].substr(0, 3);
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); ++it) {
					if(temp == it->second && slang == it->first) {
						currentspid = spids[j];
						break;
					}
				}
				if (currentspid > 0)
					break;
			}
			if (currentspid > 0) {
				playback->SelectSubtitles(currentspid, g_settings.subs_charset);
				printf("[movieplayer] spid changed to %d %s (%s)\n", currentspid, temp.c_str(), slang.c_str());
				break;
			}
		}
	}
}

void CMoviePlayerGui::parsePlaylist(CFile *file)
{
	std::ifstream infile;
	char cLine[1024];
	char name[1024] = { 0 };
	infile.open(file->Name.c_str(), std::ifstream::in);
	filelist_it = filelist.erase(filelist_it);
	CFile tmp_file;
	while (infile.good())
	{
		infile.getline(cLine, sizeof(cLine));
		size_t len = strlen(cLine);
		if (len > 0 && cLine[len-1]=='\r')
			cLine[len-1]=0;

		int dur;
		sscanf(cLine, "#EXTINF:%d,%[^\n]\n", &dur, name);
		if (len > 0 && cLine[0]!='#')
		{
			char *url = NULL;
			if ((url = strstr(cLine, "http://")) || (url = strstr(cLine, "https://")) || (url = strstr(cLine, "rtmp://")) || (url = strstr(cLine, "rtsp://")) || (url = strstr(cLine, "mmsh://")) ) {
				if (url != NULL) {
					printf("name %s [%d] url: %s\n", name, dur, url);
					tmp_file.Name = name;
					tmp_file.Url = url;
					filelist.push_back(tmp_file);
				}
			}
		}
	}
	filelist_it = filelist.begin();
}

bool CMoviePlayerGui::mountIso(CFile *file)
{
	printf("ISO file passed: %s\n", file->Name.c_str());
	safe_mkdir(ISO_MOUNT_POINT);
	if (my_system(5, "mount", "-o", "loop", file->Name.c_str(), ISO_MOUNT_POINT) == 0) {
		makeFilename();
		file_name = "/media/iso";
		iso_file = true;
		return true;
	}
	return false;
}

void CMoviePlayerGui::makeScreenShot(bool autoshot, bool forcover)
{
	if (autoshot && (autoshot_done || !g_settings.auto_cover))
		return;

	bool cover = autoshot || g_settings.screenshot_cover || forcover;
	char ending[(sizeof(int)*2) + 6] = ".jpg";
	if (!cover)
		snprintf(ending, sizeof(ending) - 1, "_%x.jpg", position);

	std::string fname = file_name;
	if (p_movie_info)
		fname = p_movie_info->file.Name;

	/* quick check we have file and not url as file name */
	if (fname.c_str()[0] != '/') {
		if (autoshot)
			autoshot_done = true;
		return;
	}

	std::string::size_type pos = fname.find_last_of('.');
	if (pos != std::string::npos) {
		fname.replace(pos, fname.length(), ending);
	} else
		fname += ending;

	if (autoshot && !access(fname.c_str(), F_OK)) {
		printf("CMoviePlayerGui::makeScreenShot: cover [%s] already exist..\n", fname.c_str());
		autoshot_done = true;
		return;
	}

	if (!cover) {
		pos = fname.find_last_of('/');
		if (pos != std::string::npos)
			fname.replace(0, pos, g_settings.screenshot_dir);
	}


	CScreenShot * sc = new CScreenShot(fname);
	if (cover) {
		sc->EnableOSD(false);
		sc->EnableVideo(true);
	}
	if (autoshot || forcover) {
		int xres = 0, yres = 0, framerate;
		videoDecoder->getPictureInfo(xres, yres, framerate);
		if (xres && yres) {
			int w = std::min(300, xres);
			int h = (float) yres / ((float) xres / (float) w);
			sc->SetSize(w, h);
		}
	}
	sc->Start();
	if (autoshot)
		autoshot_done = true;
}

size_t CMoviePlayerGui::GetReadCount()
{
        uint64_t this_read = 0;
        this_read = playback->GetReadCount();
        uint64_t res;
        if (this_read < last_read)
                res = 0;
        else
                res = this_read - last_read;
        last_read = this_read;
//printf("GetReadCount: %lld\n", res);
        return (size_t) res;
}

void CMoviePlayerGui::screensaver(bool on)
{
	if (on)
	{
		m_screensaver = true;
		CScreenSaver::getInstance()->Start();
	}
	else
	{
		CScreenSaver::getInstance()->Stop();
		m_screensaver = false;
		m_idletime = time(NULL);
	}
}
