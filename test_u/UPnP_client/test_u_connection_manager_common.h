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

#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

#include "http_common.h"
//#include "upnp_session.h"

#include "test_u_connection_common.h"
#include "test_u_message.h"

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 UPnP_Client_SSDP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_I_SSDP_ConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 UPnP_Client_SSDP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_SSDP_ConnectionManager_t;

typedef ACE_Singleton<UPnP_Client_SSDP_ConnectionManager_t,
                      ACE_SYNCH_MUTEX> UPNP_CLIENT_SSDP_CONNECTIONMANAGER_SINGLETON;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 UPnP_Client_HTTP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_I_HTTP_ConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 UPnP_Client_HTTP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_HTTP_ConnectionManager_t;

typedef ACE_Singleton<UPnP_Client_HTTP_ConnectionManager_t,
                      ACE_SYNCH_MUTEX> UPNP_CLIENT_HTTP_CONNECTIONMANAGER_SINGLETON;

//////////////////////////////////////////

//// session
//typedef UPnP_ISession_T<struct UPnP_SessionState,
//                        UPnP_Client_ConnectionConfiguration> UPnP_ISession_t;
//
//typedef UPnP_Session_T<struct UPnP_SessionState,
//                       UPnP_Client_ConnectionConfiguration,
//                       UPnP_Client_ConnectionManager_t,
//                       Test_U_Message> UPnP_Session_t;

#endif
