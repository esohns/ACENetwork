add_library (${PROJECT_NAME}_Protocol_SMTP STATIC "")

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
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
# target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src>)
# target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_SMTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
     smtp_codes.h
     smtp_common.h
     smtp_common_modules.h
     smtp_configuration.h
     smtp_defines.h
#     smtp_exports.h
     smtp_iparser.h
     smtp_message.cpp
     smtp_message.h
     smtp_message.inl
     smtp_module_parser.cpp
     smtp_module_parser.h
     smtp_module_parser.inl
     smtp_module_send.cpp
     smtp_module_send.h
     smtp_module_send.inl
     smtp_module_streamer.cpp
     smtp_module_streamer.h
     smtp_module_streamer.inl
     smtp_network.h
     smtp_parser.cpp
     smtp_parser.h
     smtp_parser_driver.cpp
     smtp_parser_driver.h
     smtp_parser_driver.inl
     smtp_scanner.cpp
     smtp_scanner.h
     smtp_sessionmessage.cpp
     smtp_sessionmessage.h
     smtp_sessionmessage.inl
     smtp_stream.cpp
     smtp_stream.h
     smtp_stream.inl
     smtp_stream_common.h
     smtp_tools.cpp
     smtp_tools.h)
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
target_sources (${PROJECT_NAME}_Protocol_SMTP
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE
                ${absolute_source_files}
                INTERFACE
                ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_Protocol_SMTP
                           PUBLIC stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_SMTP_SRCS PROPERTIES COMPILE_DEFINITIONS -DSMTP_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Protocol_SMTP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_SMTP Common)
add_dependencies (${PROJECT_NAME}_Protocol_SMTP ACEStream)
#add_dependencies (${PROJECT_NAME}_Protocol_SMTP ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Protocol_SMTP PROPERTIES DEFINE_SYMBOL NET_Protocol_SMTP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_SMTP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC ${ACE_LIBRARY})
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC ntdll.lib)
 target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC CommonMath)
target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC CommonTimer)

target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC ACEStream)

target_link_libraries (${PROJECT_NAME}_Protocol_SMTP PUBLIC ${PROJECT_NAME})

# add PCH
target_precompile_headers (${PROJECT_NAME}_Protocol_SMTP
                           PUBLIC stdafx.h)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_SMTP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         smtp_codes.h
         smtp_common.h
         smtp_common_modules.h
         smtp_configuration.h
         smtp_defines.h
    #     smtp_exports.h
         smtp_iparser.h
         smtp_message.h
         smtp_message.inl
         smtp_module_parser.h
         smtp_module_parser.inl
         smtp_module_send.h
         smtp_module_send.inl
         smtp_module_streamer.h
         smtp_module_streamer.inl
         smtp_network.h
         smtp_parser.h
         smtp_parser_driver.h
         smtp_parser_driver.inl
         smtp_scanner.h
         smtp_sessionmessage.h
         smtp_sessionmessage.inl
         smtp_stream.h
         smtp_stream.inl
         smtp_stream_common.h
         smtp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_SMTP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
