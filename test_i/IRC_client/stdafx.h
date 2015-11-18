// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

// Windows Header Files
#define WINVER 0x0602 // MS Windows 8.1
#include <windows.h>
#endif

// C RunTime Header Files
// *WORKAROUND*
#include <iostream>
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H
//#include <string>

// System Library Header Files
//#include "ace/iosfwd.h"
//#include "ace/streams.h"
//#include "ace/Log_Msg.h"
//#include "ace/OS.h"
//#include "ace/Synch_Traits.h"

// Local Header Files
#include "net_macros.h"

using namespace std;
