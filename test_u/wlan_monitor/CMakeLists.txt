option (GTK3_SUPPORT "enable Gtk3 support"
        ON)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
 check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)

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
 find_library (ISC_LIBRARY ${ISC_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6/bind /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES lib x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${ISC_LIB_FILE} (system paths)")
 if (NOT ISC_LIBRARY)
  message (FATAL_ERROR "could not find ${ISC_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found isc library \"${ISC_LIBRARY}\"")
 endif ()

 set (ISC_CFG_LIB_FILE libisccfg.a)
 find_library (ISC_CFG_LIBRARY ${ISC_CFG_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6/bind /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES lib x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${ISC_CFG_LIB_FILE} (system paths)")
 if (NOT ISC_CFG_LIBRARY)
  message (FATAL_ERROR "could not find ${ISC_CFG_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found isccfg library \"${ISC_CFG_LIBRARY}\"")
 endif ()

  set (KRB5_LIB_FILE libkrb5.so.3)
  find_library (KRB5_LIBRARY ${KRB5_LIB_FILE}
                HINTS /usr/lib
                PATH_SUFFIXES x86_64-linux-gnu
                DOC "searching for ${KRB5_LIB_FILE} (system paths)")
  if (NOT KRB5_LIBRARY)
   message (FATAL_ERROR "could not find ${KRB5_LIB_FILE}, aborting")
  else ()
   message (STATUS "Found MIT kerberos library \"${KRB5_LIBRARY}\"")
  endif ()

 set (GSS_KRB5_LIB_FILE libgssapi_krb5.so.2)
 find_library (GSS_KRB5_LIBRARY ${GSS_KRB5_LIB_FILE}
               HINTS /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${GSS_KRB5_LIB_FILE} (system paths)")
 if (NOT GSS_KRB5_LIBRARY)
  message (FATAL_ERROR "could not find ${GSS_KRB5_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found MIT kerberos library \"${GSS_KRB5_LIBRARY}\"")
 endif ()

 set (GSS_LIB_FILE libgss.so.3)
 find_library (GSS_LIBRARY ${GSS_LIB_FILE}
               HINTS /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${GSS_LIB_FILE} (system paths)")
 if (NOT GSS_LIBRARY)
  message (FATAL_ERROR "could not find ${GSS_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found generic security services library \"${GSS_LIBRARY}\"")
 endif ()

 set (IRS_LIB_FILE libirs.a)
 find_library (IRS_LIBRARY ${IRS_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6/bind /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES lib x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${IRS_LIB_FILE} (system paths)")
 if (NOT IRS_LIBRARY)
  message (FATAL_ERROR "could not find ${IRS_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found irs library \"${IRS_LIBRARY}\"")
 endif ()

 set (GEOIP_LIB_FILE libGeoIP.so.1)
 find_library (GEOIP_LIBRARY ${GEOIP_LIB_FILE}
               HINTS /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${GEOIP_LIB_FILE} (system paths)")
 if (NOT GEOIP_LIBRARY)
  message (FATAL_ERROR "could not find ${GEOIP_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found GeoIP library \"${GEOIP_LIBRARY}\"")
 endif ()

 set (DNS_LIB_FILE libdns.a)
 find_library (DNS_LIBRARY ${DNS_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6/bind /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES lib x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${DNS_LIB_FILE} (system paths)")
 if (NOT DNS_LIBRARY)
  message (FATAL_ERROR "could not find ${DNS_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found dns library \"${DNS_LIBRARY}\"")
 endif ()

 set (OMAPI_LIB_FILE libomapi.a)
 find_library (OMAPI_LIBRARY ${OMAPI_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6 /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES omapip x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${OMAPI_LIB_FILE} (system paths)")
 if (NOT OMAPI_LIBRARY)
  message (FATAL_ERROR "could not find ${OMAPI_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found omapi library \"${OMAPI_LIBRARY}\"")
 endif ()

# set (DHCP_LIB_FILE libdhcp.a)
# find_library (DHCP_LIBRARY ${DHCP_LIB_FILE}
##               HINTS ../../../dhcpctl/dhcp-4.3.6 /usr/lib
#               HINTS /usr/lib
##               PATH_SUFFIXES common x86_64-linux-gnu
#               PATH_SUFFIXES x86_64-linux-gnu
#               DOC "searching for ${DHCP_LIB_FILE} (system paths)")
# if (NOT DHCP_LIBRARY)
#  message (FATAL_ERROR "could not find ${DHCP_LIB_FILE}, aborting")
# else ()
#  message (STATUS "Found dhcp library \"${DHCP_LIBRARY}\"")
# endif ()

 set (DHCPCTL_LIB_FILE libdhcpctl.a)
 find_library (DHCPCTL_LIBRARY ${DHCPCTL_LIB_FILE}
#               HINTS ../../../dhcpctl/dhcp-4.3.6 /usr/lib
               HINTS /usr/lib
#               PATH_SUFFIXES dhcpctl x86_64-linux-gnu
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for ${DHCPCTL_LIB_FILE} (system paths)")
 if (NOT DHCPCTL_LIBRARY)
  message (FATAL_ERROR "could not find ${DHCPCTL_LIB_FILE}, aborting")
 else ()
  message (STATUS "Found dhcpctl library \"${DHCPCTL_LIBRARY}\"")
 endif ()
# add_library (dhcpctl STATIC IMPORTED)
# set_property (TARGET dhcpctl PROPERTY IMPORTED_LOCATION /usr/lib/libdhcpctl.a)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
#else ()
 #message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
 pkg_check_modules (PKG_SSL REQUIRED openssl)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_LIBPNG REQUIRED libpng)
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()
# *TODO*: repair WIN32 pkgconfig support

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/dbus)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libACEStream/src)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})

# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gtk-3.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACEStream/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/wlan)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

########### next target ###############
set (wlan_monitor_SRCS
     stdafx.cpp
     stdafx.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_signalhandler.cpp
     test_u_signalhandler.h
     test_u_ui_callbacks.cpp
     test_u_ui_callbacks.h
     wlan_monitor.cpp
     wlan_monitor_common.h
     wlan_monitor_configuration.h
     wlan_monitor_defines.h)

set (EXECUTABLE wlan_monitor)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${wlan_monitor_SRCS}
                etc/wlan_monitor.glade)
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (UNIX)
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_INTERFACE_MULTIPLICITY 4)
 set_target_properties (${EXECUTABLE} PROPERTIES IMPORTED_LINK_INTERFACE_MULTIPLICITY 4)
elseif (WIN32)
  if (NOT ${BUILD_TEST_U})
   set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
endif ()

#message (STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
#set (CMAKE_EXE_LINKER_FLAGS "-Wl,-t -Wl,--verbose -Wl,--print-map -Wl,--trace-symbol=dhcpctl_initialize")
#message (STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
if (UNIX)
 add_dependencies (${EXECUTABLE} CommonDBus)
endif ()
add_dependencies (${EXECUTABLE} CommonTimer)
add_dependencies (${EXECUTABLE} CommonUI)

add_dependencies (${EXECUTABLE} ACEStream)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_WLAN)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} iw)
 target_link_libraries (${EXECUTABLE} ${JSON_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBXML_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_DBUS_LIBRARIES})
 target_link_libraries (${EXECUTABLE} X11)
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ntdll.lib)
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)

#target_link_libraries (${EXECUTABLE} ACEStream)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_WLAN)
if (UNIX)
 target_link_libraries (${EXECUTABLE} CommonDBus)
 target_link_libraries (${EXECUTABLE} ${DHCPCTL_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${DHCP_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${ISC_LIBRARY})
# target_link_libraries (${EXECUTABLE} -Wl,--start-group ${OMAPI_LIBRARY} ${ISC_LIBRARY} ${DNS_LIBRARY} -Wl,--end-group)
 target_link_libraries (${EXECUTABLE} ${OMAPI_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${IRS_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${DNS_LIBRARY})
 target_link_libraries (${EXECUTABLE} -Wl,--start-group ${DNS_LIBRARY} ${GSS_KRB5_LIBRARY} ${KRB5_LIBRARY} -Wl,--end-group)
# target_link_libraries (${EXECUTABLE} ${KRB5_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${ISC_CFG_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${ISC_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${PKG_SSL_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${GEOIP_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${GSS_LIBRARY})
endif ()

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/file_server.glade
         DESTINATION etc/lib${PROJECT_NAME}/test_u COMPONENT config)

##########################################

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-f /mnt/win_d/temp/H264_720p.ts -g -l -p 5555 -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              /usr/local/lib
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/file_server
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c -f D:/temp/H264_720p.ts -g -l -p 5555 -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../openssl/bin
                                              ${PROJECT_SOURCE_DIR}/../pdcurses/win32
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/image/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACEStream/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/file_server
                         ENVIRONMENT foo=bar)
endif ()
