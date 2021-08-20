if (UNIX)
 if (NETLINK_SUPPORT)
  include_directories (${PKG_LIBNL_INCLUDE_DIRS})
  add_definitions (-DNETLINK_SUPPORT)
 endif (NETLINK_SUPPORT)
endif (UNIX)

if (UNIX)
# *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
 if (NETLINK_SUPPORT)
  pkg_check_modules (PKG_LIBNL REQUIRED libnl-3.0)
 endif (NETLINK_SUPPORT)
# pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
endif (UNIX)

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
add_definitions (-DCOMMON_HAS_DLL)
add_definitions (-DCOMMON_UI_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DACESTREAM_HAS_DLL)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()
add_definitions (-DACENETWORK_HAS_DLL)

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
# include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (NETLINK_SUPPORT)
  include_directories (${PKG_LIBNL_INCLUDE_DIRS})
 endif (NETLINK_SUPPORT)
# include_directories (${PKG_DBUS_INCLUDE_DIRS})

# include_directories (/usr/local/src/ACE_wrappers)
 include_directories ($ENV{LIB_ROOT}/ACE_TAO/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

# include_directories ($ENV{ACE_ROOT})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 # *TODO*: repair pkgconfig support
# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gtk-3.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (EXECUTABLE_SOURCE client)
set (${EXECUTABLE_SOURCE}_SRCS
     net_client.cpp
     net_client_common.h
     net_client_connector_common.h
     net_client_server_defines.h
     net_client_signalhandler.cpp
     net_client_signalhandler.h
     net_client_stream_common.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     stdafx.cpp
     stdafx.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
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
     test_u_stream.inl
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     test_u_udpconnection.cpp
     test_u_udpconnection.h)
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE_SOURCE}_SRCS
       ${${EXECUTABLE_SOURCE}_SRCS}
       test_u_callbacks.cpp
       test_u_callbacks.h)
 elseif (WXWIDGETS_USE)
  set (${EXECUTABLE_SOURCE}_SRCS
       ${${EXECUTABLE_SOURCE}_SRCS}
       net_server_wxui_base.cpp
       net_server_wxui_base.h)
 endif ()
endif (GUI_SUPPORT)

add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SOURCE}_SRCS}
                etc/net_client.glade)
set_property (TARGET ${EXECUTABLE_SOURCE} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_SOURCE} Common)
add_dependencies (${EXECUTABLE_SOURCE} CommonUI)
#add_dependencies (${EXECUTABLE_SOURCE} CommonUI_GTK)
add_dependencies (${EXECUTABLE_SOURCE} ACEStream)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Client)

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} X11)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} ${SSL_LIBRARIES})

 target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE_SOURCE} Common)
target_link_libraries (${EXECUTABLE_SOURCE} CommonLog)
target_link_libraries (${EXECUTABLE_SOURCE} CommonSignal)
target_link_libraries (${EXECUTABLE_SOURCE} CommonTimer)

if (UNIX)
# set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES LINK_FLAGS -nostartfiles)
elseif (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()

target_link_libraries (${EXECUTABLE_SOURCE} ACEStream)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Network)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
#target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_WLAN)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Test_U)

#target_link_libraries (${EXECUTABLE_SOURCE} Common)

# add PCH
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE_SOURCE})

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/net_client.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_u COMPONENT config)

########### next target ###############
set (EXECUTABLE_TARGET server)
set (${EXECUTABLE_TARGET}_SRCS
     net_client_server_defines.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     net_server.cpp
     net_server_common.h
     net_server_listener_common.h
     net_server_signalhandler.cpp
     net_server_signalhandler.h
     net_server_stream_common.h
     stdafx.cpp
     stdafx.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_message.cpp
     test_u_message.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
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
     test_u_stream.inl
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     test_u_udpconnection.cpp
     test_u_udpconnection.h)
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE_TARGET}_SRCS
       ${${EXECUTABLE_TARGET}_SRCS}
       test_u_callbacks.cpp
       test_u_callbacks.h)
 elseif (WXWIDGETS_USE)
  set (${EXECUTABLE_TARGET}_SRCS
       ${${EXECUTABLE_TARGET}_SRCS}
       net_server_wxui.cpp
       net_server_wxui.h
       net_server_wxui_callbacks.cpp
       net_server_wxui_base.cpp
       net_server_wxui_base.h)
  list (REMOVE_ITEM ${EXECUTABLE_TARGET}_SRCS net_server.cpp)
 endif ()
endif (GUI_SUPPORT)

add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET}_SRCS}
                etc/net_server.glade)
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_TARGET} Common)
add_dependencies (${EXECUTABLE_TARGET} CommonLog)
add_dependencies (${EXECUTABLE_TARGET} CommonSignal)
add_dependencies (${EXECUTABLE_TARGET} CommonTimer)

add_dependencies (${EXECUTABLE_TARGET} ACEStream)

add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Server)

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} X11)
endif (UNIX)

target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})

if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} ${SSL_LIBRARIES})

 target_link_libraries (${EXECUTABLE_TARGET} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE_TARGET} Common)
target_link_libraries (${EXECUTABLE_TARGET} CommonLog)
target_link_libraries (${EXECUTABLE_TARGET} CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET} CommonTimer)

if (UNIX)
# set_target_properties (${EXECUTABLE_TARGET} PROPERTIES LINK_FLAGS -nostartfiles)
elseif (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()

target_link_libraries (${EXECUTABLE_TARGET} ACEStream)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Network)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Server)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Test_U)

# add PCH
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE_TARGET})

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/net_server.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_u COMPONENT config)

##########################################

if (SSL_USE)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_LIB_PATH})
endif (SSL_USE)

if (GUI_SUPPORT)
 file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
# message (STATUS "ui_definition_file: ${ui_definition_file}")
 string (REGEX REPLACE "\\\\" "/" ui_definition_file "${ui_definition_file}")
# message (STATUS "ui_definition_file: ${ui_definition_file}")
 set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")

 if (GTK_USE)
  if (GTK3_USE)
   file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk3/bin GTK_LIB_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             G:/software/Development/dll
                             ${GTK_LIB_PATH})
  elseif (GTK2_USE)
   file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             G:/software/Development/dll
                             ${GTK_LIB_PATH})
  endif ()
 elseif (WXWIDGETS_USE)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${wxWidgets_LIB_DIR})
 endif ()
else ()
 set (ARGS "-g ${ARGS}")
endif (GUI_SUPPORT)
#file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libpng/cmake/${CMAKE_BUILD_TYPE} LIBPNG_PATH)
#file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_PATH)
set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
#                          ${LIBPNG_PATH}
#                          ${FFMPEG_PATH}
#                          ${PROJECT_SOURCE_DIR}/../assimp/cmake/code/${CMAKE_BUILD_TYPE}
                          ${PROJECT_SOURCE_DIR}/../ACE_TAO/ACE/lib)

if (UNIX)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib
 #                                             ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/client_server
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug
                                              ${PROJECT_SOURCE_DIR}/modules/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/client_server
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH}
                           ${SSL_LIB_PATH}
                           ${GTK_LIB_PATH}
                           ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                           ${PROJECT_SOURCE_DIR}/../Common/cmake/src/err/Debug
                           ${PROJECT_SOURCE_DIR}/../Common/cmake/src/image/Debug
                           ${PROJECT_SOURCE_DIR}/../Common/cmake/src/math/Debug
                           ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                           ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/Debug
                           ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dec/Debug
                           ${PROJECT_SOURCE_DIR}/cmake/src/Debug)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/client_server
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/client_server
                         ENVIRONMENT foo=bar)
endif ()
