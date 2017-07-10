if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
 else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
 endif ()
endif ()

set (${PROJECT_NAME}_Test_I_SRCS
     test_i_common.h
     test_i_curses_defines.h
     test_i_defines.h
     test_i_gtk_common.h
     test_i_stream_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

add_subdirectory (bittorrent_client)
add_subdirectory (IRC_client)
add_subdirectory (url_stream_load)

add_custom_target (test_i
                   COMMAND echo building test_i...)
add_dependencies (test_i bittorrent_client bittorrent_client_gui IRC_client IRC_client_gui url_stream_load)

if (NOT BUILD_TEST_I)
 set_target_properties (test_i PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif ()
