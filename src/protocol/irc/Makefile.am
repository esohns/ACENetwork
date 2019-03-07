#ACLOCAL_AMFLAGS = -I m4

#  -I/usr/include/c++/3.4.6/backward
AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE \
  -I$(top_srcdir)/3rd_party/bison \
  -I$(top_srcdir)/3rd_party/c++ \
  $(PKGCURSES_CFLAGS) \
  $(PKGSDL_CFLAGS) \
  $(PKGGTK_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H \
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\"

noinst_HEADERS = stdafx.h
include_HEADERS = \
  irc_bisector.cpp \
  irc_bisector.h \
  irc_codes.h \
  irc_common.h \
  irc_common_modules.h \
  irc_configuration.h \
  irc_defines.h \
  irc_exports.h \
  irc_icontrol.h \
  irc_message.h \
  irc_module_bisector.h \
  irc_module_bisector.inl \
  irc_module_parser.h \
  irc_module_parser.inl \
  irc_module_streamer.h \
  irc_module_streamer.inl \
  irc_network.h \
  irc_parser.h \
  irc_parser_driver.h \
  irc_record.h \
  irc_scanner.h \
  irc_session.h \
  irc_session.inl \
  irc_sessionmessage.h \
  irc_statemachine_registration.h \
  irc_stream.h \
  irc_stream.inl \
  irc_stream_common.h \
  irc_tools.h

lib_LTLIBRARIES = libACENetwork_Protocol_IRC.la

libACENetwork_Protocol_IRC_la_SOURCES = \
  irc_bisector.cpp \
  irc_bisector.h \
  irc_codes.h \
  irc_common.h \
  irc_common_modules.h \
  irc_configuration.h \
  irc_defines.h \
  irc_exports.h \
  irc_icontrol.h \
  irc_message.cpp \
  irc_message.h \
  irc_message.inl \
  irc_module_bisector.cpp \
  irc_module_bisector.h \
  irc_module_bisector.inl \
  irc_module_parser.cpp \
  irc_module_parser.h \
  irc_module_parser.inl \
  irc_module_streamer.cpp \
  irc_module_streamer.h \
  irc_module_streamer.inl \
  irc_network.h \
  irc_parser.cpp \
  irc_parser.h \
  irc_parser_driver.cpp \
  irc_parser_driver.h \
  irc_scanner.cpp \
  irc_scanner.h \
  irc_session.cpp \
  irc_session.h \
  irc_session.inl \
  irc_sessionmessage.cpp \
  irc_sessionmessage.h \
  irc_stream.cpp \
  irc_stream.h \
  irc_stream.inl \
  irc_stream_common.h \
  irc_tools.cpp \
  irc_tools.h
#  stdafx.cpp \
#  stdafx.h
libACENetwork_Protocol_IRC_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(srcdir)/../test_u \
  -I$(builddir)/../..
libACENetwork_Protocol_IRC_la_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGCURSES_LDFLAGS) \
  $(PKGSDL_LDFLAGS) \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../modules/libCommon/src/ui/.libs \
  -L$(abs_builddir)/../modules/libACEStream/src/.libs \
  -L$(abs_builddir)/../src/.libs

SUBDIRS = \
  . \
  scripts

