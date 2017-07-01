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
#endif

// C RunTime Header Files
#include <bitset>
#include <functional>
//#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <map>
#include <vector>

// System Library Header Files
//#include "ace/streams.h"
//#include "ace/ACE.h"
#include "ace/Assert.h"
//#include "ace/Lock_Adapter_T.h"
#include "ace/Log_Msg.h"
//#include "ace/Malloc_Allocator.h"
#include "ace/OS.h"
//#include "ace/Reactor.h"
//#include "ace/Synch.h"
//#include "ace/Task.h"

#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#include <valgrind/valgrind.h>
#endif

// Local Header Files
#include "net_macros.h"
