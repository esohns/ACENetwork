include (SSLConfig)

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
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)
#add_definitions (-DSTREAM_DEC_HAS_DLL)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
# if (DBUS_SUPPORT)
#  include_directories (${PKG_DBUS_INCLUDE_DIRS})
# endif ()
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

# include_directories (/usr/local/src/ACE_wrappers)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
elseif (WIN32)
 if (SSL_USE)
  include_directories (${SSL_INCLUDE_DIRS})
 endif (SSL_USE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
#message (STATUS "CMAKE_CURRENT_BINARY_DIR: \"${CMAKE_CURRENT_BINARY_DIR}/../..\"")

############## next target ################
set (HTTP_decoder_SRCS
#     stdafx.cpp
     stdafx.h
     test_u_common_modules.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_message.cpp
     test_u_message.h
     test_u_module_eventhandler.cpp
     test_u_module_eventhandler.h
     test_u_session_message.cpp
     test_u_session_message.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream.inl
     test_u_HTTP_decoder.cpp
     test_u_HTTP_decoder_common.h
     test_u_HTTP_decoder_defines.h
     test_u_HTTP_decoder_signalhandler.cpp
     test_u_HTTP_decoder_signalhandler.h
     test_u_HTTP_decoder_stream_common.h)
#  test_u_template_instantiations.cpp

if (UNIX)
# add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif (UNIX)

#string (TOLOWER "${PROJECT_NAME}_Protocol" EXECUTABLE)
set (EXECUTABLE http_decoder)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${HTTP_decoder_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

if (WIN32)
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /SAFESEH:NO)
endif (WIN32)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonSignal)
add_dependencies (${EXECUTABLE} CommonTimer)
add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
endif ()
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_FFMPEG_LIBRARIES})
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# *WARNING*: linking ACE(d).lib requires "/FORCE:MULTIPLE""
 #set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)

 target_link_libraries (${EXECUTABLE} PUBLIC mf.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)

# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_XML_LIBRARY})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_XML_PARSER_LIBRARY})
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_File)
if (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_MediaFramework)
endif (WIN32)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_HTTP)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -r -t -u https://www.tagesschau.de/index.html"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/dev/${CMAKE_BUILD_TYPE}
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/HTTP_decoder
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ImageMagick IMAGEMAGICK_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH}
                           ${GTK_LIB_PATH}
                           ${FFMPEG_LIB_PATH}
                           ${SSL_LIB_PATH}
                           ${IMAGEMAGICK_LIB_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-l -r -t -u https://www.tagesschau.de/index.html"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/HTTP_decoder
                         ENVIRONMENT foo=bar)
endif ()
