#  -I/usr/include/c++/3.4.6/backward
#  -I$(top_srcdir)/modules/ATCD/ACE
#  *NOTE*: -frepo is broken in gcc 4.9.2
#  *TODO*: repair test_i_template_instantiations.cpp so it works with
#          -fno-implicit-templates
AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE \
  $(PKGSDL_CFLAGS) \
  $(PKGGTK_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H \
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\" \
  -DNET_HAS_DLL
if DEBUG
DEFS += \
  -DDEBUG_DEBUGGER
endif

bin_PROGRAMS = \
  stream_source \
  stream_target

noinst_HEADERS = stdafx.h
stream_source_SOURCES = \
  test_i_callbacks.cpp \
  test_i_callbacks.h \
  test_i_common.h \
  test_i_common_modules.h \
  test_i_connection_common.h \
  test_i_connection_manager_common.h \
  test_i_defines.h \
  test_i_message.cpp \
  test_i_message.h \
  test_i_module_eventhandler.cpp \
  test_i_module_eventhandler.h \
  test_i_session_message.cpp \
  test_i_session_message.h \
  test_i_source.cpp \
  test_i_source_common.h \
  test_i_source_eventhandler.cpp \
  test_i_source_eventhandler.h \
  test_i_source_signalhandler.cpp \
  test_i_source_signalhandler.h \
  test_i_source_stream.cpp \
  test_i_source_stream.h \
  test_i_source_stream.inl \
  test_i_target_stream.cpp \
  test_i_target_stream.h
#  test_i_template_instantiations.cpp
#  stdafx.cpp \
#  stdafx.h

stream_source_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/Common/src \
  -I$(top_srcdir)/modules/Common/src/ui \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/modules/file \
  -I$(top_srcdir)/src/modules/misc \
  -I$(srcdir)/modules/ACENetwork/src \
  -I$(srcdir)/modules/ACENetwork/src/client_server \
  -I$(top_srcdir)/src/modules/net
stream_source_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../../modules/Common/src/.libs \
  -L$(abs_builddir)/../../modules/Common/src/ui/.libs \
  -L$(abs_builddir)/../../src/.libs \
  -L$(abs_builddir)/../../src/modules/file/.libs \
  -L$(abs_builddir)/../../src/modules/misc/.libs \
  -L$(abs_builddir)/modules/ACENetwork/src/.libs \
  -L$(abs_builddir)/modules/ACENetwork/src/client_server/.libs \
  -L$(abs_builddir)/../../src/modules/net/.libs
stream_source_LDADD = \
  -lACE \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACEStream_File \
  -lACEStream_Misc \
  -lACEStream_Net \
  -lACENetwork \
  -lACENetwork_Client

stream_target_SOURCES = \
  test_i_callbacks.cpp \
  test_i_callbacks.h \
  test_i_common.h \
  test_i_common_modules.h \
  test_i_connection_common.h \
  test_i_connection_manager_common.h \
  test_i_defines.h \
  test_i_message.cpp \
  test_i_message.h \
  test_i_module_eventhandler.cpp \
  test_i_module_eventhandler.h \
  test_i_session_message.cpp \
  test_i_session_message.h \
  test_i_source_stream.cpp \
  test_i_source_stream.h \
  test_i_source_stream.inl \
  test_i_target.cpp \
  test_i_target_common.h \
  test_i_target_eventhandler.cpp \
  test_i_target_eventhandler.h \
  test_i_target_listener_common.h \
  test_i_target_signalhandler.cpp \
  test_i_target_signalhandler.h \
  test_i_target_stream.cpp \
  test_i_target_stream.h
#  test_i_template_instantiations.cpp
#  stdafx.cpp \
#  stdafx.h

#  -I$(top_srcdir)/modules/Common/src \
#  -I$(top_srcdir)/modules/Common/src/ui
stream_target_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/Common/src \
  -I$(top_srcdir)/modules/Common/src/ui \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/modules/file \
  -I$(top_srcdir)/src/modules/misc \
  -I$(srcdir)/modules/ACENetwork/src \
  -I$(srcdir)/modules/ACENetwork/src/client_server \
  -I$(top_srcdir)/src/modules/net
stream_target_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../../modules/Common/src/.libs \
  -L$(abs_builddir)/../../modules/Common/src/ui/.libs \
  -L$(abs_builddir)/../../src/.libs \
  -L$(abs_builddir)/../../src/modules/file/.libs \
  -L$(abs_builddir)/../../src/modules/misc/.libs \
  -L$(abs_builddir)/modules/ACENetwork/src/.libs \
  -L$(abs_builddir)/modules/ACENetwork/src/client_server/.libs \
  -L$(abs_builddir)/../../src/modules/net/.libs
stream_target_LDADD = \
  -lACE \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACEStream_File \
  -lACEStream_Misc \
  -lACEStream_Net \
  -lACENetwork \
  -lACENetwork_Client

SUBDIRS = \
  modules \
  . \
  etc \
  scripts

DIST_SUBDIRS = \
  . \
  etc \
  scripts
if BUILD_MODULES
  DIST_SUBDIRS += modules
endif

EXTRA_DIST = \
  ReadMe.txt \
  targetver.h
