noinst_LIBRARIES = libdvbapi.a

AM_CPPFLAGS = -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS
AM_CPPFLAGS += \
    -I$(top_builddir) \
    -I$(srcdir) \
    -I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/libeplayer3/include \
	-I$(top_srcdir)/lib/libdvbci

AM_CXXFLAGS = -fno-rtti -fno-exceptions -fno-strict-aliasing

AM_LDFLAGS = \
	-lOpenThreads \
	@AVFORMAT_LIBS@ \
	@AVUTIL_LIBS@ \
	@AVCODEC_LIBS@ \
	@SWRESAMPLE_LIBS@ \
	-lpthread -lass -lrt

libdvbapi_a_SOURCES = \
	hardware_caps.c \
	proc_tools.c \
	debug.cpp \
	dmx.cpp \
	video.cpp \
	audio.cpp \
	record.cpp \
	hdmi_cec.cpp \
	playback_libeplayer3.cpp
