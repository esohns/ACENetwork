add_library (${PROJECT_NAME}_Protocol_PCP STATIC "")

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

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_PCP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
#     pcp_bisector.cpp
#     pcp_bisector.h
     pcp_common.h
     pcp_common_modules.h
     pcp_configuration.h
     pcp_codes.h
     pcp_defines.h
#     pcp_exports.h
     pcp_isession.h
     pcp_message.cpp
     pcp_message.h
     pcp_parser.cpp
     pcp_parser.h
     pcp_parser_driver.cpp
     pcp_parser_driver.h
     pcp_scanner.cpp
     pcp_scanner.h
     pcp_message.cpp
     pcp_message.h
     pcp_message.inl
#     pcp_module_bisector.cpp
#     pcp_module_bisector.h
#     pcp_module_bisector.inl
     pcp_module_parser.cpp
     pcp_module_parser.h
     pcp_module_parser.inl
     pcp_module_streamer.cpp
     pcp_module_streamer.h
     pcp_module_streamer.inl
     pcp_network.h
     pcp_session.cpp
     pcp_session.h
     pcp_session.inl
     pcp_sessionmessage.cpp
     pcp_sessionmessage.h
     pcp_sessionmessage.inl
     pcp_stream.cpp
     pcp_stream.h
     pcp_stream.inl
     pcp_stream_common.h
     pcp_tools.cpp
     pcp_tools.h)
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
target_sources (${PROJECT_NAME}_Protocol_PCP
  PRIVATE stdafx.cpp
  PRIVATE stdafx.h
  PRIVATE
  ${absolute_source_files}
  INTERFACE
  ${absolute_script_files})

#set_source_files_properties (${PROJECT_NAME}_Protocol_PCP_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_PCP_SRCS PROPERTIES COMPILE_DEFINITIONS -DPCP_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_DCHP_SCRIPTS)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Protocol_PCP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_PCP Common)
add_dependencies (${PROJECT_NAME}_Protocol_PCP ACEStream)
#add_dependencies (${PROJECT_NAME}_Protocol_PCP ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Protocol_PCP PROPERTIES DEFINE_SYMBOL NET_Protocol_PCP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_PCP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_PCP ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Protocol_PCP ${ACE_LIBRARY})
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Protocol_PCP ntdll.lib)
 target_link_libraries (${PROJECT_NAME}_Protocol_PCP ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Protocol_PCP Common)
target_link_libraries (${PROJECT_NAME}_Protocol_PCP CommonTimer)

target_link_libraries (${PROJECT_NAME}_Protocol_PCP ACEStream)

target_link_libraries (${PROJECT_NAME}_Protocol_PCP ${PROJECT_NAME})

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_PCP PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_PCP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_PCP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
#         pcp_bisector.h
         pcp_codes.h
         pcp_common.h
         pcp_common_modules.h
         pcp_configuration.h
         pcp_defines.h
#         pcp_exports.h
         pcp_isession.h
         pcp_message.h
         pcp_parser.h
         pcp_parser_driver.h
         pcp_scanner.h
         pcp_message.h
         pcp_message.inl
#         pcp_module_bisector.h
#         pcp_module_bisector.inl
         pcp_module_parser.h
         pcp_module_parser.inl
         pcp_module_streamer.h
         pcp_module_streamer.inl
         pcp_network.h
         pcp_session.h
         pcp_session.inl
         pcp_sessionmessage.h
         pcp_sessionmessage.inl
         pcp_stream.h
         pcp_stream.inl
         pcp_stream_common.h
         pcp_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_PCP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
