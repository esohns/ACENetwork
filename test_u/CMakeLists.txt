if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
 else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
 endif ()
endif ()

set (${PROJECT_NAME}_Test_U_SRCS
     test_u_common.h
     test_u_connection_common.h
     test_u_defines.h
     test_u_gtk_common.h
     test_u_socket_common.h
     test_u_stream_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_U_SRCS
#         DESTINATION include)

add_subdirectory (client_server)
add_subdirectory (DHCP_client)
add_subdirectory (file_server)
add_subdirectory (HTTP_decoder)
add_subdirectory (WLAN_monitor)

add_custom_target (test_u ALL
                   COMMENT "building test_u..."
                   SOURCES ${ACENetwork_Test_U_SRCS})
add_dependencies (test_u client server DHCP_client file_server httpdecoder WLAN_monitor)

if (NOT ${BUILD_TEST_U})
 set_target_properties (test_u PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_u PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
endif ()
