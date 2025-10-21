include (${CMAKE_CURRENT_SOURCE_DIR}/scripts/geoip2.cmake)

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

if (GEOIP2_SUPPORT)
 include_directories (${GEOIP2_INCLUDE_DIRS})
endif (GEOIP2_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (EXECUTABLE geoip2_client)
set (${EXECUTABLE}_SRCS
     geoip2_client.cpp
     geoip2_defines.h)
# if (GTK_SUPPORT)
#  set (${EXECUTABLE}_SRCS
#       ${${EXECUTABLE}_SRCS}
#       test_u_callbacks.cpp
#       test_u_callbacks.h)
# endif (GTK_SUPPORT)
# if (WXWIDGETS_SUPPORT)
#  set (${EXECUTABLE}_SRCS
#       ${${EXECUTABLE}_SRCS}
#       net_server_wxui_base.cpp
#       net_server_wxui_base.h)
# endif (WXWIDGETS_SUPPORT)

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
#add_dependencies (${EXECUTABLE} CommonUI)
#add_dependencies (${EXECUTABLE} CommonUI_GTK)
#add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
#add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Client)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (GEOIP2_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${GEOIP2_LIBRARIES})
endif (GEOIP2_SUPPORT)

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)

##########################################

file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
string (REGEX REPLACE "\\\\" "/" ui_definition_file "${ui_definition_file}")
if (GTK_USE OR WXWIDGETS_USE)
 set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
endif (GTK_USE OR WXWIDGETS_USE)

if (GTK_SUPPORT)
 if (GTK3_USE)
  file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk3/bin GTK_LIB_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_LIB_PATH})
 elseif (GTK2_USE)
  file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_LIB_PATH})
 endif ()
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${wxWidgets_LIB_DIR})
endif (WXWIDGETS_SUPPORT)

if (GTK_USE OR WXWIDGETS_USE)
else ()
 set (ARGS "-g ${ARGS}")
endif (GTK_USE OR WXWIDGETS_USE)
#file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_PATH)
set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
#                          ${LIBPNG_PATH}
#                          ${FFMPEG_PATH}
#                          ${PROJECT_SOURCE_DIR}/../assimp/cmake/code/${CMAKE_BUILD_TYPE}
                          ${PROJECT_SOURCE_DIR}/../ACE_TAO/ACE/lib)

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-f /mnt/win_e/databases/GeoLite2-City.mmdb -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_LIB_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_LIB_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 file (TO_CMAKE_PATH ${GEOIP2_LIB_DIR} GEOIP2_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_LIB_PATH}
                           ${GEOIP2_LIB_PATH}
                           ${GTK_LIB_PATH}
                           ${WXWIDGETS_LIB_PATH}
                           ${ACE_LIB_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-f e:\\\\databases\\\\GeoLite2-City.mmdb -l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         ENVIRONMENT foo=bar)
endif ()
