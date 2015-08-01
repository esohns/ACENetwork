ACLOCAL_AMFLAGS = -I m4

DEFS = \
  -DNET_CLIENT_BUILD_DLL=1 \
  -DNET_SERVER_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/../ATCD/ACE \
  -I$(top_srcdir)/../libCommon/src \
  -I$(top_srcdir)/../libACEStream/src \
  -I$(srcdir)/..

noinst_HEADERS = stdafx.h
include_HEADERS = \
  net_client_asynchconnector.h \
  net_client_connector.h \
  net_client_defines.h \
  net_client_exports.h \
  net_client_iconnector.h \
  net_server_asynchlistener.h \
  net_server_asynchlistener.inl \
  net_server_common_tools.h \
  net_server_defines.h \
  net_server_exports.h \
  net_server_ilistener.h \
  net_server_listener.h \
  net_server_listener.inl

lib_LTLIBRARIES = \
  libACENetwork_Client.la \
  libACENetwork_Server.la

libACENetwork_Client_la_SOURCES = \
  net_client_asynchconnector.cpp \
  net_client_asynchconnector.h \
  net_client_asynchconnector.inl \
  net_client_common.h \
  net_client_connector.cpp \
  net_client_connector.h \
  net_client_connector.inl \
  net_client_connector_common.h \
  net_client_defines.h \
  net_client_exports.h \
  net_client_iconnector.h \
  stdafx.cpp \
  stdafx.h

libACENetwork_Server_la_SOURCES = \
  net_server_asynchlistener.cpp \
  net_server_asynchlistener.h \
  net_server_asynchlistener.inl \
  net_server_common_tools.cpp \
  net_server_common_tools.h \
  net_server_defines.h \
  net_server_exports.h \
  net_server_ilistener.h \
  net_server_listener.cpp \
  net_server_listener.h \
  net_server_listener.inl \
  net_server_listener_common.h \
  stdafx.cpp \
  stdafx.h

