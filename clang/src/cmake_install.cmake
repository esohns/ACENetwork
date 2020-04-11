# Install script for directory: /mnt/win_d/projects/ACENetwork/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_netlinksockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_netlinksockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_ssl_tcpsockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_ssl_tcpsockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_tcpsockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_tcpsockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_udpsockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_asynch_udpsockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_common_tools.h"
    "/mnt/win_d/projects/ACENetwork/src/net_common.h"
    "/mnt/win_d/projects/ACENetwork/src/net_configuration.h"
    "/mnt/win_d/projects/ACENetwork/src/net_connection_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_connection_configuration.h"
    "/mnt/win_d/projects/ACENetwork/src/net_connection_configuration.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_connection_manager.h"
    "/mnt/win_d/projects/ACENetwork/src/net_connection_manager.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_controller_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_defines.h"
    "/mnt/win_d/projects/ACENetwork/src/net_iconnection.h"
    "/mnt/win_d/projects/ACENetwork/src/net_iconnectionmanager.h"
    "/mnt/win_d/projects/ACENetwork/src/net_iconnector.h"
    "/mnt/win_d/projects/ACENetwork/src/net_ilistener.h"
    "/mnt/win_d/projects/ACENetwork/src/net_ipconnection.h"
    "/mnt/win_d/projects/ACENetwork/src/net_itransportlayer.h"
    "/mnt/win_d/projects/ACENetwork/src/net_macros.h"
    "/mnt/win_d/projects/ACENetwork/src/net_netlinkconnection.h"
    "/mnt/win_d/projects/ACENetwork/src/net_netlinkconnection.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_netlinksockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_netlinksockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_os_tools.h"
    "/mnt/win_d/projects/ACENetwork/src/net_packet_headers.h"
    "/mnt/win_d/projects/ACENetwork/src/net_parser_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_parser_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_protocol_layer.h"
    "/mnt/win_d/projects/ACENetwork/src/net_remote_comm.h"
    "/mnt/win_d/projects/ACENetwork/src/net_session_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_session_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_sock_acceptor.h"
    "/mnt/win_d/projects/ACENetwork/src/net_sock_connector.h"
    "/mnt/win_d/projects/ACENetwork/src/net_sock_dgram.h"
    "/mnt/win_d/projects/ACENetwork/src/net_socket_common.h"
    "/mnt/win_d/projects/ACENetwork/src/net_sockethandler_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_sockethandler_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_streamconnection_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_streamconnection_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_asynch_tcpsocket_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_asynch_tcpsocket_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_asynch_udpsocket_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_asynch_udpsocket_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_tcpsocket_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_tcpsocket_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_udpsocket_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_stream_udpsocket_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_tcpconnection_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_tcpconnection_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_tcpsockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_tcpsockethandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_ip_cast.h"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_netlink.h"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_tcp.h"
    "/mnt/win_d/projects/ACENetwork/src/net_transportlayer_udp.h"
    "/mnt/win_d/projects/ACENetwork/src/net_udpconnection_base.h"
    "/mnt/win_d/projects/ACENetwork/src/net_udpconnection_base.inl"
    "/mnt/win_d/projects/ACENetwork/src/net_udpsockethandler.h"
    "/mnt/win_d/projects/ACENetwork/src/net_udpsockethandler.inl"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACENetwork.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACENetwork/clang/src/libACENetwork.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/win_d/projects/ACENetwork/clang/src/client_server/cmake_install.cmake")
  include("/mnt/win_d/projects/ACENetwork/clang/src/protocol/cmake_install.cmake")
  include("/mnt/win_d/projects/ACENetwork/clang/src/wlan/cmake_install.cmake")

endif()

