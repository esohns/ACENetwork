if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif (UNIX)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif (NOT EXISTS ${Common_DIR})

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif (NOT EXISTS ${ACEStream_DIR})

if (CURSES_SUPPORT)
 include_directories (${CURSES_INCLUDE_DIRS})
endif (CURSES_SUPPORT)
if (UNIX)
# include_directories (${PKG_CAP_INCLUDE_DIRS})
 # *WORKAROUND*
 if (CURSES_USE)
  add_definitions (-DNCURSES_INTERNALS)
  include_directories (${PKG_CURSES_INCLUDE_DIRS})
 endif (CURSES_USE)

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
#     stdafx.cpp
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
     IRC_client_gui_common.h
     IRC_client_gui_connection.cpp
     IRC_client_gui_connection.h
     IRC_client_gui_connection.inl
     IRC_client_gui_messagehandler.cpp
     IRC_client_gui_messagehandler.h
     IRC_client_gui_messagehandler.inl
     IRC_client_inputhandler.cpp
     IRC_client_inputhandler.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_network.h
     IRC_client_session.cpp
     IRC_client_session.h
     IRC_client_session.inl
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
set (${EXECUTABLE}_EXE_SRCS
     ${${EXECUTABLE}_EXE_SRCS}
     IRC_client_gui_tools.cpp
     IRC_client_gui_tools.h)
if (CURSES_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      IRC_client_curses.cpp
      IRC_client_curses.h)
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      IRC_client_gui_callbacks.cpp
      IRC_client_gui_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
  #  set (${EXECUTABLE}_EXE_SRCS
  #       ${${EXECUTABLE}_EXE_SRCS}
  #       test_u_wxwidgets_ui.cpp
  #       test_u_wxwidgets_ui.h
  #       test_u_wxwidgets_ui.inl
  #       test_u_wxwidgets_ui_base.cpp
  #       test_u_wxwidgets_ui_base.h)
endif (WXWIDGETS_SUPPORT)
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
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_IRC)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE} PUBLIC X11)
elseif (WIN32)
 target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
install (FILES
         etc/IRC_client.ini
         etc/IRC_client_phonebook.ini
         DESTINATION "etc/${EXECUTABLE}" COMPONENT config)

########### next target ###############
set (EXECUTABLE_GUI IRC_client_gui)
set (${EXECUTABLE_GUI}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     IRC_client_common.h
     IRC_client_common_modules.h
     IRC_client_configuration.h
     IRC_client_defines.h
     IRC_client_gui.cpp
     IRC_client_gui_common.h
     IRC_client_gui_connection.cpp
     IRC_client_gui_connection.h
     IRC_client_gui_connection.inl
     IRC_client_gui_defines.h
     IRC_client_gui_messagehandler.cpp
     IRC_client_gui_messagehandler.h
     IRC_client_gui_messagehandler.inl
     IRC_client_gui_tools.cpp
     IRC_client_gui_tools.h
     IRC_client_module_IRChandler.cpp
     IRC_client_module_IRChandler.h
     IRC_client_network.h
     IRC_client_session.cpp
     IRC_client_session.h
     IRC_client_session.inl
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
     IRC_client_tools.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE_GUI}_EXE_SRCS
      ${${EXECUTABLE_GUI}_EXE_SRCS}
      IRC_client_gui_callbacks.cpp
      IRC_client_gui_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
  #  set (${EXECUTABLE}_EXE_SRCS
  #       ${${EXECUTABLE}_EXE_SRCS}
  #       test_u_wxwidgets_ui.cpp
  #       test_u_wxwidgets_ui.h
  #       test_u_wxwidgets_ui.inl
  #       test_u_wxwidgets_ui_base.cpp
  #       test_u_wxwidgets_ui_base.h)
endif (WXWIDGETS_SUPPORT)

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
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_IRC)

if (UNIX)
 target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE_GUI} PUBLIC X11)
elseif (WIN32)
 target_link_options (${EXECUTABLE_GUI} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif ()

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_GUI} PUBLIC Common)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME}_Protocol_IRC)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_GUI}
                           REUSE_FROM ${EXECUTABLE})

install (TARGETS ${EXECUTABLE_GUI}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 set (ACE_SSL_DLL_FILE ACE_SSL${LIB_FILE_SUFFIX}.dll)
 set (ACE_SSL_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_SSL_DLL_FILE})
 if (DEFINED ENV{LIB_ROOT})
  file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
  string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
  install (FILES ${ACE_DLL_FQPATH}
                 ${ACE_SSL_DLL_FQPATH}
                 ${LIB_ROOT}/gtk3/bin/libatk-1.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libfreetype-6.dll
                 ${LIB_ROOT}/gtk3/bin/libintl-8.dll
                 ${LIB_ROOT}/gtk3/bin/libcairo-2.dll
                 ${LIB_ROOT}/gtk3/bin/libfontconfig-1.dll
                 ${LIB_ROOT}/gtk3/bin/libgio-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libglib-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgmodule-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgobject-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgthread-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgdk_pixbuf-2.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgdk-3-0.dll
                 ${LIB_ROOT}/gtk3/bin/libgtk-3-0.dll
                 ${LIB_ROOT}/gtk3/bin/libpango-1.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libpangocairo-1.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libpangoft2-1.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libpangowin32-1.0-0.dll
                 ${LIB_ROOT}/gtk3/bin/libiconv-2.dll
                 ${LIB_ROOT}/gtk3/bin/libpng15-15.dll
                 ${LIB_ROOT}/gtk3/bin/liblzma-5.dll
                 ${LIB_ROOT}/gtk3/bin/zlib1.dll
                 ${LIB_ROOT}/gtk3/bin/libxml2-2.dll
                 ${LIB_ROOT}/gtk3/bin/libffi-6.dll
                 ${LIB_ROOT}/gtk3/bin/libcairo-gobject-2.dll
                 ${LIB_ROOT}/gtk3/bin/libpixman-1-0.dll
                 ${LIB_ROOT}/gtk3/bin/libwinpthread-1.dll
                 ${LIB_ROOT}/assimp/cmake/bin/Release/assimp-vc143-mt.dll
                 ${LIB_ROOT}/libpng/build/Release/libpng16.dll
                 ${LIB_ROOT}/ffmpeg/libswresample/swresample-5.dll
                 ${LIB_ROOT}/ffmpeg/libswscale/swscale-8.dll
                 ${LIB_ROOT}/ffmpeg/libavformat/avformat-61.dll
                 ${LIB_ROOT}/ffmpeg/libavcodec/avcodec-61.dll
                 ${LIB_ROOT}/ffmpeg/libavutil/avutil-59.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_MagickCore_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_MagickWand_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_bzlib_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_freetype_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_fribidi_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_glib_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_harfbuzz_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_lcms_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_libxml_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_lqr_.dll
                 ${LIB_ROOT}/ImageMagick/CORE_RL_zlib_.dll
                 ${LIB_ROOT}/libepoxy/bin/epoxy-0.dll
                 ${LIB_ROOT}/vcpkg/installed/x64-windows/bin/iconv-2.dll
                 ${LIB_ROOT}/vcpkg/installed/x64-windows/bin/liblzma.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316_core_vc_x64_custom.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316_html_vc_x64_custom.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316_xrc_vc_x64_custom.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxbase316_vc_x64_custom.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxbase316_xml_vc_x64_custom.dll
                 ${LIB_ROOT}/openssl/libcrypto-3-x64.dll
                 ${LIB_ROOT}/openssl/libssl-3-x64.dll
                 ${LIB_ROOT}/zlib/build/Release/zlib.dll
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
  install (FILES
           ${LIB_ROOT}/gtk3/share/glib-2.0/schemas/gschemas.compiled
           ${LIB_ROOT}/gtk3/share/glib-2.0/schemas/org.gtk.Settings.ColorChooser.gschema.xml
           ${LIB_ROOT}/gtk3/share/glib-2.0/schemas/org.gtk.Settings.FileChooser.gschema.xml
           DESTINATION "share/glib-2.0/schemas" COMPONENT config)
 endif (DEFINED ENV{LIB_ROOT})
endif (WIN32)
install (FILES
         etc/IRC_client.ini
         etc/IRC_client_phonebook.ini
         etc/IRC_client_channel.glade
         etc/IRC_client_connection.glade
         etc/IRC_client_main.glade
         DESTINATION etc/${EXECUTABLE} COMPONENT config)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-a irc.eu.dal.net -l -n -N testeee -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/dev/${CMAKE_BUILD_TYPE}
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/ui/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/Debug
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/dev/${CMAKE_BUILD_TYPE}
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/IRC_client
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_GUI}.exe" COMMAND_GUI)
 string (REPLACE "\\" "\\\\" COMMAND_GUI ${COMMAND_GUI})
 file (TO_CMAKE_PATH ${SSL_LIB_DIR} SSL_LIB_PATH)
 file (TO_CMAKE_PATH ${ACE_LIB_DIR} ACE_LIB_PATH)
 file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_LIB_PATH}
                           ${ACE_LIB_PATH}
                           ${GTK_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-a irc.eu.dal.net -l -n -N testeee -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         COMMAND ${COMMAND_GUI}
                         ARGS "-l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/IRC_client
                         ENVIRONMENT foo=bar)
endif ()
