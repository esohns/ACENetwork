set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
include (PreXMLImplementation)
include (SetXMLImplementation)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/dhcp/scripts")
include (PreDHCPClientImplementation)
include (SetDHCPClientImplementation)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/wlan/scripts")
include (PreMonitorImplementation)
include (SetMonitorImplementation)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif (UNIX)

if (UNIX)
# *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
 if (WEXT_USE)
 elseif (NL80211_USE)
#  pkg_check_modules (PKG_NL80211 REQUIRED libnl-3.0 libnl-genl-3.0)
#  pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
#  pkg_check_modules (PKG_SYSTEMD REQUIRED libsystemd)
 elseif (DBUS_USE)
#  pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
 endif ()

 if (DHCLIENT_USE)
  pkg_check_modules (PKG_LIBXML REQUIRED libxml-2.0)
  pkg_check_modules (PKG_SSL REQUIRED openssl)
 endif (DHCLIENT_USE)
endif (UNIX)
# *TODO*: repair WIN32 pkgconfig support

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)
#add_definitions (-DCOMMON_UI_HAS_DLL)
#add_definitions (-DCOMMON_XML_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ACEStream/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DACESTREAM_HAS_DLL)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (NL80211_USE)
  include_directories (${PKG_NL80211_INCLUDE_DIRS})
 elseif (DBUS_USE)
  include_directories (${PKG_DBUS_INCLUDE_DIRS})
 endif ()
 if (DHCLIENT_USE)
#  include_directories (/usr/include/bind-export)
#  include_directories (/usr/include/dhcp)
 endif (DHCLIENT_USE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 if (DBUS_USE)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/dbus)
 endif (DBUS_USE)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/lib)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/wlan)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

########### next target ###############
set (EXECUTABLE wlan_monitor)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_signalhandler.cpp
     test_u_signalhandler.h
     wlan_monitor.cpp
     wlan_monitor_common.h
     wlan_monitor_configuration.h
     wlan_monitor_defines.h)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_u_ui_callbacks.cpp
       test_u_ui_callbacks.h)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
#  set (${EXECUTABLE}_EXE_SRCS
#       ${${EXECUTABLE}_EXE_SRCS}
#       test_u_wxwidgets_ui.cpp
#       test_u_wxwidgets_ui.h
#       test_u_wxwidgets_ui.inl
#       test_u_wxwidgets_ui_base.cpp
#       test_u_wxwidgets_ui_base.h)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
#   set (ui_definition_file ${EXECUTABLE}.gtk3)
  elseif (GTK2_USE)
#   set (ui_definition_file ${EXECUTABLE}.gtk2)
  endif ()
  set (ui_definition_file ${EXECUTABLE}.glade)
 elseif (WXWIDGETS_USE)
  if (WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
  endif (WIN32)
#  set (ui_definition_file ${EXECUTABLE}.xrc)
 endif ()
endif (GUI_SUPPORT)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                etc/${ui_definition_file})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (UNIX)
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_INTERFACE_MULTIPLICITY 4)
 set_target_properties (${EXECUTABLE} PROPERTIES IMPORTED_LINK_INTERFACE_MULTIPLICITY 4)
elseif (WIN32)
 if (NOT BUILD_TEST_U)
  set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
endif ()

#message (STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
#set (CMAKE_EXE_LINKER_FLAGS "-Wl,-t -Wl,--verbose -Wl,--print-map -Wl,--trace-symbol=dhcpctl_initialize")
#message (STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
if (UNIX)
 if (DBUS_USE)
  add_dependencies (${EXECUTABLE} CommonDBus)
 endif (DBUS_USE)
endif (UNIX)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonSignal)
add_dependencies (${EXECUTABLE} CommonTimer)
if (WIN32)
 add_dependencies (${EXECUTABLE} CommonXML)
endif (WIN32)

add_dependencies (${EXECUTABLE} ACEStream)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
if (DHCLIENT_USE)
 add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Protocol_DHCP)
endif (DHCLIENT_USE)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_WLAN)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (UNIX)
 if (WEXT_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC iw)
 elseif (NL80211_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_DBUS_LIBRARIES})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_SYSTEMD_LIBRARIES})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_NL80211_LIBRARIES})
 elseif (DBUS_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_DBUS_LIBRARIES})
 endif ()

 if (DHCLIENT_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_LIBXML_LIBRARIES})
#  target_link_libraries (${EXECUTABLE} ${JSON_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_JSON_LIBRARIES})
  target_link_libraries (${EXECUTABLE} PUBLIC ${DHCPCTL_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${DHCP_LIBRARY})
# target_link_libraries (${EXECUTABLE} -Wl,--start-group ${OMAPI_LIBRARY} ${ISC_LIBRARY} ${DNS_LIBRARY} -Wl,--end-group)
  target_link_libraries (${EXECUTABLE} PUBLIC ${OMAPI_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${IRS_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${DNS_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC -Wl,--start-group ${DNS_LIBRARY} ${GSS_KRB5_LIBRARY} ${KRB5_LIBRARY} -Wl,--end-group)
# target_link_libraries (${EXECUTABLE} ${KRB5_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${ISC_CFG_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${ISC_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_SSL_LIBRARIES})
  target_link_libraries (${EXECUTABLE} PUBLIC ${GEOIP_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_KRB5_GSSAPI_LIBRARIES})
#  target_link_libraries (${EXECUTABLE} ${GSS_LIBRARY})
 endif (DHCLIENT_USE)

 target_link_libraries (${EXECUTABLE} PUBLIC X11)
 target_link_libraries (${EXECUTABLE} PUBLIC GLX)
elseif (WIN32)
# if (MSXML_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC msxml2.lib)
# elseif (LIBXML2_USE)
#  target_link_libraries (${EXECUTABLE} ${LIBXML2_LIBRARY})
# endif ()

 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_XML_LIBRARY})
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_XML_PARSER_LIBRARY})
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC CommonXML)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
if (DHCLIENT_USE)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Protocol_DHCP)
endif (DHCLIENT_USE)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_WLAN)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_U)

if (UNIX)
 if (DHCLIENT_USE)
# target_link_libraries (${EXECUTABLE} ${JSON_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${DHCPCTL_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${DHCP_LIBRARY})
# target_link_libraries (${EXECUTABLE} -Wl,--start-group ${OMAPI_LIBRARY} ${ISC_LIBRARY} ${DNS_LIBRARY} -Wl,--end-group)
  target_link_libraries (${EXECUTABLE} PUBLIC ${OMAPI_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${IRS_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${DNS_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC -Wl,--start-group ${DNS_LIBRARY} ${GSS_KRB5_LIBRARY} ${KRB5_LIBRARY} -Wl,--end-group)
  target_link_libraries (${EXECUTABLE} PUBLIC ${KRB5_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${ISC_CFG_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${ISC_LIBRARY})
  target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_SSL_LIBRARIES})
  target_link_libraries (${EXECUTABLE} PUBLIC ${GEOIP_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${GSS_LIBRARY})
 endif (DHCLIENT_USE)

 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_LIBXML_LIBRARIES})

 if (NL80211_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonDBus)
 elseif (DBUS_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonDBus)
 endif ()
endif (UNIX)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
install (FILES etc/wlan_monitor.glade
         DESTINATION "etc/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)

##########################################

set (ARGS "-a -l")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   set (ARGS "-g./etc/${ui_definition_file} ${ARGS}")
  endif (GTK_SUPPORT)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)

 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 if (GUI_SUPPORT)
  set (ARGS "-g.\\\\etc\\\\${ui_definition_file} ${ARGS}")

  if (GTK3_SUPPORT)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             G:/software/Development/dll
                             ${PROJECT_SOURCE_DIR}/../gtk3/bin)
  elseif (GTK2_SUPPORT)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             G:/software/Development/dll
                             ${PROJECT_SOURCE_DIR}/../gtk2/bin)
  endif ()
  if (WXWIDGETS_SUPPORT)
  string (REPLACE "\\" "\\\\" wxWidgets_LIB_DIR wxWidgets_LIB_DIR) 
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${wxWidgets_LIB_DIR})
  endif (WXWIDGETS_SUPPORT)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${PROJECT_SOURCE_DIR}/../libiconv-1.9.2-1/bin
                           ${PROJECT_SOURCE_DIR}/../libxml2/win32/bin.msvc
                           ${PROJECT_SOURCE_DIR}/../libpng-1.6.25/cmake/${CMAKE_BUILD_TYPE}
                           ${GTK_PATH}
                           ${ACE_PATH}
                           ${SSL_LIB_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/${EXECUTABLE}
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -a -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
