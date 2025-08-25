get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
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

if (NETLINK_SUPPORT)
 include_directories (${NETLINK_INCLUDE_DIRS})
endif (NETLINK_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/stat)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (EXECUTABLE_SOURCE client)
set (${EXECUTABLE_SOURCE}_SRCS
     net_client.cpp
     net_client_connector_common.h
     net_client_server_defines.h
     net_client_signalhandler.cpp
     net_client_signalhandler.h
     net_client_stream_common.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     stdafx.h
     test_u_client_common.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
     test_u_message.cpp
     test_u_message.h
     test_u_module_headerparser.cpp
     test_u_module_headerparser.h
     test_u_module_protocolhandler.cpp
     test_u_module_protocolhandler.h
     test_u_sessionmessage.cpp
     test_u_sessionmessage.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream.inl
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     test_u_udpconnection.cpp
     test_u_udpconnection.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE_SOURCE}_SRCS
      ${${EXECUTABLE_SOURCE}_SRCS}
      test_u_callbacks.cpp
      test_u_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (${EXECUTABLE_SOURCE}_SRCS
      ${${EXECUTABLE_SOURCE}_SRCS}
      net_server_wxui_base.cpp
      net_server_wxui_base.h)
endif (WXWIDGETS_SUPPORT)

add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SOURCE}_SRCS}
                etc/net_client.glade
                etc/net_client.gtk3)
set_property (TARGET ${EXECUTABLE_SOURCE} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE_SOURCE} Common)
add_dependencies (${EXECUTABLE_SOURCE} CommonUI)
#add_dependencies (${EXECUTABLE_SOURCE} CommonUI_GTK)
add_dependencies (${EXECUTABLE_SOURCE} ACEStream)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Client)

if (UNIX)
 target_link_options (${EXECUTABLE_SOURCE} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC X11)
endif (UNIX)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${SSL_LIBRARIES})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC Common)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Server)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Test_U)

# add PCH
target_precompile_headers (${EXECUTABLE_SOURCE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/net_client.glade etc/net_client.gtk3
          DESTINATION "etc/${PROJECT_NAME}/client_server" COMPONENT config)
elseif (WIN32)
 install (FILES etc/net_client.glade etc/net_client.gtk3
          DESTINATION "etc/client_server" COMPONENT config)
endif ()

########### next target ###############
set (EXECUTABLE_TARGET server)
set (${EXECUTABLE_TARGET}_SRCS
     net_client_server_defines.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     net_server.cpp
     net_server_listener_common.h
     net_server_signalhandler.cpp
     net_server_signalhandler.h
     net_server_stream_common.h
     stdafx.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_message.cpp
     test_u_message.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
     test_u_module_headerparser.cpp
     test_u_module_headerparser.h
     test_u_module_protocolhandler.cpp
     test_u_module_protocolhandler.h
     test_u_server_common.h
     test_u_sessionmessage.cpp
     test_u_sessionmessage.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream.inl
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     test_u_udpconnection.cpp
     test_u_udpconnection.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE_TARGET}_SRCS
      ${${EXECUTABLE_TARGET}_SRCS}
      test_u_callbacks.cpp
      test_u_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (${EXECUTABLE_TARGET}_SRCS
      ${${EXECUTABLE_TARGET}_SRCS}
#       net_server_wxui.cpp
#       net_server_wxui.h
#       net_server_wxui_callbacks.cpp
      net_server_wxui_base.cpp
      net_server_wxui_base.h)
endif (WXWIDGETS_SUPPORT)
if (WXWIDGETS_USE)
 list (ADD_ITEM ${EXECUTABLE_TARGET}_SRCS net_server_wxui.cpp)
 list (ADD_ITEM ${EXECUTABLE_TARGET}_SRCS net_server_wxui.h)
 list (ADD_ITEM ${EXECUTABLE_TARGET}_SRCS net_server_wxui_callbacks.cpp)
 list (REMOVE_ITEM ${EXECUTABLE_TARGET}_SRCS net_server.cpp)
endif (WXWIDGETS_USE)

add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET}_SRCS}
                etc/net_server.glade
                etc/net_server.gtk3)
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

add_dependencies (${EXECUTABLE_TARGET} Common)
add_dependencies (${EXECUTABLE_TARGET} CommonLog)
add_dependencies (${EXECUTABLE_TARGET} CommonSignal)
add_dependencies (${EXECUTABLE_TARGET} CommonTimer)

add_dependencies (${EXECUTABLE_TARGET} ACEStream)

add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Server)

if (UNIX)
 target_link_options (${EXECUTABLE_TARGET} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC X11)
endif (UNIX)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${SSL_LIBRARIES})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC Common)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Server)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Test_U)

# add PCH
target_precompile_headers (${EXECUTABLE_TARGET}
                           REUSE_FROM ${EXECUTABLE_SOURCE})

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/net_server.glade etc/net_server.gtk3
          DESTINATION "etc/${PROJECT_NAME}/client_server" COMPONENT config)
elseif (WIN32)
 install (FILES etc/net_server.glade etc/net_server.gtk3
          DESTINATION "etc/client_server" COMPONENT config)
endif ()

########### next target ###############
set (EXECUTABLE_TARGET_2 server_2)
set (${EXECUTABLE_TARGET_2}_SRCS
     net_client_server_defines.h
     net_client_timeouthandler.cpp
     net_client_timeouthandler.h
     net_server_2.cpp
     net_server_listener_common.h
     net_server_signalhandler.cpp
     net_server_signalhandler.h
     net_server_stream_common.h
     stdafx.h
     test_u_common_modules.h
     test_u_configuration.h
     test_u_connection_common.h
     test_u_connection_manager_common.h
     test_u_message.cpp
     test_u_message.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
     test_u_eventhandler_2.cpp
     test_u_eventhandler_2.h
     test_u_module_headerparser.cpp
     test_u_module_headerparser.h
     test_u_module_protocolhandler.cpp
     test_u_module_protocolhandler.h
     test_u_server_common.h
     test_u_sessionmessage.cpp
     test_u_sessionmessage.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream.inl
     test_u_stream_2.cpp
     test_u_stream_2.h
     test_u_stream_2.inl
     test_u_tcpconnection.cpp
     test_u_tcpconnection.h
     test_u_udpconnection.cpp
     test_u_udpconnection.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE_TARGET_2}_SRCS
      ${${EXECUTABLE_TARGET_2}_SRCS}
      test_u_callbacks_2.cpp
      test_u_callbacks_2.h)
endif (GTK_SUPPORT)
#if (WXWIDGETS_SUPPORT)
# set (${EXECUTABLE_TARGET_2}_SRCS
#      ${${EXECUTABLE_TARGET_2}_SRCS}
#       net_server_wxui.cpp
#       net_server_wxui.h
#       net_server_wxui_callbacks.cpp
#      net_server_wxui_base.cpp
#      net_server_wxui_base.h)
#endif (WXWIDGETS_SUPPORT)
#if (WXWIDGETS_USE)
# list (ADD_ITEM ${EXECUTABLE_TARGET_2}_SRCS net_server_wxui.cpp)
# list (ADD_ITEM ${EXECUTABLE_TARGET_2}_SRCS net_server_wxui.h)
# list (ADD_ITEM ${EXECUTABLE_TARGET_2}_SRCS net_server_wxui_callbacks.cpp)
# list (REMOVE_ITEM ${EXECUTABLE_TARGET_2}_SRCS net_server_2.cpp)
#endif (WXWIDGETS_USE)

add_executable (${EXECUTABLE_TARGET_2}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET_2}_SRCS}
                etc/net_server_2.gtk3)
set_property (TARGET ${EXECUTABLE_TARGET_2} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE_TARGET_2} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

add_dependencies (${EXECUTABLE_TARGET_2} Common)
add_dependencies (${EXECUTABLE_TARGET_2} CommonLog)
add_dependencies (${EXECUTABLE_TARGET_2} CommonSignal)
add_dependencies (${EXECUTABLE_TARGET_2} CommonTimer)

add_dependencies (${EXECUTABLE_TARGET_2} ACEStream)

add_dependencies (${EXECUTABLE_TARGET_2} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET_2} ${PROJECT_NAME}_Server)

if (UNIX)
 target_link_options (${EXECUTABLE_TARGET_2} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC X11)
endif (UNIX)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${SSL_LIBRARIES})
endif (SSL_SUPPORT)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC Common)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${PROJECT_NAME})

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${PROJECT_NAME}_Server)

target_link_libraries (${EXECUTABLE_TARGET_2} PUBLIC ${PROJECT_NAME}_Test_U)

# add PCH
target_precompile_headers (${EXECUTABLE_TARGET_2}
                           REUSE_FROM ${EXECUTABLE_SOURCE})

install (TARGETS ${EXECUTABLE_TARGET_2}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/net_server_2.gtk3
          DESTINATION "etc/${PROJECT_NAME}/client_server" COMPONENT config)
elseif (WIN32)
 install (FILES etc/net_server_2.gtk3
          DESTINATION "etc/client_server" COMPONENT config)
endif ()

##########################################

if (SSL_USE)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${SSL_LIB_PATH})
endif (SSL_USE)

set (ui_definition_file "")
set (ui_definition_file_2 "")
set (ui_definition_file_3 "")
if (GTK_USE)
 if (GTK3_USE)
  set (ui_definition_file "net_client.gtk3")
  set (ui_definition_file_2 "net_server.gtk3")
  set (ui_definition_file_3 "net_server_2.gtk3")
 elseif (GTK2_USE)
  set (ui_definition_file "net_client.glade")
  set (ui_definition_file_2 "net_server.glade")
 endif ()
elseif (WXWIDGETS_USE)
 set (ui_definition_file "net_client.xrc")
endif ()

file (TO_NATIVE_PATH "etc/${ui_definition_file}" ui_definition_file)
file (TO_NATIVE_PATH "etc/${ui_definition_file_2}" ui_definition_file_2)
file (TO_NATIVE_PATH "etc/${ui_definition_file_3}" ui_definition_file_3)
if (WIN32)
 string (REPLACE "\\" "\\\\" ui_definition_file ${ui_definition_file})
 string (REPLACE "\\" "\\\\" ui_definition_file_2 ${ui_definition_file_2})
 string (REPLACE "\\" "\\\\" ui_definition_file_3 ${ui_definition_file_3})
endif (WIN32)

set (ARGS "-l -t")
set (ARGS_2 "-l -t")
set (ARGS_3 "-l -t")
if (GTK_USE OR WXWIDGETS_USE)
 if (WIN32)
  set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
  set (ARGS_2 "-g.\\\\${ui_definition_file_2} ${ARGS_2}")
  set (ARGS_3 "-g.\\\\${ui_definition_file_3} ${ARGS_3}")
 else ()
  set (ARGS "-g./${ui_definition_file} ${ARGS}")
  set (ARGS_2 "-g./${ui_definition_file_2} ${ARGS_2}")
  set (ARGS_3 "-g./${ui_definition_file_3} ${ARGS_3}")
 endif ()
else ()
 set (ARGS "-g ${ARGS}")
 set (ARGS_2 "-g ${ARGS_2}")
 set (ARGS_3 "-g ${ARGS_3}")
endif (GTK_USE OR WXWIDGETS_USE)

if (UNIX)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         ARGS ${ARGS}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS ${ARGS_2}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
 create_target_launcher (${EXECUTABLE_TARGET_2}
                         ARGS ${ARGS_3}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_LIB_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_LIB_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${wxWidgets_LIB_DIR})
 endif (WXWIDGETS_SUPPORT)

 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH}
                           ${SSL_LIB_PATH})
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 create_target_launcher (${EXECUTABLE_SOURCE}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS}
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 create_target_launcher (${EXECUTABLE_TARGET}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS_2}
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET_2}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 create_target_launcher (${EXECUTABLE_TARGET_2}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS_3}
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/client_server
                         ENVIRONMENT foo=bar)
endif ()
