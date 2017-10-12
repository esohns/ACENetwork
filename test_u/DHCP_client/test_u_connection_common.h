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

#include <map>
#include <string>

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

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
//struct Test_U_Configuration;
//struct Test_U_ConnectionState;
class Test_U_OutboundConnectionStream;
class Test_U_InboundConnectionStream;
//struct Test_U_StreamConfiguration;
//struct Test_U_StreamModuleHandlerConfiguration;
//struct Test_U_StreamSessionData;
//typedef Stream_SessionData_T<Test_U_StreamSessionData> Test_U_StreamSessionData_t;
//struct Test_U_StreamState;
//struct Test_U_UserData;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 DHCP_Statistic_t,
                                 struct Test_U_UserData> Test_U_IConnectionManager_t;
struct Test_U_SocketHandlerConfiguration;

//////////////////////////////////////////

struct Test_U_ConnectionConfiguration;
struct Test_U_SocketHandlerConfiguration
 : DHCP_SocketHandlerConfiguration
{
  Test_U_SocketHandlerConfiguration ()
   : DHCP_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ConnectionConfiguration* connectionConfiguration;

  struct Test_U_UserData*                userData;
};

//extern const char stream_name_string_[];
struct Test_U_AllocatorConfiguration;
struct Test_U_StreamConfiguration;
struct Test_U_StreamModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_AllocatorConfiguration,
                               struct Test_U_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_StreamModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct Test_U_ConnectionConfiguration
 : DHCP_ConnectionConfiguration
{
  Test_U_ConnectionConfiguration ()
   : DHCP_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  Test_U_IConnectionManager_t*             connectionManager;
  struct Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_U_StreamConfiguration_t*            streamConfiguration;

  struct Test_U_UserData*                  userData;
};
typedef std::map<std::string,
                 struct Test_U_ConnectionConfiguration> Test_U_ConnectionConfigurations_t;
typedef Test_U_ConnectionConfigurations_t::iterator Test_U_ConnectionConfigurationIterator_t;

struct Test_U_ConnectionState
 : DHCP_ConnectionState
{
  Test_U_ConnectionState ()
   : DHCP_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ConnectionConfiguration* configuration;

  struct Test_U_UserData*                userData;
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_U_ConnectionConfiguration,
                          struct Test_U_ConnectionState,
                          DHCP_Statistic_t> Test_U_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_OutboundConnectionStream,
                                enum Stream_StateMachine_ControlState> Test_U_IStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_InboundConnectionStream,
                                enum Stream_StateMachine_ControlState> Test_U_IInboundStreamConnection_t;

//////////////////////////////////////////

// inbound
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Bcast,
                               struct Test_U_SocketHandlerConfiguration> Test_U_SocketHandlerBcast_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
                                     struct Test_U_SocketHandlerConfiguration> Test_U_AsynchSocketHandlerBcast_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram,
                               struct Test_U_SocketHandlerConfiguration> Test_U_SocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram,
                                     struct Test_U_SocketHandlerConfiguration> Test_U_AsynchSocketHandler_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Test_U_SocketHandlerBcast_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_InboundConnectionStream,
                                Common_Timer_Manager_t,
                                struct Test_U_UserData> Test_U_ConnectionBcast_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_AsynchSocketHandlerBcast_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      DHCP_Statistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      Test_U_InboundConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct Test_U_UserData> Test_U_AsynchConnectionBcast_t;
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Test_U_SocketHandler_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_InboundConnectionStream,
                                Common_Timer_Manager_t,
                                struct Test_U_UserData> Test_U_Connection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_AsynchSocketHandler_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      DHCP_Statistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      Test_U_InboundConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct Test_U_UserData> Test_U_AsynchConnection_t;

// outbound
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Bcast,
                               struct Test_U_SocketHandlerConfiguration> Test_U_OutboundSocketHandlerBcast_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
                                     struct Test_U_SocketHandlerConfiguration> Test_U_OutboundAsynchSocketHandlerBcast_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_CODgram,
                               struct Test_U_SocketHandlerConfiguration> Test_U_OutboundSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_CODgram,
                                     struct Test_U_SocketHandlerConfiguration> Test_U_OutboundAsynchSocketHandler_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Test_U_OutboundSocketHandlerBcast_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_OutboundConnectionStream,
                                Common_Timer_Manager_t,
                                struct Test_U_UserData> Test_U_OutboundConnectionBcast_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_OutboundAsynchSocketHandlerBcast_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      DHCP_Statistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      Test_U_OutboundConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct Test_U_UserData> Test_U_OutboundAsynchConnectionBcast_t;
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Test_U_OutboundSocketHandler_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                DHCP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_OutboundConnectionStream,
                                Common_Timer_Manager_t,
                                struct Test_U_UserData> Test_U_OutboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_OutboundAsynchSocketHandler_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      DHCP_Statistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      Test_U_OutboundConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct Test_U_UserData> Test_U_OutboundAsynchConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_U_ConnectionConfiguration> Test_U_IConnector_t;

//////////////////////////////////////////

// inbound
typedef Net_Client_AsynchConnector_T<Test_U_AsynchConnectionBcast_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     DHCP_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_InboundConnectionStream,
                                     struct Test_U_UserData> Test_U_AsynchConnectorBcast_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_ConnectionBcast_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               DHCP_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_InboundConnectionStream,
                               struct Test_U_UserData> Test_U_ConnectorBcast_t;
typedef Net_Client_AsynchConnector_T<Test_U_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     DHCP_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_InboundConnectionStream,
                                     struct Test_U_UserData> Test_U_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               DHCP_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_InboundConnectionStream,
                               struct Test_U_UserData> Test_U_Connector_t;

// outbound
typedef Net_Client_AsynchConnector_T<Test_U_OutboundAsynchConnectionBcast_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     DHCP_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_OutboundConnectionStream,
                                     struct Test_U_UserData> Test_U_OutboundAsynchConnectorBcast_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_OutboundConnectionBcast_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               DHCP_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_OutboundConnectionStream,
                               struct Test_U_UserData> Test_U_OutboundConnectorBcast_t;
typedef Net_Client_AsynchConnector_T<Test_U_OutboundAsynchConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     DHCP_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_OutboundConnectionStream,
                                     struct Test_U_UserData> Test_U_OutboundAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_OutboundConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               DHCP_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_OutboundConnectionStream,
                               struct Test_U_UserData> Test_U_OutboundConnector_t;

#endif
