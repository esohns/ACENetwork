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
set (${PROJECT_NAME}_Protocol_HTTP_SRCS
     http_common.h
     http_common_modules.h
     http_configuration.h
     http_defines.h
     http_codes.h
     http_bisector.cpp
     http_bisector.h
     http_message.cpp
     http_message.h
     http_parser.cpp
     http_parser.h
     http_parser_driver.cpp
     http_parser_driver.h
     http_scanner.cpp
     http_scanner.h
     http_message.cpp
     http_message.h
     http_message.inl
     http_module_parser.cpp
     http_module_parser.h
     http_module_parser.inl
     http_module_bisector.cpp
     http_module_bisector.h
     http_module_bisector.inl
     http_module_streamer.cpp
     http_module_streamer.h
     http_module_streamer.inl
     http_network.h
     http_sessionmessage.cpp
     http_sessionmessage.h
     http_stream.cpp
     http_stream.h
     http_stream.inl
     http_stream_common.h
     http_tools.cpp
     http_tools.h
     stdafx.cpp
     stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS -DHTTP_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_library (${PROJECT_NAME}_Protocol_HTTP SHARED ${ACENetwork_Protocol_HTTP_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP Common)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES DEFINE_SYMBOL HTTP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ACEStream)
endif ()

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_HTTP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (TARGETS ${PROJECT_NAME}_Protocol_HTTP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)