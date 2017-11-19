#if (UNIX)
## include (CheckLibraryExists)
## check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
# include (FindThreads)
# set (CMAKE_THREAD_PREFER_PTHREAD ON)
# find_package (Threads)
#endif ()

#set (ACE_LIB_FILE libACE.so)
#if (UNIX)
# if (DEFINED ENV{ACE_ROOT})
#  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
#                HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE
#                PATHS ENV ACE_ROOT
#                PATH_SUFFIXES lib
#                DOC "searching for ${ACE_LIB_FILE}")
# else ()
#  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
#                DOC "searching for ${ACE_LIB_FILE} (system paths)")
# endif ()
#elseif (WIN32)
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  set (LIB_FILE_SUFFIX d)
# endif ()
# unset (ACE_LIB_FILE)
# set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
# find_library (ACE_LIBRARY ${ACE_LIB_FILE}
#               PATHS ENV ACE_ROOT
#               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
#               DOC "searching for ${ACE_LIB_FILE}")
#endif ()
##if (NOT EXISTS ACE_LIBRARY)
#if (NOT ACE_LIBRARY)
# message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
#else ()
# message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
#endif ()
#add_definitions (-DACE_HAS_DLL)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
 pkg_check_modules (PKG_NETWORKMANAGER REQUIRED NetworkManager)
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_DBUS_HAS_DLL)

if (UNIX)
 include_directories (${PKG_DBUS_INCLUDE_DIRS})
 #include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../dbus)
 include_directories (${PKG_NETWORKMANAGER_INCLUDE_DIRS})

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/dbus)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})
endif ()
#include_directories (${Common_INCLUDE_DIRS})

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

############## next target ################
set (${PROJECT_NAME}_WLAN_LIB_SRCS
     net_wlan_common.h
     net_wlan_configuration.h
     net_wlan_defines.h
     net_wlan_exports.h
     net_wlan_imonitor.h
     net_wlan_inetmonitor.h
     net_wlan_inetmonitor.inl
     net_wlan_inetmonitor.cpp
     net_wlan_monitor.h
     net_wlan_monitor.inl
     net_wlan_monitor.cpp
     net_wlan_tools.cpp
     net_wlan_tools.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_WLAN STATIC
             ${ACENetwork_WLAN_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_WLAN Common)
add_dependencies (${PROJECT_NAME}_WLAN Common_DBus)

set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES DEFINE_SYMBOL NET_WLAN_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${PROJECT_NAME}_WLAN)

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_WLAN ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_WLAN Common_DBus)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_WLAN
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         net_wlan_common.h
         net_wlan_configuration.h
         net_wlan_defines.h
         net_wlan_exports.h
         net_wlan_imonitor.h
         net_wlan_inetmonitor.h
         net_wlan_inetmonitor.inl
         net_wlan_inetmonitor.cpp
         net_wlan_monitor.h
         net_wlan_monitor.inl
         net_wlan_monitor.cpp
         net_wlan_tools.h
         stdafx.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_WLAN
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
