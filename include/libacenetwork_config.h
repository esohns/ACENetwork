#ifndef _LIBACENETWORK_CONFIG_H
#define _LIBACENETWORK_CONFIG_H 1
 
/* libacenetwork_config.h. Generated automatically at end of configure. */
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef LIBACENETWORK_HAVE_DLFCN_H
#define LIBACENETWORK_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef LIBACENETWORK_HAVE_INTTYPES_H
#define LIBACENETWORK_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `ACE' library (-lACE). */
/* #undef HAVE_LIBACE */

/* Define to 1 if you have the `pthread' library (-lpthread). */
#ifndef LIBACENETWORK_HAVE_LIBPTHREAD
#define LIBACENETWORK_HAVE_LIBPTHREAD 1
#endif

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#ifndef LIBACENETWORK_HAVE_MALLOC
#define LIBACENETWORK_HAVE_MALLOC 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef LIBACENETWORK_HAVE_MEMORY_H
#define LIBACENETWORK_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#ifndef LIBACENETWORK_HAVE_MEMSET
#define LIBACENETWORK_HAVE_MEMSET 1
#endif

/* Define to 1 if you have the <netinet/in.h> header file. */
#ifndef LIBACENETWORK_HAVE_NETINET_IN_H
#define LIBACENETWORK_HAVE_NETINET_IN_H 1
#endif

/* Define to 1 if you have the `socket' function. */
#ifndef LIBACENETWORK_HAVE_SOCKET
#define LIBACENETWORK_HAVE_SOCKET 1
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef LIBACENETWORK_HAVE_STDBOOL_H
#define LIBACENETWORK_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef LIBACENETWORK_HAVE_STDINT_H
#define LIBACENETWORK_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef LIBACENETWORK_HAVE_STDLIB_H
#define LIBACENETWORK_HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strerror' function. */
#ifndef LIBACENETWORK_HAVE_STRERROR
#define LIBACENETWORK_HAVE_STRERROR 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef LIBACENETWORK_HAVE_STRINGS_H
#define LIBACENETWORK_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef LIBACENETWORK_HAVE_STRING_H
#define LIBACENETWORK_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef LIBACENETWORK_HAVE_SYS_STAT_H
#define LIBACENETWORK_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef LIBACENETWORK_HAVE_SYS_TYPES_H
#define LIBACENETWORK_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef LIBACENETWORK_HAVE_UNISTD_H
#define LIBACENETWORK_HAVE_UNISTD_H 1
#endif

/* Define if a version suffix is present. */
/* #undef HAVE_VERSION_DEVEL */

/* Define to 1 if the system has the type `_Bool'. */
#ifndef LIBACENETWORK_HAVE__BOOL
#define LIBACENETWORK_HAVE__BOOL 1
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef LIBACENETWORK_LT_OBJDIR
#define LIBACENETWORK_LT_OBJDIR ".libs/"
#endif

/* meta-package-name */
#ifndef LIBACENETWORK_META_PACKAGE_NAME
#define LIBACENETWORK_META_PACKAGE_NAME "libACENetwork"
#endif

/* Name of package */
#ifndef LIBACENETWORK_PACKAGE
#define LIBACENETWORK_PACKAGE "libACENetwork"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef LIBACENETWORK_PACKAGE_BUGREPORT
#define LIBACENETWORK_PACKAGE_BUGREPORT "eriksohns@123mail.org"
#endif

/* Define to the full name of this package. */
#ifndef LIBACENETWORK_PACKAGE_NAME
#define LIBACENETWORK_PACKAGE_NAME "libACENetwork"
#endif

/* Define to the full name and version of this package. */
#ifndef LIBACENETWORK_PACKAGE_STRING
#define LIBACENETWORK_PACKAGE_STRING "libACENetwork 0.0.1-devel"
#endif

/* Define to the one symbol short name of this package. */
#ifndef LIBACENETWORK_PACKAGE_TARNAME
#define LIBACENETWORK_PACKAGE_TARNAME "libACENetwork"
#endif

/* Define to the home page for this package. */
#ifndef LIBACENETWORK_PACKAGE_URL
#define LIBACENETWORK_PACKAGE_URL "http://www.github.com/esohns/libACENetwork"
#endif

/* Define to the version of this package. */
#ifndef LIBACENETWORK_PACKAGE_VERSION
#define LIBACENETWORK_PACKAGE_VERSION "0.0.1-devel"
#endif

/* Define to 1 if you have the ANSI C header files. */
#ifndef LIBACENETWORK_STDC_HEADERS
#define LIBACENETWORK_STDC_HEADERS 1
#endif

/* Version number of package */
#ifndef LIBACENETWORK_VERSION
#define LIBACENETWORK_VERSION "0.0.1-devel"
#endif

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */
 
/* once: _LIBACENETWORK_CONFIG_H */
#endif
