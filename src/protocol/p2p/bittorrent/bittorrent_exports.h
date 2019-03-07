
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d BitTorrent
// ------------------------------
#ifndef BITTORRENT_EXPORT_H
#define BITTORRENT_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (BITTORRENT_HAS_DLL)
#  define BITTORRENT_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && BITTORRENT_HAS_DLL */

#if !defined (BITTORRENT_HAS_DLL)
#  define BITTORRENT_HAS_DLL 1
#endif /* ! BITTORRENT_HAS_DLL */

#if defined (BITTORRENT_HAS_DLL) && (BITTORRENT_HAS_DLL == 1)
#  if defined (BITTORRENT_BUILD_DLL)
#    define BitTorrent_Export ACE_Proper_Export_Flag
#    define BITTORRENT_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define BITTORRENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* BITTORRENT_BUILD_DLL */
#    define BitTorrent_Export ACE_Proper_Import_Flag
#    define BITTORRENT_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define BITTORRENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* BITTORRENT_BUILD_DLL */
#else /* BITTORRENT_HAS_DLL == 1 */
#  define BitTorrent_Export
#  define BITTORRENT_SINGLETON_DECLARATION(T)
#  define BITTORRENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* BITTORRENT_HAS_DLL == 1 */

// Set BITTORRENT_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (BITTORRENT_NTRACE)
#  if (ACE_NTRACE == 1)
#    define BITTORRENT_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define BITTORRENT_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !BITTORRENT_NTRACE */

#if (BITTORRENT_NTRACE == 1)
#  define BITTORRENT_TRACE(X)
#else /* (BITTORRENT_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define BITTORRENT_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (BITTORRENT_NTRACE == 1) */

#endif /* BITTORRENT_EXPORT_H */

// End of auto generated file.
