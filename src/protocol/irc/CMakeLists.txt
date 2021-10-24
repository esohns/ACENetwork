add_library (${PROJECT_NAME}_Protocol_IRC STATIC "")

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/3rd_party/bison>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACEStream/src/modules/stat>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/bison>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)

 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)

 target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_IRC PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
     irc_bisector.cpp
     irc_bisector.h
     irc_codes.h
     irc_common.h
     irc_common_modules.h
     irc_configuration.h
     irc_defines.h
#     irc_exports.h
     irc_icontrol.h
     irc_message.cpp
     irc_message.h
     irc_module_parser.cpp
     irc_module_parser.h
     irc_module_parser.inl
     irc_module_bisector.cpp
     irc_module_bisector.h
     irc_module_bisector.inl
     irc_module_streamer.cpp
     irc_module_streamer.h
     irc_module_streamer.inl
     irc_network.h
     irc_parser.cpp
     irc_parser.h
     irc_parser_driver.cpp
     irc_parser_driver.h
     irc_record.cpp
     irc_record.h
     irc_scanner.cpp
     irc_scanner.h
     irc_session.cpp
     irc_session.h
     irc_session.inl
     irc_sessionmessage.cpp
     irc_sessionmessage.h
     irc_sessionmessage.inl
     irc_statemachine_registration.cpp
     irc_statemachine_registration.h
     irc_stream.cpp
     irc_stream.h
     irc_stream.inl
     irc_stream_common.h
     irc_tools.cpp
     irc_tools.h)
set (script_files
     scripts/bisector.l
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
target_sources (${PROJECT_NAME}_Protocol_IRC
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files}
                INTERFACE ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_Protocol_IRC
                           PUBLIC stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_IRC_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/libACENetwork/protocol/irc")
#set_source_files_properties (${PROJECT_NAME}_Protocol_IRC_SRCS PROPERTIES COMPILE_DEFINITIONS -DIRC_BUILD_DLL)

#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "CLang")
# add_compile_options (-fPIC)
#endif ()

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_IRC_SCRIPTS)
 source_group (exports FILES etc/exports.def)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Protocol_IRC PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_IRC Common)
add_dependencies (${PROJECT_NAME}_Protocol_IRC ACEStream)

#set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES DEFINE_SYMBOL IRC_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_IRC PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_IRC PUBLIC ${ACE_LIBRARY})
endif (WIN32)
target_link_libraries (${PROJECT_NAME}_Protocol_IRC PUBLIC Common)

target_link_libraries (${PROJECT_NAME}_Protocol_IRC PUBLIC ACEStream)

# add PCH
#set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${PROJECT_NAME}_Protocol_IRC)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_IRC
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         irc_bisector.h
         irc_codes.h
         irc_common.h
         irc_common_modules.h
         irc_configuration.h
         irc_defines.h
#         irc_exports.h
         irc_icontrol.h
         irc_message.h
         irc_module_parser.h
         irc_module_parser.inl
         irc_module_bisector.h
         irc_module_bisector.inl
         irc_module_streamer.h
         irc_module_streamer.inl
         irc_network.h
         irc_parser.h
         irc_parser_driver.h
         irc_record.h
         irc_scanner.h
         irc_session.h
         irc_session.inl
         irc_sessionmessage.h
         irc_sessionmessage.inl
         irc_statemachine_registration.h
         irc_stream.h
         irc_stream.inl
         irc_stream_common.h
         irc_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_IRC
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
