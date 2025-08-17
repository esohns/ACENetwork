include (ffmpeg)
include (ImageMagick)

# disable GUI support
is_UI_graphical (${UI_FRAMEWORK})
if (NOT UI_FRAMEWORK STREQUAL "")

endif (NOT UI_FRAMEWORK STREQUAL "")

set (FLEX_LIB_FILE libfl.so)
if (UNIX)
 find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
elseif (WIN32)
 unset (FLEX_LIB_FILE)
 set (FLEX_LIB_FILE libfl.lib)
 find_library (FLEX_LIBRARY ${FLEX_LIB_FILE}
               DOC "searching for ${FLEX_LIB_FILE} (system paths)")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT FLEX_LIBRARY)
# message (FATAL_ERROR "could not find ${FLEX_LIB_FILE} (was: \"${FLEX_LIBRARY}\"), aborting")
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: Common requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
# pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
# *TODO*: repair WIN32 pkgconfig support
elseif (WIN32)
endif ()

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

if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dev)
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/p2p/bittorrent)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

########### next target ###############
set (bittorrent_client_SRCS
#     stdafx.cpp
     stdafx.h
     bittorrent_client.cpp
     bittorrent_client_common.h
     bittorrent_client_common_modules.h
     bittorrent_client_configuration.h
     bittorrent_client_defines.h
#     bittorrent_client_inputhandler.cpp
#     bittorrent_client_inputhandler.h
#     bittorrent_client_message.cpp
#     bittorrent_client_message.h
     bittorrent_client_network.h
     bittorrent_client_session_common.h
#     bittorrent_client_sessionmessage.cpp
#     bittorrent_client_sessionmessage.h
     bittorrent_client_signalhandler.cpp
     bittorrent_client_signalhandler.h
#     bittorrent_client_stream.cpp
#     bittorrent_client_stream.h
     bittorrent_client_stream_common.h
     bittorrent_client_streamhandler.cpp
     bittorrent_client_streamhandler.h
     bittorrent_client_streamhandler.inl
     bittorrent_client_tools.cpp
     bittorrent_client_tools.h)
if (CURSES_SUPPORT)
 set (bittorrent_client_SRCS
      ${bittorrent_client_SRCS}
      bittorrent_client_curses.cpp
      bittorrent_client_curses.h)
endif (CURSES_SUPPORT)

set (EXECUTABLE bittorrent_client)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${bittorrent_client_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif (WIN32)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)

add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonParser)
add_dependencies (${EXECUTABLE} CommonSignal)
add_dependencies (${EXECUTABLE} CommonTimer)

add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_BitTorrent)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

if (UNIX)
 target_link_options (${EXECUTABLE} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (UNIX)
# target_link_libraries (${EXECUTABLE} ${FLEX_LIBRARY})
 target_link_libraries (${EXECUTABLE} PUBLIC X11)
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")

 target_link_libraries (${EXECUTABLE} PUBLIC mf.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonParser)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Device)
if (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_MediaFramework)
endif (WIN32)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_BitTorrent)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

if (WIN32)
 ## *WARNING*: linking ACE(d).lib requires "/FORCE:MULTIPLE""
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /SAFESEH:NO)
endif (WIN32)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)

########### next target ###############
set (EXECUTABLE_GUI bittorrent_client_gui)
set (${EXECUTABLE_GUI}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     bittorrent_client_common.h
     bittorrent_client_common_modules.h
     bittorrent_client_configuration.h
     bittorrent_client_defines.h
     bittorrent_client_gui.cpp
     bittorrent_client_gui_common.h
     bittorrent_client_gui_defines.h
     bittorrent_client_gui_session.cpp
     bittorrent_client_gui_session.h
     bittorrent_client_gui_session.inl
     bittorrent_client_gui_tools.cpp
     bittorrent_client_gui_tools.h
#     bittorrent_client_message.cpp
#     bittorrent_client_message.h
     bittorrent_client_network.h
#     bittorrent_client_sessionmessage.cpp
#     bittorrent_client_sessionmessage.h
     bittorrent_client_signalhandler.cpp
     bittorrent_client_signalhandler.h
#     bittorrent_client_stream.cpp
#     bittorrent_client_stream.h
     bittorrent_client_stream_common.h
     bittorrent_client_streamhandler.cpp
     bittorrent_client_streamhandler.h
     bittorrent_client_streamhandler.inl
     bittorrent_client_tools.cpp
     bittorrent_client_tools.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE_GUI}_EXE_SRCS
      ${${EXECUTABLE_GUI}_EXE_SRCS}
      bittorrent_client_gui_callbacks.cpp
      bittorrent_client_gui_callbacks.h)
endif (GTK_SUPPORT)

#set_source_files_properties (${bittorrent_client_gui_SRCS} PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")

add_executable (${EXECUTABLE_GUI}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_GUI}_EXE_SRCS}
                etc/bittorrent_client_main.glade
                etc/bittorrent_client_session.glade)
set_property (TARGET ${EXECUTABLE_GUI} PROPERTY FOLDER "test_i")
if (WIN32)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_GUI} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_GUI} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)

add_dependencies (${EXECUTABLE_GUI} Common)
add_dependencies (${EXECUTABLE_GUI} CommonLog)
add_dependencies (${EXECUTABLE_GUI} CommonParser)
add_dependencies (${EXECUTABLE_GUI} CommonSignal)
add_dependencies (${EXECUTABLE_GUI} CommonTimer)

add_dependencies (${EXECUTABLE_GUI} ACEStream)

add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_BitTorrent)
add_dependencies (${EXECUTABLE_GUI} ${PROJECT_NAME}_Protocol_HTTP)

if (UNIX)
 target_link_options (${EXECUTABLE_GUI} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${ACE_LIBRARY})

if (UNIX)
 target_link_libraries (${EXECUTABLE_GUI} PUBLIC X11)
endif (UNIX)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC Common)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonParser)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Decoder)
#target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Device)
#if (WIN32)
# target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_MediaFramework)
#endif (WIN32)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME}_Protocol_BitTorrent)
target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE_GUI} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_GUI}
                           REUSE_FROM ${EXECUTABLE})

install (TARGETS ${EXECUTABLE_GUI}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
install (FILES
         etc/bittorrent_client.ini
         etc/bittorrent_client.rc
         etc/bittorrent_client_main.glade
         etc/bittorrent_client_session.glade
         DESTINATION "etc/${EXECUTABLE}" COMPONENT config)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-f etc/test.torrent -l -r -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/bittorrent_client
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         ARGS "-l -r -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/bittorrent_client
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_GUI}.exe" COMMAND_2)
 string (REPLACE "\\" "\\\\" COMMAND_2 ${COMMAND_2})
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl OPENSSL_LIB_PATH)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_LIB_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${OPENSSL_LIB_PATH}
                           ${VCPKG_LIB_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH}
                           ${ACE_LIBRARY_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-f etc\\\\test.torrent -l -r -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_GUI}
                         COMMAND ${COMMAND_2}
                         ARGS "-l -r -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/bittorrent_client
                         ENVIRONMENT foo=bar)
endif ()
