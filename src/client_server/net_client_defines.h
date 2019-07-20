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

#ifndef NET_CLIENT_DEFINES_H
#define NET_CLIENT_DEFINES_H

#define NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS        3

// define behaviour
#define NET_CLIENT_DEFAULT_CONNECT_PROBABILITY               0.5F // 50%
#define NET_CLIENT_DEFAULT_ABORT_PROBABILITY                 0.2F // 20%
#define NET_CLIENT_DEFAULT_DEFAULT_MODE                      Net_Client_TimeoutHandler::ACTION_NORMAL

#define NET_CLIENT_DEFAULT_MAX_NUM_OPEN_CONNECTIONS          0
#define NET_CLIENT_DEFAULT_SERVER_HOSTNAME                   ACE_LOCALHOST
#define NET_CLIENT_DEFAULT_SERVER_CONNECT_INTERVAL           0
#define NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL              0 // ms {0: OFF}
#define NET_CLIENT_DEFAULT_SERVER_TEST_INTERVAL              20 // ms
#define NET_CLIENT_DEFAULT_SERVER_STRESS_INTERVAL            1 // ms

// *** UI ***
#define NET_CLIENT_UI_FILE                                   "net_client.glade"

#endif
