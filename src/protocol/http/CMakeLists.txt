add_library (${PROJECT_NAME}_Protocol_HTTP STATIC "")

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

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
# target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC ${BIND9_CFLAGS})

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/dec>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/lib>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)
elseif (WIN32)
# target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
#                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/antlr4/runtime/Cpp/runtime/src>)

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/dec>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/net>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACEStream/src/modules/stat>)

 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party>)
 target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/flex>)
endif ()
target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Protocol_HTTP PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../client_server>)

########### next target ###############
set (source_files
     http_codes.h
     http_common.h
     http_defines.h
     http_iparser.h
     http_message.cpp
     http_message.h
     http_parser.cpp
     http_parser.h
     http_parser_driver.cpp
     http_parser_driver.h
     http_parser_driver.inl
     http_scanner.cpp
     http_scanner.h
     http_message.cpp
     http_message.h
     http_message.inl
     http_module_bisector.cpp
     http_module_bisector.h
     http_module_bisector.inl
     http_module_parser.cpp
     http_module_parser.h
     http_module_parser.inl
     http_module_streamer.cpp
     http_module_streamer.h
     http_module_streamer.inl
     http_network.h
     http_sessionmessage.cpp
     http_sessionmessage.h
     http_sessionmessage.inl
     http_stream.cpp
     http_stream.h
     http_stream.inl
     http_stream_common.h
     http_tools.cpp
     http_tools.h
#     HTTPParser.cpp
#     HTTPParser.h
#     HTTPParserBaseListener.cpp
#     HTTPParserBaseListener.h
#     HTTPParserBaseVisitor.cpp
#     HTTPParserBaseVisitor.h
#     HTTPParserListener.cpp
#     HTTPParserListener.h
#     HTTPParserVisitor.cpp
#     HTTPParserVisitor.h
#     HTTPScanner.cpp
#     HTTPScanner.h
     )
set (script_files
#     scripts/bisector.l
     scripts/scanner.l
     scripts/parser.y)
set (table_files)
#     etc/http_scanner.tab)

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
target_sources (${PROJECT_NAME}_Protocol_HTTP
  PRIVATE stdafx.cpp
  PRIVATE stdafx.h
  PRIVATE
  ${absolute_source_files}
  INTERFACE
  ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_Protocol_HTTP
                           PUBLIC stdafx.h)

#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/RPG/config")
#set_source_files_properties (${PROJECT_NAME}_Protocol_HTTP_SRCS PROPERTIES COMPILE_DEFINITIONS -DHTTP_BUILD_DLL)

if (WIN32)
## *NOTE*: generate .def file and export all symbols on Windows
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
 source_group (scripts FILES ${PROJECT_NAME}_Protocol_HTTP_SCRIPTS)
# source_group (lex_tables FILES ${PROJECT_NAME}_Protocol_HTTP_TABLES)
# source_group (exports FILES etc/exports.def)
endif ()

# *NOTE*: this is needed to export yydebug, so it can be imported from template
#         instances. Note that there is no way to #define YYDEBUG in the .y
#         parser definition file, as the generated '%code requires' sections are
#         inserted after the first '#define YYDEBUG' statements.
# *TODO*: this holds for GNU Bison > 2.4.2
#add_definitions (-DYYDEBUG)

set_property (TARGET ${PROJECT_NAME}_Protocol_HTTP PROPERTY FOLDER "protocols")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP Common)
add_dependencies (${PROJECT_NAME}_Protocol_HTTP ACEStream)

#set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES DEFINE_SYMBOL HTTP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)
if (WIN32 AND MSVC)
 set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/etc/exports.def\"")
endif (WIN32 AND MSVC)

#target_link_libraries (${PROJECT_NAME}_Protocol_HTTP PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_HTTP PUBLIC ${ACE_LIBRARY})
endif (WIN32)
target_link_libraries (${PROJECT_NAME}_Protocol_HTTP PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Protocol_HTTP PUBLIC ACEStream)

# add PCH
#set_target_properties (${PROJECT_NAME}_Protocol_HTTP PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME}_Protocol_HTTP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Protocol_HTTP
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         http_codes.h
         http_common.h
         http_defines.h
         http_iparser.h
         http_message.h
         http_parser.h
         http_parser_driver.h
         http_parser_driver.inl
         http_scanner.h
#         http_scanner.tab
         http_message.h
         http_message.inl
         http_module_bisector.h
         http_module_bisector.inl
         http_module_parser.h
         http_module_parser.inl
         http_module_streamer.h
         http_module_streamer.inl
         http_network.h
         http_sessionmessage.h
         http_sessionmessage.inl
         http_stream.h
         http_stream.inl
         http_stream_common.h
         http_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_HTTP
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
