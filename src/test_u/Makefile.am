ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
  -I$(top_srcdir)/../../modules/ATCD/ACE \
  -I$(builddir)/../..

DEFS = \
  -DBASEDIR=\"$(datadir)/@META_PACKAGE@\"

bin_PROGRAMS = \
  net_client \
  net_server

noinst_HEADERS = stdafx.h

net_client_SOURCES = \
  net_client.cpp \
  net_client_signalhandler.cpp \
  net_client_signalhandler.h \
  net_client_timeouthandler.cpp \
  net_client_timeouthandler.h \
  net_eventhandler.cpp \
  net_eventhandler.h \
  net_callbacks.cpp \
  net_callbacks.h \
  net_defines.h \
  net_common.h
net_client_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  $(pkg_libs_gtkglade_CFLAGS)
net_client_LDFLAGS = \
  -L$(abs_top_srcdir)/../../ATCD/ACE/build/linux/lib \
  -L$(abs_builddir)/../../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../../modules/libACENetwork/src/.libs \
  $(pkg_libs_gtkglade_LDFLAGS)
net_client_LDADD = \
  -lACE \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork \
  $(pkg_libs_gtkglade_LIBS)

net_server_SOURCES = \
  net_server.cpp \
  net_server_signalhandler.cpp \
  net_server_signalhandler.h \
  net_eventhandler.cpp \
  net_eventhandler.h \
  net_callbacks.cpp \
  net_callbacks.h \
  net_defines.h \
  net_common.h
net_server_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  $(pkg_libs_gtkglade_CFLAGS)
net_server_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  -L$(abs_builddir)/../../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../../modules/libACENetwork/src/.libs \
  -L$(abs_builddir)/../../modules/libACENetwork/src/client_server/.libs \
  $(pkg_libs_gtkglade_LDFLAGS)
net_server_LDADD = \
  -lACE \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork \
  -lACENetwork_Server \
  $(pkg_libs_gtkglade_LIBS)

configdir = $(confdir)
config_DATA = \
  net_client.glade \
  net_server.glade

EXTRA_DIST = $(config_DATA)
