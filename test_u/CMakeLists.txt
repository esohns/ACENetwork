# configure C/C++ exceptions
include (SetCppExceptions)

include (Threads)

if (VCPKG_USE)
 include (ImageMagick)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/gl/scripts")
 include (glew)
endif (VCPKG_USE)
include (ffmpeg)

include (NetlinkConfig)
include (SSLConfig)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/GeoIp2_client/scripts")
include (geoip2)

include (ACE)

option (BUILD_TEST_U "build test_u" OFF)

# user interface
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts")
# include (Macros)
include (PreUIFramework)
include (SetUIFramework)
if (GTK_SUPPORT)
 include (SetGTKVersion)
endif (GTK_SUPPORT)

if (UNIX)
 add_link_options ("LINKER:--as-needed")
endif (UNIX)

add_library (${PROJECT_NAME}_Test_U
             STATIC "")
set_property (TARGET ${PROJECT_NAME}_Test_U PROPERTY FOLDER "test_u")

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
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

if (CURSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>)
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")

 if (LIBGLADE_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${LIBGLADE_INCLUDES}>)
 endif (LIBGLADE_SUPPORT)

 if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${GTKGLAREA_INCLUDES}>)
 endif (GTKGLAREA_SUPPORT)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
# if (WIN32)
#  cmake_policy (SET CMP0020 OLD)
# endif (WIN32)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${QT_INCLUDE_DIRS}>)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
endif (WXWIDGETS_SUPPORT)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui>)
 if (CURSES_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/curses>)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/qt>)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/wxwidgets>)
 endif (WXWIDGETS_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../ACEStream/src/modules/lib>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
 if (CURSES_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/curses>)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/qt>)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
 endif (WXWIDGETS_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src/modules/lib>)
endif ()
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)

target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonUI)
if (CURSES_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${CURSES_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonUI_Curses)
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${GTK_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonUI_GTK)
 if (GTKGL_SUPPORT)
  if (GTKGLAREA_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${GTKGLAREA_LIBRARY})
  endif (GTKGLAREA_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonOpenGL)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${QT_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonUI_Qt)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${wxWidgets_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC CommonUI_wxWidgets)
endif (WXWIDGETS_SUPPORT)
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC X11)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC GL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
elseif (WIN32)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC OpenGL32.lib)
#    target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../libepoxy/cmake/${CMAKE_BUILD_TYPE}/epoxy1_static.lib)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif ()

target_link_libraries (${PROJECT_NAME}_Test_U PUBLIC ACEStream)

##########################################

add_dependencies (${PROJECT_NAME}_Test_U Common)
add_dependencies (${PROJECT_NAME}_Test_U CommonUI)
if (GTK_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_U CommonUI_GTK)
 if (GTKGL_SUPPORT)
  add_dependencies (${PROJECT_NAME}_Test_U CommonOpenGL)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_U CommonUI_wxWidgets)
endif (WXWIDGETS_SUPPORT)
add_dependencies (${PROJECT_NAME}_Test_U ACEStream)

##########################################

set (source_files
     test_u_common.h
     test_u_common_tools.cpp
     test_u_common_tools.h
     test_u_common_tools.inl
     test_u_connection_common.h
     test_u_defines.h
     test_u_network_common.h
     test_u_stream_common.h)
if (CURSES_SUPPORT)
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      test_u_gtk_common.h
      test_u_gtk_eventhandler.cpp
      test_u_gtk_eventhandler.h
      test_u_gtk_eventhandler.inl)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (source_files
      ${source_files}
      test_u_wxwidgets_common.h)
endif (WXWIDGETS_SUPPORT)
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
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Test_U
                           PUBLIC stdafx.h)

if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
# else ()
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
# endif ()
endif ()

##########################################

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Test_U
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# ----------------------------------------

add_custom_target (test_u ALL
                   COMMENT "building test_u...")
set_property (TARGET test_u PROPERTY FOLDER "test_u")

if (BUILD_TEST_U)
 add_subdirectory (client_server)
 add_subdirectory (DHCP_client)
 add_subdirectory (file_server)
 if (GEOIP2_SUPPORT)
  add_subdirectory (GeoIp2_client)
 endif (GEOIP2_SUPPORT)
 add_subdirectory (HTTP_decoder)
 add_subdirectory (PCP_client)
 add_subdirectory (UPnP_client)
 add_subdirectory (WLAN_monitor)

 add_dependencies (test_u client server server_2 http_decoder wlan_monitor)
 add_dependencies (test_u dhcp_client file_server pcp_client upnp_client)
 if (GEOIP2_SUPPORT)
  add_dependencies (test_u geoip2_client)
 endif (GEOIP2_SUPPORT)
endif (BUILD_TEST_U)

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
