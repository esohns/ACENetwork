/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef NETWORK_MACROS_H
#define NETWORK_MACROS_H

// tracing //

#define NETWORK_TRACE_IMPL(X) ACE_Trace ____ (ACE_TEXT (X), __LINE__, ACE_TEXT (__FILE__))

// by default tracing is turned off
#if !defined (NETWORK_NTRACE)
#  define NETWORK_NTRACE 1
#endif /* NETWORK_NTRACE */

#if (NETWORK_NTRACE == 1)
#  define NETWORK_TRACE(X)
#else
#  if !defined (NETWORK_HAS_TRACE)
#    define NETWORK_HAS_TRACE
#  endif /* NETWORK_HAS_TRACE */
#  define NETWORK_TRACE(X) NETWORK_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* NETWORK_NTRACE */

// application //

#define NETWORK_CHECK_VERSION(major,minor,micro)                                                                         \
  ((ACENetwork_VERSION_MAJOR > major)                                                                                 || \
   ((ACENetwork_VERSION_MAJOR == major) && (ACENetwork_VERSION_MINOR > minor))                                        || \
   ((ACENetwork_VERSION_MAJOR == major) && (ACENetwork_VERSION_MINOR == minor) && (ACENetwork_VERSION_MICRO >= micro)))

// *NOTE*: arguments must be strings
#define NETWORK_MAKE_VERSION_STRING_VARIABLE(program,version,variable)                                                          \
  std::string variable = program; variable += ACE_TEXT_ALWAYS_CHAR (\\" \\");                                                   \
  variable += version; variable += ACE_TEXT_ALWAYS_CHAR (\\" compiled on \\");                                                  \
  variable += ACE_TEXT_ALWAYS_CHAR (COMPILATION_DATE_TIME);                                                                     \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" host platform \\"); variable += Common_Tools::compilerPlatformName ();                  \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" with \\"); variable += Common_Tools::compilerName ();                                   \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" \\"); variable += Common_Tools::compilerVersion ();                                     \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" against ACE \\"); variable += Common_Tools::compiledVersion_ACE ();                     \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" , Common \\"); variable += ACE_TEXT_ALWAYS_CHAR (Common_PACKAGE_VERSION_FULL);       \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" , ACEStream \\"); variable += ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL); \
  variable += ACE_TEXT_ALWAYS_CHAR (\\" , ACENetwork \\"); variable += ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION_FULL);

#endif
