// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

// Windows Header Files
#include <windows.h>
#endif

// C RunTime Header Files
//#include <iostream>
#include <string>

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

//#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif

#if defined (HAVE_CONFIG_H)
#include "libACENetwork_config.h"
#endif

// Library Header Files
#include "common.h"
#include "common_macros.h"
#include "common_pragmas.h"

#include "stream_common.h"
#include "stream_macros.h"

// Local Header Files
#include "net_common.h"
#include "net_macros.h"

#if defined (_MSC_VER)
#include "targetver.h"
#endif

// *TODO*: reference additional headers your program requires here
