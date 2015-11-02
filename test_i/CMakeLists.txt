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
pkg_check_modules (PKG_NCURSES REQUIRED ncurses panel)
pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)

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
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/ATCD/ACE)
 include_directories (${PKG_NCURSES_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACEStream/src)
else ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ATCD/ACE)
 include_directories (${PKG_NCURSES_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../src/client_server)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

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
     IRC_client_iIRCControl.h
     IRC_client_inputhandler.cpp
     IRC_client_inputhandler.h
     IRC_client_IRC_codes.h
     IRC_client_IRCbisect.cpp
     IRC_client_IRCbisect.h
     IRC_client_IRCmessage.cpp
     IRC_client_IRCmessage.h
     IRC_client_IRCparser.cpp
     IRC_client_IRCparser.h
     IRC_client_IRCparser_driver.cpp
     IRC_client_IRCparser_driver.h
     IRC_client_IRCscanner.cpp
     IRC_client_IRCscanner.h
     IRC_client_IRCsession.cpp
     IRC_client_IRCsession.h
     IRC_client_IRCsession.inl
     IRC_client_message.cpp
     IRC_client_message.h
     IRC_client_messageallocator.cpp
     IRC_client_messageallocator.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_module_IRCparser.cpp
     IRC_client_module_IRCparser.h
     IRC_client_module_IRCsplitter.cpp
     IRC_client_module_IRCsplitter.h
     IRC_client_module_IRCstreamer.cpp
     IRC_client_module_IRCstreamer.h
     IRC_client_network.h
     IRC_client_session_common.h
     IRC_client_sessionmessage.cpp
     IRC_client_sessionmessage.h
     IRC_client_signalhandler.cpp
     IRC_client_signalhandler.h
     IRC_client_statemachine_registration.cpp
     IRC_client_statemachine_registration.h
     IRC_client_stream.cpp
     IRC_client_stream.h
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h
     IRC_common.h
     Streambuf_alt.cpp
     Streambuf_alt.h
     Streambuf_alt.inl
     stdafx.cpp
     stdafx.h)

#set_source_files_properties(${IRC_client_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

add_executable(IRC_client ${IRC_client_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (IRC_client ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (IRC_client ${ACE_LIBRARY})
target_link_libraries (IRC_client ${PKG_NCURSES_LIBRARIES})
target_link_libraries (IRC_client Common)
target_link_libraries (IRC_client ACEStream)
target_link_libraries (IRC_client ACENetwork)
target_link_libraries (IRC_client ACENetwork_Client)

install (TARGETS IRC_client
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
#install (FILES etc/net_client.glade
#         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

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
     IRC_client_iIRCControl.h
     IRC_client_IRC_codes.h
     IRC_client_IRCbisect.cpp
     IRC_client_IRCbisect.h
     IRC_client_IRCmessage.cpp
     IRC_client_IRCmessage.h
     IRC_client_IRCparser.cpp
     IRC_client_IRCparser.h
     IRC_client_IRCparser_driver.cpp
     IRC_client_IRCparser_driver.h
     IRC_client_IRCscanner.cpp
     IRC_client_IRCscanner.h
     IRC_client_message.cpp
     IRC_client_message.h
     IRC_client_messageallocator.cpp
     IRC_client_messageallocator.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_module_IRCparser.cpp
     IRC_client_module_IRCparser.h
     IRC_client_module_IRCsplitter.cpp
     IRC_client_module_IRCsplitter.h
     IRC_client_module_IRCstreamer.cpp
     IRC_client_module_IRCstreamer.h
     IRC_client_network.h
     IRC_client_sessionmessage.cpp
     IRC_client_sessionmessage.h
     IRC_client_signalhandler.cpp
     IRC_client_signalhandler.h
     IRC_client_statemachine_registration.cpp
     IRC_client_statemachine_registration.h
     IRC_client_stream.cpp
     IRC_client_stream.h
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h
     IRC_common.h
     stdafx.cpp
     stdafx.h)

#set_source_files_properties (${IRC_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

add_executable (IRC_client_gui ${IRC_client_gui_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (IRC_client_gui ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (IRC_client_gui ${ACE_LIBRARY})
target_link_libraries (IRC_client_gui ${PKG_GTK_GLADE_LIBRARIES})
target_link_libraries (IRC_client_gui Common)
target_link_libraries (IRC_client_gui CommonUI)
target_link_libraries (IRC_client_gui ACEStream)
target_link_libraries (IRC_client_gui ACENetwork)
target_link_libraries (IRC_client_gui ACENetwork_Client)

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
