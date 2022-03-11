# *TODO*: this should not be necessary...
include (OS)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (PreDHCPClientImplementation)
include (SetDHCPClientImplementation)

add_library (${PROJECT_NAME}_Protocol_DHCP STATIC "")

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${BIND9_CFLAGS})
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC /usr/include/bind9)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
#     dhcp_bisector.cpp
#     dhcp_bisector.h
     dhcp_common.h
     dhcp_common_modules.h
     dhcp_configuration.h
     dhcp_codes.h
     dhcp_defines.h
     dhcp_message.cpp
     dhcp_message.h
     dhcp_parser.cpp
     dhcp_parser.h
     dhcp_parser_driver.cpp
     dhcp_parser_driver.h
     dhcp_scanner.cpp
     dhcp_scanner.h
     dhcp_message.cpp
     dhcp_message.h
     dhcp_message.inl
#     dhcp_module_bisector.cpp
#     dhcp_module_bisector.h
#     dhcp_module_bisector.inl
     dhcp_module_discover.cpp
     dhcp_module_discover.h
     dhcp_module_discover.inl
     dhcp_module_parser.cpp
     dhcp_module_parser.h
     dhcp_module_parser.inl
     dhcp_module_streamer.cpp
     dhcp_module_streamer.h
     dhcp_module_streamer.inl
     dhcp_network.h
     dhcp_sessionmessage.cpp
     dhcp_sessionmessage.h
     dhcp_sessionmessage.inl
     dhcp_stream.cpp
     dhcp_stream.h
     dhcp_stream.inl
     dhcp_stream_common.h
     dhcp_tools.cpp
     dhcp_tools.h)
set (script_files
     scripts/scanner.l
     scripts/parser.y)

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
  get_filename_component (abs_source_file ${source_file} ABSOLUTE)
  list (APPEND absolute_source_files
        ${abs_source_file})
  #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
unset (absolute_script_files)
foreach (script_file ${script_files})
  get_filename_component (abs_script_file ${script_file} ABSOLUTE)
  list (APPEND absolute_script_files
        ${absolute_script_file})
endforeach ()
target_sources (${PROJECT_NAME}_Protocol_DHCP
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files}
                INTERFACE ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_Protocol_DHCP
                           PUBLIC stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_dhcp_SRCS PROPERTIES COMPILE_DEFINITIONS -DDHCP_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Protocol_DHCP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP Common)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP ACEStream)
#add_dependencies (${PROJECT_NAME}_Protocol_DHCP ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES DEFINE_SYMBOL NET_PROTOCOL_DHCP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (UNIX)
 if (DHCLIENT_USE)
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${DHCPCTL_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${OMAPI_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${IRS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${ISC_CFG_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${DNS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${GEOIP_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${PKG_SSL_LIBRARIES})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${GSS_KRB5_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${KRB5_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${GSS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${PKG_KRB5_GSSAPI_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${ISC_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${JSON_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${PKG_JSON_LIBRARIES})

  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${PKG_LIBXML_LIBRARIES})
 endif (DHCLIENT_USE)
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ntdll.lib)
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC CommonTimer)

target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ACEStream)

target_link_libraries (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${PROJECT_NAME})

# add PCH
target_precompile_headers (${PROJECT_NAME}_Protocol_DHCP
                           PUBLIC stdafx.h)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_DHCP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
#         dhcp_bisector.h
         dhcp_codes.h
         dhcp_common.h
         dhcp_common_modules.h
         dhcp_configuration.h
         dhcp_defines.h
#         dhcp_exports.h
         dhcp_message.h
         dhcp_parser.h
         dhcp_parser_driver.h
         dhcp_scanner.h
         dhcp_message.h
         dhcp_message.inl
#         dhcp_module_bisector.h
#         dhcp_module_bisector.inl
         dhcp_module_discover.h
         dhcp_module_discover.inl
         dhcp_module_parser.h
         dhcp_module_parser.inl
         dhcp_module_streamer.h
         dhcp_module_streamer.inl
         dhcp_network.h
         dhcp_sessionmessage.h
         dhcp_sessionmessage.inl
         dhcp_stream.h
         dhcp_stream.inl
         dhcp_stream_common.h
         dhcp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_DHCP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
