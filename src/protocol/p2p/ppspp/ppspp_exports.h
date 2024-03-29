
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d HTTP
// ------------------------------
#ifndef HTTP_EXPORT_H
#define HTTP_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (HTTP_HAS_DLL)
#  define HTTP_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && HTTP_HAS_DLL */

#if !defined (HTTP_HAS_DLL)
#  define HTTP_HAS_DLL 1
#endif /* ! HTTP_HAS_DLL */

#if defined (HTTP_HAS_DLL) && (HTTP_HAS_DLL == 1)
#  if defined (HTTP_BUILD_DLL)
#    define HTTP_Export ACE_Proper_Export_Flag
#    define HTTP_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define HTTP_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* HTTP_BUILD_DLL */
#    define HTTP_Export ACE_Proper_Import_Flag
#    define HTTP_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define HTTP_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* HTTP_BUILD_DLL */
#else /* HTTP_HAS_DLL == 1 */
#  define HTTP_Export
#  define HTTP_SINGLETON_DECLARATION(T)
#  define HTTP_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* HTTP_HAS_DLL == 1 */

// Set HTTP_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (HTTP_NTRACE)
#  if (ACE_NTRACE == 1)
#    define HTTP_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define HTTP_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !HTTP_NTRACE */

#if (HTTP_NTRACE == 1)
#  define HTTP_TRACE(X)
#else /* (HTTP_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define HTTP_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (HTTP_NTRACE == 1) */

#endif /* HTTP_EXPORT_H */

// End of auto generated file.
