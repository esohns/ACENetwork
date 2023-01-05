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
   set (SSL_INCLUDE_DIRS ${OpenSSL_INCLUDE_DIRS})
   set (SSL_LIBRARIES ${OpenSSL_LIBRARIES})
   set (SSL_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY};${OPENSSL_SSL_LIBRARY}")
  else ()
   pkg_search_module (PKG_SSL libssl openssl)
   if (PKG_SSL_FOUND)
    set (SSL_FOUND TRUE)
    set (SSL_INCLUDE_DIRS ${PKG_SSL_INCLUDE_DIRS})
    set (SSL_LIBRARIES ${PKG_SSL_LIBRARIES})
   endif (PKG_SSL_FOUND)
  endif (OPENSSL_FOUND)
 endif (SSL_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
  find_library (SSL_CRYPTO_LIBRARY libcrypto.lib
                PATHS ${VCPKG_LIB_DIR}
                PATH_SUFFIXES lib
                DOC "searching for libcrypto.lib"
                NO_DEFAULT_PATH)
  find_library (SSL_LIBRARY libssl.lib
                PATHS ${VCPKG_LIB_DIR}
                PATH_SUFFIXES lib
                DOC "searching for libssl.lib"
                NO_DEFAULT_PATH)
  if (SSL_CRYPTO_LIBRARY AND SSL_LIBRARY)
   set (SSL_FOUND TRUE)
   set (SSL_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR_BASE})
   set (SSL_LIBRARIES "${SSL_CRYPTO_LIBRARY};${SSL_LIBRARY}")
   set (SSL_LIB_DIR "${VCPKG_LIB_DIR}/bin")
  endif (SSL_CRYPTO_LIBRARY AND SSL_LIBRARY)
 endif (VCPKG_USE)
 if (NOT SSL_FOUND) 
  set (SSL_FOUND TRUE)
  set (SSL_INCLUDE_DIRS $ENV{LIB_ROOT}/openssl/include)
  set (SSL_LIBRARIES "$ENV{LIB_ROOT}/openssl/libcrypto.lib;$ENV{LIB_ROOT}/openssl/libssl.lib")
  set (SSL_LIB_DIR "$ENV{LIB_ROOT}/openssl")
 endif (NOT SSL_FOUND)
endif ()

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
 endif (VCPKG_USE)
 if (NOT ACE_SSL_LIBRARY)
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                PATHS $ENV{ACE_ROOT} $ENV{LIB_ROOT}/ACE_TAO/ACE
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_SSL_LIB_FILE}"
                NO_DEFAULT_PATH)
 endif (NOT ACE_SSL_LIBRARY)
endif ()
#if (NOT EXISTS ACE_SSL_LIBRARY)
if (NOT ACE_SSL_LIBRARY)
 message (WARNING "could not find ${ACE_SSL_LIB_FILE}, continuing")
else ()
 set (ACE_SSL_FOUND TRUE)
endif (NOT ACE_SSL_LIBRARY)

##########################################
if (SSL_FOUND AND ACE_SSL_FOUND)
 message (STATUS "found SSL/ACE_SSL")
 option (SSL_SUPPORT "enable SSL support" ON)
 if (SSL_SUPPORT)
  add_definitions (-DSSL_SUPPORT)
 endif (SSL_SUPPORT)
endif (SSL_FOUND AND ACE_SSL_FOUND)

#if (SSL_SUPPORT)
# option (SSL_USE "use SSL" ON)
#endif (SSL_SUPPORT)
#if (SSL_USE)
# add_definitions (-DSSL_USE)
#endif (SSL_USE)
