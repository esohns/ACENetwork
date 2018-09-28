// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

// Windows Header Files
#include <windows.h>

// *NOTE*: (f)lex generated scanners (re-)define some min/max basic type macros
//         that are also defined in by the platform SDK (see: <stdint.h>)
//#define RC_INVOKED
#endif

// C RunTime Header Files
// *WORKAROUND*
//#include <iostream>
//using namespace std;
//// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
////                   a problem in conjunction with the standard include headers
////                   when ACE_USES_OLD_IOSTREAMS is defined
////                   --> include the necessary headers manually (see above), and
////                       prevent ace/iosfwd.h from causing any harm
//#define ACE_IOSFWD_H
#include <string>

#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif // VALGRIND_SUPPORT

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

// Library Header Files
#if defined (HAVE_CONFIG_H)
#include "libCommon_config.h"
#endif // HAVE_CONFIG_H

#include "common.h"
#include "common_macros.h"

#if defined (HAVE_CONFIG_H)
#include "libACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_common.h"
#include "stream_macros.h"

// Local Header Files
#if defined (HAVE_CONFIG_H)
#include "libACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common.h"
#include "net_macros.h"

#include "dhcp_common.h"
