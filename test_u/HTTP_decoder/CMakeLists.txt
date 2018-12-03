option (SSL_SUPPORT "enable SSL support" OFF)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif (UNIX)

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_LIB_FILE}"
                NO_DEFAULT_PATH)
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
endif (NOT ACE_LIBRARY)

if (SSL_SUPPORT)
 set (ACE_SSL_LIB_FILE libACE_SSL.so)
 if (UNIX)
  if (DEFINED ENV{ACE_ROOT})
   find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE
                 PATHS ENV ACE_ROOT
                 PATH_SUFFIXES lib
                 DOC "searching for ${ACE_SSL_LIB_FILE}"
                 NO_DEFAULT_PATH)
  else ()
   find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                 DOC "searching for ${ACE_SSL_LIB_FILE} (system paths)")
  endif ()
 elseif (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   set (LIB_FILE_SUFFIX d)
  endif ()
  unset (ACE_SSL_LIB_FILE)
  set (ACE_SSL_LIB_FILE ACE_SSL${LIB_FILE_SUFFIX}.lib)
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib\\${CMAKE_BUILD_TYPE}\\Win32
                DOC "searching for ${ACE_SSL_LIB_FILE}"
                NO_DEFAULT_PATH)
 endif ()
 #if (NOT EXISTS ACE_LIBRARY)
 if (NOT ACE_SSL_LIBRARY)
  message (FATAL_ERROR "could not find ${ACE_SSL_LIB_FILE} (was: \"${ACE_SSL_LIBRARY}\"), aborting")
 endif ()
endif ()

set (ACE_XML_LIB_FILE libACEXML.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_XML_LIBRARY ${ACE_XML_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_XML_LIB_FILE}"
                NO_DEFAULT_PATH)
 else ()
  find_library (ACE_XML_LIBRARY ${ACE_XML_LIB_FILE}
                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
#                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
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
               PATHS ${CMAKE_CURRENT_BINARY_DIR}/../../../../ATCD/ACE
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
                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_XML_PARSER_LIB_FILE}"
                NO_DEFAULT_PATH)
 else ()
  find_library (ACE_XML_PARSER_LIBRARY ${ACE_XML_PARSER_LIB_FILE}
                HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE /usr/local
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
               PATHS ${CMAKE_CURRENT_BINARY_DIR}/../../../../ATCD/ACE
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_XML_PARSER_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_XML_PARSER_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_XML_PARSER_LIB_FILE} (was: \"${ACE_XML_PARSER_LIBRARY}\"), aborting")
endif ()

if (UNIX)
 if (SSL_SUPPORT)
  pkg_check_modules (PKG_OPENSSL REQUIRED openssl)
 endif ()

# if (DBUS_SUPPORT)
#  pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
# endif ()
# pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libswscale)
endif (UNIX)

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

if (SSL_SUPPORT)
 add_definitions (-DSSL_SUPPORT)
endif (SSL_SUPPORT)
#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
# if (DBUS_SUPPORT)
#  include_directories (${PKG_DBUS_INCLUDE_DIRS})
# endif ()
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/stat)
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
if (UNIX)
 if (SSL_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_OPENSSL_LIBRARIES})
 endif ()
 target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# *WARNING*: linking ACE(d).lib requires "/FORCE:MULTIPLE""
 #set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)

 target_link_libraries (${EXECUTABLE} mf.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)

# *TODO*: repair WIN32 pkgconfig support
 if (SSL_SUPPORT)
  #target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/libeay32.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/ssleay32.lib)
 endif (SSL_SUPPORT)

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
