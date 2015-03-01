ACLOCAL_AMFLAGS = \
  -I m4

DEFS = \
  -DCOMMON_HAS_DLL \
  -DNET_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@META_PACKAGE@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/../libCommon/src \
  -I$(top_srcdir)/../libACEStream/src

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  net_asynch_tcpsockethandler.h \
  net_asynch_udpsockethandler.h \
  net_common_modules.h \
  net_common_tools.h \
  net_configuration.h \
  net_connection_base.h \
  net_connection_common.h \
  net_connection_manager_common.h \
  net_connection_manager.h \
  net_controller_base.h \
  net_defines.h \
  net_exports.h \
  net_iconnection.h \
  net_iconnectionmanager.h \
  net_itransportlayer.h \
  net_macros.h \
  net_message.h \
  net_message_base.h \
  net_message_base.inl \
  net_module_headerparser.h \
  net_module_protocolhandler.h \
  net_module_runtimestatistic.h \
  net_module_runtimestatistic.inl \
  net_module_sockethandler.h \
  net_netlinkconnection.h \
  net_packet_headers.h \
  net_protocol_layer.h \
  net_remote_comm.h \
  net_sessionmessage.h \
  net_socket_common.h \
  net_socketconnection_base.h \
  net_socketconnection_base.inl \
  net_sockethandler_base.h \
  net_sockethandler_base.inl \
  net_stream_asynch_tcpsocket_base.h \
  net_stream_asynch_tcpsocket_base.inl \
  net_stream_asynch_udpsocket_base.h \
  net_stream_asynch_udpsocket_base.inl \
  net_stream_common.h \
  net_stream_tcpsocket_base.h \
  net_stream_tcpsocket_base.inl \
  net_stream_udpsocket_base.h \
  net_stream_udpsocket_base.inl \
  net_stream.h \
  net_tcpconnection.h \
  net_tcpsockethandler.h \
  net_transportlayer_base.h \
  net_transportlayer_ip_cast.h \
  net_transportlayer_netlink.h \
  net_transportlayer_tcp.h \
  net_transportlayer_udp.h \
  net_udpconnection.h \
  net_udpsockethandler.h \
  net_udpsockethandler.inl

lib_LTLIBRARIES = libACENetwork.la

libACENetwork_la_SOURCES = \
  net_asynch_tcpsockethandler.cpp \
  net_asynch_tcpsockethandler.h \
  net_asynch_udpsockethandler.cpp \
  net_asynch_udpsockethandler.h \
  net_common_modules.h \
  net_common_tools.cpp \
  net_common_tools.h \
  net_common.h \
  net_configuration.h \
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
  net_itransportlayer.h \
  net_macros.h \
  net_message.cpp \
  net_message.h \
  net_message_base.cpp \
  net_message_base.h \
  net_message_base.inl \
  net_module_headerparser.cpp \
  net_module_headerparser.h \
  net_module_protocolhandler.cpp \
  net_module_protocolhandler.h \
  net_module_runtimestatistic.cpp \
  net_module_runtimestatistic.h \
  net_module_runtimestatistic.inl \
  net_module_sockethandler.cpp \
  net_module_sockethandler.h \
  net_netlinkconnection.cpp \
  net_netlinkconnection.h \
  net_packet_headers.h \
  net_protocol_layer.cpp \
  net_protocol_layer.h \
  net_remote_comm.h \
  net_sessionmessage.cpp \
  net_sessionmessage.h \
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
  net_stream_common.h \
  net_stream_tcpsocket_base.cpp \
  net_stream_tcpsocket_base.h \
  net_stream_tcpsocket_base.inl \
  net_stream_udpsocket_base.cpp \
  net_stream_udpsocket_base.h \
  net_stream_udpsocket_base.inl \
  net_stream.cpp \
  net_stream.h \
  net_tcpconnection.cpp \
  net_tcpconnection.h \
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
  net_udpconnection.cpp \
  net_udpconnection.h \
  net_udpsockethandler.cpp \
  net_udpsockethandler.h \
  net_udpsockethandler.inl \
  stdafx.cpp \
  stdafx.h

SUBDIRS = \
  client
