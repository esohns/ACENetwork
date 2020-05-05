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

#ifndef NET_SERVER_DEFINES_H
#define NET_SERVER_DEFINES_H

// *** trace log ***
#define NET_SERVER_LOG_FILENAME_PREFIX                         "net_server"

// - WARNING: current implementation cannot support numbers that have
//   more than 7 digits !!!
// - WARNING: current implementation cannot support 0 !!!
#define NET_SERVER_LOG_MAXIMUM_NUMBER_OF_FILES                 5

#define NET_SERVER_DEFAULT_NUMBER_OF_REACTOR_DISPATCH_THREADS  1
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS 1
#else
// *IMPORTANT NOTE*: on Linux, specifying 1 will not work correctly for proactor
//                   scenarios using the default (rt signal) proactor
//                   implementation. The thread blocked in sigwaitinfo (see man
//                   pages) will not awaken when the dispatch set is changed
//                   (*TODO*: to be verified)
#define NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS 10
#endif // ACE_WIN32 || ACE_WIN64

#define NET_SERVER_DEFAULT_TRANSPORT_LAYER                     TRANSPORTLAYER_TCP
#define NET_SERVER_DEFAULT_LISTENING_PORT                      10001
//#define NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS    ACE::max_handles()
#define NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS          std::numeric_limits<unsigned int>::max()

#define NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL        3600 // seconds [0: off]

#define NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL                10000 // ms [0: off]
//#define NET_SERVER_DEFAULT_TCP_KEEPALIVE              0  // seconds [0 --> no timeout]

// *** socket ***
#define NET_SERVER_SOCKET_DEFAULT_LINGER                       false  // SO_LINGER

// *** UI ***
#define NET_SERVER_UI_FILE                                     "net_server.glade"

#endif
