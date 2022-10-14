noinst_LIBRARIES = libdvbci.a

AM_CPPFLAGS = \
    -I$(top_builddir) \
    -I$(srcdir) \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/include

AM_CXXFLAGS = -fno-rtti -fno-exceptions -fno-strict-aliasing
AM_LDFLAGS = -lpthread -lcrypto -lssl

if ENABLE_CI
libdvbci_a_SOURCES = \
	ca_ci.cpp \
	dvbci_session.cpp \
	dvbci_mmi.cpp \
	dvbci_camgr.cpp \
	misc.cpp \
	descrambler.cpp \
	dh_rsa_misc.cpp \
	aes_xcbc_mac.cpp \
	dvbci_ccmgr.cpp \
	dvbci_appmgr.cpp \
	dvbci_datetimemgr.cpp \
	dvbci_resmgr.cpp
else
libdvbci_a_SOURCES = \
	ca.cpp
endif
