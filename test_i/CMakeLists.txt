# configure C/C++ exceptions
include (SetCppExceptions)

include (Threads)

include (NetlinkConfig)
include (SSLConfig)

include (ACE)

option (BUILD_TEST_I "build test_i" OFF)

# user interface
option (GUI_SUPPORT "enable graphical user interface support" ON)
if (GUI_SUPPORT)
 add_definitions (-DGUI_SUPPORT)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts")
# include (Macros)
 include (PreUIFramework)
 include (SetUIFramework)
 if (GTK_USE)
  include (SetGTKVersion)
 endif (GTK_USE)
endif (GUI_SUPPORT)

if (UNIX)
 add_link_options ("LINKER:--as-needed")
endif (UNIX)

add_library (${PROJECT_NAME}_Test_I STATIC "")

if (UNIX)
 if (GUI_SUPPORT)
  if (CURSES_USE)
# *WORKAROUND*
   add_definitions (-DNCURSES_INTERNALS)
  endif (CURSES_USE)
 endif (GUI_SUPPORT)
endif (UNIX)

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
add_definitions (-DCOMMON_HAS_DLL)

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

if (GUI_SUPPORT)
 if (CURSES_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>)

  if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${GTKGLAREA_INCLUDES}>)
  endif (GTKGLAREA_SUPPORT)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  if (WIN32)
   cmake_policy (SET CMP0020 OLD)
  endif (WIN32)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${Qt_INCLUDE_DIRS}>)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/timer>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui>)
  if (CURSES_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/curses>)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/gtk>)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/qt>)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/wxwidgets>)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/src/modules/lib>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (CURSES_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/curses>)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/qt>)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src/modules/lib>)
endif ()
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)

target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if (GUI_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI)
 if (CURSES_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${CURSES_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_Curses)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${GTK_LIBRARIES})
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonOpenGL)
  endif (GTKGL_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_GTK)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${Qt_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_Qt)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${wxWidgets_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_wxWidgets)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)
target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ACEStream)

##########################################

add_dependencies (${PROJECT_NAME}_Test_I Common)
if (GUI_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI)
 if (GTK_SUPPORT)
  add_dependencies (${PROJECT_NAME}_Test_I CommonUI_GTK)
  if (GTKGL_SUPPORT)
   add_dependencies (${PROJECT_NAME}_Test_I CommonOpenGL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  add_dependencies (${PROJECT_NAME}_Test_I CommonUI_wxWidgets)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)
add_dependencies (${PROJECT_NAME}_Test_I ACEStream)

##########################################

set (source_files
     test_i_common.h
     test_i_configuration.h
     test_i_connection_common.h
     test_i_defines.h
     test_i_stream_common.h)
if (GUI_SUPPORT)
 if (CURSES_SUPPORT)
  set (source_files
       ${source_files}
       test_i_curses_defines.h)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  set (source_files
       ${source_files}
       test_i_gtk_common.h
       test_i_gtk_eventhandler.cpp
       test_i_gtk_eventhandler.h
       test_i_gtk_eventhandler.inl)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  set (source_files
       ${source_files}
       test_i_qt_common.h)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (source_files
       ${source_files}
       test_i_wxwidgets_common.h)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)
#install (FILES
#         ${PROJECT_NAME}_Test_I_GTK_SRCS
#         DESTINATION include)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Test_I
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Test_I
                           PUBLIC stdafx.h)

##########################################

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Test_I
       APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# ----------------------------------------

add_subdirectory (av_stream)
add_subdirectory (bittorrent_client)
add_subdirectory (FTP_client)
add_subdirectory (IRC_client)
add_subdirectory (trending)
add_subdirectory (url_stream_load)
add_subdirectory (web_tv)

add_custom_target (test_i ALL
                   COMMENT "building test_i..."
                   SOURCES ${ACENetwork_Test_I_SRCS})
add_dependencies (test_i av_stream_client av_stream_server bittorrent_client bittorrent_client_gui ftp_client IRC_client IRC_client_gui trending url_stream_load web_tv)
if (WIN32)
 add_dependencies (test_i av_stream_ds_source av_stream_mf_source)
endif (WIN32)

if (NOT BUILD_TEST_I)
 set_target_properties (test_i PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_i PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (WIN32)
endif (NOT BUILD_TEST_I)

# ----------------------------------------

# enable (unit/integration) tests
#include (CTest)
enable_testing ()
