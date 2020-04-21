#include (CMakeDependentOption)
include (FeatureSummary)

if (UNIX)
#  pkg_check_modules (PKG_DHCP REQUIRED bind-exports isc-dhcp-client)
 set (DHCLIENT_SUPPORT_ENABLE ON)
# pkg_check_modules (PKG_JSON json-c)
# if (NOT PKG_JSON_FOUND)
#  message (WARNING "could not find json-c, continuing")
#  set (DHCLIENT_SUPPORT_ENABLE OFF)
# endif (NOT PKG_JSON_FOUND)
#  set (JSON_LIB_FILE libjson-c.so.3)
  set (JSON_LIB_FILE libjson-c.so.4)
  find_library (JSON_LIBRARY ${JSON_LIB_FILE}
                HINTS /usr/lib
                PATH_SUFFIXES x86_64-linux-gnu
                DOC "searching for ${JSON_LIB_FILE} (system paths)")
  if (JSON_LIBRARY)
   message (STATUS "Found json library \"${JSON_LIBRARY}\"")
  else ()
   message (FATAL_ERROR "could not find ${JSON_LIB_FILE}, aborting")
  endif (JSON_LIBRARY)

#  set (ISC_LIB_FILE libisc-export.so)
# set (ISC_LIB_FILE libisc-export.so.1100)
 set (ISC_LIB_FILE libisc.so)
 find_library (ISC_LIBRARY ${ISC_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES bind9-export dhcp x86_64-linux-gnu
               DOC "searching for ${ISC_LIB_FILE} (system paths)")
 if (NOT ISC_LIBRARY)
  message (WARNING "could not find ${ISC_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found ISC library \"${ISC_LIBRARY}\"")
 endif (NOT ISC_LIBRARY)

# set (GSS_LIB_FILE libgss.so.3)
#  set (GSS_LIB_FILE libgssglue.so)
#  find_library (GSS_LIBRARY ${GSS_LIB_FILE}
#                HINTS /usr/lib64 /usr/lib
#                PATH_SUFFIXES x86_64-linux-gnu
#                DOC "searching for ${GSS_LIB_FILE} (system paths)")
#  if (NOT GSS_LIBRARY)
#   message (FATAL_ERROR "could not find ${GSS_LIB_FILE}, aborting")
#  else ()
#   message (STATUS "Found generic security services library \"${GSS_LIBRARY}\"")
#  endif (NOT GSS_LIBRARY)

 pkg_check_modules (PKG_KRB5_GSSAPI krb5-gssapi)
 if (NOT PKG_KRB5_GSSAPI_FOUND)
  message (WARNING "could not find krb5-gssapi, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 endif (NOT PKG_KRB5_GSSAPI_FOUND)
#  set (KRB5_LIB_FILE libkrb5.so.3)
#  find_library (KRB5_LIBRARY ${KRB5_LIB_FILE}
#                HINTS /usr/lib
#                PATH_SUFFIXES x86_64-linux-gnu
#                DOC "searching for ${KRB5_LIB_FILE} (system paths)")
#  if (NOT KRB5_LIBRARY)
#   message (FATAL_ERROR "could not find ${KRB5_LIB_FILE}, aborting")
#  else ()
#   message (STATUS "Found MIT kerberos library \"${KRB5_LIBRARY}\"")
#  endif ()

#  set (GSS_KRB5_LIB_FILE libgssapi_krb5.so.2)
#  find_library (GSS_KRB5_LIBRARY ${GSS_KRB5_LIB_FILE}
#                HINTS /usr/lib
#                PATH_SUFFIXES x86_64-linux-gnu
#                DOC "searching for ${GSS_KRB5_LIB_FILE} (system paths)")
#  if (NOT GSS_KRB5_LIBRARY)
#   message (FATAL_ERROR "could not find ${GSS_KRB5_LIB_FILE}, aborting")
#  else ()
#   message (STATUS "Found MIT kerberos library \"${GSS_KRB5_LIBRARY}\"")
#  endif (NOT GSS_KRB5_LIBRARY)

# set (GEOIP_LIB_FILE libGeoIP.so.1)
 set (GEOIP_LIB_FILE libGeoIP.so)
 find_library (GEOIP_LIBRARY ${GEOIP_LIB_FILE}
               HINTS /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${GEOIP_LIB_FILE} (system paths)")
 if (NOT GEOIP_LIBRARY)
  message (WARNING "could not find ${GEOIP_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found GeoIP library \"${GEOIP_LIBRARY}\"")
 endif ()

# set (DNS_LIB_FILE libdns-export.so)
#  set (DNS_LIB_FILE libdns-export.so.1104)
set (DNS_LIB_FILE libdns.so)
 find_library (DNS_LIBRARY ${DNS_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES bind9-export dhcp x86_64-linux-gnu
               DOC "searching for ${DNS_LIB_FILE} (system paths)")
 if (NOT DNS_LIBRARY)
  message (WARNING "could not find ${DNS_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found dns library \"${DNS_LIBRARY}\"")
 endif (NOT DNS_LIBRARY)

# set (ISC_CFG_LIB_FILE libisccfg-export.so)
#  set (ISC_CFG_LIB_FILE libisccfg-export.so.163)
 set (ISC_CFG_LIB_FILE libisccfg.so.163)
 find_library (ISC_CFG_LIBRARY ${ISC_CFG_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES bind9-export dhcp x86_64-linux-gnu
               DOC "searching for ${ISC_CFG_LIB_FILE} (system paths)")
 if (NOT ISC_CFG_LIBRARY)
  message (WARNING "could not find ${ISC_CFG_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found isccfg library \"${ISC_CFG_LIBRARY}\"")
 endif (NOT ISC_CFG_LIBRARY)

#  set (IRS_LIB_FILE libirs.a)
# set (IRS_LIB_FILE libirs-export.so.161)
 set (IRS_LIB_FILE libirs.so.161)
 find_library (IRS_LIBRARY ${IRS_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES bind9-export dhcp x86_64-linux-gnu
               DOC "searching for ${IRS_LIB_FILE} (system paths)")
 if (NOT IRS_LIBRARY)
  message (WARNING "could not find ${IRS_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found irs library \"${IRS_LIBRARY}\"")
 endif (NOT IRS_LIBRARY)

 set (OMAPI_LIB_FILE libomapi.a)
# set (OMAPI_LIB_FILE libomapi.so)
 find_library (OMAPI_LIBRARY ${OMAPI_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES dhcp x86_64-linux-gnu
               DOC "searching for ${OMAPI_LIB_FILE} (system paths)")
 if (NOT OMAPI_LIBRARY)
  message (WARNING "could not find ${OMAPI_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found omapi library \"${OMAPI_LIBRARY}\"")
 endif (NOT OMAPI_LIBRARY)

  set (DHCPCTL_LIB_FILE libdhcpctl.a)
# set (DHCPCTL_LIB_FILE libdhcpctl.so)
 find_library (DHCPCTL_LIBRARY ${DHCPCTL_LIB_FILE}
               HINTS /usr/lib64 /usr/lib
               PATH_SUFFIXES dhcp x86_64-linux-gnu
               DOC "searching for ${DHCPCTL_LIB_FILE} (system paths)")
 if (NOT DHCPCTL_LIBRARY)
  message (WARNING "could not find ${DHCPCTL_LIB_FILE}, continuing")
  set (DHCLIENT_SUPPORT_ENABLE OFF)
 else ()
  message (STATUS "Found dhcpctl library \"${DHCPCTL_LIBRARY}\"")
 endif (NOT DHCPCTL_LIBRARY)
 option (DHCLIENT_SUPPORT "enable ISC dhclient support" ${DHCLIENT_SUPPORT_ENABLE})
 add_feature_info (dhclient DHCLIENT_SUPPORT "support ISC dhclient")
 if (DHCLIENT_SUPPORT_ENABLE)
  add_definitions (-DDHCLIENT_SUPPORT)
 endif (DHCLIENT_SUPPORT_ENABLE)

 option (DHCPD_SUPPORT "enable dhcpd support" OFF)
 add_feature_info (dhcpd DHCPD_SUPPORT "support dhcpd")
 #add_definitions (-DDHCPD_SUPPORT)
elseif (WIN32)
 option (WLANAPI_DHCP_SUPPORT "enable wlanapi DHCP support" ON)
 add_feature_info (wlanapi_dhcp WLANAPI_DHCP_SUPPORT "support wlanapi DHCP")
 add_definitions (-DWLANAPI_DHCP_SUPPORT)
endif ()
option (FRAMEWORK_DHCP_SUPPORT "enable framework DHCP support" ON)
add_feature_info (framework_dhcp FRAMEWORK_DHCP_SUPPORT "support framework DHCP (ACENetwork)")
add_definitions (-DFRAMEWORK_DHCP_SUPPORT)
