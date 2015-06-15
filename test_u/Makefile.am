ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
  -I/usr/include/c++/3.4.6/backward \
  -I$(top_srcdir)/../../modules/ATCD/ACE \
  $(PKGSDL_CFLAGS) \
  $(PKGGTK_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H \
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\"
if DEBUG
DEFS += \
  -DDEBUG_DEBUGGER
endif

bin_PROGRAMS = \
  net_client \
  net_server

noinst_HEADERS = stdafx.h

net_client_SOURCES = \
  net_callbacks.cpp \
  net_callbacks.h \
  net_client.cpp \
  net_client_signalhandler.cpp \
  net_client_signalhandler.h \
  net_client_timeouthandler.cpp \
  net_client_timeouthandler.h \
  net_common.h \
  net_defines.h \
  net_eventhandler.cpp \
  net_eventhandler.h \
  net_module_eventhandler.cpp \
  net_module_eventhandler.h
net_client_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(builddir)/../..
net_client_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../src/.libs
net_client_LDADD = \
  -lACE \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork

net_server_SOURCES = \
  net_callbacks.cpp \
  net_callbacks.h \
  net_client_timeouthandler.cpp \
  net_client_timeouthandler.h \
  net_common.h \
  net_defines.h \
  net_server.cpp \
  net_server_signalhandler.cpp \
  net_server_signalhandler.h \
  net_eventhandler.cpp \
  net_eventhandler.h \
  net_module_eventhandler.cpp \
  net_module_eventhandler.h
net_server_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(builddir)/../..
net_server_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../src/.libs \
  -L$(abs_builddir)/../src/client_server/.libs
net_server_LDADD = \
  -lACE \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork \
  -lACENetwork_Server

configdir = $(confdir)
config_DATA = \
  net_client.glade \
  net_server.glade

EXTRA_DIST = \
  $(config_DATA)
