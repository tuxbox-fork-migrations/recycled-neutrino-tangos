#ifndef __locals_intern__
#define __locals_intern__

/*
 * $Id$
 *
 * (C) 2004 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

const char * locale_real_names[] =
{
	"INTERNAL ERROR - PLEASE REPORT",
	"AUDIOSelectMenue.head",
	"EPGMenu.epgplus",
	"EPGMenu.eventinfo",
	"EPGMenu.eventlist",
	"EPGMenu.head",
	"EPGMenu.streaminfo",
	"EPGPlus.actions",
	"EPGPlus.bybouquet_mode",
	"EPGPlus.bypage_mode",
	"EPGPlus.head",
	"EPGPlus.next_bouquet",
	"EPGPlus.options",
	"EPGPlus.page_down",
	"EPGPlus.page_up",
	"EPGPlus.prev_bouquet",
	"EPGPlus.record",
	"EPGPlus.refresh_epg",
	"EPGPlus.remind",
	"EPGPlus.scroll_mode",
	"EPGPlus.stretch_mode",
	"EPGPlus.swap_mode",
	"EPGPlus.view_mode",
	"GENRE.ARTS.0",
	"GENRE.ARTS.1",
	"GENRE.ARTS.10",
	"GENRE.ARTS.11",
	"GENRE.ARTS.2",
	"GENRE.ARTS.3",
	"GENRE.ARTS.4",
	"GENRE.ARTS.5",
	"GENRE.ARTS.6",
	"GENRE.ARTS.7",
	"GENRE.ARTS.8",
	"GENRE.ARTS.9",
	"GENRE.CHILDRENs_PROGRAMMES.0",
	"GENRE.CHILDRENs_PROGRAMMES.1",
	"GENRE.CHILDRENs_PROGRAMMES.2",
	"GENRE.CHILDRENs_PROGRAMMES.3",
	"GENRE.CHILDRENs_PROGRAMMES.4",
	"GENRE.CHILDRENs_PROGRAMMES.5",
	"GENRE.DOCUS_MAGAZINES.0",
	"GENRE.DOCUS_MAGAZINES.1",
	"GENRE.DOCUS_MAGAZINES.2",
	"GENRE.DOCUS_MAGAZINES.3",
	"GENRE.DOCUS_MAGAZINES.4",
	"GENRE.DOCUS_MAGAZINES.5",
	"GENRE.DOCUS_MAGAZINES.6",
	"GENRE.DOCUS_MAGAZINES.7",
	"GENRE.MOVIE.0",
	"GENRE.MOVIE.1",
	"GENRE.MOVIE.2",
	"GENRE.MOVIE.3",
	"GENRE.MOVIE.4",
	"GENRE.MOVIE.5",
	"GENRE.MOVIE.6",
	"GENRE.MOVIE.7",
	"GENRE.MOVIE.8",
	"GENRE.MUSIC_DANCE.0",
	"GENRE.MUSIC_DANCE.1",
	"GENRE.MUSIC_DANCE.2",
	"GENRE.MUSIC_DANCE.3",
	"GENRE.MUSIC_DANCE.4",
	"GENRE.MUSIC_DANCE.5",
	"GENRE.MUSIC_DANCE.6",
	"GENRE.NEWS.0",
	"GENRE.NEWS.1",
	"GENRE.NEWS.2",
	"GENRE.NEWS.3",
	"GENRE.NEWS.4",
	"GENRE.SHOW.0",
	"GENRE.SHOW.1",
	"GENRE.SHOW.2",
	"GENRE.SHOW.3",
	"GENRE.SOCIAL_POLITICAL.0",
	"GENRE.SOCIAL_POLITICAL.1",
	"GENRE.SOCIAL_POLITICAL.2",
	"GENRE.SOCIAL_POLITICAL.3",
	"GENRE.SPORTS.0",
	"GENRE.SPORTS.1",
	"GENRE.SPORTS.10",
	"GENRE.SPORTS.11",
	"GENRE.SPORTS.2",
	"GENRE.SPORTS.3",
	"GENRE.SPORTS.4",
	"GENRE.SPORTS.5",
	"GENRE.SPORTS.6",
	"GENRE.SPORTS.7",
	"GENRE.SPORTS.8",
	"GENRE.SPORTS.9",
	"GENRE.TRAVEL_HOBBIES.0",
	"GENRE.TRAVEL_HOBBIES.1",
	"GENRE.TRAVEL_HOBBIES.2",
	"GENRE.TRAVEL_HOBBIES.3",
	"GENRE.TRAVEL_HOBBIES.4",
	"GENRE.TRAVEL_HOBBIES.5",
	"GENRE.TRAVEL_HOBBIES.6",
	"GENRE.TRAVEL_HOBBIES.7",
	"GENRE.UNKNOWN",
	"apidselector.head",
	"audio.srs_algo",
	"audio.srs_algo_light",
	"audio.srs_algo_normal",
	"audio.srs_iq",
	"audio.srs_nmgr",
	"audio.srs_volume",
	"audiomenu.PCMOffset",
	"audiomenu.analog_mode",
	"audiomenu.analog_out",
	"audiomenu.auto_lang",
	"audiomenu.auto_subs",
	"audiomenu.avs",
	"audiomenu.avsync",
	"audiomenu.avsync_am",
	"audiomenu.clockrec",
	"audiomenu.dolbydigital",
	"audiomenu.hdmi_dd",
	"audiomenu.hdmi_dd_auto",
	"audiomenu.hdmi_dd_force",
	"audiomenu.monoleft",
	"audiomenu.monoright",
	"audiomenu.pref_lang",
	"audiomenu.pref_lang_head",
	"audiomenu.pref_languages",
	"audiomenu.pref_subs",
	"audiomenu.pref_subs_head",
	"audiomenu.spdif_dd",
	"audiomenu.stereo",
	"audiomenu.volume_step",
	"audioplayer.add",
	"audioplayer.add_ic",
	"audioplayer.add_loc",
	"audioplayer.add_sc",
	"audioplayer.artist_title",
	"audioplayer.building_search_index",
	"audioplayer.button_select_title_by_id",
	"audioplayer.button_select_title_by_name",
	"audioplayer.defdir",
	"audioplayer.delete",
	"audioplayer.deleteall",
	"audioplayer.display_order",
	"audioplayer.enable_sc_metadata",
	"audioplayer.fastforward",
	"audioplayer.follow",
	"audioplayer.head",
	"audioplayer.highprio",
	"audioplayer.jump_backwards",
	"audioplayer.jump_dialog_hint1",
	"audioplayer.jump_dialog_hint2",
	"audioplayer.jump_dialog_title",
	"audioplayer.jump_forwards",
	"audioplayer.keylevel",
	"audioplayer.load_radio_stations",
	"audioplayer.name",
	"audioplayer.pause",
	"audioplayer.play",
	"audioplayer.playing",
	"audioplayer.playlist_fileerror_msg",
	"audioplayer.playlist_fileoverwrite_msg",
	"audioplayer.playlist_fileoverwrite_title",
	"audioplayer.playlist_name",
	"audioplayer.playlist_name_hint1",
	"audioplayer.playlist_name_hint2",
	"audioplayer.reading_files",
	"audioplayer.receiving_list",
	"audioplayer.repeat_on",
	"audioplayer.rewind",
	"audioplayer.save_playlist",
	"audioplayer.screensaver_timeout",
	"audioplayer.select_title_by_name",
	"audioplayer.show_playlist",
	"audioplayer.shuffle",
	"audioplayer.spectrum",
	"audioplayer.stop",
	"audioplayer.title_artist",
	"audioplayerpicsettings.general",
	"bookmarkmanager.delete",
	"bookmarkmanager.name",
	"bookmarkmanager.rename",
	"bookmarkmanager.select",
	"bouqueteditor.add",
	"bouqueteditor.bouquetname",
	"bouqueteditor.delete",
	"bouqueteditor.delete_question",
	"bouqueteditor.discardingchanges",
	"bouqueteditor.hide",
	"bouqueteditor.lock",
	"bouqueteditor.move",
	"bouqueteditor.name",
	"bouqueteditor.newbouquetname",
	"bouqueteditor.rename",
	"bouqueteditor.return",
	"bouqueteditor.savechanges?",
	"bouqueteditor.savingchanges",
	"bouqueteditor.switch",
	"bouqueteditor.switchmode",
	"bouquetlist.head",
	"bouquetname.hdtv",
	"bouquetname.new",
	"bouquetname.other",
	"bouquetname.removed",
	"cablesetup.provider",
	"channellist.current_tp",
	"channellist.edit",
	"channellist.epgtext_align_left",
	"channellist.epgtext_align_right",
	"channellist.extended",
	"channellist.favs",
	"channellist.foot",
	"channellist.foot_freq",
	"channellist.foot_next",
	"channellist.foot_off",
	"channellist.foot_sort_alpha",
	"channellist.foot_sort_freq",
	"channellist.foot_sort_sat",
	"channellist.head",
	"channellist.history",
	"channellist.keep_numbers",
	"channellist.make_hdlist",
	"channellist.make_newlist",
	"channellist.make_removedlist",
	"channellist.new_zap_mode",
	"channellist.nonefound",
	"channellist.provs",
	"channellist.recording_not_possible",
	"channellist.reset_flags",
	"channellist.sats",
	"channellist.since",
	"channellist.start",
	"ci.clock",
	"ci.empty",
	"ci.init_failed",
	"ci.init_ok",
	"ci.inserted",
	"ci.removed",
	"ci.reset",
	"ci.reset_standby",
	"ci.settings",
	"ci.timeout",
	"ci.waiting",
	"clock_switch_off",
	"clock_switch_on",
	"colorchooser.alpha",
	"colorchooser.blue",
	"colorchooser.green",
	"colorchooser.red",
	"colormenu.background",
	"colormenu.contrast_fonts",
	"colormenu.fade",
	"colormenu.font",
	"colormenu.font_ttx",
	"colormenu.hd_preset",
	"colormenu.menucolors",
	"colormenu.osd_preset",
	"colormenu.sd_preset",
	"colormenu.textcolor",
	"colormenu.themeselect",
	"colormenu.timing",
	"colormenusetup.menucontent",
	"colormenusetup.menucontent_inactive",
	"colormenusetup.menucontent_selected",
	"colormenusetup.menuhead",
	"colorstatusbar.text",
	"colorthememenu.head",
	"colorthememenu.head2",
	"colorthememenu.name",
	"colorthememenu.neutrino_theme",
	"colorthememenu.question",
	"colorthememenu.save",
	"colorthememenu.select1",
	"colorthememenu.select2",
	"cpu.freq",
	"cpu.freq_default",
	"cpu.freq_normal",
	"cpu.freq_standby",
	"date.Apr",
	"date.Aug",
	"date.Dec",
	"date.Feb",
	"date.Fri",
	"date.Jan",
	"date.Jul",
	"date.Jun",
	"date.Mar",
	"date.May",
	"date.Mon",
	"date.Nov",
	"date.Oct",
	"date.Sat",
	"date.Sep",
	"date.Sun",
	"date.Thu",
	"date.Tue",
	"date.Wed",
	"epgextended.actors",
	"epgextended.director",
	"epgextended.guests",
	"epgextended.original_title",
	"epgextended.presenter",
	"epgextended.year_of_production",
	"epglist.noevents",
	"epgviewer.More_Screenings",
	"epgviewer.nodetailed",
	"epgviewer.notfound",
	"eventfinder.head",
	"eventfinder.keyword",
	"eventfinder.search",
	"eventfinder.search_within_epg",
	"eventfinder.search_within_list",
	"eventfinder.searching",
	"eventfinder.start_search",
	"eventlistbar.channelswitch",
	"eventlistbar.eventsort",
	"eventlistbar.recordevent",
	"extra.add_to_bouquet",
	"extra.audio_run_player",
	"extra.auto_delete",
	"extra.auto_timeshift",
	"extra.bigwindows",
	"extra.cache_txt",
	"extra.chadded",
	"extra.chalreadyinbq",
	"extra.dboxinfo",
	"extra.east",
	"extra.fec_1_2",
	"extra.fec_2_3",
	"extra.fec_3_4",
	"extra.fec_5_6",
	"extra.fec_7_8",
	"extra.fec_auto",
	"extra.fec_auto_s2",
	"extra.fec_s2_8psk_1_2",
	"extra.fec_s2_8psk_2_3",
	"extra.fec_s2_8psk_3_4",
	"extra.fec_s2_8psk_3_5",
	"extra.fec_s2_8psk_4_5",
	"extra.fec_s2_8psk_5_6",
	"extra.fec_s2_8psk_7_8",
	"extra.fec_s2_8psk_8_9",
	"extra.fec_s2_8psk_9_10",
	"extra.fec_s2_qpsk_1_2",
	"extra.fec_s2_qpsk_2_3",
	"extra.fec_s2_qpsk_3_4",
	"extra.fec_s2_qpsk_3_5",
	"extra.fec_s2_qpsk_4_5",
	"extra.fec_s2_qpsk_5_6",
	"extra.fec_s2_qpsk_7_8",
	"extra.fec_s2_qpsk_8_9",
	"extra.fec_s2_qpsk_9_10",
	"extra.key_current_transponder",
	"extra.key_list_end",
	"extra.key_list_start",
	"extra.key_plugin",
	"extra.key_screenshot",
	"extra.key_timeshift",
	"extra.key_unlock",
	"extra.ladirection",
	"extra.latitude",
	"extra.loadconfig",
	"extra.loadkeys",
	"extra.lodirection",
	"extra.longitude",
	"extra.menu_left_exit",
	"extra.north",
	"extra.record_time",
	"extra.rotor_swap",
	"extra.rounded_corners",
	"extra.rounded_corners_off",
	"extra.rounded_corners_on",
	"extra.saveconfig",
	"extra.savekeys",
	"extra.scrambled_message",
	"extra.show_mute_icon",
	"extra.sms_channel",
	"extra.south",
	"extra.start_tostandby",
	"extra.temp_timeshift",
	"extra.timeshift_pause",
	"extra.tp.pol_v",
	"extra.tp_fec",
	"extra.tp_freq",
	"extra.tp_mod",
	"extra.tp_mod_128",
	"extra.tp_mod_16",
	"extra.tp_mod_256",
	"extra.tp_mod_32",
	"extra.tp_mod_64",
	"extra.tp_pol",
	"extra.tp_pol_h",
	"extra.tp_pol_l",
	"extra.tp_pol_r",
	"extra.tp_rate",
	"extra.update_dir",
	"extra.use_gotoxx",
	"extra.volume_pos",
	"extra.west",
	"extra.zap_cycle",
	"extra.zapit_fe_timeout",
	"extra.zapit_hvoltage",
	"extra.zapit_make_bouquet",
	"extra.zapit_motor_speed",
	"extra.zapit_scanpids",
	"extra.zapit_sdt_changed",
	"fan_speed",
	"favorites.addchannel",
	"favorites.bouquetname",
	"favorites.bqcreated",
	"favorites.chadded",
	"favorites.chalreadyinbq",
	"favorites.copy",
	"favorites.finalhint",
	"favorites.menueadd",
	"favorites.nobouquets",
	"filebrowser.delete",
	"filebrowser.denydirectoryleave",
	"filebrowser.dodelete1",
	"filebrowser.dodelete2",
	"filebrowser.filter.active",
	"filebrowser.filter.inactive",
	"filebrowser.head",
	"filebrowser.mark",
	"filebrowser.nextpage",
	"filebrowser.prevpage",
	"filebrowser.scan",
	"filebrowser.select",
	"filebrowser.showrights",
	"filebrowser.sort.date",
	"filebrowser.sort.name",
	"filebrowser.sort.namedirsfirst",
	"filebrowser.sort.size",
	"filebrowser.sort.type",
	"filesystem.is.utf8",
	"filesystem.is.utf8.option.iso8859.1",
	"filesystem.is.utf8.option.utf8",
	"flashupdate.actionreadflash",
	"flashupdate.cantopenfile",
	"flashupdate.cantopenmtd",
	"flashupdate.checkupdate",
	"flashupdate.currentversion_sep",
	"flashupdate.currentversiondate",
	"flashupdate.currentversiontime",
	"flashupdate.erasefailed",
	"flashupdate.erasing",
	"flashupdate.experimentalimage",
	"flashupdate.expertfunctions",
	"flashupdate.fileis0bytes",
	"flashupdate.fileselector",
	"flashupdate.flashreadyreboot",
	"flashupdate.getinfofile",
	"flashupdate.getinfofileerror",
	"flashupdate.getupdatefile",
	"flashupdate.getupdatefileerror",
	"flashupdate.globalprogress",
	"flashupdate.head",
	"flashupdate.md5check",
	"flashupdate.md5sumerror",
	"flashupdate.msgbox",
	"flashupdate.msgbox_manual",
	"flashupdate.mtdselector",
	"flashupdate.new_found",
	"flashupdate.new_notfound",
	"flashupdate.programmingflash",
	"flashupdate.proxypassword",
	"flashupdate.proxypassword_hint1",
	"flashupdate.proxypassword_hint2",
	"flashupdate.proxyserver",
	"flashupdate.proxyserver_hint1",
	"flashupdate.proxyserver_hint2",
	"flashupdate.proxyserver_sep",
	"flashupdate.proxyusername",
	"flashupdate.proxyusername_hint1",
	"flashupdate.proxyusername_hint2",
	"flashupdate.readflash",
	"flashupdate.readflashmtd",
	"flashupdate.ready",
	"flashupdate.reallyflashmtd",
	"flashupdate.savesuccess",
	"flashupdate.selectimage",
	"flashupdate.settings",
	"flashupdate.squashfs.noversion",
	"flashupdate.titlereadflash",
	"flashupdate.titlewriteflash",
	"flashupdate.updatemode",
	"flashupdate.updatemode_internet",
	"flashupdate.updatemode_manual",
	"flashupdate.url_file",
	"flashupdate.versioncheck",
	"flashupdate.writeflash",
	"flashupdate.writeflashmtd",
	"flashupdate.wrongbase",
	"fontmenu.channellist",
	"fontmenu.epg",
	"fontmenu.eventlist",
	"fontmenu.gamelist",
	"fontmenu.head",
	"fontmenu.infobar",
	"fontmenu.menu",
	"fontmenu.scaling",
	"fontmenu.scaling_x",
	"fontmenu.scaling_x_hint2",
	"fontmenu.scaling_y",
	"fontmenu.scaling_y_hint2",
	"fontmenu.sizes",
	"fontsize.channel_num_zap",
	"fontsize.channellist",
	"fontsize.channellist_descr",
	"fontsize.channellist_number",
	"fontsize.epg_date",
	"fontsize.epg_info1",
	"fontsize.epg_info2",
	"fontsize.epg_title",
	"fontsize.eventlist_datetime",
	"fontsize.eventlist_itemlarge",
	"fontsize.eventlist_itemsmall",
	"fontsize.eventlist_title",
	"fontsize.filebrowser_item",
	"fontsize.gamelist_itemlarge",
	"fontsize.gamelist_itemsmall",
	"fontsize.hint",
	"fontsize.infobar_channame",
	"fontsize.infobar_info",
	"fontsize.infobar_number",
	"fontsize.infobar_small",
	"fontsize.menu",
	"fontsize.menu_info",
	"fontsize.menu_title",
	"hdd_10min",
	"hdd_1min",
	"hdd_20min",
	"hdd_30min",
	"hdd_5min",
	"hdd_60min",
	"hdd_activate",
	"hdd_check",
	"hdd_check_failed",
	"hdd_ext3",
	"hdd_extended_settings",
	"hdd_fast",
	"hdd_format",
	"hdd_format_failed",
	"hdd_format_warn",
	"hdd_fs",
	"hdd_manage",
	"hdd_middle",
	"hdd_noise",
	"hdd_not_found",
	"hdd_reiser",
	"hdd_removable_device",
	"hdd_settings",
	"hdd_sleep",
	"hdd_slow",
	"hdd_umount_warn",
	"imageinfo.creator",
	"imageinfo.date",
	"imageinfo.dokumentation",
	"imageinfo.forum",
	"imageinfo.head",
	"imageinfo.homepage",
	"imageinfo.image",
	"imageinfo.license",
	"imageinfo.version",
	"inetradio.name",
	"infoviewer.epgnotload",
	"infoviewer.epgwait",
	"infoviewer.eventlist",
	"infoviewer.languages",
	"infoviewer.motor_moving",
	"infoviewer.next",
	"infoviewer.nocurrent",
	"infoviewer.noepg",
	"infoviewer.notavailable",
	"infoviewer.now",
	"infoviewer.selecttime",
	"infoviewer.streaminfo",
	"infoviewer.subchan_disp_pos",
	"infoviewer.subchan_infobar",
	"infoviewer.subservice",
	"infoviewer.waittime",
	"ipsetup.hint_1",
	"ipsetup.hint_2",
	"keybindingmenu.RC",
	"keybindingmenu.addrecord",
	"keybindingmenu.addremind",
	"keybindingmenu.allchannels_on_ok",
	"keybindingmenu.bouquetchannels_on_ok",
	"keybindingmenu.bouquetdown",
	"keybindingmenu.bouquethandling",
	"keybindingmenu.bouquetlist_on_ok",
	"keybindingmenu.bouquetup",
	"keybindingmenu.cancel",
	"keybindingmenu.channeldown",
	"keybindingmenu.channellist",
	"keybindingmenu.channelup",
	"keybindingmenu.head",
	"keybindingmenu.lastchannel",
	"keybindingmenu.misc",
	"keybindingmenu.mode_left_right_key_tv",
	"keybindingmenu.mode_left_right_key_tv_infobar",
	"keybindingmenu.mode_left_right_key_tv_volume",
	"keybindingmenu.mode_left_right_key_tv_vzap",
	"keybindingmenu.mode_left_right_key_tv_zap",
	"keybindingmenu.modechange",
	"keybindingmenu.pagedown",
	"keybindingmenu.pageup",
	"keybindingmenu.poweroff",
	"keybindingmenu.quickzap",
	"keybindingmenu.remotecontrol_hardware",
	"keybindingmenu.remotecontrol_hardware_coolstream",
	"keybindingmenu.remotecontrol_hardware_dbox",
	"keybindingmenu.remotecontrol_hardware_msg_part1",
	"keybindingmenu.remotecontrol_hardware_msg_part2",
	"keybindingmenu.remotecontrol_hardware_msg_part3",
	"keybindingmenu.remotecontrol_hardware_philips",
	"keybindingmenu.remotecontrol_hardware_tripledragon",
	"keybindingmenu.repeatblock",
	"keybindingmenu.repeatblockgeneric",
	"keybindingmenu.sort",
	"keybindingmenu.subchanneldown",
	"keybindingmenu.subchannelup",
	"keybindingmenu.tvradiomode",
	"keybindingmenu.zaphistory",
	"keychooser.head",
	"keychooser.text1",
	"keychooser.text2",
	"keychoosermenu.currentkey",
	"keychoosermenu.setnew",
	"keychoosermenu.setnone",
	"languagesetup.head",
	"languagesetup.osd",
	"languagesetup.select",
	"lcd_info_line",
	"lcd_info_line_channel",
	"lcd_info_line_clock",
	"lcdcontroler.brightness",
	"lcdcontroler.brightnessdeepstandby",
	"lcdcontroler.brightnessstandby",
	"lcdcontroler.contrast",
	"lcdmenu.dim_brightness",
	"lcdmenu.dim_time",
	"lcdmenu.head",
	"lcdmenu.lcdcontroler",
	"lcdmenu.statusline",
	"lcdmenu.statusline.both",
	"lcdmenu.statusline.playtime",
	"lcdmenu.statusline.volume",
	"ledcontroler.blink",
	"ledcontroler.menu",
	"ledcontroler.mode.deepstandby",
	"ledcontroler.mode.record",
	"ledcontroler.mode.standby",
	"ledcontroler.mode.tv",
	"ledcontroler.off",
	"ledcontroler.on.all",
	"ledcontroler.on.led1",
	"ledcontroler.on.led2",
	"mainmenu.audioplayer",
	"mainmenu.clearsectionsd",
	"mainmenu.games",
	"mainmenu.head",
	"mainmenu.media",
	"mainmenu.movieplayer",
	"mainmenu.pausesectionsd",
	"mainmenu.pictureviewer",
	"mainmenu.radiomode",
	"mainmenu.reboot",
	"mainmenu.recording",
	"mainmenu.recording_start",
	"mainmenu.recording_stop",
	"mainmenu.scripts",
	"mainmenu.service",
	"mainmenu.settings",
	"mainmenu.shutdown",
	"mainmenu.sleeptimer",
	"mainmenu.tvmode",
	"mainmenu.tvradio_switch",
	"mainsettings.audio",
	"mainsettings.head",
	"mainsettings.keybinding",
	"mainsettings.language",
	"mainsettings.lcd",
	"mainsettings.manage",
	"mainsettings.misc",
	"mainsettings.network",
	"mainsettings.osd",
	"mainsettings.recording",
	"mainsettings.savesettingsnow",
	"mainsettings.savesettingsnow_hint",
	"mainsettings.timezone",
	"mainsettings.video",
	"menu.back",
	"menu.cancel",
	"menu.next",
	"messagebox.back",
	"messagebox.cancel",
	"messagebox.discard",
	"messagebox.error",
	"messagebox.info",
	"messagebox.no",
	"messagebox.ok",
	"messagebox.yes",
	"miscsettings.channellist",
	"miscsettings.channellist_colored_events",
	"miscsettings.channellist_epgtext_align",
	"miscsettings.colored_events_0",
	"miscsettings.colored_events_1",
	"miscsettings.colored_events_2",
	"miscsettings.energy",
	"miscsettings.epg_cache",
	"miscsettings.epg_cache_hint1",
	"miscsettings.epg_cache_hint2",
	"miscsettings.epg_dir",
	"miscsettings.epg_extendedcache",
	"miscsettings.epg_extendedcache_hint1",
	"miscsettings.epg_extendedcache_hint2",
	"miscsettings.epg_head",
	"miscsettings.epg_max_events",
	"miscsettings.epg_max_events_hint1",
	"miscsettings.epg_max_events_hint2",
	"miscsettings.epg_old_events",
	"miscsettings.epg_old_events_hint1",
	"miscsettings.epg_old_events_hint2",
	"miscsettings.epg_save",
	"miscsettings.general",
	"miscsettings.head",
	"miscsettings.infobar",
	"miscsettings.infobar_casystem_display",
	"miscsettings.infobar_casystem_mini",
	"miscsettings.infobar_casystem_mode",
	"miscsettings.infobar_colored_events",
	"miscsettings.infobar_disp_0",
	"miscsettings.infobar_disp_1",
	"miscsettings.infobar_disp_2",
	"miscsettings.infobar_disp_3",
	"miscsettings.infobar_disp_4",
	"miscsettings.infobar_disp_5",
	"miscsettings.infobar_disp_6",
	"miscsettings.infobar_disp_log",
	"miscsettings.infobar_logo_hdd_dir",
	"miscsettings.infobar_sat_display",
	"miscsettings.infobar_show",
	"miscsettings.infobar_show_res",
	"miscsettings.infobar_show_res_simple",
	"miscsettings.infobar_show_sysfs_hdd",
	"miscsettings.infobar_show_tuner",
	"miscsettings.radiotext",
	"miscsettings.shutdown_count",
	"miscsettings.shutdown_count_hint1",
	"miscsettings.shutdown_count_hint2",
	"miscsettings.shutdown_real",
	"miscsettings.shutdown_real_rcdelay",
	"miscsettings.sleeptimer",
	"miscsettings.zapto_pre_time",
	"motorcontrol.calc_positions",
	"motorcontrol.disable_limit",
	"motorcontrol.drive_mode",
	"motorcontrol.drive_mode_auto",
	"motorcontrol.east_limit",
	"motorcontrol.enable_limit",
	"motorcontrol.goto",
	"motorcontrol.halt",
	"motorcontrol.head",
	"motorcontrol.install_menu",
	"motorcontrol.motor_pos",
	"motorcontrol.movement",
	"motorcontrol.msec",
	"motorcontrol.no_mode",
	"motorcontrol.notdef",
	"motorcontrol.override",
	"motorcontrol.pos_decrease",
	"motorcontrol.pos_increase",
	"motorcontrol.ref_position",
	"motorcontrol.sat_pos",
	"motorcontrol.settings",
	"motorcontrol.status",
	"motorcontrol.step_decrease",
	"motorcontrol.step_drive",
	"motorcontrol.step_east",
	"motorcontrol.step_increase",
	"motorcontrol.step_mode",
	"motorcontrol.step_size",
	"motorcontrol.step_west",
	"motorcontrol.stop_moving",
	"motorcontrol.stop_stopped",
	"motorcontrol.store",
	"motorcontrol.timed_mode",
	"motorcontrol.user_menu",
	"motorcontrol.west_limit",
	"moviebrowser.book_clear_all",
	"moviebrowser.book_head",
	"moviebrowser.book_lastmoviestop",
	"moviebrowser.book_movieend",
	"moviebrowser.book_moviestart",
	"moviebrowser.book_name",
	"moviebrowser.book_new",
	"moviebrowser.book_position",
	"moviebrowser.book_type",
	"moviebrowser.book_type_backward",
	"moviebrowser.book_type_forward",
	"moviebrowser.browser_frame_high",
	"moviebrowser.browser_row_head",
	"moviebrowser.browser_row_item",
	"moviebrowser.browser_row_nr",
	"moviebrowser.browser_row_width",
	"moviebrowser.dir",
	"moviebrowser.dir_head",
	"moviebrowser.edit_book",
	"moviebrowser.edit_book_name_info1",
	"moviebrowser.edit_book_name_info2",
	"moviebrowser.edit_book_pos_info1",
	"moviebrowser.edit_book_pos_info2",
	"moviebrowser.edit_book_type_info1",
	"moviebrowser.edit_book_type_info2",
	"moviebrowser.edit_serie",
	"moviebrowser.error_no_movies",
	"moviebrowser.foot_filter",
	"moviebrowser.foot_play",
	"moviebrowser.foot_sort",
	"moviebrowser.head",
	"moviebrowser.head_filter",
	"moviebrowser.head_playlist",
	"moviebrowser.head_recordlist",
	"moviebrowser.hide_series",
	"moviebrowser.hint_jumpbackward",
	"moviebrowser.hint_jumpforward",
	"moviebrowser.hint_movieend",
	"moviebrowser.hint_newbook_backward",
	"moviebrowser.hint_newbook_forward",
	"moviebrowser.info_audio",
	"moviebrowser.info_channel",
	"moviebrowser.info_filename",
	"moviebrowser.info_genre_major",
	"moviebrowser.info_genre_minor",
	"moviebrowser.info_head",
	"moviebrowser.info_head_update",
	"moviebrowser.info_info1",
	"moviebrowser.info_length",
	"moviebrowser.info_parental_lockage",
	"moviebrowser.info_parental_lockage_0year",
	"moviebrowser.info_parental_lockage_12year",
	"moviebrowser.info_parental_lockage_16year",
	"moviebrowser.info_parental_lockage_18year",
	"moviebrowser.info_parental_lockage_6year",
	"moviebrowser.info_parental_lockage_always",
	"moviebrowser.info_path",
	"moviebrowser.info_prevplaydate",
	"moviebrowser.info_prodcountry",
	"moviebrowser.info_prodyear",
	"moviebrowser.info_quality",
	"moviebrowser.info_recorddate",
	"moviebrowser.info_serie",
	"moviebrowser.info_size",
	"moviebrowser.info_title",
	"moviebrowser.info_videoformat",
	"moviebrowser.last_play_max_items",
	"moviebrowser.last_record_max_items",
	"moviebrowser.load_default",
	"moviebrowser.menu_directories_head",
	"moviebrowser.menu_help_head",
	"moviebrowser.menu_main_bookmarks",
	"moviebrowser.menu_main_head",
	"moviebrowser.menu_nfs_head",
	"moviebrowser.menu_parental_lock_activated",
	"moviebrowser.menu_parental_lock_activated_no",
	"moviebrowser.menu_parental_lock_activated_no_temp",
	"moviebrowser.menu_parental_lock_activated_yes",
	"moviebrowser.menu_parental_lock_head",
	"moviebrowser.menu_parental_lock_rate_head",
	"moviebrowser.menu_save",
	"moviebrowser.menu_save_all",
	"moviebrowser.option_browser",
	"moviebrowser.reload_at_start",
	"moviebrowser.remount_at_start",
	"moviebrowser.scan_for_movies",
	"moviebrowser.serie_auto_create",
	"moviebrowser.serie_head",
	"moviebrowser.serie_name",
	"moviebrowser.short_audio",
	"moviebrowser.short_book",
	"moviebrowser.short_channel",
	"moviebrowser.short_country",
	"moviebrowser.short_filename",
	"moviebrowser.short_format",
	"moviebrowser.short_genre_major",
	"moviebrowser.short_genre_minor",
	"moviebrowser.short_info1",
	"moviebrowser.short_info2",
	"moviebrowser.short_length",
	"moviebrowser.short_parental_lockage",
	"moviebrowser.short_path",
	"moviebrowser.short_prevplaydate",
	"moviebrowser.short_prodyear",
	"moviebrowser.short_quality",
	"moviebrowser.short_recorddate",
	"moviebrowser.short_serie",
	"moviebrowser.short_size",
	"moviebrowser.short_title",
	"moviebrowser.start_head",
	"moviebrowser.start_record_start",
	"moviebrowser.update_if_dest_empty_only",
	"moviebrowser.use_dir",
	"moviebrowser.use_movie_dir",
	"moviebrowser.use_rec_dir",
	"movieplayer.bookmark",
	"movieplayer.bookmarkname",
	"movieplayer.bookmarkname_hint1",
	"movieplayer.bookmarkname_hint2",
	"movieplayer.defplugin",
	"movieplayer.fileplayback",
	"movieplayer.head",
	"movieplayer.toomanybookmarks",
	"movieplayer.tshelp1",
	"movieplayer.tshelp10",
	"movieplayer.tshelp11",
	"movieplayer.tshelp12",
	"movieplayer.tshelp2",
	"movieplayer.tshelp3",
	"movieplayer.tshelp4",
	"movieplayer.tshelp5",
	"movieplayer.tshelp6",
	"movieplayer.tshelp7",
	"movieplayer.tshelp8",
	"movieplayer.tshelp9",
	"movieplayer.tsplayback",
	"mpkey.audio",
	"mpkey.bookmark",
	"mpkey.forward",
	"mpkey.pause",
	"mpkey.play",
	"mpkey.plugin",
	"mpkey.rewind",
	"mpkey.stop",
	"mpkey.time",
	"networkmenu.apply_settings",
	"networkmenu.apply_settings_now",
	"networkmenu.broadcast",
	"networkmenu.dhcp",
	"networkmenu.error_no_address",
	"networkmenu.gateway",
	"networkmenu.hostname",
	"networkmenu.inactive_network",
	"networkmenu.ipaddress",
	"networkmenu.mount",
	"networkmenu.nameserver",
	"networkmenu.netmask",
	"networkmenu.ntpenable",
	"networkmenu.ntprefresh",
	"networkmenu.ntprefresh_hint1",
	"networkmenu.ntprefresh_hint2",
	"networkmenu.ntpserver",
	"networkmenu.ntpserver_hint1",
	"networkmenu.ntpserver_hint2",
	"networkmenu.ntptitle",
	"networkmenu.password",
	"networkmenu.reset_settings_now",
	"networkmenu.select_if",
	"networkmenu.setupnow",
	"networkmenu.setuponstartup",
	"networkmenu.show",
	"networkmenu.ssid",
	"networkmenu.test",
	"neutrino_starting",
	"nfs.alreadymounted",
	"nfs.automount",
	"nfs.dir",
	"nfs.ip",
	"nfs.localdir",
	"nfs.mount",
	"nfs.mount_options",
	"nfs.mounterror",
	"nfs.mounterror_notsup",
	"nfs.mountnow",
	"nfs.mountok",
	"nfs.mounttimeout",
	"nfs.password",
	"nfs.remount",
	"nfs.type",
	"nfs.type_cifs",
	"nfs.type_lufs",
	"nfs.type_nfs",
	"nfs.umount",
	"nfs.umounterror",
	"nfs.username",
	"nfsmenu.head",
	"nvod.percentage",
	"nvod.starting",
	"nvodselector.directormode",
	"nvodselector.head",
	"nvodselector.subservice",
	"options.default",
	"options.fb",
	"options.ntp_off",
	"options.ntp_on",
	"options.null",
	"options.off",
	"options.on",
	"options.on.without_messages",
	"options.serial",
	"parentallock.changepin",
	"parentallock.changepin_hint1",
	"parentallock.changetolocked",
	"parentallock.head",
	"parentallock.lockage",
	"parentallock.lockage12",
	"parentallock.lockage16",
	"parentallock.lockage18",
	"parentallock.lockedchannel",
	"parentallock.lockedprogram",
	"parentallock.never",
	"parentallock.onsignal",
	"parentallock.parentallock",
	"parentallock.prompt",
	"personalize.access",
	"personalize.apply_settings",
	"personalize.button_auto",
	"personalize.button_blue",
	"personalize.button_green",
	"personalize.button_red",
	"personalize.button_yellow",
	"personalize.disabled",
	"personalize.enabled",
	"personalize.head",
	"personalize.help",
	"personalize.help_line1",
	"personalize.help_line2",
	"personalize.help_line3",
	"personalize.help_line4",
	"personalize.help_line5",
	"personalize.help_line6",
	"personalize.help_line7",
	"personalize.help_line8",
	"personalize.menuconfiguration",
	"personalize.menudisabledhint",
	"personalize.notprotected",
	"personalize.notvisible",
	"personalize.pin",
	"personalize.pin_in_use",
	"personalize.pincode",
	"personalize.pinhint",
	"personalize.pinprotect",
	"personalize.pinstatus",
	"personalize.plugins",
	"personalize.usermenu_preferred_buttons",
	"personalize.usermenu_show_cancel",
	"personalize.visible",
	"pictureviewer.defdir",
	"pictureviewer.head",
	"pictureviewer.help1",
	"pictureviewer.help10",
	"pictureviewer.help11",
	"pictureviewer.help12",
	"pictureviewer.help13",
	"pictureviewer.help14",
	"pictureviewer.help15",
	"pictureviewer.help16",
	"pictureviewer.help17",
	"pictureviewer.help18",
	"pictureviewer.help19",
	"pictureviewer.help2",
	"pictureviewer.help20",
	"pictureviewer.help21",
	"pictureviewer.help22",
	"pictureviewer.help3",
	"pictureviewer.help4",
	"pictureviewer.help5",
	"pictureviewer.help6",
	"pictureviewer.help7",
	"pictureviewer.help8",
	"pictureviewer.help9",
	"pictureviewer.resize.color_average",
	"pictureviewer.resize.none",
	"pictureviewer.resize.simple",
	"pictureviewer.scaling",
	"pictureviewer.show",
	"pictureviewer.slide_time",
	"pictureviewer.slideshow",
	"pictureviewer.sortorder.date",
	"pictureviewer.sortorder.filename",
	"ping.ok",
	"ping.protocol",
	"ping.socket",
	"ping.unreachable",
	"pinprotection.head",
	"pinprotection.wrongcode",
	"plugins.hdd_dir",
	"plugins.result",
	"progressbar.color",
	"rclock.lockmsg",
	"rclock.menueadd",
	"rclock.title",
	"rclock.unlockmsg",
	"recording.is_running",
	"recording.start",
	"recording.stop",
	"recordingmenu.apids",
	"recordingmenu.apids_ac3",
	"recordingmenu.apids_alt",
	"recordingmenu.apids_std",
	"recordingmenu.defdir",
	"recordingmenu.end_of_recording_epg",
	"recordingmenu.end_of_recording_max",
	"recordingmenu.end_of_recording_name",
	"recordingmenu.file",
	"recordingmenu.help",
	"recordingmenu.multimenu.ask_stop_all",
	"recordingmenu.multimenu.info_stop_all",
	"recordingmenu.multimenu.rec_akt",
	"recordingmenu.multimenu.stop_all",
	"recordingmenu.multimenu.timeshift",
	"recordingmenu.off",
	"recordingmenu.record_is_not_running",
	"recordingmenu.record_is_running",
	"recordingmenu.save_in_channeldir",
	"recordingmenu.server",
	"recordingmenu.server_mac",
	"recordingmenu.setupnow",
	"recordingmenu.timeshift",
	"recordingmenu.tsdir",
	"recordingmenu.vcr",
	"recordingmenu.zap_on_announce",
	"recordtimer.announce",
	"repeatblocker.hint_1",
	"repeatblocker.hint_2",
	"reset_all",
	"reset_channels",
	"reset_confirm",
	"reset_settings",
	"satsetup.auto_scan",
	"satsetup.auto_scan_all",
	"satsetup.cable_nid",
	"satsetup.comm_input",
	"satsetup.diseqc",
	"satsetup.diseqc10",
	"satsetup.diseqc11",
	"satsetup.diseqc12",
	"satsetup.diseqc_advanced",
	"satsetup.diseqc_com_uncom",
	"satsetup.diseqc_input",
	"satsetup.diseqc_uncom_com",
	"satsetup.diseqcrepeat",
	"satsetup.extended",
	"satsetup.extended_motor",
	"satsetup.fastscan_all",
	"satsetup.fastscan_hd",
	"satsetup.fastscan_head",
	"satsetup.fastscan_prov",
	"satsetup.fastscan_prov_cd",
	"satsetup.fastscan_prov_telesat",
	"satsetup.fastscan_prov_tvv",
	"satsetup.fastscan_sd",
	"satsetup.fastscan_type",
	"satsetup.fe_mode",
	"satsetup.fe_mode_alone",
	"satsetup.fe_mode_loop",
	"satsetup.fe_mode_single",
	"satsetup.fe_mode_twin",
	"satsetup.fe_setup",
	"satsetup.lofh",
	"satsetup.lofl",
	"satsetup.lofs",
	"satsetup.logical_numbers",
	"satsetup.manual_scan",
	"satsetup.minidiseqc",
	"satsetup.motor_pos",
	"satsetup.nodiseqc",
	"satsetup.reset_numbers",
	"satsetup.sat_setup",
	"satsetup.satellite",
	"satsetup.select_sat",
	"satsetup.smatvremote",
	"satsetup.uncomm_input",
	"satsetup.usals_repeat",
	"satsetup.use_bat",
	"satsetup.use_fta_flag",
	"satsetup.use_nit",
	"satsetup.use_usals",
	"sc.empty",
	"sc.init_failed",
	"sc.init_ok",
	"sc.inserted",
	"sc.removed",
	"sc.reset",
	"sc.timeout",
	"sc.waiting",
	"scants.abort_body",
	"scants.abort_header",
	"scants.actcable",
	"scants.actsatellite",
	"scants.bouquet",
	"scants.bouquet_create",
	"scants.bouquet_erase",
	"scants.bouquet_leave",
	"scants.bouquet_satellite",
	"scants.bouquet_update",
	"scants.channel",
	"scants.failed",
	"scants.finished",
	"scants.freqdata",
	"scants.head",
	"scants.numberofdataservices",
	"scants.numberofradioservices",
	"scants.numberoftotalservices",
	"scants.numberoftvservices",
	"scants.preverences_receiving_system",
	"scants.preverences_scan",
	"scants.provider",
	"scants.select_tp",
	"scants.startnow",
	"scants.test",
	"scants.transponders",
	"scrambled_channel",
	"screensetup.lowerright",
	"screensetup.upperleft",
	"screenshot.count",
	"screenshot.cover",
	"screenshot.defdir",
	"screenshot.format",
	"screenshot.info",
	"screenshot.menu",
	"screenshot.osd",
	"screenshot.res",
	"screenshot.scale",
	"screenshot.tv",
	"screenshot.video",
	"servicemenu.getplugins",
	"servicemenu.getplugins_hint",
	"servicemenu.head",
	"servicemenu.imageinfo",
	"servicemenu.reload",
	"servicemenu.reload_hint",
	"servicemenu.restart",
	"servicemenu.restart_hint",
	"servicemenu.restart_refused_recording",
	"servicemenu.scants",
	"servicemenu.update",
	"settings.backup",
	"settings.backup_failed",
	"settings.help",
	"settings.menu_pos",
	"settings.missingoptionsconffile",
	"settings.noconffile",
	"settings.pos_bottom_left",
	"settings.pos_bottom_right",
	"settings.pos_default_center",
	"settings.pos_higher_center",
	"settings.pos_top_left",
	"settings.pos_top_right",
	"settings.restore",
	"settings.restore_warn",
	"shutdown.recoding_query",
	"shutdowntimer.announce",
	"sleeptimerbox.announce",
	"sleeptimerbox.hint1",
	"sleeptimerbox.hint2",
	"sleeptimerbox.hint3",
	"sleeptimerbox.title",
	"sleeptimerbox.title2",
	"streaminfo.aratio",
	"streaminfo.aratio_unknown",
	"streaminfo.audiotype",
	"streaminfo.average_bitrate",
	"streaminfo.bitrate",
	"streaminfo.framerate",
	"streaminfo.framerate_unknown",
	"streaminfo.head",
	"streaminfo.not_available",
	"streaminfo.resolution",
	"streaminfo.signal",
	"streaming.busy",
	"streaming.dir_not_writable",
	"streaming.write_error",
	"stringinput.caps",
	"stringinput.clear",
	"subtitles.head",
	"subtitles.stop",
	"timer.eventrecord.msg",
	"timer.eventrecord.title",
	"timer.eventtimed.msg",
	"timer.eventtimed.title",
	"timerbar.channelswitch",
	"timerbar.recordevent",
	"timerlist.alarmtime",
	"timerlist.apids",
	"timerlist.apids_dflt",
	"timerlist.bouquetselect",
	"timerlist.channel",
	"timerlist.channelselect",
	"timerlist.delete",
	"timerlist.menumodify",
	"timerlist.menunew",
	"timerlist.message",
	"timerlist.moderadio",
	"timerlist.modeselect",
	"timerlist.modetv",
	"timerlist.modify",
	"timerlist.name",
	"timerlist.new",
	"timerlist.overlapping_timer",
	"timerlist.plugin",
	"timerlist.program.unknown",
	"timerlist.recording_dir",
	"timerlist.reload",
	"timerlist.repeat",
	"timerlist.repeat.biweekly",
	"timerlist.repeat.byeventdescription",
	"timerlist.repeat.daily",
	"timerlist.repeat.fourweekly",
	"timerlist.repeat.friday",
	"timerlist.repeat.monday",
	"timerlist.repeat.monthly",
	"timerlist.repeat.once",
	"timerlist.repeat.saturday",
	"timerlist.repeat.sunday",
	"timerlist.repeat.thursday",
	"timerlist.repeat.tuesday",
	"timerlist.repeat.unknown",
	"timerlist.repeat.wednesday",
	"timerlist.repeat.weekdays",
	"timerlist.repeat.weekly",
	"timerlist.repeatcount",
	"timerlist.repeatcount.help1",
	"timerlist.repeatcount.help2",
	"timerlist.save",
	"timerlist.standby",
	"timerlist.standby.off",
	"timerlist.standby.on",
	"timerlist.stoptime",
	"timerlist.type",
	"timerlist.type.execplugin",
	"timerlist.type.nextprogram",
	"timerlist.type.record",
	"timerlist.type.remind",
	"timerlist.type.shutdown",
	"timerlist.type.sleeptimer",
	"timerlist.type.standby",
	"timerlist.type.unknown",
	"timerlist.type.zapto",
	"timerlist.weekdays",
	"timerlist.weekdays.hint_1",
	"timerlist.weekdays.hint_2",
	"timersettings.record_safety_time_after",
	"timersettings.record_safety_time_after.hint_1",
	"timersettings.record_safety_time_after.hint_2",
	"timersettings.record_safety_time_before",
	"timersettings.record_safety_time_before.hint_1",
	"timersettings.record_safety_time_before.hint_2",
	"timersettings.separator",
	"timing.chanlist",
	"timing.epg",
	"timing.filebrowser",
	"timing.hint_1",
	"timing.hint_2",
	"timing.infobar",
	"timing.infobar_movieplayer",
	"timing.infobar_radio",
	"timing.menu",
	"timing.numericzap",
	"upnpbrowser.head",
	"upnpbrowser.noservers",
	"upnpbrowser.rescan",
	"upnpbrowser.scanning",
	"usermenu.button_blue",
	"usermenu.button_green",
	"usermenu.button_red",
	"usermenu.button_yellow",
	"usermenu.head",
	"usermenu.item_bar",
	"usermenu.item_epg_misc",
	"usermenu.item_none",
	"usermenu.item_vtxt",
	"usermenu.msg_info_is_empty",
	"usermenu.msg_warning_name",
	"usermenu.msg_warning_no_items",
	"usermenu.name",
	"video_mode_ok",
	"videomenu.43mode",
	"videomenu.analog_hd_rgb_cinch",
	"videomenu.analog_hd_rgb_scart",
	"videomenu.analog_hd_yprpb_cinch",
	"videomenu.analog_hd_yprpb_scart",
	"videomenu.analog_mode",
	"videomenu.analog_sd_rgb_cinch",
	"videomenu.analog_sd_rgb_scart",
	"videomenu.analog_sd_yprpb_cinch",
	"videomenu.analog_sd_yprpb_scart",
	"videomenu.auto",
	"videomenu.brightness",
	"videomenu.cinch",
	"videomenu.contrast",
	"videomenu.csync",
	"videomenu.dbdr",
	"videomenu.dbdr_both",
	"videomenu.dbdr_deblock",
	"videomenu.dbdr_none",
	"videomenu.enabled_modes",
	"videomenu.fullscreen",
	"videomenu.hdmi_cec",
	"videomenu.hdmi_cec_mode",
	"videomenu.hdmi_cec_mode_off",
	"videomenu.hdmi_cec_mode_recorder",
	"videomenu.hdmi_cec_mode_tuner",
	"videomenu.hdmi_cec_standby",
	"videomenu.hdmi_cec_view_on",
	"videomenu.hue",
	"videomenu.letterbox",
	"videomenu.panscan",
	"videomenu.panscan2",
	"videomenu.saturation",
	"videomenu.scart",
	"videomenu.screensetup",
	"videomenu.sharpness",
	"videomenu.tv-scart",
	"videomenu.vcrsignal",
	"videomenu.vcrsignal_composite",
	"videomenu.vcrsignal_svideo",
	"videomenu.videoformat",
	"videomenu.videoformat_149",
	"videomenu.videoformat_169",
	"videomenu.videoformat_43",
	"videomenu.videomode",
	"wizard.welcome_head",
	"wizard.welcome_text",
	"word.from",
	"zapit.scantype",
	"zapit.scantype.all",
	"zapit.scantype.radio",
	"zapit.scantype.tv",
	"zapit.scantype.tvradio",
	"zapitsetup.head",
	"zapitsetup.info",
	"zapitsetup.last_radio",
	"zapitsetup.last_tv",
	"zapitsetup.last_use",
	"zaptotimer.announce",
};
#endif
