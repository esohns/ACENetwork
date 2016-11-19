/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef HTTP_NETWORK_H
#define HTTP_NETWORK_H

#include <string>

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "http_common.h"
#include "http_configuration.h"
//#include "http_stream.h"
#include "http_stream_common.h"

/////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<HTTP_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct HTTP_Configuration,
                                  struct HTTP_ConnectionState,
                                  HTTP_RuntimeStatistic_t,
                                  HTTP_Stream_t,
                                  struct HTTP_Stream_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct HTTP_ModuleHandlerConfiguration> HTTP_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct HTTP_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct HTTP_Configuration,
                                        struct HTTP_ConnectionState,
                                        HTTP_RuntimeStatistic_t,
                                        HTTP_Stream_t,
                                        struct HTTP_Stream_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct HTTP_ModuleHandlerConfiguration> HTTP_AsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<HTTP_TCPHandler_t,
                                struct HTTP_Configuration,
                                struct HTTP_ConnectionState,
                                HTTP_RuntimeStatistic_t,
                                HTTP_Stream_t,
                                struct HTTP_SocketHandlerConfiguration,
                                struct HTTP_Stream_UserData> HTTP_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<HTTP_AsynchTCPHandler_t,
                                      struct HTTP_Configuration,
                                      struct HTTP_ConnectionState,
                                      HTTP_RuntimeStatistic_t,
                                      HTTP_Stream_t,
                                      struct HTTP_SocketHandlerConfiguration,
                                      struct HTTP_Stream_UserData> HTTP_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct HTTP_Configuration,
                          struct HTTP_ConnectionState,
                          HTTP_RuntimeStatistic_t> HTTP_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct HTTP_Configuration,
                                struct HTTP_ConnectionState,
                                HTTP_RuntimeStatistic_t,
                                HTTP_Stream_t,
                                enum Stream_StateMachine_ControlState,
                                struct Net_SocketConfiguration,
                                struct HTTP_SocketHandlerConfiguration> HTTP_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct HTTP_Configuration,
//                       struct HTTP_ConnectionState,
//                       HTTP_RuntimeStatistic_t,
//                       HTTP_Stream> HTTP_ISession_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct HTTP_SocketHandlerConfiguration> HTTP_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         struct HTTP_ConnectorConfiguration> HTTP_IConnector_t;
typedef Net_Client_Connector_T<HTTP_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct HTTP_Configuration,
                               struct HTTP_ConnectionState,
                               HTTP_RuntimeStatistic_t,
                               HTTP_Stream_t,
                               struct HTTP_SocketHandlerConfiguration,
                               struct HTTP_Stream_UserData> HTTP__Connector_t;
typedef Net_Client_AsynchConnector_T<HTTP_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct HTTP_Configuration,
                                     struct HTTP_ConnectionState,
                                     HTTP_RuntimeStatistic_t,
                                     HTTP_Stream_t,
                                     struct HTTP_SocketHandlerConfiguration,
                                     struct HTTP_Stream_UserData> HTTP_AsynchConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct HTTP_Configuration,
                                 struct HTTP_ConnectionState,
                                 HTTP_RuntimeStatistic_t,
                                 struct HTTP_Stream_UserData> HTTP_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct HTTP_Configuration,
                                 struct HTTP_ConnectionState,
                                 HTTP_RuntimeStatistic_t,
                                 struct HTTP_Stream_UserData> HTTP_Connection_Manager_t;

typedef ACE_Singleton<HTTP_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> HTTP_CONNECTIONMANAGER_SINGLETON;

#endif
