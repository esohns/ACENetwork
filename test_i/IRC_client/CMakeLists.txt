option (GTK3_SUPPORT "enable Gtk3 support" ON)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
# check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
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

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
# pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
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

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
#message (STATUS "BUILD_PATH_SUFFIX; ${BUILD_PATH_SUFFIX}")
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

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

#add_definitions (-DHAVE_CONFIG_H)
#add_definitions (-DIRC_HAS_DLL)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 # *WORKAROUND*
 add_definitions (-DNCURSES_INTERNALS)
 include_directories (${PKG_CURSES_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/stat)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/stat)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/irc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (IRC_client_SRCS
     stdafx.cpp
     stdafx.h
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
#     IRC_client_messageallocator.cpp
#     IRC_client_messageallocator.h
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
     IRC_client_stream.inl
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h
     Streambuf_alt.cpp
     Streambuf_alt.h
     Streambuf_alt.inl)
set (IRC_client_INI
     etc/IRC_client.ini
     etc/IRC_client_phonebook.ini)

#set_source_files_properties(${IRC_client_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

set (EXECUTABLE IRC_client)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${IRC_client_SRCS}
                ${IRC_client_INI})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} CommonUI)
add_dependencies (${EXECUTABLE} CommonUI_GTK)

add_dependencies (${EXECUTABLE} ACEStream)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 #target_link_libraries (${EXECUTABLE} ${PKG_DBUS_LIBRARIES})
 target_link_libraries (${EXECUTABLE} iw)
 target_link_libraries (${EXECUTABLE} ${PKG_CURSES_LIBRARIES})
 target_link_libraries (${EXECUTABLE} X11)
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ntdll.lib)
 target_link_libraries (${EXECUTABLE} ${CURSES_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PANEL_LIBRARY})

 target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonTimer)
#target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} CommonUI_GTK)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACEStream_Network)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
#target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Client)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_IRC)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES
         etc/IRC_client.ini
         etc/IRC_client_phonebook.ini
         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

################################################################################

if (UNIX)
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui/gtk)
elseif (WIN32)
 # *TODO*: repair pkgconfig support
# gtk
 if (GTK3_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gtk-3.0)
 else ()
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/gtk-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gtk-2.0)
 endif ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui/gtk)
endif ()

########### next target ###############
set (IRC_client_gui_SRCS
     stdafx.cpp
     stdafx.h
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
#     IRC_client_messageallocator.cpp
#     IRC_client_messageallocator.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_network.h
     IRC_client_sessionmessage.cpp
     IRC_client_sessionmessage.h
     IRC_client_signalhandler.cpp
     IRC_client_signalhandler.h
     IRC_client_stream.cpp
     IRC_client_stream.h
     IRC_client_stream.inl
     IRC_client_stream_common.h
     IRC_client_tools.cpp
     IRC_client_tools.h)

#set_source_files_properties (${IRC_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

set (EXECUTABLE_GUI IRC_client_gui)
add_executable (${EXECUTABLE_GUI}
                EXCLUDE_FROM_ALL
                ${IRC_client_gui_SRCS}
                ${IRC_client_INI}
                etc/IRC_client_channel.glade
                etc/IRC_client_connection.glade
                etc/IRC_client_main.glade)
set_property (TARGET ${EXECUTABLE_GUI} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE_GUI} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_GUI} Common)
add_dependencies (${EXECUTABLE_GUI} CommonTimer)
add_dependencies (${EXECUTABLE_GUI} CommonUI)
add_dependencies (${EXECUTABLE_GUI} CommonUI_GTK)

add_dependencies (${EXECUTABLE_GUI} ACEStream)

add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_GUI} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_GUI} rt)
 target_link_libraries (${EXECUTABLE_GUI} ${PKG_CAP_LIBRARIES})
 #target_link_libraries (${EXECUTABLE_GUI} ${PKG_DBUS_LIBRARIES})
 target_link_libraries (${EXECUTABLE_GUI} iw)
 target_link_libraries (${EXECUTABLE_GUI} X11)
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_GUI} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_GUI} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_GUI} ntdll.lib)

# *TODO*: repair WIN32 pkgconfig support
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
 else ()
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgio-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libglib-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgobject-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgthread-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgtk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libpango-1.0.dll.a)
 endif ()

 target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
endif ()
target_link_libraries (${EXECUTABLE_GUI} Common)
target_link_libraries (${EXECUTABLE_GUI} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_GUI} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
#target_link_libraries (${EXECUTABLE_GUI} CommonUI)
target_link_libraries (${EXECUTABLE_GUI} CommonUI_GTK)

target_link_libraries (${EXECUTABLE_GUI} ACEStream)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Network)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME})
#target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME}_Client)
target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_IRC)

# add PCH
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE_GUI})

install (TARGETS ${EXECUTABLE_GUI}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES
         etc/IRC_client.ini
         etc/IRC_client_phonebook.ini
         etc/IRC_client_channel.glade
         etc/IRC_client_connection.glade
         etc/IRC_client_main.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/protocol/irc/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/protocol/irc/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
endif ()
