cmake_minimum_required (VERSION 2.8)

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
                PATH_SUFFIXES build/linux/lib
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

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_CURSES REQUIRED ncurses panel)
 pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)

 if ((!PKG_CURSES) OR (!PKG_GTK_GLADE))
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

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ATCD/ACE)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
if (UNIX)
 include_directories (${PKG_NCURSES_INCLUDE_DIRS})
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
#include_directories (${Common_INCLUDE_DIRS})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/irc)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (IRC_client_SRCS
     FILE_Stream.cpp
     FILE_Stream.h
     FILE_Stream.inl
     IOStream_alt_macros.h
     IOStream_alt_T.cpp
     IOStream_alt_T.h
     IOStream_alt_T.inl
     IRC_client.cpp
     IRC_client_common.h
     IRC_client_common_modules.h
     IRC_client_configuration.h
     IRC_client_curses.cpp
     IRC_client_curses.h
     IRC_client_defines.h
     IRC_client_inputhandler.cpp
     IRC_client_inputhandler.h
     IRC_client_messageallocator.cpp
     IRC_client_messageallocator.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_network.h
     IRC_client_session_common.h
     IRC_client_sessionmessage.cpp
     IRC_client_sessionmessage.h
     IRC_client_signalhandler.cpp
     IRC_client_signalhandler.h
     IRC_client_stream.cpp
     IRC_client_stream.h
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h
     Streambuf_alt.cpp
     Streambuf_alt.h
     Streambuf_alt.inl
     stdafx.cpp
     stdafx.h)

#set_source_files_properties(${IRC_client_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

set (EXECUTABLE IRC_client)
add_executable(${EXECUTABLE} ${IRC_client_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_NCURSES_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ${CURSES_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PANEL_LIBRARY})
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Protocol_IRC)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

install (TARGETS IRC_client
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
#install (FILES etc/net_client.glade
#         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

################################################################################

if (UNIX)
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
elseif (WIN32)
 # *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/include/libglade-2.0)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)

########### next target ###############
set (IRC_client_gui_SRCS
     IRC_client_common.h
     IRC_client_common_modules.h
     IRC_client_configuration.h
     IRC_client_defines.h
     IRC_client_gui.cpp
     IRC_client_gui_callbacks.cpp
     IRC_client_gui_callbacks.h
     IRC_client_gui_common.h
     IRC_client_gui_connection.cpp
     IRC_client_gui_connection.h
     IRC_client_gui_defines.h
     IRC_client_gui_messagehandler.cpp
     IRC_client_gui_messagehandler.h
     IRC_client_gui_tools.cpp
     IRC_client_gui_tools.h
     IRC_client_messageallocator.cpp
     IRC_client_messageallocator.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_network.h
     IRC_client_sessionmessage.cpp
     IRC_client_sessionmessage.h
     IRC_client_signalhandler.cpp
     IRC_client_signalhandler.h
     IRC_client_stream.cpp
     IRC_client_stream.h
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h
     stdafx.cpp
     stdafx.h)

#set_source_files_properties (${IRC_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

unset (EXECUTABLE)
set (EXECUTABLE IRC_client_gui)
add_executable (${EXECUTABLE} ${IRC_client_gui_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gobject-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-win32-2.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
# *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Protocol_IRC)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

install (TARGETS IRC_client_gui
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES
         IRC_client.ini
         IRC_client_channel_tab.glade
         IRC_client_main.glade
         IRC_client_server_page.glade
         servers.ini
         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)
