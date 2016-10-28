if (WIN32)
 add_definitions (-DWIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # C++ exception handling
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd") # multithreaded debug dll
 else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")  # multithreaded dll
 endif ()
endif ()

set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 add_definitions (-D_DEBUG)
 if (DEBUG_DEBUGGER)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

set (${PROJECT_NAME}_Test_I_SRCS
     test_i_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

add_subdirectory (bittorrent_client)
add_subdirectory (IRC_client)
