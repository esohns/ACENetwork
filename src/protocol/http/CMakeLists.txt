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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libACEStream/src/modules/dev)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src/modules/dev)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/flex)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../client_server)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../..)

########### next target ###############
set (${PROJECT_NAME}_Protocol_HTTP_SRCS
     stdafx.cpp
     stdafx.h
     http_bisector.cpp
     http_bisector.h
     http_codes.h
     http_common.h
     http_defines.h
     http_exports.h
     http_iparser.h
     http_message.cpp
     http_message.h
     http_parser.cpp
     http_parser.h
     http_parser_driver.cpp
     http_parser_driver.h
     http_parser_driver.inl
     http_scanner.cpp
     http_scanner.h
     http_message.cpp
     http_message.h
     http_message.inl
     http_module_bisector.cpp
     http_module_bisector.h
     http_module_bisector.inl
     http_module_parser.cpp
     http_module_parser.h
     http_module_parser.inl
     http_module_streamer.cpp
     http_module_streamer.h
     http_module_streamer.inl
     http_network.h
     http_sessionmessage.cpp
     http_sessionmessage.h
     http_sessionmessage.inl
     http_stream.cpp
     http_stream.h
     http_stream.inl
     http_stream_common.h
     http_tools.cpp
     http_tools.h)
set (${PROJECT_NAME}_Protocol_HTTP_SCRIPTS
     scripts/bisector.l
     scripts/scanner.l
     scripts/parser.y)
set (${PROJECT_NAME}_Protocol_HTTP_TABLES
     etc/http_scanner.tab)

#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS -DHTTP_BUILD_DLL)

if (WIN32)
## *NOTE*: generate .def file and export all symbols on Windows
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_HTTP_SCRIPTS)
 source_group (lex_tables FILES ${PROJECT_NAME}_Protocol_HTTP_TABLES)
 source_group (exports FILES etc/exports.def)
endif ()

# *NOTE*: this is needed to export yydebug, so it can be imported from template
#         instances. Note that there is no way to #define YYDEBUG in the .y
#         parser definition file, as the generated '%code requires' sections are
#         inserted after the first '#define YYDEBUG' statements.
# *TODO*: this holds for GNU Bison > 2.4.2
#add_definitions (-DYYDEBUG)

add_library (${PROJECT_NAME}_Protocol_HTTP SHARED
             ${ACENetwork_Protocol_HTTP_SRCS}
             ${ACENetwork_Protocol_HTTP_SCRIPTS})
set_property (TARGET ${PROJECT_NAME}_Protocol_HTTP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP Common)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES DEFINE_SYMBOL HTTP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)
if (WIN32 AND MSVC)
 set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/etc/exports.def\"")
endif ()

#target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP ACEStream)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_HTTP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_HTTP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         http_bisector.h
         http_codes.h
         http_common.h
         http_defines.h
         http_exports.h
         http_iparser.h
         http_message.h
         http_parser.h
         http_parser_driver.h
         http_parser_driver.inl
         http_scanner.h
         http_scanner.tab
         http_message.h
         http_message.inl
         http_module_bisector.h
         http_module_bisector.inl
         http_module_parser.h
         http_module_parser.inl
         http_module_streamer.h
         http_module_streamer.inl
         http_network.h
         http_sessionmessage.h
         http_sessionmessage.inl
         http_stream.h
         http_stream.inl
         http_stream_common.h
         http_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_HTTP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
