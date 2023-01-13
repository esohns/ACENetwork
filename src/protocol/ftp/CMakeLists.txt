add_library (${PROJECT_NAME}_Protocol_FTP STATIC "")

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
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
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
# target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_FTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
     ftp_codes.h
     ftp_common.h
     ftp_common_modules.h
     ftp_configuration.h
     ftp_defines.h
     ftp_iparser.h
     ftp_message.cpp
     ftp_message.h
     ftp_message.inl
     ftp_module_parser.cpp
     ftp_module_parser.h
     ftp_module_parser.inl
     ftp_module_send.cpp
     ftp_module_send.h
     ftp_module_send.inl
     ftp_module_streamer.cpp
     ftp_module_streamer.h
     ftp_module_streamer.inl
     ftp_network.h
     ftp_parser.cpp
     ftp_parser.h
     ftp_parser_driver.cpp
     ftp_parser_driver.h
     ftp_parser_driver.inl
     ftp_scanner.cpp
     ftp_scanner.h
     ftp_sessionmessage.cpp
     ftp_sessionmessage.h
     ftp_sessionmessage.inl
     ftp_stream.cpp
     ftp_stream.h
     ftp_stream.inl
     ftp_stream_common.h
     ftp_tools.cpp
     ftp_tools.h)
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
target_sources (${PROJECT_NAME}_Protocol_FTP
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files}
#                INTERFACE ${absolute_script_files}
                scripts/scanner.l
                scripts/parser.y)
target_precompile_headers (${PROJECT_NAME}_Protocol_FTP
                           PUBLIC stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_FTP_SRCS PROPERTIES COMPILE_DEFINITIONS -DFTP_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Protocol_FTP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_FTP Common)
add_dependencies (${PROJECT_NAME}_Protocol_FTP ACEStream)
#add_dependencies (${PROJECT_NAME}_Protocol_FTP ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Protocol_FTP PROPERTIES DEFINE_SYMBOL NET_Protocol_FTP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_FTP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC ${ACE_LIBRARY})
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC ntdll.lib)
 target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC CommonMath)
target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC CommonTimer)

target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC ACEStream)

target_link_libraries (${PROJECT_NAME}_Protocol_FTP PUBLIC ${PROJECT_NAME})

# add PCH
target_precompile_headers (${PROJECT_NAME}_Protocol_FTP
                           PUBLIC stdafx.h)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_FTP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         ftp_codes.h
         ftp_common.h
         ftp_common_modules.h
         ftp_configuration.h
         ftp_defines.h
         ftp_iparser.h
         ftp_message.h
         ftp_message.inl
         ftp_module_parser.h
         ftp_module_parser.inl
         ftp_module_send.h
         ftp_module_send.inl
         ftp_module_streamer.h
         ftp_module_streamer.inl
         ftp_network.h
         ftp_parser.h
         ftp_parser_driver.h
         ftp_parser_driver.inl
         ftp_scanner.h
         ftp_sessionmessage.h
         ftp_sessionmessage.inl
         ftp_stream.h
         ftp_stream.inl
         ftp_stream_common.h
         ftp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_FTP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
