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

#ifndef Net_CONNECTION_MANAGER_COMMON_H
#define Net_CONNECTION_MANAGER_COMMON_H

#include "ace/Singleton.h"
#include "ace/Synch.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_exports.h"
#include "net_iconnectionmanager.h"
#include "net_itransportlayer.h"
#include "net_stream_common.h"

typedef Net_IConnectionManager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_IIPConnectionManager_t;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_IConnectionManager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_INetlinkTransportLayer_t> Net_INetlinkConnectionManager_t;
#endif
typedef Net_IConnectionManager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_IUDPConnectionManager_t;

typedef Net_Connection_Manager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_IPConnection_Manager_t;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_Connection_Manager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_INetlinkTransportLayer_t> Net_NetlinkConnection_Manager_t;
#endif
typedef Net_Connection_Manager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_TCPConnection_Manager_t;
typedef Net_Connection_Manager_T<Net_Configuration_t,
                                 Net_UserData_t,
                                 Stream_Statistic_t,
                                 Net_IInetTransportLayer_t> Net_UDPConnection_Manager_t;

typedef ACE_Singleton<Net_IPConnection_Manager_t,
                      ACE_Recursive_Thread_Mutex> NET_IPBROADCASTCONNECTIONMANAGER_SINGLETON;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef ACE_Singleton<Net_NetlinkConnection_Manager_t,
                      ACE_Recursive_Thread_Mutex> NET_NETLINKCONNECTIONMANAGER_SINGLETON;
#endif
typedef ACE_Singleton<Net_TCPConnection_Manager_t,
                      ACE_Recursive_Thread_Mutex> NET_TCPCONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Net_UDPConnection_Manager_t,
                      ACE_Recursive_Thread_Mutex> NET_UDPCONNECTIONMANAGER_SINGLETON;

//NET_SINGLETON_DECLARE (ACE_Singleton,
//                       Net_IPConnection_Manager_t,
//                       ACE_Recursive_Thread_Mutex);
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
NET_SINGLETON_DECLARE (ACE_Singleton,
                       Net_NetlinkConnection_Manager_t,
                       ACE_Recursive_Thread_Mutex);
#endif
//NET_SINGLETON_DECLARE (ACE_Singleton,
//                       Net_TCPConnection_Manager_t,
//                       ACE_Recursive_Thread_Mutex);
NET_SINGLETON_DECLARE (ACE_Singleton,
                       Net_UDPConnection_Manager_t,
                       ACE_Recursive_Thread_Mutex);

#endif
