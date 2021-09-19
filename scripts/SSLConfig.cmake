if (UNIX)
 find_package (SSL MODULE)
 if (SSL_FOUND)
  message (STATUS "found SSL")
 else ()
  find_package (OpenSSL MODULE
                COMPONENTS Crypto SSL)
  if (OPENSSL_FOUND)
   message (STATUS "found OpenSSL")
   set (SSL_FOUND TRUE)
#   set (SSL_INCLUDE_DIRS ${OpenSSL_INCLUDE_DIRS})
#   set (SSL_LIBRARIES ${OpenSSL_LIBRARIES})
   set (SSL_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY};${OPENSSL_SSL_LIBRARY} CACHE STRING "SSL libraries")
  else ()
   pkg_search_module (PKG_SSL REQUIRED libssl openssl)
   if (PKG_SSL_FOUND)
    set (SSL_FOUND TRUE)
    set (SSL_INCLUDE_DIRS ${PKG_SSL_INCLUDE_DIRS})
    set (SSL_LIBRARIES ${PKG_SSL_LIBRARIES})
   endif (PKG_SSL_FOUND)
  endif (OPENSSL_FOUND)
 endif (SSL_FOUND)
elseif (WIN32)
 set (SSL_FOUND TRUE CACHE BOOL "found SSL") # *NOTE*: it lives under $ENV{LIB_ROOT}/openssl
 set (SSL_INCLUDE_DIRS $ENV{LIB_ROOT}/openssl/include CACHE STRING "SSL include directories")
 set (SSL_LIBRARIES $ENV{LIB_ROOT}/openssl/libcrypto.lib;$ENV{LIB_ROOT}/openssl/libssl.lib CACHE STRING "SSL libraries")
 set (SSL_LIB_DIR "$ENV{LIB_ROOT}/openssl")
endif ()
#message (STATUS "SSL_LIBRARIES \"${SSL_LIBRARIES}\")")

if (UNIX)
 set (ACE_SSL_LIB_FILE libACE_SSL.so)
 find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
               PATHS $ENV{ACE_ROOT} ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE
               PATH_SUFFIXES lib
               DOC "searching for ${ACE_SSL_LIB_FILE}")
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 set (ACE_SSL_LIB_FILE ACE_SSL${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
               PATHS $ENV{LIB_ROOT}/ACE_TAO/ACE
               PATHS $ENV{ACE_ROOT}
               PATH_SUFFIXES lib
               DOC "searching for ${ACE_SSL_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_SSL_LIBRARY)
if (NOT ACE_SSL_LIBRARY)
 message (WARNING "could not find ${ACE_SSL_LIB_FILE} (was: \"${ACE_SSL_LIBRARY}\"), continuing")
else ()
 set (ACE_SSL_FOUND TRUE)
endif ()

##########################################
if (SSL_FOUND AND ACE_SSL_FOUND)
 message (STATUS "found SSL")
 set (SSL_SUPPORT ON CACHE BOOL "SSL support available")
 add_definitions (-DSSL_SUPPORT)
endif (SSL_FOUND AND ACE_SSL_FOUND)

if (SSL_SUPPORT)
 option (SSL_USE "enable SSL support" OFF)
endif (SSL_SUPPORT)
if (SSL_USE)
 set (SSL_USE ON CACHE BOOL "SSL support enabled")
 add_definitions (-DSSL_USE)
endif (SSL_USE)
