#  -I/usr/include/c++/3.4.6/backward
#  -I$(top_srcdir)/modules/ATCD/ACE
#  *NOTE*: -frepo is broken in gcc 4.9.2
#  *TODO*: repair test_i_template_instantiations.cpp so it works with
#          -fno-implicit-templates
AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE \
  $(PKGXML2_CFLAGS)
if VALGRIND_SUPPORT
  AM_CPPFLAGS += $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H \
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\" \
  -DNET_HAS_DLL
if DEBUG
  DEFS += -DDEBUG_DEBUGGER
endif

bin_PROGRAMS = \
  http_get

noinst_HEADERS = stdafx.h
http_get_SOURCES = \
  test_i_common.h \
  test_i_common_modules.h \
  test_i_connection_common.h \
  test_i_connection_manager_common.h \
  test_i_defines.h \
  test_i_message.cpp \
  test_i_message.h \
  test_i_module_databasewriter.cpp \
  test_i_module_databasewriter.h \
  test_i_module_htmlparser.cpp \
  test_i_module_htmlparser.h \
  test_i_module_htmlwriter.cpp \
  test_i_module_htmlwriter.h \
  test_i_session_message.cpp \
  test_i_session_message.h \
  test_i_source.cpp \
  test_i_source_common.h \
  test_i_source_signalhandler.cpp \
  test_i_source_signalhandler.h \
  test_i_source_stream.cpp \
  test_i_source_stream.h \
  test_i_source_stream.inl
#  test_i_template_instantiations.cpp
#  stdafx.cpp \
#  stdafx.h
http_get_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/modules/file \
  -I$(top_srcdir)/src/modules/misc \
  -I$(top_srcdir)/src/modules/net/modules/libACENetwork/src \
  -I$(top_srcdir)/src/modules/net/modules/libACENetwork/src/client_server \
  -I$(top_srcdir)/src/modules/net \
  -I$(top_srcdir)/src/modules/net/protocols
http_get_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../../modules/libCommon/src/.libs \
  -L$(abs_builddir)/../../src/.libs \
  -L$(abs_builddir)/../../src/modules/file/.libs \
  -L$(abs_builddir)/../../src/modules/misc/.libs \
  -L$(abs_builddir)/modules/libACENetwork/src/.libs \
  -L$(abs_builddir)/modules/libACENetwork/src/client_server/.libs \
  -L$(abs_builddir)/../../src/modules/net/.libs \
  -L$(abs_builddir)/../../src/modules/net/protocols/.libs
http_get_LDADD = \
  -lACE \
  $(PKGXML2_LIBS) \
  -lCommon \
  -lACEStream \
  -lACEStream_File \
  -lACEStream_Misc \
  -lACEStream_Net \
  -lACEStream_Net_Protocol_HTTP_Get \
  -lACENetwork \
  -lACENetwork_Client

SUBDIRS = \
  modules \
  .

DIST_SUBDIRS = \
  .
if BUILD_MODULES
  DIST_SUBDIRS += modules
endif

EXTRA_DIST = \
  ReadMe.txt \
  targetver.h
