# configure C/C++ exceptions
include (SetCppExceptions)

include (Threads)

include (SSLConfig)
include (NetlinkConfig)

include (ACE)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
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
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)

if (UNIX)
 add_link_options ("LINKER:--as-needed")
endif (UNIX)

# ........................................

add_library (${PROJECT_NAME} STATIC "")

if (SSL_SUPPORT)
 target_include_directories (${PROJECT_NAME} PUBLIC ${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 if (NETLINK_SUPPORT)
  target_include_directories (${PROJECT_NAME} PUBLIC ${PKG_NETLINK_INCLUDE_DIRS})
 endif (NETLINK_SUPPORT)

 if (DHCLIENT_SUPPORT)
#  set (BIND9_CONFIG_EXECUTABLE /usr/bin/bind9-export-config)
  set (BIND9_CONFIG_EXECUTABLE /usr/bin/bind9-config)
  execute_process (COMMAND ${BIND9_CONFIG_EXECUTABLE} --cflags
                   OUTPUT_VARIABLE BIND9_CFLAGS
                   ERROR_VARIABLE BIND9_ERROR
                   RESULT_VARIABLE BIND9_RESULT
                   OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (NOT ${BIND9_RESULT} EQUAL 0)
  # message (FATAL_ERROR "Command \"${BIND9_CONFIG_EXECUTABLE} --cflags\" failed with output:\n${BIND9_CFLAGS}\n${BIND9_ERROR}, aborting")
  endif (NOT ${BIND9_RESULT} EQUAL 0)
#  message (STATUS "BIND9_CFLAGS: ${BIND9_CFLAGS}")
  string (REGEX REPLACE "^-I(.+)$" "\\1" BIND9_CFLAGS "${BIND9_CFLAGS}")
#  message (STATUS "BIND9_CFLAGS: ${BIND9_CFLAGS}")
  target_include_directories (${PROJECT_NAME} PUBLIC ${BIND9_CFLAGS})
  target_include_directories (${PROJECT_NAME} PUBLIC /usr/include/bind9)
 endif (DHCLIENT_SUPPORT)

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/parser>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/xml>)

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/3rd_party/tree>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/bison>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/wlan>)

############## next target ################
set (source_files
     net_asynch_tcpsockethandler.cpp
     net_asynch_tcpsockethandler.h
     net_asynch_tcpsockethandler.inl
     net_asynch_udpsockethandler.cpp
     net_asynch_udpsockethandler.h
     net_asynch_udpsockethandler.inl
     net_common_tools.cpp
     net_common_tools.h
     net_common.h
     net_configuration.cpp
     net_configuration.h
     net_connection_base.cpp
     net_connection_base.h
     net_connection_base.inl
     net_connection_configuration.cpp
     net_connection_configuration.h
     net_connection_configuration.inl
     net_connection_manager.cpp
     net_connection_manager.h
     net_connection_manager.inl
     net_controller_base.cpp
     net_controller_base.h
     net_defines.h
#     net_exports.h
     net_iconnection.h
     net_iconnectionmanager.h
     net_iconnector.h
     net_ilistener.h
     net_ipconnection.cpp
     net_ipconnection.h
     net_itransportlayer.h
     net_macros.h
#     net_module_sockethandler.cpp
#     net_module_sockethandler.h
#     net_module_sockethandler.inl
     net_os_tools.cpp
     net_os_tools.h
     net_packet_headers.h
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
     net_socket_common.h
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
if (UNIX)
 if (NETLINK_SUPPORT)
  set (source_files
       net_asynch_netlinksockethandler.cpp
       net_asynch_netlinksockethandler.h
       net_asynch_netlinksockethandler.inl
       net_netlink_address.cpp
       net_netlink_address.h
       net_netlinkconnection.cpp
       net_netlinkconnection.h
       net_netlinkconnection.inl
       net_netlinksockethandler.cpp
       net_netlinksockethandler.h
       net_netlinksockethandler.inl
       net_transportlayer_netlink.cpp
       net_transportlayer_netlink.h
       ${source_files})
 endif (NETLINK_SUPPORT)
endif (UNIX)
if (SSL_SUPPORT)
 set (source_files
      net_asynch_ssl_tcpsockethandler.cpp
      net_asynch_ssl_tcpsockethandler.h
      net_asynch_ssl_tcpsockethandler.inl
      ${source_files})
endif (SSL_SUPPORT)
set (source_files
     ${CMAKE_CURRENT_BINARY_DIR}/../${PROJECT_NAME}_config.h
     ${source_files})
if (WIN32)
 set (source_files
      ../include/targetver.h
      ${source_files})
endif (WIN32)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}
                           PUBLIC stdafx.h)

#add_library (${PROJECT_NAME} STATIC ${ACENetwork_LIB_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)

add_dependencies (${PROJECT_NAME} Common)

add_dependencies (${PROJECT_NAME} ACEStream)

#set_target_properties (${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL NET_BUILD_DLL)
#set_target_properties (${PROJECT_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (SSL_SUPPORT)
 target_link_libraries (${PROJECT_NAME} ${SSL_LIBRARIES})

 target_link_libraries (${PROJECT_NAME} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)
if (WIN32)
 target_link_libraries (${PROJECT_NAME} Ntdll.lib)
 target_link_libraries (${PROJECT_NAME} Iphlpapi.lib)
 target_link_libraries (${PROJECT_NAME} Wlanapi.lib)

 target_link_libraries (${PROJECT_NAME} Common)
endif (WIN32)
target_link_libraries (${PROJECT_NAME} ${ACE_LIBRARY})

if (UNIX)
 target_link_libraries (${PROJECT_NAME} ${PROJECT_NAME}_WLAN)
endif (UNIX)

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
         net_connection_configuration.h
         net_connection_configuration.inl
         net_connection_manager.h
         net_connection_manager.inl
         net_controller_base.h
         net_defines.h
         net_iconnection.h
         net_iconnectionmanager.h
         net_iconnector.h
         net_ilistener.h
         net_ipconnection.h
         net_itransportlayer.h
         net_macros.h
         net_netlinkconnection.h
         net_netlinkconnection.inl
         net_netlinksockethandler.h
         net_netlinksockethandler.inl
         net_os_tools.h
         net_packet_headers.h
         net_protocol_layer.h
         net_remote_comm.h
         net_session_base.h
         net_session_base.inl
         net_sock_acceptor.h
         net_sock_connector.h
         net_sock_dgram.h
         net_socket_common.h
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

# ////////////////////////////////////////

add_subdirectory (client_server)
add_subdirectory (protocol)
add_subdirectory (wlan)
