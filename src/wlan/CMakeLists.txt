if (UNIX)
 option (DBUS_SUPPORT "enable DBus support"
         ON)
endif ()

#set (JSON_LIB_FILE libjson-c.so.3)
#set (ISC_LIB_FILE libisc.a)
#set (DHCPCTL_LIB_FILE libdhcpctl.a)
#if (UNIX)
#  find_library (JSON_LIBRARY ${JSON_LIB_FILE}
#                HINTS /usr/lib
#                PATH_SUFFIXES x86_64-linux-gnu
#                DOC "searching for ${JSON_LIB_FILE} (system paths)")
#  find_library (ISC_LIBRARY ${ISC_LIB_FILE}
#                HINTS /usr/lib
#                PATH_SUFFIXES x86_64-linux-gnu
#                DOC "searching for ${ISC_LIB_FILE} (system paths)")
#  find_library (DHCPCTL_LIBRARY ${DHCPCTL_LIB_FILE}
#               HINTS /usr/lib
#               PATH_SUFFIXES x86_64-linux-gnu
#               DOC "searching for ${DHCPCTL_LIB_FILE} (system paths)")
#endif ()
#if (UNIX)
# if (NOT JSON_LIBRARY)
#  message (FATAL_ERROR "could not find ${JSON_LIB_FILE}, aborting")
# else ()
#  message (STATUS "Found json library \"${JSON_LIBRARY}\"")
# endif ()
# if (NOT ISC_LIBRARY)
#  message (FATAL_ERROR "could not find ${ISC_LIB_FILE}, aborting")
# else ()
#  message (STATUS "Found isc library \"${ISC_LIBRARY}\"")
# endif ()
# if (NOT DHCPCTL_LIBRARY)
#  message (FATAL_ERROR "could not find ${DHCPCTL_LIB_FILE}, aborting")
# else ()
#  message (STATUS "Found dhcpctl library \"${DHCPCTL_LIBRARY}\"")
# endif ()
#endif ()

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_LIBXML REQUIRED libxml-2.0)
#  pkg_check_modules (PKG_DHCP REQUIRED bind-exports isc-dhcp-client)
 if (DBUS_SUPPORT)
  pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
  pkg_check_modules (PKG_NETWORKMANAGER REQUIRED NetworkManager)
 endif ()
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
add_definitions (-DCOMMON_HAS_DLL)
if (DBUS_SUPPORT)
 add_definitions (-DCOMMON_DBUS_HAS_DLL)
endif ()

if (UNIX)
  include_directories (/usr/include/bind-export)
  if (DBUS_SUPPORT)
  include_directories (${PKG_DBUS_INCLUDE_DIRS})
  include_directories (${PKG_NETWORKMANAGER_INCLUDE_DIRS})

  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/dbus)
 endif ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})
endif ()
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
     net_wlan_monitor_statemachine.cpp
     net_wlan_monitor_statemachine.h
     net_wlan_tools.cpp
     net_wlan_tools.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_WLAN STATIC
             ${ACENetwork_WLAN_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_WLAN Common)
if (UNIX)
 if (DBUS_SUPPORT)
  add_dependencies (${PROJECT_NAME}_WLAN CommonDBus)
 endif ()
endif ()

set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES DEFINE_SYMBOL NET_WLAN_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)
#set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# add PCH
set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${PROJECT_NAME}_WLAN PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${PROJECT_NAME}_WLAN)

if (UNIX)
# target_link_libraries (${PROJECT_NAME}_WLAN ${JSON_LIBRARY})
# target_link_libraries (${PROJECT_NAME}_WLAN ${PKG_LIBXML_LIBRARIES})
# target_link_libraries (${PROJECT_NAME}_WLAN ${ISC_LIBRARY})
# target_link_libraries (${PROJECT_NAME}_WLAN ${DHCPCTL_LIBRARY})
# if (DBUS_SUPPORT)
#  target_link_libraries (${PROJECT_NAME}_WLAN CommonDBus)
# endif ()
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_WLAN ${ACE_LIBRARY})
endif ()

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
         net_wlan_monitor_statemachine.h
         net_wlan_tools.h
         stdafx.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_WLAN
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
