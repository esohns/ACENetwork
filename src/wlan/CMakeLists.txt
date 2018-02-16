#include (CMakeDependentOption)

if (UNIX)
 option (WEXT_SUPPORT "enable wext support" ON)
 option (NL80211_SUPPORT "enable nl80211 support" ON)
 option (DBUS_SUPPORT "enable DBus support" ON)
 option (DHCLIENT_SUPPORT "enable ISC dhclient support" ON)
# option (DBUS_SUPPORT "enable DBus support" OFF)
 # *NOTE*: implies DHCLIENT_SUPPORT off
# CMAKE_DEPENDENT_OPTION (DBUS_SUPPORT "enable DBus support" OFF
#                         "NOT DHCLIENT_SUPPORT" OFF)
elseif (WIN32)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/scripts/cmake_macros.txt)
 get_WIN32_Version (WINDOWS_VERSION_MAJOR WINDOWS_VERSION_MINOR WINDOWS_VERSION_MICRO)
# message (STATUS "WINDOWS_VERSION: ${WINDOWS_VERSION_MAJOR}.${WINDOWS_VERSION_MINOR}.${WINDOWS_VERSION_MICRO}")
 if (WINDOWS_VERSION_MAJOR GREATER 5)
  option (WINXP_SUPPORT "enable WinXP support" OFF)
  if (WINXP_SUPPORT)
    add_definitions (-DWINXP_SUPPORT)
  endif ()
 endif (WINDOWS_VERSION_MAJOR GREATER 5)
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 if (WEXT_SUPPORT)
  check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
#   pkg_check_modules (PKG_WEXT REQUIRED iwlib)
  add_definitions (-DWEXT_SUPPORT)
 endif (WEXT_SUPPORT)
 if (NL80211_SUPPORT)
  pkg_check_modules (PKG_NL80211 REQUIRED libnl-3.0 libnl-genl-3.0)
  add_definitions (-DNL80211_SUPPORT)
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
  pkg_check_modules (PKG_NETWORKMANAGER REQUIRED NetworkManager)
  add_definitions (-DDBUS_SUPPORT)
 endif (DBUS_SUPPORT)
 if (DHCLIENT_SUPPORT)
#  pkg_check_modules (PKG_DHCP REQUIRED bind-exports isc-dhcp-client)
  set (JSON_LIB_FILE libjson-c.so.3)
  find_library (JSON_LIBRARY ${JSON_LIB_FILE}
                HINTS /usr/lib
                PATH_SUFFIXES x86_64-linux-gnu
                DOC "searching for ${JSON_LIB_FILE} (system paths)")
  if (NOT JSON_LIBRARY)
   message (FATAL_ERROR "could not find ${JSON_LIB_FILE}, aborting")
  else ()
   message (STATUS "Found json library \"${JSON_LIBRARY}\"")
  endif ()

  set (ISC_LIB_FILE libisc.a)
  #set (ISC_LIB_FILE libisc.so.166)
  find_library (ISC_LIBRARY ${ISC_LIB_FILE}
                HINTS /usr/lib64 /usr/lib
                PATH_SUFFIXES dhcp x86_64-linux-gnu
                DOC "searching for ${ISC_LIB_FILE} (system paths)")
  if (NOT ISC_LIBRARY)
   message (FATAL_ERROR "could not find ${ISC_LIB_FILE}, aborting")
  else ()
   message (STATUS "Found isc library \"${ISC_LIBRARY}\"")
  endif ()

  set (DHCPCTL_LIB_FILE libdhcpctl.a)
  find_library (DHCPCTL_LIBRARY ${DHCPCTL_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES dhcp x86_64-linux-gnu
               DOC "searching for ${DHCPCTL_LIB_FILE} (system paths)")
  if (NOT DHCPCTL_LIBRARY)
   message (FATAL_ERROR "could not find ${DHCPCTL_LIB_FILE}, aborting")
  else ()
   message (STATUS "Found dhcpctl library \"${DHCPCTL_LIBRARY}\"")
  endif ()
# pkg_check_modules (PKG_LIBXML REQUIRED libxml-2.0)
  add_definitions (-DDHCLIENT_SUPPORT)
 endif (DHCLIENT_SUPPORT)
endif (UNIX)

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
if (UNIX)
 if (DBUS_SUPPORT)
  add_definitions (-DCOMMON_DBUS_HAS_DLL)
 endif ()
endif ()
#find_package (CommonXML ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
#              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${CommonXML_DIR})
# message (FATAL_ERROR "could not find package \"CommonXML\", aborting")
#endif ()
#add_definitions (-DCOMMONXML_HAS_DLL)

if (UNIX)
 if (NL80211_SUPPORT)
  include_directories (${PKG_NL80211_INCLUDE_DIRS})
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  include_directories (${PKG_DBUS_INCLUDE_DIRS})
  include_directories (${PKG_NETWORKMANAGER_INCLUDE_DIRS})

  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/dbus)
 endif ()
 if (DHCLIENT_SUPPORT)
  include_directories (/usr/include/bind-export)
  include_directories (/usr/include/dhcp)
 endif ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/xml)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/xml)
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
     net_wlan_inetmonitor.cpp
     net_wlan_inetmonitor.h
     net_wlan_inetmonitor.inl
     net_wlan_monitor.cpp
     net_wlan_monitor.h
     net_wlan_monitor.inl
     net_wlan_monitor_base.cpp
     net_wlan_monitor_base.h
     net_wlan_monitor_base.inl
     net_wlan_monitor_statemachine.cpp
     net_wlan_monitor_statemachine.h
     net_wlan_tools.cpp
     net_wlan_tools.h
     stdafx.cpp
     stdafx.h)
if (WIN32)
 set (${PROJECT_NAME}_WLAN_LIB_SRCS
      ${ACENetwork_WLAN_LIB_SRCS}
      net_wlan_profile_xml_handler.cpp
      net_wlan_profile_xml_handler.h)
endif ()

add_library (${PROJECT_NAME}_WLAN STATIC
             ${ACENetwork_WLAN_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_WLAN Common)
if (UNIX)
 if (NL80211_SUPPORT)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  add_dependencies (${PROJECT_NAME}_WLAN CommonDBus)
 endif (DBUS_SUPPORT)
endif (UNIX)

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
# target_link_libraries (${PROJECT_NAME}_WLAN CommonXML)
endif ()

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_WLAN
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (${PROJECT_NAME}_WLAN_LIB_HDRS
     net_wlan_common.h
     net_wlan_configuration.h
     net_wlan_defines.h
     net_wlan_exports.h
     net_wlan_imonitor.h
     net_wlan_inetmonitor.h
     net_wlan_inetmonitor.inl
     net_wlan_monitor.h
     net_wlan_monitor.inl
     net_wlan_monitor_base.h
     net_wlan_monitor_base.inl
     net_wlan_monitor_statemachine.h
     net_wlan_tools.h)
if (WIN32)
 set (${PROJECT_NAME}_WLAN_LIB_HDRS
      ${ACENetwork_WLAN_LIB_HDRS}
      net_wlan_profile_xml_handler.h)
endif ()

install (FILES
         ${ACENetwork_WLAN_LIB_HDRS}
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_WLAN
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
