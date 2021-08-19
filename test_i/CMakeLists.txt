include (Threads)

include (SSLConfig)
include (NetlinkConfig)

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

 if (CURSES_USE)
 elseif (GTK_USE)
  include (SetGTKVersion)

  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions (-DG_ERRORCHECK_MUTEXES)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 elseif (WXWIDGETS_USE)
  message (STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
  message (STATUS "wxWidgets_DEFINITIONS_DEBUG: ${wxWidgets_DEFINITIONS_DEBUG}")
  message (STATUS "wxWidgets_CXX_FLAGS: ${wxWidgets_CXX_FLAGS}")
  message (STATUS "wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
# # *NOTE*: re-enable ANSI mode, iff available
# add_definitions (-DwxUSE_UNICODE=0)
  foreach (DEFINITION ${wxWidgets_DEFINITIONS})
   add_definitions (-D${DEFINITION})
  endforeach ()
  if (WIN32)
#  add_definitions (-D__WXMSW__)
#  add_definitions (-DWXUSINGDLL)
   if (MSVC)
    add_definitions (-DwxMSVC_VERSION_AUTO)
   endif (MSVC)
  endif (WIN32)
  add_definitions (-DwxUSE_GUI=1)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions (-D__WXDEBUG__)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 endif ()
endif (GUI_SUPPORT)

add_library (${PROJECT_NAME}_Test_I STATIC "")

if (UNIX)
 if (GUI_SUPPORT)
  if (CURSES_USE)
# *WORKAROUND*
   add_definitions (-DNCURSES_INTERNALS)
  endif (CURSES_USE)
 endif (GUI_SUPPORT)
elseif (WIN32)
endif ()

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

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 if (DHCLIENT_SUPPORT)
#  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
#                              $<BUILD_INTERFACE:/usr/include/bind-export>)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC ${BIND9_CFLAGS})
 endif (DHCLIENT_SUPPORT)

 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>)
  elseif (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")

   if (GTKGLAREA_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${GTKGLAREA_INCLUDES}>)
   endif (GTKGLAREA_SUPPORT)

   if (LIBGLADE_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${PKG_GLADE_INCLUDE_DIRS}>)
   endif (LIBGLADE_SUPPORT)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
#                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses>)
  elseif (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>)

   if (GTKGLAREA_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${GTKGLAREA_INCLUDES}>)
   endif (GTKGLAREA_SUPPORT)

# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/cmake>)
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
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
endif ()
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/cmake>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src/modules/lib>)

target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)

if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_USE)
   if (GTK3_FOUND)
    target_link_libraries (${PROJECT_NAME}_Test_I ${GTK3_LIBRARIES})
   elseif (GTK2_FOUND)
    target_link_libraries (${PROJECT_NAME}_Test_I ${GTK2_LIBRARIES})
   elseif (GTK_FOUND)
    target_link_libraries (${PROJECT_NAME}_Test_I ${GTK_LIBRARIES})
   elseif (GTK2_USE)
    target_link_libraries (${PROJECT_NAME}_Test_I ${PKG_GTK2_LIBRARIES})
   elseif (GTK3_USE)
    target_link_libraries (${PROJECT_NAME}_Test_I ${PKG_GTK3_LIBRARIES})
   endif ()

   if (GTKGL_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${wxWidgets_LIBRARIES})

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_wxWidgets)
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_I CommonUI)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${CURSES_LIBRARY})
   target_link_libraries (${PROJECT_NAME}_Test_I ${PANEL_LIBRARY})
  elseif (GTK_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${GTK_LIBRARIES})
   if (GTKGL_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I OpenGL32.lib)

    target_link_libraries (${PROJECT_NAME}_Test_I CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${wxWidgets_LIBRARIES})
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_I CommonUI)
 endif (GUI_SUPPORT)
endif ()

##########################################

add_dependencies (${PROJECT_NAME}_Test_I Common)
if (GUI_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI)
 if (GTK_USE)
  add_dependencies (${PROJECT_NAME}_Test_I CommonUI_GTK)
  if (GTKGL_SUPPORT)
   add_dependencies (${PROJECT_NAME}_Test_I CommonOpenGL)
  endif (GTKGL_SUPPORT)
 elseif (WXWIDGETS_USE)
  add_dependencies (${PROJECT_NAME}_Test_I CommonUI_wxWidgets)
 endif (GTK_USE)
endif (GUI_SUPPORT)

##########################################

#if (WIN32)
# add_definitions (-DWIN32)
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
# else ()
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
# endif ()
#endif ()

set (source_files
     test_i_common.h
     test_i_defines.h
     test_i_stream_common.h)
if (GUI_SUPPORT)
 if (CURSES_USE)
  set (source_files
       ${source_files}
       test_i_curses_defines.h)
 elseif (GTK_USE)
  set (source_files
       ${source_files}
       test_i_gtk_common.h
       test_i_gtk_eventhandler.cpp
       test_i_gtk_eventhandler.h
       test_i_gtk_eventhandler.inl)
 elseif (WXWIDGETS_USE)
  set (source_files
       ${source_files}
       test_i_wxwidgets_common.h)
 endif ()
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
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

# ----------------------------------------

add_subdirectory (bittorrent_client)
add_subdirectory (IRC_client)
add_subdirectory (url_stream_load)

add_custom_target (test_i ALL
                   COMMENT "building test_i..."
                   SOURCES ${ACENetwork_Test_I_SRCS})
add_dependencies (test_i bittorrent_client bittorrent_client_gui IRC_client IRC_client_gui url_stream_load)

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
