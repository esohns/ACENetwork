find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libACEStream/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libACEStream/src/modules/misc)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../client_server)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

########### next target ###############
set (${PROJECT_NAME}_Protocol_IRC_SRCS
     stdafx.cpp
     stdafx.h
     irc_bisector.cpp
     irc_bisector.h
     irc_codes.h
     irc_common.h
     irc_common_modules.h
     irc_configuration.h
     irc_defines.h
     irc_exports.h
     irc_icontrol.h
     irc_message.cpp
     irc_message.h
     irc_parser.cpp
     irc_parser.h
     irc_parser_driver.cpp
     irc_parser_driver.h
     irc_record.cpp
     irc_record.h
     irc_scanner.cpp
     irc_scanner.h
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
     irc_session.cpp
     irc_session.h
     irc_session.inl
     irc_sessionmessage.cpp
     irc_sessionmessage.h
     irc_statemachine_registration.cpp
     irc_statemachine_registration.h
     irc_stream.cpp
     irc_stream.h
     irc_stream.inl
     irc_stream_common.h
     irc_tools.cpp
     irc_tools.h
     location.hh
     position.hh
     stack.hh)

#set_source_files_properties (${PROJECT_NAME}_Protocol_IRC_SRCS PROPERTIES COMPILE_DEFINITIONS DATADIR="/usr/local/share/libACENetwork/protocol/irc")
#set_source_files_properties (${PROJECT_NAME}_Protocol_IRC_SRCS PROPERTIES COMPILE_DEFINITIONS -DIRC_BUILD_DLL)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_library (${PROJECT_NAME}_Protocol_IRC SHARED ${ACENetwork_Protocol_IRC_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME}_Protocol_IRC Common)
add_dependencies (${PROJECT_NAME}_Protocol_IRC ACEStream)

set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES DEFINE_SYMBOL IRC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

#target_link_libraries (${PROJECT_NAME}_Protocol_IRC ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Protocol_IRC ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Protocol_IRC Common)
 target_link_libraries (${PROJECT_NAME}_Protocol_IRC ACEStream)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME}_Protocol_IRC PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Protocol_IRC)

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
         irc_exports.h
         irc_icontrol.h
         irc_message.h
         irc_parser.h
         irc_parser_driver.h
         irc_record.h
         irc_scanner.h
         irc_message.h
         irc_module_parser.h
         irc_module_parser.inl
         irc_module_bisector.h
         irc_module_bisector.inl
         irc_module_streamer.h
         irc_module_streamer.inl
         irc_network.h
         irc_session.h
         irc_session.inl
         irc_sessionmessage.h
         irc_statemachine_registration.h
         irc_stream.h
         irc_stream.inl
         irc_stream_common.h
         irc_tools.h
         location.hh
         position.hh
         stack.hh
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Protocol_IRC
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
