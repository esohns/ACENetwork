cmake_minimum_required (VERSION 2.8)

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)
# *TODO*: repair WIN32 pkgconfig support

if (UNIX)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
else ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../)

############## next target ################
set (${PROJECT_NAME}_Client_LIB_SRCS
     net_client_asynchconnector.cpp
     net_client_asynchconnector.h
     net_client_asynchconnector.inl
     net_client_connector.cpp
     net_client_connector.h
     net_client_connector.inl
     net_client_ssl_connector.cpp
     net_client_ssl_connector.h
     net_client_ssl_connector.inl
     net_client_defines.h
     net_client_exports.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Client SHARED ${ACENetwork_Client_LIB_SRCS})

set_target_properties (${PROJECT_NAME}_Client PROPERTIES DEFINE_SYMBOL NET_CLIENT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Client PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

############## next target ################
set (${PROJECT_NAME}_Server_LIB_SRCS
     net_server_asynchlistener.cpp
     net_server_asynchlistener.h
     net_server_asynchlistener.inl
     net_server_common_tools.cpp
     net_server_common_tools.h
     net_server_defines.h
     net_server_exports.h
     net_server_listener.cpp
     net_server_listener.h
     net_server_listener.inl
     net_server_ssl_listener.cpp
     net_server_ssl_listener.h
     net_server_ssl_listener.inl
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Server SHARED ${ACENetwork_Server_LIB_SRCS})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Server ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Server Common)
endif ()

set_target_properties (${PROJECT_NAME}_Server PROPERTIES DEFINE_SYMBOL NET_SERVER_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Server PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Client ${PROJECT_NAME}_Server
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         net_client_asynchconnector.h
         net_client_connector.h
         net_client_defines.h
         net_client_exports.h
         net_server_asynchlistener.h
         net_server_asynchlistener.inl
         net_server_common_tools.h
         net_server_defines.h
         net_server_exports.h
         net_server_listener.h
         net_server_listener.inl
         net_server_ssl_listener.h
         net_server_ssl_listener.inl
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Client ${PROJECT_NAME}_Server
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
