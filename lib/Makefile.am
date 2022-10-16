SUBDIRS = \
	connection \
	libeventserver \
	sectionsdclient \
	timerdclient \
	libconfigfile \
	libmd5sum \
	libnet \
	xmltree \
	jsoncpp \
	libtuxtxt \
	libdvbsub \
	luaclient \
	libdvbapi \
	libdvbci

if !BOXTYPE_GENERIC
SUBDIRS += \
	libeplayer3
endif

if ENABLE_UPNP
SUBDIRS += \
	libupnpclient
endif

if ENABLE_WIFI
SUBDIRS += \
	libiw
endif

if ENABLE_LCD
SUBDIRS += \
	liblcddisplay
endif

