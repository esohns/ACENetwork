#message (STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
# message (STATUS "using ${CMAKE_CXX_COMPILER_ID} options")
 add_compile_options (-std=c++11)
# add_compile_options (-fPIC)
 add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

add_library (${PROJECT_NAME}_Protocol_BitTorrent STATIC "")

set (PROJECTS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)
 if (DHCLIENT_SUPPORT)
#  target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
#                              $<BUILD_INTERFACE:/usr/include/bind-export>)
  target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC ${BIND9_CFLAGS})
 endif (DHCLIENT_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${PKG_FLEX_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/Common/src/timer>)
 # *TODO*: remove ASAP
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/src/modules/dec>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party/flex>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ffmpeg>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../openssl/include>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ATCD/ACE>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Common/src/log>)
 # *TODO*: remove ASAP
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ACEStream/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ACEStream/src/modules/dec>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../client_server>)
target_include_directories (${PROJECT_NAME}_Protocol_BitTorrent PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../http>)

########### next target ###############
set (source_files
     bittorrent_bencoding_parser.cpp
     bittorrent_bencoding_parser.h
     bittorrent_bencoding_parser_driver.cpp
     bittorrent_bencoding_parser_driver.h
     bittorrent_bencoding_parser_driver.inl
     bittorrent_bencoding_scanner.cpp
     bittorrent_bencoding_scanner.h
     bittorrent_common.h
     bittorrent_control.cpp
     bittorrent_control.h
     bittorrent_control.inl
     bittorrent_defines.h
#     bittorrent_exports.h
     bittorrent_iparser.h
     bittorrent_message.cpp
     bittorrent_message.h
     bittorrent_message.inl
     bittorrent_module_parser.cpp
     bittorrent_module_parser.h
     bittorrent_module_parser.inl
     bittorrent_module_streamer.cpp
     bittorrent_module_streamer.h
     bittorrent_module_streamer.inl
     bittorrent_network.h
     bittorrent_parser.cpp
     bittorrent_parser.h
     bittorrent_parser_driver.cpp
     bittorrent_parser_driver.h
     bittorrent_parser_driver.inl
     bittorrent_scanner.cpp
     bittorrent_scanner.h
     bittorrent_session.cpp
     bittorrent_session.h
     bittorrent_session.inl
     bittorrent_sessionmessage.cpp
     bittorrent_sessionmessage.h
     bittorrent_sessionmessage.inl
     bittorrent_stream_peer.cpp
     bittorrent_stream_peer.h
     bittorrent_stream_peer.inl
     bittorrent_stream_tracker.cpp
     bittorrent_stream_tracker.h
     bittorrent_stream_tracker.inl
     bittorrent_stream_common.h
     bittorrent_streamhandler.cpp
     bittorrent_streamhandler.h
     bittorrent_streamhandler.inl
     bittorrent_tools.cpp
     bittorrent_tools.h)
set (script_files
     scripts/bencoding_scanner.ll
     scripts/bencoding_parser.yy
     scripts/scanner.l
     scripts/parser.yy)

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
  ${abs_source_file})
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
unset (absolute_script_files)
foreach (script_file ${script_files})
  get_filename_component (abs_script_file ${script_file} ABSOLUTE)
  list (APPEND absolute_script_files
    ${absolute_script_file})
endforeach ()
target_sources (${PROJECT_NAME}_Protocol_BitTorrent
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE
                ${absolute_source_files}
                INTERFACE
                ${absolute_script_files})

#set_source_files_properties (${PROJECT_NAME}_Protocol_BitTorrent_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_BitTorrent_SRCS PROPERTIES COMPILE_DEFINITIONS -Dbittorrent_BUILD_DLL)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "CLang")
 add_compile_options (-fPIC)
endif ()

if (WIN32)
# *NOTE*: generate .def file and export all symbols on Windows
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_BitTorrent_SCRIPTS)
 source_group (exports FILES etc/exports.def)
endif (WIN32)

# *NOTE*: this is needed to export yydebug, so it can be imported from template
#         instances. Note that there is no way to #define YYDEBUG in the .y
#         parser definition file, as the generated '%code requires' sections are
#         inserted after the first '#define YYDEBUG' statements.
# *TODO*: this holds for GNU Bison > 2.4.2
#add_definitions (-DYYDEBUG)

set_property (TARGET ${PROJECT_NAME}_Protocol_BitTorrent PROPERTY FOLDER "protocols")

#add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent ACE)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent Common)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES DEFINE_SYMBOL BITTORRENT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)
if (WIN32 AND MSVC)
 set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/etc/exports.def\"")
endif (WIN32 AND MSVC)

#target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${PKG_OPENSSL_LIBRARIES})
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${OPENSSL_LIBRARY})

 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent Common)
target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)
# target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${PROJECT_NAME}_Protocol_HTTP)

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_BitTorrent)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_BitTorrent
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         bittorrent_bencoding_parser.h
         bittorrent_bencoding_parser_driver.h
         bittorrent_bencoding_parser_driver.inl
         bittorrent_bencoding_scanner.h
         bittorrent_common.h
         bittorrent_control.h
         bittorrent_control.inl
         bittorrent_defines.h
#         bittorrent_exports.h
         bittorrent_iparser.h
         bittorrent_message.h
         bittorrent_message.inl
         bittorrent_module_parser.h
         bittorrent_module_parser.inl
         bittorrent_module_streamer.h
         bittorrent_module_streamer.inl
         bittorrent_network.h
         bittorrent_parser.h
         bittorrent_parser_driver.h
         bittorrent_parser_driver.inl
         bittorrent_scanner.h
         bittorrent_session.h
         bittorrent_session.inl
         bittorrent_sessionmessage.h
         bittorrent_sessionmessage.inl
         bittorrent_stream_peer.h
         bittorrent_stream_peer.inl
         bittorrent_stream_tracker.h
         bittorrent_stream_tracker.inl
         bittorrent_streamhandler.h
         bittorrent_streamhandler.inl
         bittorrent_stream_common.h
         bittorrent_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_BitTorrent
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
