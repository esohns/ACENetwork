include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../scripts")
include (SSLConfig)

if (UNIX)
 include (FindPkgConfig)
# *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
# pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavutil libavcodec)
elseif (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{LIB_ROOT}/DShowBaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow")
endif ()

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
else ()
 message (STATUS "found package \"ACEStream\": \"${ACEStream_DIR}\"")
endif ()
add_definitions (-DACESTREAM_HAS_DLL)

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
 add_definitions (-DDEBUG) # activate DirectShow debug logging
endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)

if (GUI_SUPPORT)
 if (CURSES_SUPPORT)
  include_directories (${CURSES_INCLUDE_DIRS})
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  include_directories (${GTK_INCLUDE_DIRS})

  if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
   include_directories (${GTKGLAREA_INCLUDES})
  endif (GTKGLAREA_SUPPORT)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  include_directories (${Qt_INCLUDE_DIRS})
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  include_directories (${wxWidgets_INCLUDE_DIRS})
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

if (UNIX)
 include_directories (${PKG_V4L2_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)
 if (GUI_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui)
  if (CURSES_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/curses)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/gtk)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/qt)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/wxwidgets)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/stat)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/vis)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 if (GUI_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
  if (CURSES_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/curses)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/gtk)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/qt)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/wxwidgets)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/vis)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../test_i)

include_directories (${CMAKE_BINARY_DIR})

############## next target ################
set (EXECUTABLE_CLIENT av_stream_client)
set (${EXECUTABLE_CLIENT}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_av_stream_client.cpp
     test_i_av_stream_client_common.h
     test_i_av_stream_client_eventhandler.cpp
     test_i_av_stream_client_eventhandler.h
     test_i_av_stream_client_eventhandler.inl
     test_i_av_stream_client_message.cpp
     test_i_av_stream_client_message.h
     test_i_av_stream_client_session_message.cpp
     test_i_av_stream_client_session_message.h
     test_i_av_stream_client_signalhandler.cpp
     test_i_av_stream_client_signalhandler.h
     test_i_av_stream_client_signalhandler.inl
     test_i_av_stream_client_stream.cpp
     test_i_av_stream_client_stream.h
     test_i_av_stream_client_stream.inl
     test_i_av_stream_common.h
     test_i_av_stream_defines.h
     test_i_av_stream_parser.cpp
     test_i_av_stream_parser.h
     test_i_av_stream_parser_driver.cpp
     test_i_av_stream_parser_driver.h
     test_i_av_stream_scanner.cpp
     test_i_av_stream_scanner.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_module_streamer.cpp
     test_i_module_streamer.h
     test_i_module_streamer.inl
     test_i_network.h
     test_i_av_stream_server_message.cpp
     test_i_av_stream_server_message.h
     test_i_av_stream_server_session_message.cpp
     test_i_av_stream_server_session_message.h
     test_i_av_stream_server_stream.cpp
     test_i_av_stream_server_stream.h
     test_i_av_stream_stream_common.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE_CLIENT}_EXE_SRCS
       ${${EXECUTABLE_CLIENT}_EXE_SRCS}
       test_i_gtk_callbacks.cpp
       test_i_gtk_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_SERVER}_EXE_SRCS
#       ${${EXECUTABLE_SERVER}_EXE_SRCS})
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
   set (ui_definition_file etc/test_i_client.gtk3)
   set (ui_definition_file_2 etc/test_i_server.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file etc/test_i_client.gtk2)
   set (ui_definition_file_2 etc/test_i_server.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  if (WIN32)
#   add_definitions(-DUNICODE -D_UNICODE)
   endif (WIN32)
  set (ui_definition_file etc/test_i_client.xrc)
  set (ui_definition_file_2 etc/test_i_server.xrc)
 endif ()
endif (GUI_SUPPORT)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE_CLIENT}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_CLIENT}_EXE_SRCS}
                etc/resources.rc
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE_CLIENT} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_CLIENT} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (NOT BUILD_TEST_I)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_CLIENT} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (WIN32)

#add_dependencies (${EXECUTABLE_CLIENT} ACE)

add_dependencies (${EXECUTABLE_CLIENT} Common)
add_dependencies (${EXECUTABLE_CLIENT} CommonImage)
add_dependencies (${EXECUTABLE_CLIENT} CommonLog)
add_dependencies (${EXECUTABLE_CLIENT} CommonTimer)

add_dependencies (${EXECUTABLE_CLIENT} ACEStream)
add_dependencies (${EXECUTABLE_CLIENT} ACEStream_Device)
add_dependencies (${EXECUTABLE_CLIENT} ACEStream_MediaFramework)
add_dependencies (${EXECUTABLE_CLIENT} ACEStream_Miscellaneous)
add_dependencies (${EXECUTABLE_CLIENT} ACEStream_Network)
add_dependencies (${EXECUTABLE_CLIENT} ACEStream_Visualization)

add_dependencies (${EXECUTABLE_CLIENT} ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${PKG_V4L2_LIBRARIES})
elseif (WIN32)
# set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC evr.lib)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC mfplat.lib)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC mfuuid.lib)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC strmiids.lib)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC Common)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonTask)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonUI)
 if (GTK_USE)
  target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonUI_GTK)
 elseif (WXWIDGETS_USE)
  target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC CommonUI_wxWidgets)
 endif ()
endif (GUI_SUPPORT)

target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Device)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_MediaFramework)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Statistic)
target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ACEStream_Visualization)

target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_CLIENT} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_CLIENT}
                           PUBLIC stdafx.h)

set (RESOURCE_FILES
     etc/resources.rc
     ${ui_definition_file}
     ${ui_definition_file_2})
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE_CLIENT}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
         COMPONENT test_i)
if (UNIX)
 install (FILES ${ui_definition_file}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/av_stream" COMPONENT config)
elseif (WIN32)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/av_stream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/av_stream" COMPONENT config)
endif ()

set (COMMAND "$<TARGET_FILE:${EXECUTABLE_CLIENT}>")
set (ENVIRONMENT "")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_CLIENT}" COMMAND)
 if (GTK_SUPPORT)
  set (ARGS "-g./${ui_definition_file} -l -t")
 else ()
  set (ARGS "-g -l -t")
 endif (GTK_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
 file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/av_stream WORKING_DIRECTORY)
elseif (WIN32)
# file (TO_NATIVE_PATH "${COMMAND}" COMMAND)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_CLIENT}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
  string (REGEX REPLACE "\\\\" "/" ui_definition_file "${ui_definition_file}")
  set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
  set (ARGS "${ARGS} -l -t")
 else ()
  set (ARGS "-c -g -l -t")
 endif (GUI_SUPPORT)
 if (FFMPEG_SUPPORT)
  file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 endif (FFMPEG_SUPPORT)
 if (IMAGEMAGICK_SUPPORT)
  file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 endif (IMAGEMAGICK_SUPPORT)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${SSL_LIB_DIR}" SSL_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH}
                           ${SSL_PATH}
                           G:/software/Development/dll
                           ${ACE_PATH})
 string (REPLACE "/" "\\\\" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/av_stream) 
endif ()
create_target_launcher (${EXECUTABLE_CLIENT}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                         FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${WORKING_DIRECTORY}
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE_CLIENT}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_CLIENT} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_CLIENT}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\ACENetwork\cmake\src\Debug

############## next target ################

if (WIN32)
 set (LIBRARY_DS av_stream_ds_source)
 set (${LIBRARY_DS}_LIB_SRCS
#      stdafx.cpp
      stdafx.h
      test_i_av_stream_ds_source.cpp
      test_i_av_stream_server_message.cpp
      test_i_av_stream_server_message.h)

 add_library (${LIBRARY_DS} MODULE
              EXCLUDE_FROM_ALL
              ${${LIBRARY_DS}_LIB_SRCS}
              etc/${LIBRARY_DS}.def)
 set_property (TARGET ${LIBRARY_DS} PROPERTY FOLDER "test_i")

 target_include_directories (${LIBRARY_DS} PUBLIC "${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")

 add_dependencies (${LIBRARY_DS} Common)
 add_dependencies (${LIBRARY_DS} CommonLog)

 add_dependencies (${LIBRARY_DS} ACEStream)

 set (CMAKE_MODULE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
 set_target_properties (${LIBRARY_DS} PROPERTIES LINK_FLAGS /SAFESEH:NO)

 target_link_libraries (${LIBRARY_DS} PUBLIC strmiids.lib)
 target_link_libraries (${LIBRARY_DS} PUBLIC winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${LIBRARY_DS} PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/build/msvc/baseclasses/Debug/BaseClassesd.lib)
#  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${LIBRARY_DS} PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/build/msvc/baseclasses/Release/BaseClasses.lib)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)

 if (GUI_SUPPORT)
  if (CURSES_SUPPORT)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   if (GTKGL_SUPPORT)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   target_link_libraries (${LIBRARY_DS} PUBLIC ${wxWidgets_LIBRARIES})
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 target_link_libraries (${LIBRARY_DS} PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_DS} PUBLIC Common)
 target_link_libraries (${LIBRARY_DS} PUBLIC CommonLog)

 target_link_libraries (${LIBRARY_DS} PUBLIC ACEStream)
 target_link_libraries (${LIBRARY_DS} PUBLIC ACEStream_Device)
 target_link_libraries (${LIBRARY_DS} PUBLIC ACEStream_MediaFramework)
 target_link_libraries (${LIBRARY_DS} PUBLIC ACEStream_Miscellaneous)

# target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_DS} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_DS} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
 target_precompile_headers (${LIBRARY_DS}
                            PUBLIC stdafx.h)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_DS}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_DS}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 file (TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/test_i/av_stream/${CMAKE_BUILD_TYPE}/${LIBRARY_DS}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 file (TO_NATIVE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" ACE_LIB_PATH_NATIVE ${ACE_LIB_PATH_NATIVE})
 create_generic_launcher (regsvr32_directshow
                          COMMAND regsvr32.exe ${TARGET_PATH_NATIVE}
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})

#-----------------------------------------

 set (LIBRARY_MF av_stream_mf_source)
 set (${LIBRARY_MF}_LIB_SRCS
#      stdafx.cpp
      stdafx.h
      test_i_av_stream_mf_source.cpp)

 add_library (${LIBRARY_MF} MODULE
              EXCLUDE_FROM_ALL
              ${${LIBRARY_MF}_LIB_SRCS}
              etc/${LIBRARY_MF}.def)
 set_property (TARGET ${LIBRARY_MF} PROPERTY FOLDER "test_i")

 target_include_directories (${LIBRARY_MF} PUBLIC "${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")

 add_dependencies (${LIBRARY_MF} Common)

 add_dependencies (${LIBRARY_MF} ACEStream)

 if (GUI_SUPPORT)
  if (CURSES_SUPPORT)
  endif (CURSES_SUPPORT)
  if (GTK_SUPPORT)
   if (GTKGL_SUPPORT)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
  if (QT_SUPPORT)
  endif (QT_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   target_link_libraries (${LIBRARY_MF} PUBLIC ${wxWidgets_LIBRARIES})
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)

 target_link_libraries (${LIBRARY_MF} PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_MF} PUBLIC Common)
 target_link_libraries (${LIBRARY_MF} PUBLIC CommonLog)

 target_link_libraries (${LIBRARY_MF} PUBLIC ACEStream)
 target_link_libraries (${LIBRARY_MF} PUBLIC ACEStream_Device)
 target_link_libraries (${LIBRARY_MF} PUBLIC ACEStream_MediaFramework)
 target_link_libraries (${LIBRARY_MF} PUBLIC ACEStream_Miscellaneous)

# target_link_libraries (${LIBRARY_MF} PUBLIC ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_MF} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_MF} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
 target_precompile_headers (${LIBRARY_MF}
                            PUBLIC stdafx.h)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_MF}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_MF}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 file (TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/test_i/av_stream/${CMAKE_BUILD_TYPE}/${LIBRARY_MF}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 create_generic_launcher (regsvr32_mediafoundation
                          COMMAND regsvr32.exe ${TARGET_PATH_NATIVE}
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})
endif (WIN32)

############## next target ################
set (EXECUTABLE_SERVER av_stream_server)
set (${EXECUTABLE_SERVER}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_av_stream_common.h
     test_i_av_stream_defines.h
     test_i_av_stream_parser.cpp
     test_i_av_stream_parser.h
     test_i_av_stream_parser_driver.cpp
     test_i_av_stream_parser_driver.h
     test_i_av_stream_scanner.cpp
     test_i_av_stream_scanner.h
     test_i_av_stream_server.cpp
     test_i_av_stream_server_common.h
     test_i_av_stream_server_eventhandler.cpp
     test_i_av_stream_server_eventhandler.h
     test_i_av_stream_server_eventhandler.inl
     test_i_av_stream_server_listener_common.h
     test_i_av_stream_server_message.cpp
     test_i_av_stream_server_message.h
     test_i_av_stream_server_session_message.cpp
     test_i_av_stream_server_session_message.h
     test_i_av_stream_server_signalhandler.cpp
     test_i_av_stream_server_signalhandler.h
     test_i_av_stream_server_signalhandler.inl
     test_i_av_stream_server_stream.cpp
     test_i_av_stream_server_stream.h
     test_i_av_stream_stream_common.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_module_parser.cpp
     test_i_module_parser.h
     test_i_module_parser.inl
     test_i_module_splitter.cpp
     test_i_module_splitter.h
     test_i_module_splitter.inl
     test_i_network.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE_SERVER}_EXE_SRCS
       ${${EXECUTABLE_SERVER}_EXE_SRCS}
       test_i_gtk_callbacks.cpp
       test_i_gtk_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_SERVER}_EXE_SRCS
#       ${${EXECUTABLE_SERVER}_EXE_SRCS})
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

add_executable (${EXECUTABLE_SERVER}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SERVER}_EXE_SRCS}
                etc/resources.rc
                ${ui_definition_file_2})
set_property (TARGET ${EXECUTABLE_SERVER} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_SERVER} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_SERVER} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${EXECUTABLE_SERVER} ACE)
add_dependencies (${EXECUTABLE_SERVER} Common)
add_dependencies (${EXECUTABLE_SERVER} CommonImage)
add_dependencies (${EXECUTABLE_SERVER} CommonLog)
add_dependencies (${EXECUTABLE_SERVER} CommonSignal)
add_dependencies (${EXECUTABLE_SERVER} CommonTimer)

add_dependencies (${EXECUTABLE_SERVER} ACEStream)
add_dependencies (${EXECUTABLE_SERVER} ACEStream_MediaFramework)
add_dependencies (${EXECUTABLE_SERVER} ACEStream_Miscellaneous)
add_dependencies (${EXECUTABLE_SERVER} ACEStream_Network)
add_dependencies (${EXECUTABLE_SERVER} ACEStream_Visualization)

add_dependencies (${EXECUTABLE_SERVER} ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${PKG_V4L2_LIBRARIES})

# target_link_libraries (${EXECUTABLE_SERVER} PUBLIC X11)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_SERVER} PUBLIC evr.lib)
 target_link_libraries (${EXECUTABLE_SERVER} PUBLIC strmiids.lib)
 target_link_libraries (${EXECUTABLE_SERVER} PUBLIC winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_SERVER} PUBLIC Common)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Device)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_MediaFramework)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Statistic)
target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ACEStream_Visualization)

target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_SERVER} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_SERVER}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE_SERVER}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES ${ui_definition_file_2}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/av_stream" COMPONENT config)
elseif (WIN32)
 install (FILES ${ui_definition_file_2}
          DESTINATION "data/${PROJECT_NAME}/av_stream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/av_stream" COMPONENT config)
endif ()

#cmake_policy (SET CMP0053 OLD)
set (ARGS "")
set (COMMAND "$<TARGET_FILE:${EXECUTABLE_SERVER}>")
set (ENVIRONMENT "")
if (UNIX)
 set (ARGS "-g./${ui_definition_file_2} -l -o -t")
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
 file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/av_stream WORKING_DIRECTORY)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SERVER}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file_2}" ui_definition_file_2)
  string (REGEX REPLACE "\\\\" "/" ui_definition_file_2 "${ui_definition_file_2}")
  set (ARGS "-g.\\\\${ui_definition_file_2} ${ARGS}")
  set (ARGS "${ARGS} -l -o -t")
 else ()
  set (ARGS "-c -g -l -o -t")
 endif (GUI_SUPPORT)
 file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/av_stream WORKING_DIRECTORY)
 string (REPLACE "\\" "\\\\" WORKING_DIRECTORY ${WORKING_DIRECTORY})
endif ()
create_target_launcher (${EXECUTABLE_SERVER}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${WORKING_DIRECTORY}
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE_SERVER}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_SERVER}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\ACENetwork\cmake\src\Debug
