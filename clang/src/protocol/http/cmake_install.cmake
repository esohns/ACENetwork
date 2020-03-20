# Install script for directory: /mnt/win_d/projects/ACENetwork/src/protocol/http

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
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_bisector.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_codes.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_common.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_defines.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_iparser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_message.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_parser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_parser_driver.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_parser_driver.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_scanner.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_scanner.tab"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_message.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_message.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_bisector.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_bisector.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_parser.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_parser.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_streamer.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_module_streamer.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_network.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_sessionmessage.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_sessionmessage.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_stream.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_stream.inl"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_stream_common.h"
    "/mnt/win_d/projects/ACENetwork/src/protocol/http/http_tools.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACENetwork_Protocol_HTTP.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACENetwork/clang/src/protocol/http/libACENetwork_Protocol_HTTP.a")
endif()

