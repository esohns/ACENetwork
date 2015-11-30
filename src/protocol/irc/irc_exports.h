
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d IRC
// ------------------------------
#ifndef IRC_EXPORT_H
#define IRC_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (IRC_HAS_DLL)
#  define IRC_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && IRC_HAS_DLL */

#if !defined (IRC_HAS_DLL)
#  define IRC_HAS_DLL 1
#endif /* ! IRC_HAS_DLL */

#if defined (IRC_HAS_DLL) && (IRC_HAS_DLL == 1)
#  if defined (IRC_BUILD_DLL)
#    define IRC_Export ACE_Proper_Export_Flag
#    define IRC_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define IRC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* IRC_BUILD_DLL */
#    define IRC_Export ACE_Proper_Import_Flag
#    define IRC_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define IRC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* IRC_BUILD_DLL */
#else /* IRC_HAS_DLL == 1 */
#  define IRC_Export
#  define IRC_SINGLETON_DECLARATION(T)
#  define IRC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* IRC_HAS_DLL == 1 */

// Set IRC_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (IRC_NTRACE)
#  if (ACE_NTRACE == 1)
#    define IRC_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define IRC_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !IRC_NTRACE */

#if (IRC_NTRACE == 1)
#  define IRC_TRACE(X)
#else /* (IRC_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define IRC_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (IRC_NTRACE == 1) */

#endif /* IRC_EXPORT_H */

// End of auto generated file.
