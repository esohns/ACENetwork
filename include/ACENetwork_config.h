/* config.h.cmake.in */

/* Name of package */
/* #undef  */
#define ACENetwork_PACKAGE "ACENetwork"
/* Define to the full name of this package. */
#define ACENetwork_PACKAGE_NAME "ACENetwork"
/* Define to the full name and version of this package. */
#define ACENetwork_PACKAGE_STRING "ACENetwork 0.0.1-devel"
#define ACENetwork_PACKAGE_DESCRIPTION "user-space wrapper library for networking, based on the ACE framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html)."
#define ACENetwork_PACKAGE_DESCRIPTION_SUMMARY "(wrapper) library for network functionality, based on the ACE framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html)"

/* Define to the home page for this package. */
#define ACENetwork_PACKAGE_URL "https://github.com/esohns/libACENetwork"
/* Define the name of the current package maintainer. */
#define ACENetwork_PACKAGE_MAINTAINER "Erik Sohns"
/* Define to the address where bug reports for this package should be sent. */
#define ACENetwork_PACKAGE_BUGREPORT "eriksohns@posteo.de"

/* Define to the version of this package. */
#define ACENetwork_PACKAGE_VERSION "0.0.1"
#define ACENetwork_PACKAGE_VERSION_FULL "0.0.1-devel"
/* Version numbers of package */
/* #undef ACENetwork_VERSION_MAJOR */
/* #undef ACENetwork_VERSION_MINOR */
//#define ACENetwork_VERSION_MICRO 1
#define ACENetwork_VERSION_MAJOR 0
#define ACENetwork_VERSION_MINOR 0
#define ACENetwork_VERSION_MICRO 1
#define ACENetwork_VERSION_DEVEL "devel"

/* Define to the one symbol short name of this package. */
#define ACENetwork_PACKAGE_TARNAME "ACENetwork-0.0.1-devel.tar.gz"

/****************************************/

// options
#define DEBUG_DEBUGGER 1
/* #undef TRACING */
/* #undef VALGRIND_SUPPORT */
// sub-options - src
/* #undef SSL_SUPPORT */
/* #undef NETLINK_SUPPORT */
// sub-options - wlan
/* #undef WEXT_SUPPORT */
/* #undef NL80211_SUPPORT */
/* #undef DBUS_SUPPORT */
/* #undef DHCLIENT_SUPPORT */
#define WLANAPI_SUPPORT
/* #undef WINXP_SUPPORT */
