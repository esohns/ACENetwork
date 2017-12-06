# target system introspection (see configure.ac)
#include (CheckFunctionExists)
#include (InstallRequiredSystemLibraries)
#check_function_exists (memset HAVE_FUNC_MEMSET)
#check_function_exists (socket HAVE_FUNC_SOCKET)
#check_function_exists (strerror HAVE_FUNC_STRERROR)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 include (FindThreads)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
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
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 #message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_SSL REQUIRED libssl)
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
#message (STATUS "BUILD_PATH_SUFFIX: ${BUILD_PATH_SUFFIX}")
#message (STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
#if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
 # stand-alone build
# message ("stand-alone build")
#if (WIN32)
 find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
               REQUIRED
               CONFIG
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
               PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
               NO_DEFAULT_PATH)
 if (NOT EXISTS ${Common_DIR})
  message (FATAL_ERROR "could not find package \"Common\", aborting")
 endif ()
 add_definitions (-DCOMMON_HAS_DLL)

 find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
               REQUIRED
               CONFIG
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
               PATH_SUFFIXES libACEStream/${BUILD_PATH_SUFFIX}
               NO_DEFAULT_PATH)
 if (NOT EXISTS ${ACEStream_DIR})
  message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
 endif ()
 add_definitions (-DSTREAM_HAS_DLL)
#endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_SSL_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACEStream/src/modules/stat)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../openssl/include)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libACEStream/src/modules/stat)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/bison)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

############## next target ################
set (${PROJECT_NAME}_LIB_SRCS
     stdafx.cpp
     stdafx.h
     net_asynch_netlinksockethandler.cpp
     net_asynch_netlinksockethandler.h
     net_asynch_netlinksockethandler.inl
     net_asynch_ssl_tcpsockethandler.cpp
     net_asynch_ssl_tcpsockethandler.h
     net_asynch_ssl_tcpsockethandler.inl
     net_asynch_tcpsockethandler.cpp
     net_asynch_tcpsockethandler.h
     net_asynch_tcpsockethandler.inl
     net_asynch_udpsockethandler.cpp
     net_asynch_udpsockethandler.h
     net_asynch_udpsockethandler.inl
     net_common_tools.cpp
     net_common_tools.h
     net_common.h
     net_configuration.h
     net_connection_base.cpp
     net_connection_base.h
     net_connection_base.inl
     net_connection_manager.cpp
     net_connection_manager.h
     net_connection_manager.inl
     net_controller_base.cpp
     net_controller_base.h
     net_defines.h
     net_exports.h
     net_iconnection.h
     net_iconnectionmanager.h
     net_iconnector.h
     net_ilistener.h
     net_iparser.h
     net_ipconnection.cpp
     net_ipconnection.h
     net_itransportlayer.h
     net_macros.h
     net_module_sockethandler.cpp
     net_module_sockethandler.h
     net_module_sockethandler.inl
     net_netlinkconnection.cpp
     net_netlinkconnection.h
     net_netlinkconnection.inl
     net_netlinksockethandler.cpp
     net_netlinksockethandler.h
     net_netlinksockethandler.inl
     net_packet_headers.h
     net_parser_base.cpp
     net_parser_base.h
     net_parser_base.inl
     net_protocol_layer.cpp
     net_protocol_layer.h
     net_remote_comm.h
     net_session_base.cpp
     net_session_base.h
     net_session_base.inl
     net_sock_acceptor.cpp
     net_sock_acceptor.h
     net_sock_connector.cpp
     net_sock_connector.h
     net_sock_dgram.cpp
     net_sock_dgram.h
     net_sockethandler_base.cpp
     net_sockethandler_base.h
     net_sockethandler_base.inl
     net_streamconnection_base.cpp
     net_streamconnection_base.h
     net_streamconnection_base.inl
     net_stream_asynch_tcpsocket_base.cpp
     net_stream_asynch_tcpsocket_base.h
     net_stream_asynch_tcpsocket_base.inl
     net_stream_asynch_udpsocket_base.cpp
     net_stream_asynch_udpsocket_base.h
     net_stream_asynch_udpsocket_base.inl
     net_stream_tcpsocket_base.cpp
     net_stream_tcpsocket_base.h
     net_stream_tcpsocket_base.inl
     net_stream_udpsocket_base.cpp
     net_stream_udpsocket_base.h
     net_stream_udpsocket_base.inl
     net_tcpconnection_base.cpp
     net_tcpconnection_base.h
     net_tcpconnection_base.inl
     net_tcpsockethandler.cpp
     net_tcpsockethandler.h
     net_tcpsockethandler.inl
     net_transportlayer_base.cpp
     net_transportlayer_base.h
     net_transportlayer_base.inl
     net_transportlayer_ip_cast.cpp
     net_transportlayer_ip_cast.h
     net_transportlayer_netlink.cpp
     net_transportlayer_netlink.h
     net_transportlayer_tcp.cpp
     net_transportlayer_tcp.h
     net_transportlayer_udp.cpp
     net_transportlayer_udp.h
     net_udpconnection_base.cpp
     net_udpconnection_base.h
     net_udpconnection_base.inl
     net_udpsockethandler.cpp
     net_udpsockethandler.h
     net_udpsockethandler.inl)

if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
 else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
 endif ()
endif ()

message (STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11 -fPIC)
 add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

add_library (${PROJECT_NAME} STATIC ${ACENetwork_LIB_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME} Common)

add_dependencies (${PROJECT_NAME} ACEStream)

set_target_properties (${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL NET_BUILD_DLL)
set_target_properties (${PROJECT_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (UNIX)
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME} iphlpapi.lib)
 target_link_libraries (${PROJECT_NAME} Wlanapi.lib)

 target_link_libraries (${PROJECT_NAME} ${ACE_LIBRARY})

 target_link_libraries (${PROJECT_NAME} Common)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME})

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}
        FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         net_asynch_netlinksockethandler.h
         net_asynch_netlinksockethandler.inl
         net_asynch_ssl_tcpsockethandler.h
         net_asynch_ssl_tcpsockethandler.inl
         net_asynch_tcpsockethandler.h
         net_asynch_tcpsockethandler.inl
         net_asynch_udpsockethandler.h
         net_asynch_udpsockethandler.inl
         net_common_tools.h
         net_common.h
         net_configuration.h
         net_connection_base.h
         net_connection_manager.h
         net_connection_manager.inl
         net_controller_base.h
         net_defines.h
         net_exports.h
         net_iconnection.h
         net_iconnectionmanager.h
         net_iconnector.h
         net_ilistener.h
         net_iparser.h
         net_ipconnection.h
         net_itransportlayer.h
         net_macros.h
         net_module_sockethandler.h
         net_module_sockethandler.inl
         net_netlinkconnection.h
         net_netlinkconnection.inl
         net_netlinksockethandler.h
         net_netlinksockethandler.inl
         net_packet_headers.h
         net_parser_base.h
         net_parser_base.inl
         net_protocol_layer.h
         net_remote_comm.h
         net_session_base.h
         net_session_base.inl
         net_sock_acceptor.h
         net_sock_connector.h
         net_sock_dgram.h
         net_sockethandler_base.h
         net_sockethandler_base.inl
         net_streamconnection_base.h
         net_streamconnection_base.inl
         net_stream_asynch_tcpsocket_base.h
         net_stream_asynch_tcpsocket_base.inl
         net_stream_asynch_udpsocket_base.h
         net_stream_asynch_udpsocket_base.inl
         net_stream_tcpsocket_base.h
         net_stream_tcpsocket_base.inl
         net_stream_udpsocket_base.h
         net_stream_udpsocket_base.inl
         net_tcpconnection_base.h
         net_tcpconnection_base.inl
         net_tcpsockethandler.h
         net_tcpsockethandler.inl
         net_transportlayer_base.h
         net_transportlayer_base.inl
         net_transportlayer_ip_cast.h
         net_transportlayer_netlink.h
         net_transportlayer_tcp.h
         net_transportlayer_udp.h
         net_udpconnection_base.h
         net_udpconnection_base.inl
         net_udpsockethandler.h
         net_udpsockethandler.inl
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)

add_subdirectory (client_server)
add_subdirectory (protocol)
add_subdirectory (wlan)
