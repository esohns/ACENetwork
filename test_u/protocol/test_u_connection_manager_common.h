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

#ifndef TEST_I_CONNECTION_MANAGER_COMMON_H
#define TEST_I_CONNECTION_MANAGER_COMMON_H

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

// forward declarations
struct Test_I_Configuration;
struct Test_I_ConnectionState;
typedef Stream_Statistic Test_I_RuntimeStatistic_t;
struct Test_I_UserData;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_IConnectionManager_T<ACE_Netlink_Addr,
                                 Test_I_Configuration,
                                 Test_I_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_UserData> Test_I_Stream_INetlinkConnectionManager_t;
#endif
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Configuration,
                                 Test_I_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_UserData> Test_I_Stream_IInetConnectionManager_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_Connection_Manager_T<ACE_Netlink_Addr,
                                 Test_I_Configuration,
                                 Test_I_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_UserData> Test_I_Stream_NetlinkConnectionManager_t;
#endif
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Configuration,
                                 Test_I_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_UserData> Test_I_Stream_InetConnectionManager_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef ACE_Singleton<Test_I_Stream_NetlinkConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_STREAM_NETLINKCONNECTIONMANAGER_SINGLETON;
#endif
typedef ACE_Singleton<Test_I_Stream_InetConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_STREAM_CONNECTIONMANAGER_SINGLETON;

#endif
