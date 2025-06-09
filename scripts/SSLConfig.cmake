# SSL support
if (UNIX)
 find_package (SSL MODULE)
 if (SSL_FOUND)
  message (STATUS "found SSL")
 else ()
  find_package (OpenSSL MODULE
                COMPONENTS Crypto SSL)
  if (OPENSSL_FOUND)
   message (STATUS "found OpenSSL")
   set (OPENSSL_FOUND TRUE)
   set (OPENSSL_INCLUDE_DIRS ${OpenSSL_INCLUDE_DIRS})
#   set (OPENSSL_LIBRARIES ${OpenSSL_LIBRARIES})
   set (OPENSSL_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY};${OPENSSL_SSL_LIBRARY}")
  else ()
   pkg_search_module (PKG_SSL openssl libssl)
   if (PKG_SSL_FOUND)
    set (OPENSSL_FOUND TRUE)
    set (OPENSSL_INCLUDE_DIRS ${PKG_SSL_INCLUDE_DIRS})
    set (OPENSSL_LIBRARIES ${PKG_SSL_LIBRARIES})
   endif (PKG_SSL_FOUND)
  endif (OPENSSL_FOUND)
 endif (SSL_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
  find_library (OPENSSL_CRYPTO_LIBRARY libcrypto.lib
                PATHS ${VCPKG_LIB_DIR_BASE}
                PATH_SUFFIXES lib
                DOC "searching for libcrypto.lib"
                NO_DEFAULT_PATH)
  find_library (OPENSSL_LIBRARY libssl.lib
                PATHS ${VCPKG_LIB_DIR_BASE}
                PATH_SUFFIXES lib
                DOC "searching for libssl.lib"
                NO_DEFAULT_PATH)
  if (OPENSSL_CRYPTO_LIBRARY AND OPENSSL_LIBRARY)
   message (STATUS "found OpenSSL")
   set (OPENSSL_FOUND TRUE)
   set (OPENSSL_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR})
   set (OPENSSL_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY};${OPENSSL_LIBRARY}")
   set (OPENSSL_LIB_DIR ${VCPKG_BIN_DIR})
  endif (OPENSSL_CRYPTO_LIBRARY AND OPENSSL_LIBRARY)
 else ()
  find_library (OPENSSL_CRYPTO_LIBRARY libcrypto.lib
                PATHS $ENV{LIB_ROOT}
                PATH_SUFFIXES openssl
                DOC "searching for libcrypto.lib"
                NO_DEFAULT_PATH)
  find_library (OPENSSL_LIBRARY libssl.lib
                PATHS $ENV{LIB_ROOT}
                PATH_SUFFIXES openssl
                DOC "searching for libssl.lib"
                NO_DEFAULT_PATH)
  if (OPENSSL_CRYPTO_LIBRARY AND OPENSSL_LIBRARY)
   message (STATUS "found OpenSSL")
   set (OPENSSL_FOUND TRUE)
   set (OPENSSL_INCLUDE_DIRS $ENV{LIB_ROOT}/openssl/include)
   set (OPENSSL_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY};${OPENSSL_LIBRARY}")
   set (OPENSSL_LIB_DIR $ENV{LIB_ROOT}/openssl)
  endif (OPENSSL_CRYPTO_LIBRARY AND OPENSSL_LIBRARY)

  find_library (LIBRESSL_CRYPTO_LIBRARY crypto-50.lib
                PATHS $ENV{LIB_ROOT}/libressl
                PATH_SUFFIXES build/msvc/crypto/${CMAKE_BUILD_TYPE}
                DOC "searching for crypto-50.lib"
                NO_DEFAULT_PATH)
  find_library (LIBRESSL_SSL_LIBRARY ssl-53.lib
                PATHS $ENV{LIB_ROOT}/libressl
                PATH_SUFFIXES build/msvc/ssl/${CMAKE_BUILD_TYPE}
                DOC "searching for ssl-53.lib"
                NO_DEFAULT_PATH)
  find_library (LIBRESSL_TLS_LIBRARY tls-26.lib
                PATHS $ENV{LIB_ROOT}/libressl
                PATH_SUFFIXES build/msvc/tls/${CMAKE_BUILD_TYPE}
                DOC "searching for tls-26.lib"
                NO_DEFAULT_PATH)
  if (LIBRESSL_CRYPTO_LIBRARY AND LIBRESSL_SSL_LIBRARY AND LIBRESSL_TLS_LIBRARY)
   message (STATUS "found LibreSSL")
   set (LIBRESSL_FOUND TRUE)
   set (LIBRESSL_INCLUDE_DIRS $ENV{LIB_ROOT}/libressl/include)
   set (LIBRESSL_LIBRARIES "${LIBRESSL_CRYPTO_LIBRARY};${LIBRESSL_SSL_LIBRARY};${LIBRESSL_TLS_LIBRARY}")
   set (LIBRESSL_LIB_DIR "$ENV{LIB_ROOT}/libressl/build/msvc/crypto/${CMAKE_BUILD_TYPE};$ENV{LIB_ROOT}/libressl/build/msvc/ssl/${CMAKE_BUILD_TYPE};$ENV{LIB_ROOT}/libressl/build/msvc/tls/${CMAKE_BUILD_TYPE}")
  endif (LIBRESSL_CRYPTO_LIBRARY AND LIBRESSL_SSL_LIBRARY AND LIBRESSL_TLS_LIBRARY)
 endif (VCPKG_USE)
endif ()

################################################################################
# ACE SSL support
if (UNIX)
 set (ACE_SSL_LIB_FILE libACE_SSL.so)
 find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
               PATHS $ENV{ACE_ROOT} ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE
               PATH_SUFFIXES lib
               DOC "searching for ${ACE_SSL_LIB_FILE}")
elseif (WIN32)
 set (ACE_SSL_LIB_FILE ACE_SSL${LIB_FILE_SUFFIX}.lib)
 if (VCPKG_USE)
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                PATHS ${VCPKG_LIB_DIR}
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_SSL_LIB_FILE}"
                NO_DEFAULT_PATH)
 else ()
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                PATHS $ENV{ACE_ROOT} $ENV{LIB_ROOT}/ACE_TAO/ACE
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_SSL_LIB_FILE}"
                NO_DEFAULT_PATH)
 endif (VCPKG_USE)
endif ()
#if (NOT EXISTS ACE_SSL_LIBRARY)
if (NOT ACE_SSL_LIBRARY)
 message (WARNING "could not find ${ACE_SSL_LIB_FILE}, continuing")
else ()
 message (STATUS "found ACE_SSL")
 set (ACE_SSL_FOUND TRUE)
endif (NOT ACE_SSL_LIBRARY)

################################################################################

# *NOTE*: the first entry is the default option
if (OPENSSL_FOUND)
 set (SSL_IMPLEMENTATION "OpenSSL" CACHE STRING "use OpenSSL (default)")
endif (OPENSSL_FOUND)
if (LIBRESSL_FOUND)
 set (SSL_IMPLEMENTATION "LibreSSL" CACHE STRING "use LibreSSL")
endif (LIBRESSL_FOUND)
set_property (CACHE SSL_IMPLEMENTATION PROPERTY STRINGS "OpenSSL" "LibreSSL")

if (NOT DEFINED SSL_IMPLEMENTATION_LAST)
 set (SSL_IMPLEMENTATION_LAST "NotAnImplementation" CACHE STRING "last SSL implementation used")
 mark_as_advanced (FORCE SSL_IMPLEMENTATION_LAST)
endif (NOT DEFINED SSL_IMPLEMENTATION_LAST)
if (NOT (${SSL_IMPLEMENTATION} MATCHES ${SSL_IMPLEMENTATION_LAST}))
 unset (OPENSSL_USE CACHE)
 unset (LIBRESSL_USE CACHE)
 set (SSL_IMPLEMENTATION_LAST ${SSL_IMPLEMENTATION} CACHE STRING "Updating SSL Implementation Option" FORCE)
endif (NOT (${SSL_IMPLEMENTATION} MATCHES ${SSL_IMPLEMENTATION_LAST}))

if (${SSL_IMPLEMENTATION} MATCHES "OpenSSL")
 if (NOT OPENSSL_FOUND)
  message (FATAL_ERROR "OpenSSL not found")
 endif (NOT OPENSSL_FOUND)
 set (OPENSSL_USE ON CACHE STRING "use OpenSSL")
 mark_as_advanced (FORCE OPENSSL_USE)
 add_definitions (-DOPENSSL_USE)
elseif (${MONITOR_IMPLEMENTATION} MATCHES "LibreSSL")
 if (NOT LIBRESSL_FOUND)
  message (FATAL_ERROR "LibreSSL not found")
 endif (NOT LIBRESSL_FOUND)
 set (LIBRESSL_USE ON CACHE STRING "use LibreSSL")
 mark_as_advanced (FORCE LIBRESSL_USE)
 add_definitions (-DLIBRESSL_USE)
endif ()

################################################################################

if (OPENSSL_FOUND OR LIBRESSL_FOUND)
 set (SSL_FOUND TRUE)

 # prefer openssl over libressl
 if (OPENSSL_USE)
  message (STATUS "using OpenSSL")
  set (SSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIRS})
  set (SSL_LIBRARIES ${OPENSSL_LIBRARIES})
  set (SSL_LIB_DIR ${OPENSSL_LIB_DIR})
 elseif (LIBRESSL_USE)
  message (STATUS "using LibreSSL")
  set (SSL_INCLUDE_DIRS ${LIBRESSL_INCLUDE_DIRS})
  set (SSL_LIBRARIES ${LIBRESSL_LIBRARIES})
  set (SSL_LIB_DIR ${LIBRESSL_LIB_DIR})
 endif ()
endif (OPENSSL_FOUND OR LIBRESSL_FOUND)

# *WARNING*: ACE needs to be compiled with the same SSL library !
if (SSL_FOUND AND ACE_SSL_FOUND)
 message (STATUS "found SSL/ACE_SSL")
 option (SSL_SUPPORT "enable SSL support" ON)
 if (SSL_SUPPORT)
  add_definitions (-DSSL_SUPPORT)
 endif (SSL_SUPPORT)
endif (SSL_FOUND AND ACE_SSL_FOUND)
