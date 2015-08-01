ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DHAVE_CONFIG_H \
  -DNET_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@PACKAGE_NAME@\"

#  -I/usr/include/c++/3.4.6/backward
#  -I$(top_srcdir)/3rd_party/c++
AM_CPPFLAGS = \
  -I$(top_srcdir)/../ATCD/ACE

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  net_asynch_netlinksockethandler.h \
  net_asynch_tcpsockethandler.h \
  net_asynch_udpsockethandler.h \
  net_common_tools.h \
  net_connection_base.h \
  net_connection_manager.h \
  net_controller_base.h \
  net_defines.h \
  net_exports.h \
  net_iconnection.h \
  net_iconnectionmanager.h \
  net_ipconnection.h \
  net_itransportlayer.h \
  net_macros.h \
  net_message_base.h \
  net_message_base.inl \
  net_module_messagehandler.h \
  net_module_messagehandler.inl \
  net_module_runtimestatistic.h \
  net_module_runtimestatistic.inl \
  net_module_sockethandler.h \
  net_module_sockethandler.inl \
  net_netlinkconnection.h \
  net_netlinkconnection.inl \
  net_netlinksockethandler.h \
  net_packet_headers.h \
  net_protocol_layer.h \
  net_remote_comm.h \
  net_socket_common.h \
  net_socketconnection_base.h \
  net_socketconnection_base.inl \
  net_sockethandler_base.h \
  net_sockethandler_base.inl \
  net_stream_asynch_tcpsocket_base.h \
  net_stream_asynch_tcpsocket_base.inl \
  net_stream_asynch_udpsocket_base.h \
  net_stream_asynch_udpsocket_base.inl \
  net_stream_tcpsocket_base.h \
  net_stream_tcpsocket_base.inl \
  net_stream_udpsocket_base.h \
  net_stream_udpsocket_base.inl \
  net_tcpconnection_base.h \
  net_tcpconnection_base.inl \
  net_tcpsockethandler.h \
  net_transportlayer_base.h \
  net_transportlayer_ip_cast.h \
  net_transportlayer_netlink.h \
  net_transportlayer_tcp.h \
  net_transportlayer_udp.h \
  net_udpconnection_base.h \
  net_udpconnection_base.inl \
  net_udpsockethandler.h \
  net_udpsockethandler.inl

lib_LTLIBRARIES = libACENetwork.la

libACENetwork_la_SOURCES = \
  net_asynch_netlinksockethandler.cpp \
  net_asynch_netlinksockethandler.h \
  net_asynch_tcpsockethandler.cpp \
  net_asynch_tcpsockethandler.h \
  net_asynch_udpsockethandler.cpp \
  net_asynch_udpsockethandler.h \
  net_common_modules.h \
  net_common_tools.cpp \
  net_common_tools.h \
  net_common.h \
  net_connection_base.cpp \
  net_connection_base.h \
  net_connection_base.inl \
  net_connection_common.h \
  net_connection_manager_common.h \
  net_connection_manager.cpp \
  net_connection_manager.h \
  net_connection_manager.inl \
  net_controller_base.cpp \
  net_controller_base.h \
  net_defines.h \
  net_exports.h \
  net_iconnection.h \
  net_iconnectionmanager.h \
  net_ipconnection.cpp \
  net_ipconnection.h \
  net_itransportlayer.h \
  net_macros.h \
  net_message_base.cpp \
  net_message_base.h \
  net_message_base.inl \
  net_module_messagehandler.cpp \
  net_module_messagehandler.h \
  net_module_messagehandler.inl \
  net_module_runtimestatistic.cpp \
  net_module_runtimestatistic.h \
  net_module_runtimestatistic.inl \
  net_module_sockethandler.cpp \
  net_module_sockethandler.h \
  net_module_sockethandler.inl \
  net_netlinkconnection.cpp \
  net_netlinkconnection.h \
  net_netlinkconnection.inl \
  net_netlinksockethandler.cpp \
  net_netlinksockethandler.h \
  net_packet_headers.h \
  net_protocol_layer.cpp \
  net_protocol_layer.h \
  net_remote_comm.h \
  net_socket_common.h \
  net_socketconnection_base.cpp \
  net_socketconnection_base.h \
  net_socketconnection_base.inl \
  net_sockethandler_base.cpp \
  net_sockethandler_base.h \
  net_sockethandler_base.inl \
  net_stream_asynch_tcpsocket_base.cpp \
  net_stream_asynch_tcpsocket_base.h \
  net_stream_asynch_tcpsocket_base.inl \
  net_stream_asynch_udpsocket_base.cpp \
  net_stream_asynch_udpsocket_base.h \
  net_stream_asynch_udpsocket_base.inl \
  net_stream_tcpsocket_base.cpp \
  net_stream_tcpsocket_base.h \
  net_stream_tcpsocket_base.inl \
  net_stream_udpsocket_base.cpp \
  net_stream_udpsocket_base.h \
  net_stream_udpsocket_base.inl \
  net_tcpconnection_base.cpp \
  net_tcpconnection_base.h \
  net_tcpconnection_base.inl \
  net_tcpsockethandler.cpp \
  net_tcpsockethandler.h \
  net_transportlayer_base.cpp \
  net_transportlayer_base.h \
  net_transportlayer_ip_cast.cpp \
  net_transportlayer_ip_cast.h \
  net_transportlayer_netlink.cpp \
  net_transportlayer_netlink.h \
  net_transportlayer_tcp.cpp \
  net_transportlayer_tcp.h \
  net_transportlayer_udp.cpp \
  net_transportlayer_udp.h \
  net_udpconnection_base.cpp \
  net_udpconnection_base.h \
  net_udpconnection_base.inl \
  net_udpsockethandler.cpp \
  net_udpsockethandler.h \
  net_udpsockethandler.inl
#  stdafx.cpp \
#  stdafx.h

libACENetwork_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/../libCommon/src \
  -I$(top_srcdir)/../libACEStream/src
libACENetwork_la_LDFLAGS = \
  -version-info @LIBACENETWORK_LT_VERSION_INFO@

SUBDIRS = \
  . \
  client_server
