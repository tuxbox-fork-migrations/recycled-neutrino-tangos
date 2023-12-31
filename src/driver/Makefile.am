AM_CXXFLAGS = -fno-rtti -fno-exceptions -D__STDC_FORMAT_MACROS

SUBDIRS = pictureviewer

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/jsoncpp \
	@SIGC_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@AVFORMAT_CFLAGS@ \
	@LUA_CFLAGS@ \
	@HWLIB_CFLAGS@

noinst_LIBRARIES = libneutrino_driver.a libneutrino_driver_netfile.a

libneutrino_driver_a_SOURCES = \
	abstime.c \
	colorgradient.cpp \
	fade.cpp \
	fb_window.cpp \
	file.cpp \
	fontrenderer.cpp \
	genpsi.cpp \
	moviecut.cpp \
	movieinfo.cpp \
	neutrinofonts.cpp \
	radiotext.cpp \
	radiotools.cpp \
	rcinput.cpp \
	record.cpp \
	scanepg.cpp \
	screen_max.cpp \
	screenshot.cpp \
	shutdown_count.cpp \
	streamts.cpp \
	volume.cpp

if ENABLE_LCD
AM_CPPFLAGS += -I$(top_srcdir)/lib/liblcddisplay
endif

if ENABLE_LCD4LINUX
libneutrino_driver_a_SOURCES += \
	lcd4l.cpp
endif

if ENABLE_GRAPHLCD
SUBDIRS += glcd
endif

if BOXTYPE_CST
libneutrino_driver_a_SOURCES += \
	fb_generic.cpp \
	fb_accel.cpp \
	fb_accel_cs_hdx.cpp
if BOXMODEL_CST_HD2
libneutrino_driver_a_SOURCES += \
	fb_accel_cs_hd2.cpp \
	vfd.cpp
else
libneutrino_driver_a_SOURCES += \
	fb_accel_cs_hd1.cpp \
	vfd.cpp
endif
endif

if BOXTYPE_GENERIC
libneutrino_driver_a_SOURCES += \
	fb_generic.cpp \
	fb_accel.cpp \
	fb_accel_glfb.cpp \
	simple_display.cpp
endif

if BOXTYPE_ARMBOX
if ENABLE_LCD
libneutrino_driver_a_SOURCES += \
	hdmi_cec.cpp \
	fb_generic.cpp \
	fb_accel.cpp \
	fb_accel_arm.cpp \
	lcdd.cpp
else
libneutrino_driver_a_SOURCES += \
	hdmi_cec.cpp \
	fb_generic.cpp \
	fb_accel.cpp \
	fb_accel_arm.cpp \
	simple_display.cpp
endif
endif

if BOXTYPE_MIPSBOX
libneutrino_driver_a_SOURCES += \
	hdmi_cec.cpp \
	fb_generic.cpp \
	fb_accel.cpp \
	fb_accel_mips.cpp \
	simple_display.cpp
endif

libneutrino_driver_netfile_a_SOURCES = netfile.cpp
