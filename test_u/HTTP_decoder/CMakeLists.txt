set (ACE_XML_LIB_FILE libACEXML.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
#  message (STATUS "ACE_ROOT: \"$ENV{ACE_ROOT}\"")
  find_library (ACE_XML_LIBRARY ${ACE_XML_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
                PATHS ENV{ACE_ROOT}
                PATHS $ENV{LIB_ROOT}/ACE_TAO/ACE /usr/local/src/ACE_wrappers/ACEXML
#                PATHS ENV{ACE_ROOT}
                PATH_SUFFIXES lib common
                DOC "searching for ${ACE_XML_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
# *TODO*: this does not work
 set (LIB_FILE_SUFFIX "")
 if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
     CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set (LIB_FILE_SUFFIX "d")
 endif ()
 unset (ACE_XML_LIB_FILE)
 set (ACE_XML_LIB_FILE ACEXML${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_XML_LIBRARY ${ACE_XML_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATHS $ENV{LIB_ROOT}/ACE_TAO/ACE
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_XML_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_XML_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_XML_LIB_FILE} (was: \"${ACE_XML_LIBRARY}\"), aborting")
endif ()

set (ACE_XML_PARSER_LIB_FILE libACEXML_Parser.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_XML_PARSER_LIBRARY ${ACE_XML_PARSER_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../ACE_TAO/ACE /usr/local/src/ACE_wrappers/ACEXML/parser
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib parser
                DOC "searching for ${ACE_XML_PARSER_LIB_FILE}"
                NO_DEFAULT_PATH)
 else ()
  find_library (ACE_XML_PARSER_LIBRARY ${ACE_XML_PARSER_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../ACE_TAO/ACE /usr/local
#                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_XML_PARSER_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
# *TODO*: this does not work
 set (LIB_FILE_SUFFIX "")
 if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
     CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set (LIB_FILE_SUFFIX "d")
 endif ()
 unset (ACE_XML_PARSER_LIB_FILE)
 set (ACE_XML_PARSER_LIB_FILE ACEXML_Parser${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_XML_PARSER_LIBRARY ${ACE_XML_PARSER_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATHS $ENV{LIB_ROOT}/ACE_TAO/ACE
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_XML_PARSER_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_XML_PARSER_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_XML_PARSER_LIB_FILE} (was: \"${ACE_XML_PARSER_LIBRARY}\"), aborting")
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
add_definitions (-DSTREAM_DEC_HAS_DLL)

find_package (${PROJECT_NAME} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"${PROJECT_NAME}\", aborting")
endif ()
add_definitions (-DNET_HAS_DLL)
add_definitions (-DHTTP_HAS_DLL)

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
# if (DBUS_SUPPORT)
#  include_directories (${PKG_DBUS_INCLUDE_DIRS})
# endif ()
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

 include_directories (/usr/local/src/ACE_wrappers)

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
 if (SSL_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)
 endif (SSL_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

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
     stdafx.cpp
     stdafx.h
     test_u_common_modules.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_defines.h
     test_u_message.cpp
     test_u_message.h
     test_u_session_message.cpp
     test_u_session_message.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream.inl
     test_u_HTTP_decoder.cpp
     test_u_HTTP_decoder_common.h
     test_u_HTTP_decoder_signalhandler.cpp
     test_u_HTTP_decoder_signalhandler.h
     test_u_HTTP_decoder_stream_common.h)
#  test_u_template_instantiations.cpp

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif (UNIX)

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
# add_compile_options (-fPIC)
 add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

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
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /SAFESEH:NO)
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

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${SSL_LIBRARIES})
endif ()
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# *WARNING*: linking ACE(d).lib requires "/FORCE:MULTIPLE""
 #set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)

 target_link_libraries (${EXECUTABLE} mf.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)

# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE} ${ACE_XML_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${ACE_XML_PARSER_LIBRARY})

# target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_File)
if (WIN32)
 target_link_libraries (${EXECUTABLE} ACEStream_MediaFramework)
endif (WIN32)
target_link_libraries (${EXECUTABLE} ACEStream_Network)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
#target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/HTTP_decoder
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH}
                           ${GTK_LIB_PATH}
                           ${FFMPEG_LIB_PATH})
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t -u http://www.tagesschau.de/index.html"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACEStream/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/protocol/dhcp/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/HTTP_decoder
                         ENVIRONMENT foo=bar)
endif ()
