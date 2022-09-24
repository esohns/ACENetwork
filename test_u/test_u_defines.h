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

#ifndef TEST_U_DEFINES_H
#define TEST_U_DEFINES_H

#include "ace/config-lite.h"

#define TEST_U_STREAM_THREAD_NAME                    "stream processor"

// event dispatch
#define TEST_U_DEFAULT_NUMBER_OF_TP_THREADS          3
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS 1
#else
// *IMPORTANT NOTE*: on Linux, specifying 1 will not work correctly for proactor
//                   scenarios using the default (rt signal) proactor
//                   implementation. The thread blocked in sigwaitinfo (see man
//                   pages) will not awaken when the dispatch set is changed
//                   (*TODO*: to be verified)
#define TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS 2
#endif // ACE_WIN32 || ACE_WIN64

// stream
#define TEST_U_STREAM_MODULE_PROTOCOLHANDLER_NAME    "ProtocolHandler"
#define TEST_U_STREAM_MODULE_HEADERPARSER_NAME       "HeaderParser"

#define TEST_U_DEFAULT_BUFFER_SIZE                   16384 // bytes
#define TEST_U_MAX_MESSAGES                          0 // 0 --> no limits

// (asynchronous) connections
#define TEST_U_CONNECTION_ASYNCH_TIMEOUT             60 // second(s)
// *IMPORTANT NOTE*: this means that asynchronous connections take at least this
//                   amount of time to establish
#define TEST_U_CONNECTION_ASYNCH_TIMEOUT_INTERVAL    1  // second(s)

// sessions
#define TEST_U_SESSION_LOG_FILENAME_PREFIX           "test_u_session"
#define TEST_U_SESSION_USE_CURSES                    true // use (PD|N)curses library ?

#define TEST_U_DEFAULT_SESSION_LOG                   false // log to file ? : stdout

// statistic
#define TEST_U_DEFAULT_STATISTIC_REPORTING_INTERVAL  0 // seconds: 0 --> OFF

// SSL
#define TEST_U_PEM_CERTIFICATE_FILE                  "dummy.pem"
#define TEST_U_PEM_PRIVATE_KEY_FILE                  "key.pem"

#endif
