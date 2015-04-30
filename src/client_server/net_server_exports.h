
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -n Net_Client
// ------------------------------
#ifndef NET_SERVER_EXPORT_H
#define NET_SERVER_EXPORT_H

#include "ace/config-all.h"

#if !defined (NET_SERVER_HAS_DLL)
#  define NET_SERVER_HAS_DLL 1
#endif /* ! NET_SERVER_HAS_DLL */

#if defined (NET_SERVER_HAS_DLL) && (NET_SERVER_HAS_DLL == 1)
#  if defined (NET_SERVER_BUILD_DLL)
#    define Net_Server_Export ACE_Proper_Export_Flag
#    define NET_SERVER_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define NET_SERVER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* NET_SERVER_BUILD_DLL */
#    define Net_Server_Export ACE_Proper_Import_Flag
#    define NET_SERVER_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define NET_SERVER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* NET_SERVER_BUILD_DLL */
#else /* NET_SERVER_HAS_DLL == 1 */
#  define Net_Server_Export
#  define NET_SERVER_SINGLETON_DECLARATION(T)
#  define NET_SERVER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* NET_SERVER_HAS_DLL == 1 */

// Set NET_SERVER_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (NET_SERVER_NTRACE)
#  if (ACE_NTRACE == 1)
#    define NET_SERVER_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define NET_SERVER_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !NET_SERVER_NTRACE */

#if (NET_SERVER_NTRACE == 1)
#  define NET_SERVER_TRACE(X)
#else /* (NET_SERVER_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define NET_SERVER_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (NET_SERVER_NTRACE == 1) */

#endif /* NET_SERVER_EXPORT_H */

// End of auto generated file.
