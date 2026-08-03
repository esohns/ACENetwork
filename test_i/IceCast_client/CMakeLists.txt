#include (OpenGL)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (glut)
include (glew)
include (glm)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/math/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/math/scripts")
include (fftw)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec/scripts")
include (mpg123)

include (ffmpeg)
include (ImageMagick)
#set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
#include (PreXMLImplementation)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/vis/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/vis/scripts")
include (projectm)

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

if (GLEW_SUPPORT)
 include_directories (${GLEW_INCLUDE_DIRS})
endif (GLEW_SUPPORT)
if (MPG123_SUPPORT)
 include_directories (${MPG123_INCLUDE_DIRS})
endif (MPG123_SUPPORT)
if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
#if (LIBXML2_SUPPORT)
# include_directories (${LIBXML2_INCLUDE_DIRS})
#endif (LIBXML2_SUPPORT)
if (PROJECTM_SUPPORT)
 include_directories (${PROJECTM_INCLUDE_DIRS})
endif (PROJECTM_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser/m3u)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/html)
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/html)
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
set (EXECUTABLE IceCast_client)
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
     test_i_module_httpget.cpp
     test_i_module_httpget.h
     test_i_module_httpparser.cpp
     test_i_module_httpparser.h
     test_i_m3u_module_parser.cpp
     test_i_m3u_module_parser.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i.cpp
     test_i_icecast_client_common.h
     test_i_icecast_client_defines.h
     test_i_icecast_client_stream_common.h)
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

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

if (UNIX)
 target_link_options (${EXECUTABLE} PUBLIC "-rdynamic")
 target_link_options (${EXECUTABLE} PUBLIC "LINKER:--allow-multiple-definition")

 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)
if (GLEW_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${GLEW_LIBRARIES})
endif (GLEW_SUPPORT)
if (FFTW_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${FFTW_LIBRARIES})
endif (FFTW_SUPPORT)
if (MPG123_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${MPG123_LIBRARIES})
endif (MPG123_SUPPORT)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
#if (LIBXML2_SUPPORT)
# target_link_libraries (${EXECUTABLE} PUBLIC ${LIBXML2_LIBRARIES})
#endif (LIBXML2_SUPPORT)
if (PROJECTM_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECTM_LIBRARIES})
endif (PROJECTM_SUPPORT)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (WIN32)
 target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")

 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif (WIN32)

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonMath)
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

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 install (FILES ${ACE_DLL_FQPATH}
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libatk-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/freetype6.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/intl.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libcairo-2.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libfontconfig-1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgio-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libglib-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgmodule-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgobject-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgthread-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk_pixbuf-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgtk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpango-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangocairo-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangoft2-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangowin32-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpng14-14.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/zlib1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/bin/libglade-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1/.libs/libxml2-2.dll
                H:/software/Development/dll/libgcc_s_dw2-1.dll
                H:/software/Development/dll/libexpat-1.dll
                H:/software/Development/dll/libiconv-2.dll
                ${Common_DIR}/src/Debug/Common.dll
                ${Common_DIR}/src/ui/Debug/CommonUI.dll
                ${ACEStream_DIR}/src/Debug/ACEStream.dll
                ${ACENetwork_DIR}/src/Debug/ACENetwork.dll
                ${ACENetwork_DIR}/src/protocol/http/Debug/ACENetwork_Protocol_HTTP.dll
          DESTINATION bin COMPONENT bin)
 install (FILES
          DESTINATION $ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE} COMPONENT config)
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
 if (GLEW_SUPPORT)
  file (TO_CMAKE_PATH ${GLEW_LIB_DIR} GLEW_PATH)
 endif (GLEW_SUPPORT)
 if (FFTW_SUPPORT)
  file (TO_CMAKE_PATH "${FFTW_LIB_DIR}" FFTW_PATH)
 endif (FFTW_SUPPORT)
 file (TO_CMAKE_PATH "${MPG123_LIB_DIR}" MPG123_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 if (PROJECTM_SUPPORT)
  file (TO_CMAKE_PATH "${PROJECTM_LIB_DIRS}" PROJECTM_PATH)
 endif (PROJECTM_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${SSL_PATH}
#                           ${LIBXML2_PATH}
                           ${VCPKG_LIB_PATH}
                           ${VCPKG_LIB_PATH_2}
                           ${GLEW_PATH}
                           ${FFTW_PATH}
                           ${MPG123_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${PROJECTM_PATH}
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
