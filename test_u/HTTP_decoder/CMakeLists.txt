if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
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
endif ()

include (FindPkgConfig)
if (UNIX)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
endif ()

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACEStream/cmake
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
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"${PROJECT_NAME}\", aborting")
endif ()
add_definitions (-DNET_HAS_DLL)
add_definitions (-DHTTP_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src/modules/net)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/dev)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src/modules/net)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
#message (STATUS "CMAKE_CURRENT_BINARY_DIR: \"${CMAKE_CURRENT_BINARY_DIR}/../..\"")

############## next target ################
set (${PROJECT_NAME}_Protocol_EXE_SRCS
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
     test_u_HTTP_decoder.cpp
     test_u_HTTP_decoder_common.h
     test_u_HTTP_decoder_signalhandler.cpp
     test_u_HTTP_decoder_signalhandler.h)
#  test_u_template_instantiations.cpp

#string (TOLOWER "${PROJECT_NAME}_Protocol" EXECUTABLE)
set (EXECUTABLE httpdecoder)
add_executable (${EXECUTABLE} ${ACENetwork_Protocol_EXE_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
elseif (WIN32)
# *WARNING*: linking ACE(d).lib requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
 target_link_libraries (${EXECUTABLE} ntdll.lib)
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Protocol_HTTP)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
