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
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
endif ()
# *TODO*: repair WIN32 pkgconfig support

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
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/misc)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/misc)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (client_SRCS
     net_client.cpp
     net_client_common.h
     net_client_connector_common.h
     net_client_signalhandler.cpp
     net_client_signalhandler.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     test_u_callbacks.cpp
     test_u_callbacks.h
     test_u_common.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_defines.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_message.cpp
     test_u_message.h
     test_u_module_eventhandler.cpp
     test_u_module_eventhandler.h
     test_u_module_headerparser.cpp
     test_u_module_headerparser.h
     test_u_module_protocolhandler.cpp
     test_u_module_protocolhandler.h
     test_u_sessionmessage.cpp
     test_u_sessionmessage.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_socket_common.h
     test_u_stream_common.h
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     stdafx.cpp
     stdafx.h)

set (EXECUTABLE client)
add_executable (${EXECUTABLE} ${client_SRCS} etc/net_client.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME} Common)
add_dependencies (${PROJECT_NAME} ACEStream)
add_dependencies (${PROJECT_NAME} ACENetwork)
add_dependencies (${PROJECT_NAME} ACENetwork_Client)

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
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/net_client.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_u COMPONENT config)

########### next target ###############
set (server_SRCS
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     net_server.cpp
     net_server_common.h
     net_server_listener_common.h
     net_server_signalhandler.cpp
     net_server_signalhandler.h
     test_u_callbacks.cpp
     test_u_callbacks.h
     test_u_common.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_defines.h
     test_u_message.cpp
     test_u_message.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_module_eventhandler.cpp
     test_u_module_eventhandler.h
     test_u_module_headerparser.cpp
     test_u_module_headerparser.h
     test_u_module_protocolhandler.cpp
     test_u_module_protocolhandler.h
     test_u_sessionmessage.cpp
     test_u_sessionmessage.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_socket_common.h
     test_u_stream_common.h
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     stdafx.cpp
     stdafx.h)

unset (EXECUTABLE)
set (EXECUTABLE server)
add_executable (${EXECUTABLE} ${server_SRCS} etc/net_server.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME} Common)
add_dependencies (${PROJECT_NAME} ACEStream)
add_dependencies (${PROJECT_NAME} ACENetwork)
add_dependencies (${PROJECT_NAME} ACENetwork_Server)

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
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Server)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/net_server.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_u COMPONENT config)
