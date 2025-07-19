#include (iconv)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
include (PreXMLImplementation)
#include (SetXMLImplementation)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (PreMonitorImplementation)
include (SetMonitorImplementation)

add_library (${PROJECT_NAME}_WLAN STATIC "")

if (WIN32)
# include (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/scripts/Macros.cmake)
 get_WIN32_Version (WINDOWS_VERSION_MAJOR WINDOWS_VERSION_MINOR WINDOWS_VERSION_MICRO)
# message (STATUS "WINDOWS_VERSION: ${WINDOWS_VERSION_MAJOR}.${WINDOWS_VERSION_MINOR}.${WINDOWS_VERSION_MICRO}")
 if (WINDOWS_VERSION_MAJOR GREATER 5)
  option (WINXP_SUPPORT "enable Windows XP (TM) support" OFF)
  add_feature_info (WinXP WINXP_SUPPORT "support Windows XP (TM)")
  if (WINXP_SUPPORT)
   add_definitions (-DWINXP_SUPPORT)
  endif (WINXP_SUPPORT)
 endif (WINDOWS_VERSION_MAJOR GREATER 5)
endif (WIN32)

if (UNIX)
 if (DBUS_SUPPORT)
  add_definitions (-DSD_BUS_SUPPORT)
 endif (DBUS_SUPPORT)
endif (UNIX)

if (UNIX)
 if (NL80211_SUPPORT)
  if (PKG_LIBNL3_FOUND)
   target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                               $<BUILD_INTERFACE:${PKG_LIBNL3_INCLUDE_DIRS}>)
  elseif (PKG_LIBNL2_FOUND)
   target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                               $<BUILD_INTERFACE:${PKG_LIBNL2_INCLUDE_DIRS}>)
  else ()
   message (FATAL_ERROR "libnl not found, aborting")
  endif ()
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              "$<BUILD_INTERFACE:${PKG_DBUS_INCLUDE_DIRS}>")
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              "$<BUILD_INTERFACE:${PKG_NM_INCLUDE_DIRS}>")
 endif (DBUS_SUPPORT)

 if (DHCLIENT_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC ${BIND9_CFLAGS})
 endif (DHCLIENT_SUPPORT)

 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src>)
 if (DBUS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/dbus>)
 endif (DBUS_SUPPORT)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/error>)
 if (NL80211_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/math>)
 endif (NL80211_SUPPORT)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml>)

 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/lib>)
elseif (WIN32)
 if (LIBXML2_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              "$<BUILD_INTERFACE:${LIBXML2_INCLUDE_DIRS}>")
 endif (LIBXML2_SUPPORT)

 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer>)
 if (LIBXML2_SUPPORT)
  target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml>)
 endif (LIBXML2_SUPPORT)

 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src>)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib>)
endif ()
target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_WLAN PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../protocol/dhcp>)
endif (UNIX)

############## next target ################
set (source_files
     net_wlan_common.h
     net_wlan_configuration.h
     net_wlan_defines.h
     net_wlan_imanager.h
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
     net_wlan_monitor_common.h
     net_wlan_monitor_statemachine.cpp
     net_wlan_monitor_statemachine.h
     net_wlan_tools.cpp
     net_wlan_tools.h)
if (UNIX)
 set (source_files
      ${source_files}
      net_wlan_dhcp_common.h)
 if (WEXT_SUPPORT)
  set (source_files
       ${source_files}
       net_wlan_monitor_wext.inl
       net_wlan_tools_linux_wext.cpp)
 endif (WEXT_SUPPORT)
 if (NL80211_SUPPORT)
  set (source_files
       ${source_files}
       net_wlan_monitor_nl80211.inl
       net_wlan_tools_linux_nl80211.cpp)
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  set (source_files
       ${source_files}
       net_wlan_monitor_dbus.inl
       net_wlan_tools_linux_dbus.cpp)
 endif (DBUS_SUPPORT)
elseif (WIN32)
 if (WLANAPI_SUPPORT)
  set (source_files
       ${source_files}
       net_wlan_monitor_win32_wlanapi.inl
       net_wlan_profile_xml_handler.cpp
       net_wlan_profile_xml_handler.h
       net_wlan_tools_win32_wlanapi.cpp)
  set (script_files
       etc/wlanapi_profile_template.xml)
 endif (WLANAPI_SUPPORT)
endif ()

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       ${abs_source_file})
  #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
unset (absolute_script_files)
if (WIN32)
 if (WLANAPI_SUPPORT)
  foreach (script_file ${script_files})
   get_filename_component (abs_script_file ${script_file} ABSOLUTE)
   list (APPEND absolute_script_files
         ${absolute_script_file})
  endforeach ()
 endif (WLANAPI_SUPPORT)
endif (WIN32)
target_sources (${PROJECT_NAME}_WLAN
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files}
                INTERFACE ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_WLAN
                           PUBLIC stdafx.h)

add_dependencies (${PROJECT_NAME}_WLAN Common)
if (UNIX)
 if (NL80211_SUPPORT)
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  add_dependencies (${PROJECT_NAME}_WLAN CommonDBus)
 endif (DBUS_SUPPORT)
endif (UNIX)

add_dependencies (${PROJECT_NAME}_WLAN ${PROJECT_NAME})
add_dependencies (${PROJECT_NAME}_WLAN ${PROJECT_NAME}_Protocol_DHCP)

if (UNIX)
 if (WEXT_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC iw)
 endif (WEXT_SUPPORT)
 if (NL80211_SUPPORT)
  if (PKG_LIBNL3_FOUND)
   target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PKG_LIBNL3_LIBRARIES})
  elseif (PKG_LIBNL2_FOUND)
   target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PKG_LIBNL2_LIBRARIES})
  else ()
   message (FATAL_ERROR "libnl not found, aborting")
  endif ()
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PKG_DBUS_LIBRARIES})
  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PKG_NM_LIBRARIES})

  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC CommonDBus)
 endif (DBUS_SUPPORT)
elseif (WIN32)
 if (MSXML_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC msxml2.lib)
 elseif (LIBXML2_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${LIBXML2_LIBRARY})
 endif ()

# target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC CommonXML)
endif ()
target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PROJECT_NAME})
target_link_libraries (${PROJECT_NAME}_WLAN PUBLIC ${PROJECT_NAME}_Protocol_DHCP)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_WLAN
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (include_files
     net_wlan_common.h
     net_wlan_configuration.h
     net_wlan_defines.h
     net_wlan_imanager.h
     net_wlan_imonitor.h
     net_wlan_inetmonitor.h
     net_wlan_inetmonitor.inl
     net_wlan_monitor.h
     net_wlan_monitor.inl
     net_wlan_monitor_base.h
     net_wlan_monitor_base.inl
     net_wlan_monitor_common.h
     net_wlan_monitor_statemachine.h
     net_wlan_tools.h)
if (UNIX)
 set (include_files
      ${include_files}
      net_wlan_dhcp_common.h)
 if (WEXT_SUPPORT)
  set (include_files
       ${include_files}
       net_wlan_monitor_wext.inl
       net_wlan_tools_linux_wext.cpp)
 endif (WEXT_SUPPORT)
 if (NL80211_SUPPORT)
  set (include_files
       ${include_files}
       net_wlan_monitor_nl80211.inl
       net_wlan_tools_linux_nl80211.cpp)
 endif (NL80211_SUPPORT)
 if (DBUS_SUPPORT)
  set (include_files
       ${include_files}
       net_wlan_monitor_dbus.inl
       net_wlan_tools_linux_dbus.cpp)
 endif (DBUS_SUPPORT)
elseif (WIN32)
 set (include_files
      ${include_files}
      net_wlan_monitor_win32_wlanapi.inl
      net_wlan_profile_xml_handler.h)
endif ()

install (FILES
         ${include_files}
         DESTINATION include)
if (WIN32)
install (FILES
         etc/wlanapi_profile_template.xml
         DESTINATION etc)
endif (WIN32)
install (TARGETS ${PROJECT_NAME}_WLAN
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
