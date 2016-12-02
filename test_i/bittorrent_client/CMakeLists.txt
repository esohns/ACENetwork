option (GTK3_SUPPORT "enable Gtk3 support"
        ON)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
endif ()

set (FLEX_LIB_FILE libfl.so)
if (UNIX)
 find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
elseif (WIN32)
 unset (FLEX_LIB_FILE)
 set (FLEX_LIB_FILE libfl.lib)
 find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT FLEX_LIBRARY)
# message (FATAL_ERROR "could not find ${FLEX_LIB_FILE} (was: \"${FLEX_LIBRARY}\"), aborting")
endif ()

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_CURSES REQUIRED ncurses panel)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
 if ((!PKG_CURSES) OR (!PKG_GTK))
  message (FATAL_ERROR "required packages not found, aborting")
 endif ()
# *TODO*: repair WIN32 pkgconfig support
elseif (WIN32)
 find_library (CURSES_LIBRARY pdcurses.lib
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses
               PATH_SUFFIXES win32
               DOC "searching for pdcurses.lib")
 find_library (PANEL_LIBRARY panel.lib
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses
               PATH_SUFFIXES win32
               DOC "searching for panel.lib")

 if (NOT CURSES_LIBRARY)
  message (FATAL_ERROR "could not find pdcurses.lib, aborting")
 endif ()
 if (NOT PANEL_LIBRARY)
  message (FATAL_ERROR "could not find panel.lib, aborting")
 endif ()
endif ()
#if (NOT EXISTS PDCURSES_LIBRARY)

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

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

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/..
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
add_definitions (-DIRC_HAS_DLL)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_CURSES_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/misc)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/p2p/bittorrent)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (bittorrent_client_SRCS
     stdafx.cpp
     stdafx.h
     bittorrent_client.cpp
     bittorrent_client_common.h
     bittorrent_client_common_modules.h
     bittorrent_client_configuration.h
     bittorrent_client_curses.cpp
     bittorrent_client_curses.h
     bittorrent_client_defines.h
#     bittorrent_client_inputhandler.cpp
#     bittorrent_client_inputhandler.h
#     bittorrent_client_message.cpp
#     bittorrent_client_message.h
     bittorrent_client_network.h
     bittorrent_client_session_common.h
#     bittorrent_client_sessionmessage.cpp
#     bittorrent_client_sessionmessage.h
     bittorrent_client_signalhandler.cpp
     bittorrent_client_signalhandler.h
#     bittorrent_client_stream.cpp
#     bittorrent_client_stream.h
     bittorrent_client_stream_common.h
     bittorrent_client_tools.cpp
     bittorrent_client_tools.h)

#set_source_files_properties(${bittorrent_client_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

set (EXECUTABLE bittorrent_client)
add_executable(${EXECUTABLE} ${bittorrent_client_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_BitTorrent)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${FLEX_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PKG_CURSES_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ${CURSES_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PANEL_LIBRARY})
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_BitTorrent)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
#install (FILES etc/net_client.glade
#         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

################################################################################

if (UNIX)
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
 # *TODO*: repair pkgconfig support
# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
endif ()

########### next target ###############
set (bittorrent_client_gui_SRCS
     stdafx.cpp
     stdafx.h
     bittorrent_client_common.h
     bittorrent_client_common_modules.h
     bittorrent_client_configuration.h
     bittorrent_client_defines.h
     bittorrent_client_gui.cpp
     bittorrent_client_gui_callbacks.cpp
     bittorrent_client_gui_callbacks.h
     bittorrent_client_gui_common.h
     bittorrent_client_gui_defines.h
     bittorrent_client_gui_session.cpp
     bittorrent_client_gui_session.h
     bittorrent_client_gui_session.inl
     bittorrent_client_gui_tools.cpp
     bittorrent_client_gui_tools.h
#     bittorrent_client_message.cpp
#     bittorrent_client_message.h
     bittorrent_client_network.h
#     bittorrent_client_sessionmessage.cpp
#     bittorrent_client_sessionmessage.h
     bittorrent_client_signalhandler.cpp
     bittorrent_client_signalhandler.h
#     bittorrent_client_stream.cpp
#     bittorrent_client_stream.h
     bittorrent_client_stream_common.h
     bittorrent_client_tools.cpp
     bittorrent_client_tools.h)

#set_source_files_properties (${bittorrent_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

set (EXECUTABLE_GUI bittorrent_client_gui)
add_executable (${EXECUTABLE_GUI} ${bittorrent_client_gui_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_GUI} Common)
add_dependencies (${EXECUTABLE_GUI} CommonUI)
add_dependencies (${EXECUTABLE_GUI} ACEStream)
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_BitTorrent)
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_GUI} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_GUI} rt)
 target_link_libraries (${EXECUTABLE_GUI} ${PKG_CAP_LIBRARIES})
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_GUI} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_GUI} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/pango-1.0.lib)
endif ()
target_link_libraries (${EXECUTABLE_GUI} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_GUI} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE_GUI} CommonUI)
target_link_libraries (${EXECUTABLE_GUI} ACEStream)
target_link_libraries (${EXECUTABLE_GUI} ACENetwork)
target_link_libraries (${EXECUTABLE_GUI} ACENetwork_Protocol_BitTorrent)
target_link_libraries (${EXECUTABLE_GUI} ACENetwork_Protocol_HTTP)

# add PCH
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE} ${EXECUTABLE_GUI})

install (TARGETS ${EXECUTABLE_GUI}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES
         bittorrent_client.ini
         bittorrent_client.rc
         bittorrent_client_main.glade
         bittorrent_client_session.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

