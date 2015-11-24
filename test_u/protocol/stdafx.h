// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#ifndef STDAFX_H
#define STDAFX_H

#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

// Windows Header Files
#include <windows.h>
#endif

// C RunTime Header Files
#include <algorithm>
#include <functional>
//#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

// System Library Header Files
//#include "ace/ACE.h"
#include "ace/Assert.h"
//#include "ace/Lock_Adapter_T.h"
//#include "ace/Log_Msg.h"
//#include "ace/Malloc_Allocator.h"
#include "ace/OS.h"
//#include "ace/streams.h"
//#include "ace/Stream.h"
//#include "ace/Synch.h"
//#include "ace/Task.h"

// Local Header Files
#include "net_macros.h"

#if defined _MSC_VER
#include "targetver.h"
#endif

// *TODO*: reference additional headers your program requires here

#endif
