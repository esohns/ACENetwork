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

#ifndef TEST_I_DEFINES_H
#define TEST_I_DEFINES_H

#include "ace/config-lite.h"

#define TEST_I_STREAM_THREAD_NAME                        "stream processor"

// event dispatch
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS 1
#else
// *IMPORTANT NOTE*: on Linux, specifying 1 will not work correctly for proactor
//                   scenarios using the default (rt signal) implementation.
//                   Apparently, the thread already blocked in sigwaitinfo (see
//                   man pages) will not react to signals pertaining to a
//                   changed dispatch set
// *TODO*: retest and qualify
#define TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS 2
#endif // ACE_WIN32 || ACE_WIN64
#define TEST_I_DEFAULT_NUMBER_OF_SERVER_DISPATCH_THREADS 10

// stream
#define TEST_I_DEFAULT_BUFFER_SIZE                       16384 // bytes
#define TEST_I_MAX_MESSAGES                              0 // 0 --> no limits

// (asynchronous) connections
#define TEST_I_CONNECTION_ASYNCH_TIMEOUT                 60 // second(s)
// *IMPORTANT NOTE*: this means that asynchronous connections take at least this
//                   amount of time to establish
#define TEST_I_CONNECTION_ASYNCH_TIMEOUT_INTERVAL        1  // second(s)

// sessions
#define TEST_I_SESSION_LOG_FILENAME_PREFIX               "test_i"
#define TEST_I_SESSION_USE_CURSES                        true // use (PD|N)curses library ?

#define TEST_I_DEFAULT_SESSION_LOG                       false // log to file ? : stdout

// statistic
#define TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL      0 // seconds: 0 --> OFF

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
// UI
#define TEST_I_UI_THREAD_NAME                            "stream processor"

#if defined (GTK_SUPPORT)
// UI - GTK
#define TEST_I_UI_GTK_DEFAULT_RC_FILE                    "resources.rc"
#if defined (GTK2_USE)
#define TEST_I_UI_GTK_DEFAULT_GLADE_FILE                 "test_i.gtk2"
#elif defined (GTK3_USE) || defined (GTK4_USE)
#define TEST_I_UI_GTK_DEFAULT_GLADE_FILE                 "test_i.gtk3"
#else
#define TEST_I_UI_GTK_DEFAULT_GLADE_FILE                 "test_i.glade"
#endif // GTK2_USE || GTK3_USE || GTK4_USE

//#define TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION          "Monospace 8"
//#define TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE                "#FFFFFF" // white
//#define TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT                "#000000" // black

#define TEST_I_UI_GTK_STATUSBAR_CONTEXT_DATA             "data"
#define TEST_I_UI_GTK_STATUSBAR_CONTEXT_INFORMATION      "information"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#endif
