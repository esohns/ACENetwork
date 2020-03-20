# Install script for directory: /mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent

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
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_bencoding_parser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_bencoding_parser_driver.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_bencoding_parser_driver.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_bencoding_scanner.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_common.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_control.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_control.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_defines.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_iparser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_message.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_message.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_module_parser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_module_parser.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_module_streamer.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_module_streamer.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_network.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_parser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_parser_driver.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_parser_driver.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_scanner.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_session.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_session.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_sessionmessage.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_sessionmessage.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_stream_peer.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_stream_peer.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_stream_tracker.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_stream_tracker.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_streamhandler.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_streamhandler.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_stream_common.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/p2p/bittorrent/bittorrent_tools.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACENetwork_Protocol_BitTorrent.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACENetwork/clang/src/protocol/p2p/bittorrent/libACENetwork_Protocol_BitTorrent.a")
endif()

