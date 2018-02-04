#option (GTK3_SUPPORT "compile Gtk3 support" ON)

include (FindPkgConfig)
if (UNIX)
 #pkg_check_modules (PKG_ACE REQUIRED ACE)
# if (GTK3_SUPPORT)
#  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
else ()
## pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
#  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
# endif ()
endif ()

set (FLEX_LIB_FILE libfl.so)
if (UNIX)
 find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
elseif (WIN32)
 unset (FLEX_LIB_FILE)
 set (FLEX_LIB_FILE libfl.lib)
# find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
#               PATHS ${PROJECTS_DIRECTORY}/flex
#               PATH_SUFFIXES lib
#               DOC "searching for ${FLEX_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT FLEX_LIBRARY)
# message (FATAL_ERROR "could not find ${FLEX_LIB_FILE} (was: \"${FLEX_LIBRARY}\"), aborting")
endif ()

set (PROJECTS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
set (OPENSSL_LIB_FILE libeay32.lib)
if (UNIX)
 pkg_check_modules (PKG_OPENSSL REQUIRED openssl)
elseif (WIN32)
 find_library (OPENSSL_LIBRARY ${OPENSSL_LIB_FILE}
               PATHS ${PROJECTS_DIRECTORY}/openssl
               PATH_SUFFIXES lib
               DOC "searching for ${OPENSSL_LIB_FILE}")
#if (NOT EXISTS OPENSSL_LIBRARY)
 if (NOT OPENSSL_LIBRARY)
  message (FATAL_ERROR "could not find ${OPENSSL_LIB_FILE} (was: \"${OPENSSL_LIBRARY}\"), aborting")
 endif ()
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
# if (GTK3_SUPPORT)
#  include_directories (${PKG_GTK3_INCLUDE_DIRS})
# else ()
#  include_directories (${PKG_GTK2_INCLUDE_DIRS})
# endif ()
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dec)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/stat)
elseif (WIN32)
# *TODO*: repair pkgconfig support
# include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
# gtk
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ffmpeg)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dec)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/stat)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party/flex)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../http)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

########### next target ###############
set (${PROJECT_NAME}_Protocol_BitTorrent_SRCS
     stdafx.cpp
     stdafx.h
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
     bittorrent_exports.h
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
set (${PROJECT_NAME}_Protocol_BitTorrent_SCRIPTS
     scripts/bencoding_scanner.ll
     scripts/bencoding_parser.yy
     scripts/scanner.l
     scripts/parser.yy)

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
endif ()

# *NOTE*: this is needed to export yydebug, so it can be imported from template
#         instances. Note that there is no way to #define YYDEBUG in the .y
#         parser definition file, as the generated '%code requires' sections are
#         inserted after the first '#define YYDEBUG' statements.
# *TODO*: this holds for GNU Bison > 2.4.2
#add_definitions (-DYYDEBUG)

add_library (${PROJECT_NAME}_Protocol_BitTorrent SHARED
             ${ACENetwork_Protocol_BitTorrent_SRCS}
             ${ACENetwork_Protocol_BitTorrent_SCRIPTS})
set_property (TARGET ${PROJECT_NAME}_Protocol_BitTorrent PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent Common)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES DEFINE_SYMBOL BITTORRENT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)
if (WIN32 AND MSVC)
 set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/etc/exports.def\"")
endif ()

#target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
# target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${FLEX_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${PKG_OPENSSL_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${OPENSSL_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)
# target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${PROJECT_NAME}_Protocol_HTTP)
endif ()

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
         bittorrent_exports.h
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
