ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
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
  IRC_client \
  IRC_client_gui

noinst_HEADERS = stdafx.h

IRC_client_SOURCES = \
  IRC_client.cpp \
  IRC_client_common.h \
  IRC_client_common_modules.h \
  IRC_client_configuration.h \
  IRC_client_defines.h \
  IRC_client_iIRCControl.h \
  IRC_client_IRC_codes.h \
  IRC_client_IRCbisect.cpp \
  IRC_client_IRCbisect.h \
  IRC_client_IRCmessage.cpp \
  IRC_client_IRCmessage.h \
  IRC_client_IRCparser.cpp \
  IRC_client_IRCparser.h \
  IRC_client_IRCparser_driver.cpp \
  IRC_client_IRCparser_driver.h \
  IRC_client_IRCscanner.cpp \
  IRC_client_IRCscanner.h \
  IRC_client_message.cpp \
  IRC_client_message.h \
  IRC_client_messageallocator.cpp \
  IRC_client_messageallocator.h \
  IRC_client_module_IRChandler.cpp \
  IRC_client_module_IRChandler.h \
  IRC_client_module_IRCparser.cpp \
  IRC_client_module_IRCparser.h \
  IRC_client_module_IRCsplitter.cpp \
  IRC_client_module_IRCsplitter.h \
  IRC_client_module_IRCstreamer.cpp \
  IRC_client_module_IRCstreamer.h \
  IRC_client_network.h \
  IRC_client_sessionmessage.cpp \
  IRC_client_sessionmessage.h \
  IRC_client_signalhandler.cpp \
  IRC_client_signalhandler.h \
  IRC_client_stream.cpp \
  IRC_client_stream.h \
  IRC_client_stream_common.h \
  IRC_client_tools.cpp \
  IRC_client_tools.h \
  IRC_common.h
IRC_client_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(top_srcdir)/3rd_party/bison \
  -I$(builddir)/../..
IRC_client_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGSDL_LDFLAGS) \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../src/.libs
IRC_client_LDADD = \
  -lACE \
  $(PKGSDL_LIBS) \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork

IRC_client_gui_SOURCES = \
  IRC_client_defines.h \
  IRC_client_common.h \
  IRC_client_common_modules.h \
  IRC_client_configuration.h \
  IRC_client_defines.h \
  IRC_client_gui.cpp \
  IRC_client_gui_callbacks.cpp \
  IRC_client_gui_callbacks.h \
  IRC_client_gui_common.h \
  IRC_client_gui_connection.cpp \
  IRC_client_gui_connection.h \
  IRC_client_gui_defines.h \
  IRC_client_gui_messagehandler.cpp \
  IRC_client_gui_messagehandler.h \
  IRC_client_iIRCControl.h \
  IRC_client_IRC_codes.h \
  IRC_client_IRCbisect.cpp \
  IRC_client_IRCbisect.h \
  IRC_client_IRCmessage.cpp \
  IRC_client_IRCmessage.h \
  IRC_client_IRCparser.cpp \
  IRC_client_IRCparser.h \
  IRC_client_IRCparser_driver.cpp \
  IRC_client_IRCparser_driver.h \
  IRC_client_IRCscanner.cpp \
  IRC_client_IRCscanner.h \
  IRC_client_message.cpp \
  IRC_client_message.h \
  IRC_client_messageallocator.cpp \
  IRC_client_messageallocator.h \
  IRC_client_module_IRChandler.cpp \
  IRC_client_module_IRChandler.h \
  IRC_client_module_IRCparser.cpp \
  IRC_client_module_IRCparser.h \
  IRC_client_module_IRCsplitter.cpp \
  IRC_client_module_IRCsplitter.h \
  IRC_client_module_IRCstreamer.cpp \
  IRC_client_module_IRCstreamer.h \
  IRC_client_network.h \
  IRC_client_sessionmessage.cpp \
  IRC_client_sessionmessage.h \
  IRC_client_signalhandler.cpp \
  IRC_client_signalhandler.h \
  IRC_client_stream.cpp \
  IRC_client_stream.h \
  IRC_client_stream_common.h \
  IRC_client_tools.cpp \
  IRC_client_tools.h \
  IRC_common.h
IRC_client_gui_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(top_srcdir)/3rd_party/bison \
  -I$(builddir)/../..
IRC_client_gui_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGSDL_LDFLAGS) \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../src/.libs
IRC_client_gui_LDADD = \
  -lACE \
  $(PKGSDL_LIBS) \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACENetwork

configdir = $(datadir)/@PACKAGE_NAME@/config
config_DATA = \
  IRC_client_channel.glade \
  IRC_client_connection.glade \
  IRC_client_main.glade \
  IRC_client.ini \
  servers.ini

EXTRA_DIST = \
  $(config_DATA)
