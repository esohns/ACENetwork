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
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

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

#include "pcp_common.h"
#include "pcp_configuration.h"

//#include "test_u_common.h"

// forward declarations
class Test_U_InboundConnectionStream;
class Test_U_OutboundConnectionStream;
class PCPClient_ConnectionConfiguration;

//extern const char stream_name_string_[];
//struct PCPClient_StreamConfiguration;
struct PCPClient_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct PCP_StreamConfiguration,
                               struct PCPClient_ModuleHandlerConfiguration> PCPClient_StreamConfiguration_t;
class PCPClient_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<PCPClient_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_UDP>
{
 public:
  PCPClient_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};
//typedef std::map<std::string,
//                 PCPClient_ConnectionConfiguration> PCPClient_ConnectionConfigurations_t;
//typedef PCPClient_ConnectionConfigurations_t::iterator PCPClient_ConnectionConfigurationIterator_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct PCP_ConnectionState,
                          PCP_Statistic_t> PCPClient_IOutboundConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                PCPClient_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                Net_UDPSocketConfiguration_t,
                                Test_U_OutboundConnectionStream,
                                enum Stream_StateMachine_ControlState> PCPClient_IOutboundStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                PCPClient_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                Net_UDPSocketConfiguration_t,
                                Test_U_InboundConnectionStream,
                                enum Stream_StateMachine_ControlState> PCPClient_IInboundStreamConnection_t;

//////////////////////////////////////////

// inbound
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                PCPClient_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                Test_U_InboundConnectionStream,
                                struct Net_UserData> PCPClient_InboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      PCPClient_ConnectionConfiguration,
                                      struct PCP_ConnectionState,
                                      PCP_Statistic_t,
                                      Test_U_InboundConnectionStream,
                                      struct Net_UserData> PCPClient_InboundAsynchConnection_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandlerMcast_t,
                                PCPClient_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                Test_U_InboundConnectionStream,
                                struct Net_UserData> PCPClient_InboundConnectionMcast_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandlerMcast_t,
                                      PCPClient_ConnectionConfiguration,
                                      struct PCP_ConnectionState,
                                      PCP_Statistic_t,
                                      Test_U_InboundConnectionStream,
                                      struct Net_UserData> PCPClient_InboundAsynchConnectionMcast_t;

// outbound
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                PCPClient_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                Test_U_OutboundConnectionStream,
                                struct Net_UserData> PCPClient_OutboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      PCPClient_ConnectionConfiguration,
                                      struct PCP_ConnectionState,
                                      PCP_Statistic_t,
                                      Test_U_OutboundConnectionStream,
                                      struct Net_UserData> PCPClient_OutboundAsynchConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         PCPClient_ConnectionConfiguration> PCPClient_IConnector_t;

//////////////////////////////////////////

// inbound
typedef Net_Client_AsynchConnector_T<PCPClient_InboundAsynchConnection_t,
                                     ACE_INET_Addr,
                                     PCPClient_ConnectionConfiguration,
                                     struct PCP_ConnectionState,
                                     PCP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_InboundConnectionStream,
                                     struct Net_UserData> PCPClient_InboundAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               PCPClient_InboundConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               PCPClient_ConnectionConfiguration,
                               struct PCP_ConnectionState,
                               PCP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_InboundConnectionStream,
                               struct Net_UserData> PCPClient_InboundConnector_t;

typedef Net_Client_AsynchConnector_T<PCPClient_InboundAsynchConnectionMcast_t,
                                     ACE_INET_Addr,
                                     PCPClient_ConnectionConfiguration,
                                     struct PCP_ConnectionState,
                                     PCP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_InboundConnectionStream,
                                     struct Net_UserData> PCPClient_InboundAsynchConnectorMcast_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               PCPClient_InboundConnectionMcast_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               PCPClient_ConnectionConfiguration,
                               struct PCP_ConnectionState,
                               PCP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_InboundConnectionStream,
                               struct Net_UserData> PCPClient_InboundConnectorMcast_t;

// outbound
typedef Net_Client_AsynchConnector_T<PCPClient_OutboundAsynchConnection_t,
                                     ACE_INET_Addr,
                                     PCPClient_ConnectionConfiguration,
                                     struct PCP_ConnectionState,
                                     PCP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_OutboundConnectionStream,
                                     struct Net_UserData> PCPClient_OutboundAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               PCPClient_OutboundConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               PCPClient_ConnectionConfiguration,
                               struct PCP_ConnectionState,
                               PCP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_OutboundConnectionStream,
                               struct Net_UserData> PCPClient_OutboundConnector_t;

#endif
