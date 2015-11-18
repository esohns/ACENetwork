#ACLOCAL_AMFLAGS = -I m4

#  -I/usr/include/c++/3.4.6/backward
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
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\"
if DEBUG
DEFS += \
  -DDEBUG_DEBUGGER
endif

check_PROGRAMS = \
  net_client \
  net_server

noinst_HEADERS = stdafx.h

net_client_SOURCES = \
  net_client.cpp \
  net_client_common.h \
  net_client_connector_common.h \
  net_client_signalhandler.cpp \
  net_client_signalhandler.h \
  net_client_timeouthandler.cpp \
  net_client_timeouthandler.h \
  test_u_callbacks.cpp \
  test_u_callbacks.h \
  test_u_common.h \
  test_u_common_modules.h \
  test_u_configuration.h \
  test_u_connection_common.h \
  test_u_connection_manager_common.h \
  test_u_defines.h \
  test_u_eventhandler.cpp \
  test_u_eventhandler.h \
  test_u_message.cpp \
  test_u_message.h \
  test_u_module_eventhandler.cpp \
  test_u_module_eventhandler.h \
  test_u_module_headerparser.cpp \
  test_u_module_headerparser.h \
  test_u_module_protocolhandler.cpp \
  test_u_module_protocolhandler.h \
  test_u_sessionmessage.cpp \
  test_u_sessionmessage.h \
  test_u_stream.cpp \
  test_u_stream.h \
  test_u_socket_common.h \
  test_u_stream_common.h \
  test_u_tcpconnection.cpp \
  test_u_tcpconnection.h
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
  net_client_timeouthandler.cpp \
  net_client_timeouthandler.h \
  net_server.cpp \
  net_server_common.h \
  net_server_listener_common.h \
  net_server_signalhandler.cpp \
  net_server_signalhandler.h \
  test_u_callbacks.cpp \
  test_u_callbacks.h \
  test_u_common.h \
  test_u_common_modules.h \
  test_u_configuration.h \
  test_u_connection_common.h \
  test_u_connection_manager_common.h \
  test_u_defines.h \
  test_u_message.cpp \
  test_u_message.h \
  test_u_eventhandler.cpp \
  test_u_eventhandler.h \
  test_u_module_eventhandler.cpp \
  test_u_module_eventhandler.h \
  test_u_module_headerparser.cpp \
  test_u_module_headerparser.h \
  test_u_module_protocolhandler.cpp \
  test_u_module_protocolhandler.h \
  test_u_sessionmessage.cpp \
  test_u_sessionmessage.h \
  test_u_stream.cpp \
  test_u_stream.h \
  test_u_socket_common.h \
  test_u_stream_common.h \
  test_u_tcpconnection.cpp \
  test_u_tcpconnection.h
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

SUBDIRS = \
  etc
