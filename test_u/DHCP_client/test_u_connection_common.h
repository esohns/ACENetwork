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

#ifndef TEST_U_CONNECTION_COMMON_H
#define TEST_U_CONNECTION_COMMON_H

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "stream_module_io_stream.h"
#include "stream_session_data.h"

#include "net_asynch_udpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_udpconnection_base.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "dhcp_common.h"

#include "test_u_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;
//struct Test_U_Configuration;
//struct Test_U_ConnectionState;
class Test_U_ConnectionStream;
//struct Test_U_StreamConfiguration;
//struct Test_U_StreamModuleHandlerConfiguration;
//struct Test_U_StreamSessionData;
//typedef Stream_SessionData_T<Test_U_StreamSessionData> Test_U_StreamSessionData_t;
//struct Test_U_StreamState;
//struct Test_U_UserData;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_U_Configuration,
                                 Test_U_ConnectionState,
                                 Test_U_RuntimeStatistic_t,
                                 ////////
                                 Test_U_UserData> Test_U_ConnectionManager_t;
struct Test_U_SocketHandlerConfiguration;

/////////////////////////////////////////

struct Test_U_ConnectionState
 : DHCP_ConnectionState
{
  inline Test_U_ConnectionState ()
   : DHCP_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  Test_U_Configuration* configuration;

  Test_U_UserData*      userData;
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_U_Configuration,
                          Test_U_ConnectionState,
                          Test_U_RuntimeStatistic_t> Test_U_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                Test_U_Configuration,
                                Test_U_ConnectionState,
                                Test_U_RuntimeStatistic_t,
                                Test_U_ConnectionStream,
                                Stream_StateMachine_ControlState,
                                Net_SocketConfiguration,
                                Test_U_SocketHandlerConfiguration> Test_U_ISocketConnection_t;

/////////////////////////////////////////

// inbound
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         Test_U_SocketHandlerConfiguration>,
                                                         ///////
                                                         ACE_INET_Addr,
                                                         Test_U_Configuration,
                                                         Test_U_ConnectionState,
                                                         Test_U_RuntimeStatistic_t,
                                                         Test_U_ConnectionStream,
                                                         ///////
                                                         Test_U_UserData,
                                                         ///////
                                                         Stream_ModuleConfiguration,
                                                         Test_U_StreamModuleHandlerConfiguration,
                                                         ///////
                                                         Test_U_SocketHandlerConfiguration> Test_U_Handler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_U_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,

                                        ACE_INET_Addr,
                                        Test_U_Configuration,
                                        Test_U_ConnectionState,
                                        Test_U_RuntimeStatistic_t,
                                        Test_U_ConnectionStream,

                                        Test_U_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_U_StreamModuleHandlerConfiguration,

                                        Test_U_SocketHandlerConfiguration> Test_U_AsynchHandler_t;

typedef Net_UDPConnectionBase_T<Test_U_Handler_t,
                                /////////
                                Test_U_Configuration,
                                Test_U_ConnectionState,
                                Test_U_RuntimeStatistic_t,
                                Test_U_ConnectionStream,
                                /////////
                                Test_U_SocketHandlerConfiguration,
                                /////////
                                Test_U_UserData> Test_U_Connection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_AsynchHandler_t,
                                      ///
                                      Test_U_Configuration,
                                      Test_U_ConnectionState,
                                      Test_U_RuntimeStatistic_t,
                                      Test_U_ConnectionStream,
                                      ///
                                      Test_U_SocketHandlerConfiguration,
                                      ///
                                      Test_U_UserData> Test_U_AsynchConnection_t;

// outbound
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
                                                         Test_U_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_U_Configuration,
                                  Test_U_ConnectionState,
                                  Test_U_RuntimeStatistic_t,
                                  Test_U_ConnectionStream,
                                  ///////
                                  Test_U_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_U_StreamModuleHandlerConfiguration,
                                  ///////
                                  Test_U_SocketHandlerConfiguration> Test_U_OutboundHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_U_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram_Bcast,

                                        ACE_INET_Addr,
                                        Test_U_Configuration,
                                        Test_U_ConnectionState,
                                        Test_U_RuntimeStatistic_t,
                                        Test_U_ConnectionStream,

                                        Test_U_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_U_StreamModuleHandlerConfiguration,

                                        Test_U_SocketHandlerConfiguration> Test_U_OutboundAsynchHandler_t;

typedef Net_UDPConnectionBase_T<Test_U_OutboundHandler_t,
                                /////////
                                Test_U_Configuration,
                                Test_U_ConnectionState,
                                Test_U_RuntimeStatistic_t,
                                Test_U_ConnectionStream,
                                /////////
                                Test_U_SocketHandlerConfiguration,
                                /////////
                                Test_U_UserData> Test_U_OutboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_OutboundAsynchHandler_t,
                                      ///
                                      Test_U_Configuration,
                                      Test_U_ConnectionState,
                                      Test_U_RuntimeStatistic_t,
                                      Test_U_ConnectionStream,
                                      ///
                                      Test_U_SocketHandlerConfiguration,
                                      ///
                                      Test_U_UserData> Test_U_OutboundAsynchConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_SocketHandlerConfiguration> Test_U_IConnector_t;

/////////////////////////////////////////

// inbound
typedef Net_Client_AsynchConnector_T<Test_U_AsynchConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_U_Configuration,
                                     Test_U_ConnectionState,
                                     Test_U_RuntimeStatistic_t,
                                     Test_U_ConnectionStream,
                                     ////
                                     Test_U_SocketHandlerConfiguration,
                                     ////
                                     Test_U_UserData> Test_U_AsynchConnector_t;
typedef Net_Client_Connector_T<Test_U_Connection_t,
                               Net_SOCK_Connector,
                               //////////
                               ACE_INET_Addr,
                               Test_U_Configuration,
                               Test_U_ConnectionState,
                               Test_U_RuntimeStatistic_t,
                               Test_U_ConnectionStream,
                               //////////
                               Test_U_SocketHandlerConfiguration,
                               //////////
                               Test_U_UserData> Test_U_Connector_t;

// outbound
typedef Net_Client_AsynchConnector_T<Test_U_OutboundAsynchConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_U_Configuration,
                                     Test_U_ConnectionState,
                                     Test_U_RuntimeStatistic_t,
                                     Test_U_ConnectionStream,
                                     ////
                                     Test_U_SocketHandlerConfiguration,
                                     ////
                                     Test_U_UserData> Test_U_OutboundAsynchConnector_t;
typedef Net_Client_Connector_T<Test_U_OutboundConnection_t,
                               Net_SOCK_Connector,
                               //////////
                               ACE_INET_Addr,
                               Test_U_Configuration,
                               Test_U_ConnectionState,
                               Test_U_RuntimeStatistic_t,
                               Test_U_ConnectionStream,
                               //////////
                               Test_U_SocketHandlerConfiguration,
                               //////////
                               Test_U_UserData> Test_U_OutboundConnector_t;

#endif
