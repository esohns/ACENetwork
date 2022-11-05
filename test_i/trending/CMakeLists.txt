include (ffmpeg)
include (ImageMagick)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
include (PreXMLImplementation)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec/scripts")
include (zlib)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/doc/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/doc/scripts")
include (Office)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../scripts")
include (SSLConfig)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (LIBREOFFICE_SUPPORT)
 include_directories (${LIBREOFFICE_INCLUDE_DIRS})
endif (LIBREOFFICE_SUPPORT)
if (LIBXML2_SUPPORT)
 include_directories (${LIBXML2_INCLUDE_DIRS})
endif (LIBXML2_SUPPORT)
if (ZLIB_SUPPORT)
 include_directories (${ZLIB_INCLUDE_DIRS})
endif (ZLIB_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/doc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/html)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net/protocols/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/stat)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/dec)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/doc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/html)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net/protocols/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/stat)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/client_server)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/protocol/http)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE trending)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i.cpp
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_htmlparser.cpp
     test_i_module_htmlparser.h
     test_i_module_httpget.cpp
     test_i_module_httpget.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h
     test_i_stream.inl
     test_i_trending_common.h
     test_i_trending_connection_common.h
     test_i_trending_defines.h)
if (LIBREOFFICE_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_module_spreadsheetwriter.cpp
      test_i_module_spreadsheetwriter.h)
endif (LIBREOFFICE_SUPPORT)
set (${EXECUTABLE}_EXE_SCRIPTS
     etc/soffice.ini
     etc/sofficerc
     etc/template.ods)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${${EXECUTABLE}_EXE_SCRIPTS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

if (LIBREOFFICE_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBREOFFICE_LIBRARIES})
endif (LIBREOFFICE_SUPPORT)
if (LIBXML2_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBXML2_LIBRARIES})
endif (LIBXML2_SUPPORT)
if (ZLIB_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ZLIB_LIBRARIES})
endif (ZLIB_SUPPORT)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
endif (SSL_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
#if (SSL_SUPPORT)
# target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
#endif (SSL_SUPPORT)
if (UNIX)
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# target_link_libraries (${EXECUTABLE} PUBLIC ntdll.lib)
# target_link_libraries (${EXECUTABLE} PUBLIC dxguid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mf.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
# set (LIB_FILE_SUFFIX "")
# if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
#     CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
#  set (LIB_FILE_SUFFIX "d")
# endif ()
# target_link_libraries (${EXECUTABLE} PUBLIC $ENV{LIB_ROOT}/zlib/cmake/${CMAKE_BUILD_TYPE}/zlib${LIB_FILE_SUFFIX}.lib)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Document)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_HTML)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Network_Protocol_HTTPGet)
target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE} PUBLIC ACENetwork)
target_link_libraries (${EXECUTABLE} PUBLIC ACENetwork_Protocol_HTTP)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 set (ACE_SSL_DLL_FILE ACE_SSL${LIB_FILE_SUFFIX}.dll)
 set (ACE_SSL_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_SSL_DLL_FILE})
 file (TO_CMAKE_PATH "$ENV{OO_SDK_HOME}" OO_SDK_HOME)
# string (REPLACE "\\" "\\\\" OO_SDK_HOME "${OO_SDK_HOME}")
 file (TO_CMAKE_PATH $ENV{LIB_ROOT} LIB_ROOT)
# string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${ACE_SSL_DLL_FQPATH}
                ${OO_SDK_HOME}/bin/sal3.dll
                ${OO_SDK_HOME}/../program/cppu3.dll
                ${OO_SDK_HOME}/../program/cppuhelper3MSC.dll
                ${OO_SDK_HOME}/../program/sal3.dll
                ${LIB_ROOT}/openssl/libssl-3-x64.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
endif (WIN32)

if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 file (TO_NATIVE_PATH "${UNO_HOME}" "/usr/lib64/libreoffice/program")
 set (ENVIRONMENT "UNO_HOME=${UNO_HOME_PATH};URE_MORE_TYPES=$ORIGIN/types/offapi.rdb")
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-c./etc/soffice.ini -f./etc/template.ods -h localhost -iDE000ETFL581 -l -o ./output.ods -p 2083 -r -t -u https://www.tagesschau.de/wirtschaft/boersenkurse/ajax/get_search.htn"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT ${ENVIRONMENT})
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libiconv/bin ICONV_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libmysql/lib LIBMYSQL_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libxml2/bin LIBXML_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${LIBREOFFICE_LIB_DIRS}" OO_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIBRARY_PATH)
 file (TO_CMAKE_PATH ${ZLIB_LIB_DIR} ZLIB_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${ACE_LIBRARY_PATH}
                           ${GTK_LIBRARY_PATH}
                           ${WXWIDGETS_LIBRARY_PATH}
                           ${FFMPEG_LIBRARY_PATH}
                           ${IMAGEMAGICK_LIBRARY_PATH}
#                           ${ICONV_LIBRARY_PATH}
 #                          ${LIBXML_LIBRARY_PATH}
  #                         ${LIBMYSQL_LIBRARY_PATH}
                           ${SSL_LIBRARY_PATH}
                           ${ZLIB_LIBRARY_PATH}
                           ${OO_LIBRARY_PATH})
 file (TO_NATIVE_PATH "${UNO_HOME}" UNO_HOME_PATH)
 string (REPLACE "\\" "\\\\" UNO_HOME_PATH ${UNO_HOME_PATH})
 set (ENVIRONMENT "UNO_HOME=${UNO_HOME_PATH};URE_MORE_TYPES=$ORIGIN/types/offapi.rdb")
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-c.\\\\etc\\\\soffice.ini -f.\\\\etc\\\\template.ods -h localhost -iDE000ETFL581 -l -o .\\\\output.ods -p 2083 -r -t -u https://www.tagesschau.de/wirtschaft/boersenkurse/ajax/get_search.htn"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT ${ENVIRONMENT})
endif ()
