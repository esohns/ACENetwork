cmake_minimum_required (VERSION 2.8)

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libACEStream/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libACEStream/src)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../client_server)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

########### next target ###############
set (${PROJECT_NAME}_Protocol_DHCP_SRCS
     dhcp_common.h
     dhcp_common_modules.h
     dhcp_configuration.h
     dhcp_defines.h
     dhcp_codes.h
#     dhcp_bisector.cpp
#     dhcp_bisector.h
     dhcp_message.cpp
     dhcp_message.h
     dhcp_parser.cpp
     dhcp_parser.h
     dhcp_parser_driver.cpp
     dhcp_parser_driver.h
     dhcp_scanner.cpp
     dhcp_scanner.h
     dhcp_message.cpp
     dhcp_message.h
     dhcp_message.inl
#     dhcp_module_bisector.cpp
#     dhcp_module_bisector.h
#     dhcp_module_bisector.inl
     dhcp_module_discover.cpp
     dhcp_module_discover.h
     dhcp_module_discover.inl
     dhcp_module_parser.cpp
     dhcp_module_parser.h
     dhcp_module_parser.inl
     dhcp_module_streamer.cpp
     dhcp_module_streamer.h
     dhcp_module_streamer.inl
     dhcp_network.h
     dhcp_sessionmessage.cpp
     dhcp_sessionmessage.h
     dhcp_stream.cpp
     dhcp_stream.h
     dhcp_stream.inl
     dhcp_stream_common.h
     dhcp_tools.cpp
     dhcp_tools.h
     stdafx.cpp
     stdafx.h)
set (${PROJECT_NAME}_Protocol_DCHP_SCRIPTS
     scripts/scanner.l
     scripts/parser.y)

#set_source_files_properties (${PROJECT_NAME}_Protocol_dhcp_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_dhcp_SRCS PROPERTIES COMPILE_DEFINITIONS -Ddhcp_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif ()

add_library (${PROJECT_NAME}_Protocol_DHCP SHARED
             ${ACENetwork_Protocol_DHCP_SRCS}
             ${ACENetwork_Protocol_DCHP_SCRIPTS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP Common)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES DEFINE_SYMBOL NET_PROTOCOL_DHCP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ACEStream)
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ACENetwork)
endif ()

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_DHCP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         dhcp_common.h
         dhcp_common_modules.h
         dhcp_configuration.h
         dhcp_defines.h
         dhcp_codes.h
         dhcp_bisector.h
         dhcp_message.h
         dhcp_parser.h
         dhcp_parser_driver.h
         dhcp_scanner.h
         dhcp_message.h
         dhcp_message.inl
         dhcp_module_bisector.h
         dhcp_module_bisector.inl
         dhcp_module_discover.h
         dhcp_module_discover.inl
         dhcp_module_parser.h
         dhcp_module_parser.inl
         dhcp_module_streamer.h
         dhcp_module_streamer.inl
         dhcp_network.h
         dhcp_sessionmessage.h
         dhcp_stream.h
         dhcp_stream.inl
         dhcp_stream_common.h
         dhcp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_DHCP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
