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

#ifndef TEST_U_CONNECTION_MANAGER_COMMON_H
#define TEST_U_CONNECTION_MANAGER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

#include "test_u_connection_common.h"

// forward declarations
struct Test_U_UserData;

#if defined (NETLINK_SUPPORT)
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 Net_Netlink_Addr,
                                 ClientServer_ConnectionConfiguration_t,
                                 struct ClientServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Test_U_UserData> ClientServer_INetlinkConnectionManager_t;
#endif // NETLINK_SUPPORT
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 ClientServer_ConnectionConfiguration_t,
                                 struct ClientServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Test_U_UserData> ClientServer_IInetConnectionManager_t;

#if defined (NETLINK_SUPPORT)
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 Net_Netlink_Addr,
                                 ClientServer_ConnectionConfiguration_t,
                                 struct ClientServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Test_U_UserData> ClientServer_NetlinkConnectionManager_t;
#endif // NETLINK_SUPPORT
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 ClientServer_ConnectionConfiguration_t,
                                 struct ClientServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Test_U_UserData> ClientServer_InetConnectionManager_t;

#if defined (NETLINK_SUPPORT)
typedef ACE_Singleton<ClientServer_NetlinkConnectionManager_t,
                      ACE_SYNCH_MUTEX> CLIENTSERVER_NETLINKCONNECTIONMANAGER_SINGLETON;
#endif // NETLINK_SUPPORT
typedef ACE_Singleton<ClientServer_InetConnectionManager_t,
                      ACE_SYNCH_MUTEX> CLIENTSERVER_CONNECTIONMANAGER_SINGLETON;

#endif
