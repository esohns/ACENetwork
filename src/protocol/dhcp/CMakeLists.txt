include (${CMAKE_CURRENT_SOURCE_DIR}/scripts/PreDHCPClientImplementation.cmake)

if (UNIX)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavutil)
endif (UNIX)

add_library (${PROJECT_NAME}_Protocol_DHCP STATIC "")

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
add_definitions (-DNET_HAS_DLL)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC ${BIND9_CFLAGS})
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC /usr/include/bind9)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
#                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party>)

# target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACE_TAO/ACE>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_DHCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/cmake>)
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
#     dhcp_exports.h
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
  PRIVATE stdafx.cpp
  PRIVATE stdafx.h
  PRIVATE
  ${absolute_source_files}
  INTERFACE
  ${absolute_script_files})

#set_source_files_properties (${PROJECT_NAME}_Protocol_dhcp_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_dhcp_SRCS PROPERTIES COMPILE_DEFINITIONS -DDHCP_BUILD_DLL)

#message (STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
#set (LLVM_COMPILER_ID_STRING "Clang")
#string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
#string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
#if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
# add_compile_options (-std=c++0x -fPIC)
#endif ()

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif ()

set_property (TARGET ${PROJECT_NAME}_Protocol_DHCP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP Common)
add_dependencies (${PROJECT_NAME}_Protocol_DHCP ACEStream)
#add_dependencies (${PROJECT_NAME}_Protocol_DHCP ${PROJECT_NAME})

set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES DEFINE_SYMBOL NET_PROTOCOL_DHCP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 if (DHCLIENT_USE)
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${DHCPCTL_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${OMAPI_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${IRS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${ISC_CFG_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${DNS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${GEOIP_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${PKG_SSL_LIBRARIES})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${GSS_KRB5_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${KRB5_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${GSS_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${PKG_KRB5_GSSAPI_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${ISC_LIBRARY})
#  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${JSON_LIBRARY})
  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${PKG_JSON_LIBRARIES})

  target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${PKG_LIBXML_LIBRARIES})
 endif (DHCLIENT_USE)
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ntdll.lib)
 target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_DHCP Common)
target_link_libraries (${PROJECT_NAME}_Protocol_DHCP CommonTimer)

target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ACEStream)

target_link_libraries (${PROJECT_NAME}_Protocol_DHCP ${PROJECT_NAME})

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_DHCP PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_DHCP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_DHCP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         dhcp_bisector.h
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
         dhcp_module_bisector.h
         dhcp_module_bisector.inl
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
