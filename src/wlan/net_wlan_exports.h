
// -*- C++ -*-
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d Net_WLAN
// ------------------------------
#ifndef NET_WLAN_EXPORT_H
#define NET_WLAN_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (NET_WLAN_HAS_DLL)
#  define NET_WLAN_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && NET_WLAN_HAS_DLL */

#if !defined (NET_WLAN_HAS_DLL)
#  define NET_WLAN_HAS_DLL 1
#endif /* ! NET_WLAN_HAS_DLL */

#if defined (NET_WLAN_HAS_DLL) && (NET_WLAN_HAS_DLL == 1)
#  if defined (NET_WLAN_BUILD_DLL)
#    define Net_WLAN_Export ACE_Proper_Export_Flag
#    define NET_WLAN_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define NET_WLAN_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* NET_WLAN_BUILD_DLL */
#    define Net_WLAN_Export ACE_Proper_Import_Flag
#    define NET_WLAN_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define NET_WLAN_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* NET_WLAN_BUILD_DLL */
#else /* NET_WLAN_HAS_DLL == 1 */
#  define Net_WLAN_Export
#  define NET_WLAN_SINGLETON_DECLARATION(T)
#  define NET_WLAN_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* NET_WLAN_HAS_DLL == 1 */

// Set NET_WLAN_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (NET_WLAN_NTRACE)
#  if (ACE_NTRACE == 1)
#    define NET_WLAN_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define NET_WLAN_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !NET_WLAN_NTRACE */

#if (NET_WLAN_NTRACE == 1)
#  define NET_WLAN_TRACE(X)
#else /* (NET_WLAN_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define NET_WLAN_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (NET_WLAN_NTRACE == 1) */

#endif /* NET_WLAN_EXPORT_H */

// End of auto generated file.
