include (ffmpeg)
include (ImageMagick)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec/scripts")
include (faad)

#set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
#include (PreXMLImplementation)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
#if (LIBXML2_SUPPORT)
# include_directories (${LIBXML2_INCLUDE_DIRS})
#endif (LIBXML2_SUPPORT)
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net/protocols/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
elseif (WIN32)
 include_directories ("$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")

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
set (EXECUTABLE web_tv)
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
     test_i_module_converter.cpp
     test_i_module_converter.h
     test_i_module_httpget.cpp
     test_i_module_httpget.h
     test_i_m3u_module_parser.cpp
     test_i_m3u_module_parser.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h
     test_i_timeouthandler.cpp
     test_i_timeouthandler.h
     test_i_web_tv.cpp
     test_i_web_tv_common.h
     test_i_web_tv_defines.h
     test_i_web_tv_stream_common.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_gtk_callbacks.cpp
       test_i_gtk_callbacks.h)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)
set (${EXECUTABLE}_EXE_CFG
     etc/test_i.gtk2
     etc/test_i.gtk3)


#if (UNIX)
# set (CMAKE_ENABLE_EXPORTS ON)
#endif (UNIX)

#string (TOLOWER "${PROJECT_NAME}_URLStreamLoad" EXECUTABLE)
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
add_dependencies (${EXECUTABLE} CommonParser)
add_dependencies (${EXECUTABLE} CommonParser_M3U)
add_dependencies (${EXECUTABLE} CommonSignal)
add_dependencies (${EXECUTABLE} CommonTimer)

add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Network_Protocol_HTTPGet)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_2_LIBRARIES})
endif (FFMPEG_SUPPORT)
#if (LIBXML2_SUPPORT)
# target_link_libraries (${EXECUTABLE} PUBLIC ${LIBXML2_LIBRARIES})
#endif (LIBXML2_SUPPORT)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (UNIX)
elseif (WIN32)
# set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

# target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif ()

if (WIN32)
 target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif (WIN32)
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

if (UNIX)
 if (GTK_USE)
  set_target_properties (${EXECUTABLE} PROPERTIES ENABLE_EXPORTS TRUE)
 endif (GTK_USE)
endif (UNIX)

set (configuration_file "etc/web_tv.ini")
set (ui_definition_file "")
set (ui_rc_file "resources.rc")
if (GUI_SUPPORT)
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
 set (ui_rc_file "etc/${ui_rc_file}")
endif (GUI_SUPPORT)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 install (FILES ${ACE_DLL_FQPATH}
          DESTINATION bin COMPONENT bin)
 install (FILES ${configuration_file}
                ${ui_definition_file}
                ${ui_rc_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
#          DESTINATION $ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE} COMPONENT config)
elseif (UNIX)
  install (FILES
           ${configuration_file}
           etc/test_u.gtk2
           etc/test_u.gtk3
           etc/test_u.xrc
           ${ui_rc_file}
           DESTINATION "etc/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (configuration_file "${CMAKE_SOURCE_DIR}/test_i/web_tv/${configuration_file}")
if (GUI_SUPPORT)
 set (ui_definition_file "${CMAKE_SOURCE_DIR}/test_i/web_tv/${ui_definition_file}")
 set (ui_rc_file "${CMAKE_SOURCE_DIR}/test_i/web_tv/${ui_rc_file}")
endif (GUI_SUPPORT)

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c${configuration_file} -e${ui_rc_file} -g${ui_definition_file} -l -r -t -z"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_i/web_tv
#                         ENVIRONMENT foo=bar)
                         ENVIRONMENT LIBVA_DRIVER_NAME=i965)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)
 if (SSL_SUPPORT)
  file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_PATH)
 endif (SSL_SUPPORT)
 if (FAAD_SUPPORT)
  file (TO_CMAKE_PATH "${faad_LIB_DIR}" FAAD_PATH)
 endif (FAAD_SUPPORT)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_LIB_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_2_LIB_DIR}" FFMPEG_2_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_PATH}
                           ${FAAD_PATH}
                           ${VCPKG_LIB_PATH}
                           ${FFMPEG_PATH}
                           ${FFMPEG_2_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH}
                           ${ACE_LIBRARY_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-l -r -t -z"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test_i/web_tv
                         ENVIRONMENT foo=bar)
endif ()
