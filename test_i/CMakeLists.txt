include (CMakeDependentOption)
option (BUILD_TEST_I "build test_i" OFF)

# user interface
option (GUI_SUPPORT "enable graphical user interface support" ON)
if (GUI_SUPPORT)
 add_definitions (-DGUI_SUPPORT)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/scripts/Macros.cmake)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/scripts/PreUIFramework.cmake)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/scripts/SetUIFramework.cmake)

 if (WXWIDGETS_SUPPORT)
  message (STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
  message (STATUS "wxWidgets_DEFINITIONS_DEBUG: ${wxWidgets_DEFINITIONS_DEBUG}")
  message (STATUS "wxWidgets_CXX_FLAGS: ${wxWidgets_CXX_FLAGS}")
  message (STATUS "wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
  # *NOTE*: re-enable ANSI mode
  add_definitions (-DwxUSE_UNICODE=0)
  add_definitions (-D__WXMSW__)
  add_definitions (-DWXUSINGDLL)
  add_definitions (-DwxUSE_GUI=1)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions (-D__WXDEBUG__)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 endif (WXWIDGETS_SUPPORT)

 if (GTK_USE)
  include (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/scripts/SetGTKVersion.cmake)
  CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                          "GUI_SUPPORT" OFF)
# *IMPORTANT NOTE*: to use gtkglarea on gtk2, check out the 'gtkglarea-2' branch
#                   of the project
  CMAKE_DEPENDENT_OPTION (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF
                          "GUI_SUPPORT; GTKGL_SUPPORT" OFF)
 endif (GTK_USE)
endif (GUI_SUPPORT)

add_library (${PROJECT_NAME}_Test_I STATIC "")

include (FindPkgConfig)
if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK2_SUPPORT)
    pkg_check_modules (PKG_GTK REQUIRED gmodule-2.0 gthread-2.0 gtk+-2.0)
   elseif (GTK3_SUPPORT)
    pkg_check_modules (PKG_GTK REQUIRED gtk+-3.0)
   endif ()
  endif (GTK_SUPPORT)
  if (CURSES_SUPPORT)
   pkg_check_modules (PKG_CURSES REQUIRED ncurses panel)
# *WORKAROUND*
   add_definitions (-DNCURSES_INTERNALS)
  endif (CURSES_SUPPORT)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (CURSES_SUPPORT)
   find_library (CURSES_LIBRARY pdcurses.lib
                 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../pdcurses
                 PATH_SUFFIXES win32
                 DOC "searching for pdcurses.lib")
   if (NOT CURSES_LIBRARY)
    message (FATAL_ERROR "could not find pdcurses.lib, aborting")
   endif ()
   find_library (PANEL_LIBRARY panel.lib
                 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../pdcurses
                 PATH_SUFFIXES win32
                 DOC "searching for panel.lib")
   if (NOT PANEL_LIBRARY)
    message (FATAL_ERROR "could not find panel.lib, aborting")
   endif ()
  endif (CURSES_SUPPORT)
 endif (GUI_SUPPORT)
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${PKG_GTK_INCLUDE_DIRS}>)
  endif (GTK_SUPPORT)
  if (CURSES_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${PKG_CURSES_INCLUDE_DIRS}>)
  endif (CURSES_SUPPORT)

  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui>)
  if (GTK_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui/gtk>)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src>)
 #include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/timer)

 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui>)
  if (GTK_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui/gtk>)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK2_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/atk-1.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gdk-pixbuf-2.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/cairo>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/pango-1.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/glib-2.0/include>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/glib-2.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/gtk-2.0/include>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gtk-2.0>)
   elseif (GTK3_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/atk-1.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gdk-pixbuf-2.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/cairo>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/pango-1.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/glib-2.0/include>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/glib-2.0>)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gtk-3.0>)
   endif ()
# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)
  endif (GTK_SUPPORT)
  if (CURSES_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses>)
  endif (CURSES_SUPPORT)
 endif (GUI_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ATCD/ACE>)
 endif ()

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/cmake>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/error>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/timer>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui>)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/gtk>)
 endif (GTK_SUPPORT)
endif ()
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/cmake>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src/modules/lib>)

target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)

if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I ${PKG_GTK_LIBRARIES})
  endif (GTK_SUPPORT)
  if (CURSES_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I ${PKG_CURSES_LIBRARIES})
  endif (CURSES_SUPPORT)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (CURSES_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I ${CURSES_LIBRARY})
   target_link_libraries (${PROJECT_NAME}_Test_I ${PANEL_LIBRARY})
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK3_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gio-2.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/glib-2.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gobject-2.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gthread-2.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gdk_pixbuf-2.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gdk-win32-3.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/gtk-win32-3.0.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/pango-1.0.lib)
   elseif (GTK2_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgio-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libglib-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgobject-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgthread-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgdk-win32-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgdk_pixbuf-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libgtk-win32-2.0.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libcairo.dll.a)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/libpango-1.0.dll.a)
 #  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea/gtkgl/.libs/libgtkglarea-win32-1.0.dll.a)
   endif ()
   if (GTKGL_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I OpenGL32.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I ${CMAKE_CURRENT_SOURCE_DIR}/../../libepoxy/cmake/${CMAKE_BUILD_TYPE}/epoxy1_static.lib)
    target_link_libraries (${PROJECT_NAME}_Test_I "C:/Documents and Settings/Erik/My Documents/Visual Studio 2010/Projects/gtkglarea/${CMAKE_BUILD_TYPE}/gtkglarea.lib")

    target_link_libraries (${PROJECT_NAME}_Test_I CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_GTK)
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I ${wxWidgets_LIBRARIES})
  endif (WXWIDGETS_SUPPORT)

  target_link_libraries (${PROJECT_NAME}_Test_I CommonUI)
 endif (GUI_SUPPORT)
endif ()

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
     test_i_curses_defines.h
     test_i_defines.h
     test_i_stream_common.h)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      test_i_gtk_common.h
      test_i_gtk_eventhandler.cpp
      test_i_gtk_eventhandler.h
      test_i_gtk_eventhandler.inl)
endif (GTK_SUPPORT)
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
