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

#ifndef FILE_SERVER_CONNECTION_COMMON_H
#define FILE_SERVER_CONNECTION_COMMON_H

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram_Mcast.h>

#include "stream_module_io_stream.h"

#include "net_asynch_udpsockethandler.h"
#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnection.h"
#include "net_iconnector.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_udpconnection_base.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "test_u_common.h"
//#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
//#include "test_u_socket_common.h"
#include "test_u_stream.h"

//struct Test_U_StreamConfiguration;
//struct Test_U_ConnectionConfiguration
// : Net_ConnectionConfiguration
//{
//  inline Test_U_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration ()
//   ///////////////////////////////////////
//   , socketHandlerConfiguration (NULL)
//   , streamConfiguration (NULL)
//   , userData (NULL)
//  {};
//
//  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
//  struct Test_U_StreamConfiguration*        streamConfiguration;
//
//  struct Test_U_UserData*                   userData;
//};
//
//struct Test_U_ConnectionState
// : Net_ConnectionState
//{
//  inline Test_U_ConnectionState ()
//   : Net_ConnectionState ()
//   , userData (NULL)
//  {};
//
//  struct Test_U_UserData* userData;
//};

//////////////////////////////////////////

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct Test_U_ConnectionConfiguration,
//                          struct Test_U_ConnectionState,
//                          Net_RuntimeStatistic_t> Test_U_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_Stream,
                                enum Stream_StateMachine_ControlState> Test_U_IStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_UDPStream,
                                enum Stream_StateMachine_ControlState> Test_U_UDPIStreamConnection_t;

//////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_U_StreamState,
                                      struct Test_U_StreamConfiguration,
                                      Net_RuntimeStatistic_t,
                                      struct Stream_ModuleConfiguration,
                                      struct Test_U_ModuleHandlerConfiguration,
                                      struct Test_U_FileServer_SessionData,
                                      Test_U_FileServer_SessionData_t,
                                      Test_U_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_U_InetConnectionManager_t,
                                      struct Test_U_UserData> Test_U_NetStream_t;

typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct Test_U_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct Test_U_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPHandler_2;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_MCAST,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct Test_U_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct Test_U_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPIPMulticastHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_BCAST,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct Test_U_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct Test_U_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPIPBroadcastHandler_t;

typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct Test_U_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct Test_U_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_MCAST,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct Test_U_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct Test_U_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchIPMulticastHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_BCAST,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct Test_U_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct Test_U_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchIPBroadcastHandler_t;

typedef Net_UDPConnectionBase_T<Test_U_UDPHandler_2,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_NetStream_t,
                                struct Test_U_UserData> Test_U_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_UDPAsynchHandler_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      Net_RuntimeStatistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      Test_U_NetStream_t,
                                      struct Test_U_UserData> Test_U_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_U_SocketHandlerConfiguration> Test_U_IInetConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     Net_RuntimeStatistic_t,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_NetStream_t,
                                     struct Test_U_UserData> Test_U_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_U_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               Net_RuntimeStatistic_t,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_NetStream_t,
                               struct Test_U_UserData> Test_U_UDPConnector_t;

#endif
