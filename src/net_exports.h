
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -n Net
// ------------------------------
#ifndef NET_EXPORT_H
#define NET_EXPORT_H

#include "ace/config-all.h"

#if !defined (NET_HAS_DLL)
#  define NET_HAS_DLL 1
#endif /* ! NET_HAS_DLL */

#if defined (NET_HAS_DLL) && (NET_HAS_DLL == 1)
#  if defined (NET_BUILD_DLL)
#    define Net_Export ACE_Proper_Export_Flag
#    define NET_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define NET_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* NET_BUILD_DLL */
#    define Net_Export ACE_Proper_Import_Flag
#    define NET_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define NET_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* NET_BUILD_DLL */
#else /* NET_HAS_DLL == 1 */
#  define Net_Export
#  define NET_SINGLETON_DECLARATION(T)
#  define NET_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* NET_HAS_DLL == 1 */

// Set NET_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (NET_NTRACE)
#  if (ACE_NTRACE == 1)
#    define NET_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define NET_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !NET_NTRACE */

#if (NET_NTRACE == 1)
#  define NET_TRACE(X)
#else /* (NET_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define NET_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (NET_NTRACE == 1) */

#endif /* NET_EXPORT_H */

// End of auto generated file.
