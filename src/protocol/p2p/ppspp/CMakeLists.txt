find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/cmake
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

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/Common/src)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dev)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Common/src)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dev)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../client_server)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

########### next target ###############
set (${PROJECT_NAME}_Protocol_PPSPP_SRCS
     stdafx.cpp
     stdafx.h
     ppspp_bisector.cpp
     ppspp_bisector.h
     ppspp_codes.h
     ppspp_common.h
     ppspp_defines.h
     ppspp_exports.h
     ppspp_message.cpp
     ppspp_message.h
     ppspp_parser.cpp
     ppspp_parser.h
     ppspp_parser_driver.cpp
     ppspp_parser_driver.h
     ppspp_parser_driver.inl
     ppspp_scanner.cpp
     ppspp_scanner.h
     ppspp_message.cpp
     ppspp_message.h
     ppspp_message.inl
     ppspp_module_bisector.cpp
     ppspp_module_bisector.h
     ppspp_module_bisector.inl
     ppspp_module_parser.cpp
     ppspp_module_parser.h
     ppspp_module_parser.inl
     ppspp_module_streamer.cpp
     ppspp_module_streamer.h
     ppspp_module_streamer.inl
     ppspp_network.h
     ppspp_sessionmessage.cpp
     ppspp_sessionmessage.h
     ppspp_sessionmessage.inl
     ppspp_stream.cpp
     ppspp_stream.h
     ppspp_stream.inl
     ppspp_stream_common.h
     ppspp_tools.cpp
     ppspp_tools.h)
set (${PROJECT_NAME}_Protocol_PPSPP_SCRIPTS
     scripts/scanner.l
     scripts/parser.y)

#set_source_files_properties (${PROJECT_NAME}_Protocol_ppspp_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_ppspp_SRCS PROPERTIES COMPILE_DEFINITIONS -Dppspp_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_PPSPP_SCRIPTS)
endif ()

# *NOTE*: this is needed to export yydebug, so it can be imported from template
#         instances. Note that there is no way to #define YYDEBUG in the .y
#         parser definition file, as the generated '%code requires' sections are
#         inserted after the first '#define YYDEBUG' statements.
# *TODO*: this holds for GNU Bison > 2.4.2
#add_definitions (-DYYDEBUG)

add_library (${PROJECT_NAME}_Protocol_PPSPP SHARED
             ${ACENetwork_Protocol_PPSPP_SRCS}
             ${ACENetwork_Protocol_PPSPP_SCRIPTS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_PPSPP Common)
add_dependencies (${PROJECT_NAME}_Protocol_PPSPP ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_PPSPP PROPERTIES DEFINE_SYMBOL PPSPP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_PPSPP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_bittorrent ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_PPSPP ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_PPSPP Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_PPSPP ACEStream)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_PPSPP PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_PPSPP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_PPSPP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         ppspp_bisector.h
         ppspp_codes.h
         ppspp_common.h
         ppspp_defines.h
         ppspp_exports.h
         ppspp_message.h
         ppspp_parser.h
         ppspp_parser_driver.h
         ppspp_parser_driver.inl
         ppspp_scanner.h
         ppspp_message.h
         ppspp_message.inl
         ppspp_module_bisector.h
         ppspp_module_bisector.inl
         ppspp_module_parser.h
         ppspp_module_parser.inl
         ppspp_module_streamer.h
         ppspp_module_streamer.inl
         ppspp_network.h
         ppspp_sessionmessage.h
         ppspp_sessionmessage.inl
         ppspp_stream.h
         ppspp_stream.inl
         ppspp_stream_common.h
         ppspp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_PPSPP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)

