﻿# *NOTE*: the first entry is the default option
if (UNIX)
 set (DHCP_CLIENT_IMPLEMENTATION "none" CACHE STRING "none (default)")
 set (DHCP_CLIENT_IMPLEMENTATION "dhclient" CACHE STRING "use ISC dhclient")
 set (DHCP_CLIENT_IMPLEMENTATION "dhcpd" CACHE STRING "use dhcpd")
 set (DHCP_CLIENT_IMPLEMENTATION "framework_dhcp" CACHE STRING "use framework DHCP (ACENetwork)")

 set_property (CACHE DHCP_CLIENT_IMPLEMENTATION PROPERTY STRINGS "none" "dhclient" "dhcpd" "framework_dhcp")
elseif (WIN32)
 set (DHCP_CLIENT_IMPLEMENTATION "wlanapi_dhcp" CACHE STRING "use wlanapi DHCP (default)")

 set_property (CACHE DHCP_CLIENT_IMPLEMENTATION PROPERTY STRINGS "wlanapi_dhcp")
endif ()

if (NOT (DEFINED DHCP_CLIENT_IMPLEMENTATION_LAST))
 set (DHCP_CLIENT_IMPLEMENTATION_LAST "NotAnImplementation" CACHE STRING "last DHCP client implementation used")
 mark_as_advanced (FORCE DHCP_CLIENT_IMPLEMENTATION_LAST)
endif ()
if (NOT (${DHCP_CLIENT_IMPLEMENTATION} MATCHES ${DHCP_CLIENT_IMPLEMENTATION_LAST}))
 if (UNIX)
  unset (DHCLIENT_USE CACHE)
  unset (DHCPD_USE CACHE)
  unset (FRAMEWORK_DHCP_USE CACHE)
 elseif (WIN32)
  unset (WLANAPI_DHCP_USE CACHE)
 endif ()
  set (DHCP_CLIENT_IMPLEMENTATION_LAST ${DHCP_CLIENT_IMPLEMENTATION} CACHE STRING "Updating DHCP Client Implementation Option" FORCE)
endif ()

if (UNIX)
 if (${DHCP_CLIENT_IMPLEMENTATION} MATCHES "dhclient")
  if (NOT DHCLIENT_SUPPORT)
   message (FATAL_ERROR "ISC dhclient not supported")
  endif (NOT DHCLIENT_SUPPORT)
  set (DHCLIENT_USE ON CACHE STRING "use ISC dhclient")
  mark_as_advanced (FORCE DHCLIENT_USE)
  add_definitions (-DDHCLIENT_USE)
 elseif (${DHCP_CLIENT_IMPLEMENTATION} MATCHES "dhcpd")
  if (NOT DHCPD_SUPPORT)
   message (FATAL_ERROR "dhcpd not supported")
  endif (NOT DHCPD_SUPPORT)
  set (DHCPD_USE ON CACHE STRING "use dhcpd")
  mark_as_advanced (FORCE DHCPD_USE)
  add_definitions (-DDHCPD_USE)
 elseif (${DHCP_CLIENT_IMPLEMENTATION} MATCHES "framework_dhcp")
  if (NOT FRAMEWORK_DHCP_SUPPORT)
   message (FATAL_ERROR "framework DHCP (ACENetwork) not supported")
  endif (NOT FRAMEWORK_DHCP_SUPPORT)
  set (FRAMEWORK_DHCP_USE ON CACHE STRING "use framework DHCP (ACENetwork)")
  mark_as_advanced (FORCE FRAMEWORK_DHCP_USE)
  add_definitions (-DFRAMEWORK_DHCP_USE)
 endif ()
elseif (WIN32)
 if (${DHCP_CLIENT_IMPLEMENTATION} MATCHES "wlanapi_dhcp")
  if (NOT WLANAPI_DHCP_SUPPORT)
   message (FATAL_ERROR "wlanapi DHCP not supported")
  endif (NOT WLANAPI_DHCP_SUPPORT)
  set (WLANAPI_USE ON CACHE STRING "use wlanapi DHCP")
  mark_as_advanced (FORCE WLANAPI_DHCP_USE)
  add_definitions (-DWLANAPI_DHCP_USE)
 endif ()
endif ()
