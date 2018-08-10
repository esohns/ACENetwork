option (BUILD_TEST_U "build test_u" OFF)
option (GTK_SUPPORT "enable GTK support" ON)
# option (GTK3_SUPPORT "enable Gtk3 support" ON)
 # *NOTE*: implies DHCLIENT_SUPPORT off
include (CMakeDependentOption)
CMAKE_DEPENDENT_OPTION (GTK3_SUPPORT "enable GTK 3 support" ON
                        "GTK_SUPPORT" OFF)
CMAKE_DEPENDENT_OPTION (GTK2_SUPPORT "enable GTK 2 support" ON
                        "GTK_SUPPORT; NOT GTK3_SUPPORT" OFF)
CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                        "GTK_SUPPORT" OFF)

add_library (${PROJECT_NAME}_Test_U STATIC "")

if (UNIX)
 if (GTK_SUPPORT)
  if (GTK2_SUPPORT)
   pkg_check_modules (PKG_GTK REQUIRED gmodule-2.0 gthread-2.0 gtk+-2.0)
  elseif (GTK3_SUPPORT)
   pkg_check_modules (PKG_GTK REQUIRED gtk+-3.0)
  endif ()
 endif ()
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${PKG_GTK_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/timer>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui>)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui/gtk>)
 endif ()
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACEStream/src>)
elseif (WIN32)
# gtk
 if (GTK_SUPPORT)
  if (GTK2_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/atk-1.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gdk-pixbuf-2.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/cairo>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/pango-1.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/glib-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/glib-2.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/gtk-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gtk-2.0>)
  elseif (GTK3_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/atk-1.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gdk-pixbuf-2.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/cairo>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/pango-1.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/glib-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/glib-2.0>)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gtk-3.0>)
  endif ()
 endif (GTK_SUPPORT)
# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)

# include_directories ($ENV{ACE_ROOT})
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ATCD/ACE>)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/timer>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui>)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/gtk>)
 endif (GTK_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src>)
endif ()
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)

if (WIN32)
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gio-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/glib-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gobject-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gthread-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gdk_pixbuf-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gdk-3.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gtk-3.0.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/pango-1.0.lib)
  else ()
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgio-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libglib-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgobject-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgthread-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgdk-win32-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgdk_pixbuf-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgtk-win32-2.0.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libcairo.dll.a)
   target_link_libraries (${PROJECT_NAME}_Test_U ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libpango-1.0.dll.a)
 #  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea/gtkgl/.libs/libgtkglarea-win32-1.0.dll.a)
  endif (GTK3_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U OpenGL32.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U "C:/Documents and Settings/Erik/My Documents/Visual Studio 2010/Projects/gtkglarea/${CMAKE_BUILD_TYPE}/gtkglarea.lib")
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (WIN32)

set (source_files
     test_u_common.h
     test_u_connection_common.h
     test_u_defines.h
     test_u_socket_common.h
     test_u_stream_common.h)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      test_u_gtk_common.h
      test_u_gtk_eventhandler.cpp
      test_u_gtk_eventhandler.h
      test_u_gtk_eventhandler.inl)
endif ()
#install (FILES
#         ${PROJECT_NAME}_Test_U_GTK_SRCS
#         DESTINATION include)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Test_U
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
# else ()
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
# endif ()
endif ()

# ----------------------------------------

add_subdirectory (client_server)
add_subdirectory (DHCP_client)
add_subdirectory (file_server)
add_subdirectory (HTTP_decoder)
add_subdirectory (WLAN_monitor)

add_custom_target (test_u ALL
                   COMMENT "building test_u...")
add_dependencies (test_u client server DHCP_client file_server HTTP_decoder WLAN_monitor)

cmake_policy (SET CMP0012 NEW)
if (NOT BUILD_TEST_U)
 set_target_properties (test_u PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_u PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (WIN32)
endif (NOT BUILD_TEST_U)

# ----------------------------------------

# enable (unit/integration) tests
#include (CTest)
enable_testing ()
