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
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpconnection_base.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "http_common.h"
#include "http_network.h"

#include "ssdp_common.h"
#include "ssdp_control.h"
#include "ssdp_isession.h"
#include "ssdp_session.h"

// forward declarations
class Test_U_InboundSSDPConnectionStream;
class Test_U_OutboundSSDPConnectionStream;
class Test_U_HTTP_ConnectionStream;
class Test_U_Message;
class UPnP_Client_HTTP_ConnectionConfiguration;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 UPnP_Client_HTTP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_HTTP_ConnectionManager_t;

struct UPnP_Client_StreamConfiguration;
struct UPnP_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<struct UPnP_Client_StreamConfiguration,
                               struct UPnP_Client_ModuleHandlerConfiguration> UPnP_Client_StreamConfiguration_t;
class UPnP_Client_SSDP_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<UPnP_Client_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_UDP>
{
 public:
  UPnP_Client_SSDP_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};

class UPnP_Client_HTTP_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<UPnP_Client_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  UPnP_Client_HTTP_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> UPnP_Client_IConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                UPnP_Client_SSDP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_UDPSocketConfiguration_t,
                                Test_U_OutboundSSDPConnectionStream,
                                enum Stream_StateMachine_ControlState> UPnP_Client_IOutbound_SSDP_StreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                UPnP_Client_SSDP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_UDPSocketConfiguration_t,
                                Test_U_InboundSSDPConnectionStream,
                                enum Stream_StateMachine_ControlState> UPnP_Client_IInbound_SSDP_StreamConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                UPnP_Client_HTTP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_U_HTTP_ConnectionStream,
                                enum Stream_StateMachine_ControlState> UPnP_Client_I_HTTP_StreamConnection_t;

//////////////////////////////////////////

// inbound
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                UPnP_Client_SSDP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_U_InboundSSDPConnectionStream,
                                struct Net_UserData> UPnP_Client_InboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      UPnP_Client_SSDP_ConnectionConfiguration,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_U_InboundSSDPConnectionStream,
                                      struct Net_UserData> UPnP_Client_InboundAsynchConnection_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandlerMcast_t,
                                UPnP_Client_SSDP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_U_InboundSSDPConnectionStream,
                                struct Net_UserData> UPnP_Client_InboundConnectionMcast_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandlerMcast_t,
                                      UPnP_Client_SSDP_ConnectionConfiguration,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_U_InboundSSDPConnectionStream,
                                      struct Net_UserData> UPnP_Client_InboundAsynchConnectionMcast_t;

// outbound
typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                UPnP_Client_SSDP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_U_OutboundSSDPConnectionStream,
                                struct Net_UserData> UPnP_Client_OutboundConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      UPnP_Client_SSDP_ConnectionConfiguration,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_U_OutboundSSDPConnectionStream,
                                      struct Net_UserData> UPnP_Client_OutboundAsynchConnection_t;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                UPnP_Client_HTTP_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_U_HTTP_ConnectionStream,
                                struct Net_UserData> UPnP_Client_Connection_t;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      UPnP_Client_HTTP_ConnectionConfiguration,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_U_HTTP_ConnectionStream,
                                      struct Net_UserData> UPnP_Client_AsynchConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         UPnP_Client_SSDP_ConnectionConfiguration> UPnP_Client_IConnector_t;

typedef Net_IConnector_T<ACE_INET_Addr,
                         UPnP_Client_HTTP_ConnectionConfiguration> UPnP_Client_IConnector_2;

//////////////////////////////////////////

// inbound
typedef Net_Client_AsynchConnector_T<UPnP_Client_InboundAsynchConnection_t,
                                     ACE_INET_Addr,
                                     UPnP_Client_SSDP_ConnectionConfiguration,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_InboundSSDPConnectionStream,
                                     struct Net_UserData> UPnP_Client_InboundAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               UPnP_Client_InboundConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               UPnP_Client_SSDP_ConnectionConfiguration,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_InboundSSDPConnectionStream,
                               struct Net_UserData> UPnP_Client_InboundConnector_t;

typedef Net_Client_AsynchConnector_T<UPnP_Client_InboundAsynchConnectionMcast_t,
                                     ACE_INET_Addr,
                                     UPnP_Client_SSDP_ConnectionConfiguration,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_InboundSSDPConnectionStream,
                                     struct Net_UserData> UPnP_Client_InboundAsynchConnectorMcast_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               UPnP_Client_InboundConnectionMcast_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               UPnP_Client_SSDP_ConnectionConfiguration,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_InboundSSDPConnectionStream,
                               struct Net_UserData> UPnP_Client_InboundConnectorMcast_t;

// outbound
typedef Net_Client_AsynchConnector_T<UPnP_Client_OutboundAsynchConnection_t,
                                     ACE_INET_Addr,
                                     UPnP_Client_SSDP_ConnectionConfiguration,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_OutboundSSDPConnectionStream,
                                     struct Net_UserData> UPnP_Client_OutboundAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               UPnP_Client_OutboundConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               UPnP_Client_SSDP_ConnectionConfiguration,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_OutboundSSDPConnectionStream,
                               struct Net_UserData> UPnP_Client_OutboundConnector_t;

//////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<UPnP_Client_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     UPnP_Client_HTTP_ConnectionConfiguration,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_U_HTTP_ConnectionStream,
                                     struct Net_UserData> UPnP_Client_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               UPnP_Client_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               UPnP_Client_HTTP_ConnectionConfiguration,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_U_HTTP_ConnectionStream,
                               struct Net_UserData> UPnP_Client_Connector_t;

//////////////////////////////////////////

typedef SSDP_ISession_T<struct SSDP_SessionState,
                        UPnP_Client_HTTP_ConnectionConfiguration,
                        struct Net_UserData> SSDP_ISession_t;
typedef SSDP_Session_T<struct SSDP_SessionState,
                       UPnP_Client_HTTP_ConnectionConfiguration,
                       UPnP_Client_HTTP_ConnectionManager_t,
                       Test_U_Message,
                       UPnP_Client_Connector_t,
                       struct Net_UserData> SSDP_Session_t;
typedef SSDP_Session_T<struct SSDP_SessionState,
                       UPnP_Client_HTTP_ConnectionConfiguration,
                       UPnP_Client_HTTP_ConnectionManager_t,
                       Test_U_Message,
                       UPnP_Client_AsynchConnector_t,
                       struct Net_UserData> SSDP_AsynchSession_t;

typedef SSDP_Control_T<SSDP_ISession_t> SSDP_Control_t;

#endif
