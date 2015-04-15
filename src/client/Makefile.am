ACLOCAL_AMFLAGS = -I m4

DEFS = \
  -DNET_CLIENT_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/../ATCD/ACE \
  -I$(top_srcdir)/../libCommon/src \
  -I$(top_srcdir)/../libACEStream/src \
  -I$(srcdir)/..

noinst_HEADERS = stdafx.h
include_HEADERS = \
  net_client_asynchconnector.h \
  net_client_common.h \
  net_client_connector.h \
  net_client_defines.h \
  net_client_exports.h \
  net_client_iconnector.h

lib_LTLIBRARIES = libACENetwork_Client.la

libACENetwork_Client_la_SOURCES = \
  net_client_asynchconnector.cpp \
  net_client_asynchconnector.h \
  net_client_asynchconnector.inl \
  net_client_common.h \
  net_client_connector.cpp \
  net_client_connector.h \
  net_client_connector.inl \
  net_client_defines.h \
  net_client_exports.h \
  net_client_iconnector.h \
  stdafx.cpp \
  stdafx.h
