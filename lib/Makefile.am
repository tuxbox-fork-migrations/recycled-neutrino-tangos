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
	luaclient

if ENABLE_UPNP
SUBDIRS += \
	libupnpclient
endif

if ENABLE_WIFI
SUBDIRS += \
	libiw
endif

if BOXTYPE_CST
SUBDIRS += \
	hardware/coolstream
endif
