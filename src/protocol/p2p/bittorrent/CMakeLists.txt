#option (GTK3_SUPPORT "compile Gtk3 support"
#        ON)

#include (FindPkgConfig)
#if (UNIX)
# #pkg_check_modules (PKG_ACE REQUIRED ACE)
# if (GTK3_SUPPORT)
#  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
# else ()
## pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
#  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
# endif ()
#endif ()

#set (FLEX_LIB_FILE libfl.so)
#if (UNIX)
# find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
#               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
#elseif (WIN32)
# unset (FLEX_LIB_FILE)
# set (FLEX_LIB_FILE libfl.lib)
# find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
#               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
#endif ()
##if (NOT EXISTS ACE_LIBRARY)
#if (NOT FLEX_LIBRARY)
# message (FATAL_ERROR "could not find ${FLEX_LIB_FILE} (was: \"${FLEX_LIBRARY}\"), aborting")
#endif ()

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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
# if (GTK3_SUPPORT)
#  include_directories (${PKG_GTK3_INCLUDE_DIRS})
# else ()
#  include_directories (${PKG_GTK2_INCLUDE_DIRS})
# endif ()

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/libACEStream/src/modules/misc)
elseif (WIN32)
# *TODO*: repair pkgconfig support
# include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../libACEStream/src/modules/misc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rd_party)
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
     bencoding_scanner.cpp
     bencoding_scanner.h
     bittorrent_common.h
     bittorrent_defines.h
     bittorrent_exports.h
     bittorrent_iparser.h
     bittorrent_message.cpp
     bittorrent_message.h
     bittorrent_message.inl
     bittorrent_metainfo_parser.cpp
     bittorrent_metainfo_parser.h
     bittorrent_metainfo_parser_driver.cpp
     bittorrent_metainfo_parser_driver.h
     bittorrent_metainfo_parser_driver.inl
     bittorrent_module_handler.cpp
     bittorrent_module_handler.h
     bittorrent_module_handler.inl
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
     bittorrent_stream.cpp
     bittorrent_stream.h
     bittorrent_stream.inl
     bittorrent_stream_common.h
     bittorrent_streamhandler.cpp
     bittorrent_streamhandler.h
     bittorrent_streamhandler.inl
     bittorrent_tools.cpp
     bittorrent_tools.h
     location.hh
     position.hh
     stack.hh)
set (${PROJECT_NAME}_Protocol_BitTorrent_SCRIPTS
     scripts/bencoding.ll
     scripts/metainfo_parser.yy
     scripts/scanner.l
     scripts/parser.y)

#set_source_files_properties (${PROJECT_NAME}_Protocol_BitTorrent_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_BitTorrent_SRCS PROPERTIES COMPILE_DEFINITIONS -Dbittorrent_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_BitTorrent_SCRIPTS)
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

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent Common)
add_dependencies (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES DEFINE_SYMBOL bittorrent_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
# target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${FLEX_LIBRARY})
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_BitTorrent ACEStream)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_BitTorrent PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_BitTorrent)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_BitTorrent
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         bencoding_scanner.h
         bittorrent_common.h
         bittorrent_defines.h
         bittorrent_exports.h
         bittorrent_iparser.h
         bittorrent_message.h
         bittorrent_message.inl
         bittorrent_metainfo_parser.h
         bittorrent_module_handler.h
         bittorrent_module_handler.inl
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
         bittorrent_stream.h
         bittorrent_stream.inl
         bittorrent_streamhandler.h
         bittorrent_streamhandler.inl
         bittorrent_stream_common.h
         bittorrent_tools.h
         location.hh
         position.hh
         stack.hh
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_BitTorrent
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
