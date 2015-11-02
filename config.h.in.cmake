/* config.h.in.cmake */

//#define libACENetwork_VERSION_MAJOR @VERSION_MAJOR@
//#define libACENetwork_VERSION_MINOR @VERSION_MINOR@
//#define libACENetwork_VERSION_MICRO @VERSION_MICRO@
//#define libACENetwork_VERSION_DEVEL @VERSION_DEVEL@

/* Name of package */
//#cmakedefine @PACKAGE@_PACKAGE "@PACKAGE@"
#define LIBACENETWORK_PACKAGE "@PACKAGE@"
//#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
//#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#define LIBACENETWORK_PACKAGE_NAME "@PACKAGE@"
//#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#define LIBACENETWORK_PACKAGE_STRING "@PACKAGE@ @VERSION@"
//#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
//#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
//#undef PACKAGE_URL

/* Define to the version of this package. */
#define LIBACENETWORK_PACKAGE_VERSION "@VERSION@"
//#undef PACKAGE_VERSION

/* Version number of package */
//#undef VERSION

#cmakedefine VALGRIND_SUPPORT
