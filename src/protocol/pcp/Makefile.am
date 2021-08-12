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
  http_bisector.h \
  http_codes.h \
  http_common.h \
  http_common_modules.h \
  http_configuration.h \
  http_defines.h \
  http_exports.h \
  http_icontrol.h \
  http_message.h \
  http_message.inl \
  http_module_bisector.h \
  http_module_bisector.inl \
  http_module_parser.h \
  http_module_parser.inl \
  http_module_streamer.h \
  http_module_streamer.inl \
  http_network.h \
  http_parser.h \
  http_parser_driver.h \
  http_scanner.h \
  http_session.h \
  http_session.inl \
  http_sessionmessage.h \
  http_stream.h \
  http_stream.inl \
  http_stream_common.h \
  http_tools.h

lib_LTLIBRARIES = libACENetwork_Protocol_HTTP.la

libACENetwork_Protocol_HTTP_la_SOURCES = \
  http_bisector.cpp \
  http_bisector.h \
  http_codes.h \
  http_common.h \
  http_common_modules.h \
  http_configuration.h \
  http_defines.h \
  http_exports.h \
  http_icontrol.h \
  http_message.cpp \
  http_message.h \
  http_message.inl \
  http_module_bisector.cpp \
  http_module_bisector.h \
  http_module_bisector.inl \
  http_module_parser.cpp \
  http_module_parser.h \
  http_module_parser.inl \
  http_module_streamer.cpp \
  http_module_streamer.h \
  http_module_streamer.inl \
  http_network.h \
  http_parser.cpp \
  http_parser.h \
  http_parser_driver.cpp \
  http_parser_driver.h \
  http_scanner.cpp \
  http_scanner.h \
  http_session.cpp \
  http_session.h \
  http_session.inl \
  http_sessionmessage.cpp \
  http_sessionmessage.h \
  http_stream.cpp \
  http_stream.h \
  http_stream.inl \
  http_stream_common.h \
  http_tools.cpp \
  http_tools.h
#  stdafx.cpp \
#  stdafx.h
libACENetwork_Protocol_HTTP_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/modules/libCommon/src/ui \
  -I$(top_srcdir)/modules/libACEStream/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/client_server \
  -I$(srcdir)/../test_u \
  -I$(builddir)/../..
libACENetwork_Protocol_HTTP_la_LDFLAGS = \
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
  doc \
  scripts

