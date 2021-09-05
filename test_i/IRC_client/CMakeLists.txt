if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
# check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
endif ()

#include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: Common requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
# pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
# *TODO*: repair WIN32 pkgconfig support
elseif (WIN32)
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

#find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/..
#              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACENetwork_DIR})
# message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
#endif ()

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 # *WORKAROUND*
 if (GUI_SUPPORT)
  if (CURSES_USE)
   add_definitions (-DNCURSES_INTERNALS)
   include_directories (${PKG_CURSES_INCLUDE_DIRS})
  endif (CURSES_USE)
 endif (GUI_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/stat)
elseif (WIN32)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

# if (GUI_SUPPORT)
#  if (CURSES_USE)
#   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses)
#  endif (CURSES_USE)
# endif (GUI_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/irc)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

########### next target ###############
set (EXECUTABLE IRC_client)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
     stdafx.h
     FILE_Stream.cpp
     FILE_Stream.h
     FILE_Stream.inl
   #  IOStream_alt_macros.h
   #  IOStream_alt_T.cpp
   #  IOStream_alt_T.h
   #  IOStream_alt_T.inl
     IRC_client.cpp
     IRC_client_common.h
     IRC_client_common_modules.h
     IRC_client_configuration.h
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
if (GUI_SUPPORT)
 if (CURSES_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       IRC_client_curses.cpp
       IRC_client_curses.h)
 endif (CURSES_USE)
endif (GUI_SUPPORT)
set (${EXECUTABLE}_INI
     etc/IRC_client.ini
     etc/IRC_client_phonebook.ini)

#set_source_files_properties(${IRC_client_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${${EXECUTABLE}_INI})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_I)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonSignal)
add_dependencies (${EXECUTABLE} CommonTimer)

add_dependencies (${EXECUTABLE} ACEStream)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
#add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE} X11)
endif (UNIX)

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

if (WIN32)
# target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif (WIN32)
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACEStream_Network)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
if (UNIX)
 cotire (${EXECUTABLE})
endif (UNIX)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES
         etc/IRC_client.ini
         etc/IRC_client_phonebook.ini
         DESTINATION etc/lib${PROJECT_NAME}/test_i COMPONENT config)

########### next target ###############
set (EXECUTABLE_GUI IRC_client_gui)
set (${EXECUTABLE_GUI}_EXE_SRCS
     stdafx.cpp
     stdafx.h
     IRC_client_common.h
     IRC_client_common_modules.h
     IRC_client_configuration.h
     IRC_client_defines.h
     IRC_client_gui.cpp
     IRC_client_gui_common.h
     IRC_client_gui_connection.cpp
     IRC_client_gui_connection.h
     IRC_client_gui_defines.h
     IRC_client_gui_messagehandler.cpp
     IRC_client_gui_messagehandler.h
     IRC_client_gui_tools.cpp
     IRC_client_gui_tools.h
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
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE_GUI}_EXE_SRCS
       ${${EXECUTABLE_GUI}_EXE_SRCS}
       IRC_client_gui_callbacks.cpp
       IRC_client_gui_callbacks.h)
 elseif (WXWIDGETS_USE)
   #  set (${EXECUTABLE}_EXE_SRCS
   #       ${${EXECUTABLE}_EXE_SRCS}
   #       test_u_wxwidgets_ui.cpp
   #       test_u_wxwidgets_ui.h
   #       test_u_wxwidgets_ui.inl
   #       test_u_wxwidgets_ui_base.cpp
   #       test_u_wxwidgets_ui_base.h)
 endif ()
endif (GUI_SUPPORT)

#set_source_files_properties (${IRC_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

add_executable (${EXECUTABLE_GUI}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_GUI}_EXE_SRCS}
                ${${EXECUTABLE}_INI}
                etc/IRC_client_channel.glade
                etc/IRC_client_connection.glade
                etc/IRC_client_main.glade)
set_property (TARGET ${EXECUTABLE_GUI} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE_GUI} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_GUI} Common)
add_dependencies (${EXECUTABLE_GUI} CommonLog)
add_dependencies (${EXECUTABLE_GUI} CommonSignal)
add_dependencies (${EXECUTABLE_GUI} CommonTimer)

add_dependencies (${EXECUTABLE_GUI} ACEStream)

add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME})
#add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_WLAN)
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE_GUI} ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
 target_link_libraries (${EXECUTABLE_GUI} X11)
endif (UNIX)

target_link_libraries (${EXECUTABLE_GUI} ${ACE_LIBRARY})

if (WIN32)
 target_link_options (${EXECUTABLE_GUI} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif (WIN32)
target_link_libraries (${EXECUTABLE_GUI} Common)
target_link_libraries (${EXECUTABLE_GUI} CommonLog)
target_link_libraries (${EXECUTABLE_GUI} CommonSignal)
target_link_libraries (${EXECUTABLE_GUI} CommonTimer)

target_link_libraries (${EXECUTABLE_GUI} ACEStream)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Network)
target_link_libraries (${EXECUTABLE_GUI} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE_GUI} ${PROJECT_NAME}_Test_I)

# add PCH
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_GUI} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE_GUI})

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
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
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
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_LIB_PATH}
                           ${ACE_LIB_PATH}
                           ${GTK_LIB_PATH})
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/protocol/irc/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/protocol/irc/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
endif ()
