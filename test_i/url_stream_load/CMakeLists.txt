include (rapidjson)

#set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
#include (PreXMLImplementation)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/scripts")
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../scripts")
include (PreScannerParserImplementation)
include (SetScannerParserImplementation)

include (ffmpeg)
include (ImageMagick)

if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)

 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dev/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dev/scripts")
 include (pipewire)
endif (UNIX)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

if (RAPIDJSON_SUPPORT)
 include_directories (${RAPIDJSON_INCLUDE_DIRS})
endif (RAPIDJSON_SUPPORT)
if (LIBXML2_SUPPORT)
 include_directories (${LIBXML2_INCLUDE_DIRS})
endif (LIBXML2_SUPPORT)
if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 if (LIBPIPEWIRE_SUPPORT)
  include_directories (${LIBPIPEWIRE_INCLUDE_DIRS})
 endif (LIBPIPEWIRE_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser/m3u)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/html)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net/protocols/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
elseif (WIN32)
# *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser/m3u)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/html)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net/protocols/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE url_stream_load)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_connection_stream.cpp
     test_i_connection_stream.h
     test_i_eventhandler.cpp
     test_i_eventhandler.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_encoder.cpp
     test_i_module_encoder.h
     test_i_module_httpget.cpp
     test_i_module_httpget.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h
     test_i_url_stream_load.cpp
     test_i_url_stream_load_common.h
     test_i_url_stream_load_defines.h
     test_i_url_stream_load_stream_common.h)
#  test_i_template_instantiations.cpp
if (GTK_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_gtk_callbacks.cpp
      test_i_gtk_callbacks.h)
endif (GTK_SUPPORT)
set (${EXECUTABLE}_EXE_CFG
     etc/test_i.gtk2
     etc/test_i.gtk3)

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${${EXECUTABLE}_EXE_CFG})
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
add_dependencies (${EXECUTABLE} ACEStream_Network_Protocol_HTTPGet)
add_dependencies (${EXECUTABLE} ACEStream_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

if (UNIX)
 target_link_options (${EXECUTABLE} PUBLIC "-rdynamic")

 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (LIBXML2_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBXML2_LIBRARIES})
endif (LIBXML2_SUPPORT)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (UNIX)
 if (LIBPIPEWIRE_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${LIBPIPEWIRE_LIBRARIES})
 endif (LIBPIPEWIRE_SUPPORT)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonParser)
target_link_libraries (${EXECUTABLE} PUBLIC CommonParser_M3U)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_File)
#target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_HTML)
if (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_MediaFramework)
endif (WIN32)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network_Protocol_HTTPGet)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

if (GTK_SUPPORT)
 if (GTK2_USE)
  set (ui_definition_file "test_i.gtk2")
 elseif (GTK3_USE)
  set (ui_definition_file "test_i.gtk3")
 endif ()
endif (GTK_SUPPORT)
if (NOT ${ui_definition_file} STREQUAL "")
 set (ui_definition_file "etc/${ui_definition_file}")
endif (NOT ${ui_definition_file} STREQUAL "")

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
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
                ${LIB_ROOT}/vcpkg/installed/x64-windows/bin/bz2.dll
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
                ${LIB_ROOT}/../bin/yt-dlp.exe
          DESTINATION bin COMPONENT bin)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
elseif (UNIX)
  install (FILES
           ${${EXECUTABLE}_EXE_CFG}
           DESTINATION "etc/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -r -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 if (SSL_SUPPORT)
  file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_PATH)
 endif (SSL_SUPPORT)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "${VCPKG_BIN_DIR}" VCPKG_LIB_PATH)
 file (TO_CMAKE_PATH "${VCPKG_RELEASE_BIN_DIR}" VCPKG_LIB_PATH_2)
 if (LIBXML2_SUPPORT)
  file (TO_CMAKE_PATH "${LIBXML2_LIB_DIR}" LIBXML2_PATH)
 endif (LIBXML2_SUPPORT)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${LIBXML2_PATH}
                           ${SSL_PATH}
                           ${VCPKG_LIB_PATH}
                           ${VCPKG_LIB_PATH_2}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH}
                           ${ACE_LIBRARY_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-l -r -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
endif ()
