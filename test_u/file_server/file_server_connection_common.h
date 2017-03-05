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

#include "stream_common.h"
#include "stream_control_message.h"
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
#include "test_u_connection_common.h"
//#include "test_u_connection_manager_common.h"
//#include "test_u_socket_common.h"
//#include "test_u_stream.h"

#include "file_server_defines.h"
#include "file_server_stream_common.h"

// forward declarations
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_ControlMessage_t;
class Test_U_Stream;
class Test_U_UDPStream;
struct FileServer_ConnectionConfiguration;
struct FileServer_ConnectionState;
struct FileServer_UserData;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct FileServer_UserData> FileServer_IConnectionManager_t;

struct FileServer_SocketHandlerConfiguration;
struct FileServer_StreamConfiguration;
struct FileServer_ConnectionConfiguration
 : Test_U_ConnectionConfiguration
{
  inline FileServer_ConnectionConfiguration ()
   : Test_U_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {
    PDUSize = FILE_SERVER_DEFAULT_MESSAGE_DATA_BUFFER_SIZE;
  };

  FileServer_IConnectionManager_t*              connectionManager;
  struct FileServer_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct FileServer_StreamConfiguration*        streamConfiguration;

  struct FileServer_UserData*                   userData;
};

struct FileServer_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline FileServer_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct FileServer_ConnectionConfiguration* connectionConfiguration;

  struct FileServer_UserData*                userData;
};

struct FileServer_ConnectionState
 : Net_ConnectionState
{
  inline FileServer_ConnectionState ()
   : Net_ConnectionState ()
   , userData (NULL)
  {};

  struct FileServer_UserData* userData;
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct FileServer_ConnectionConfiguration,
                          struct FileServer_ConnectionState,
                          Net_RuntimeStatistic_t> FileServer_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct FileServer_ConnectionConfiguration,
                                struct FileServer_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_Stream,
                                enum Stream_StateMachine_ControlState> FileServer_IStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct FileServer_ConnectionConfiguration,
                                struct FileServer_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_UDPStream,
                                enum Stream_StateMachine_ControlState> FileServer_UDPIStreamConnection_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct FileServer_UserData> FileServer_InetConnectionManager_t;

//////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct FileServer_StreamState,
                                      struct FileServer_StreamConfiguration,
                                      Net_RuntimeStatistic_t,
                                      struct Stream_ModuleConfiguration,
                                      struct Test_U_ModuleHandlerConfiguration,
                                      struct FileServer_SessionData,
                                      FileServer_SessionData_t,
                                      Test_U_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      ACE_INET_Addr,
                                      FileServer_InetConnectionManager_t,
                                      struct FileServer_UserData> Test_U_NetStream_t;

typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct FileServer_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct FileServer_ConnectionConfiguration,
                                  struct FileServer_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct FileServer_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct FileServer_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPHandler_2;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_MCAST,
                                                         struct FileServer_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct FileServer_ConnectionConfiguration,
                                  struct FileServer_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct FileServer_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct FileServer_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPIPMulticastHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_BCAST,
                                                         struct FileServer_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct FileServer_ConnectionConfiguration,
                                  struct FileServer_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct FileServer_SocketHandlerConfiguration,
                                  Test_U_NetStream_t,
                                  struct FileServer_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_UDPIPBroadcastHandler_t;

typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct FileServer_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        struct FileServer_ConnectionConfiguration,
                                        struct FileServer_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct FileServer_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct FileServer_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct FileServer_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_MCAST,
                                        ACE_INET_Addr,
                                        struct FileServer_ConnectionConfiguration,
                                        struct FileServer_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct FileServer_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct FileServer_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchIPMulticastHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct FileServer_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_BCAST,
                                        ACE_INET_Addr,
                                        struct FileServer_ConnectionConfiguration,
                                        struct FileServer_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct FileServer_SocketHandlerConfiguration,
                                        Test_U_NetStream_t,
                                        struct FileServer_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_UDPAsynchIPBroadcastHandler_t;

typedef Net_UDPConnectionBase_T<Test_U_UDPHandler_2,
                                struct FileServer_ConnectionConfiguration,
                                struct FileServer_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_NetStream_t,
                                struct FileServer_UserData> Test_U_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_UDPAsynchHandler_t,
                                      struct FileServer_ConnectionConfiguration,
                                      struct FileServer_ConnectionState,
                                      Net_RuntimeStatistic_t,
                                      struct FileServer_SocketHandlerConfiguration,
                                      Test_U_NetStream_t,
                                      struct FileServer_UserData> Test_U_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct FileServer_SocketHandlerConfiguration> Test_U_IInetConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     struct FileServer_ConnectionConfiguration,
                                     struct FileServer_ConnectionState,
                                     Net_RuntimeStatistic_t,
                                     struct FileServer_SocketHandlerConfiguration,
                                     Test_U_NetStream_t,
                                     struct FileServer_UserData> Test_U_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_U_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct FileServer_ConnectionConfiguration,
                               struct FileServer_ConnectionState,
                               Net_RuntimeStatistic_t,
                               struct FileServer_SocketHandlerConfiguration,
                               Test_U_NetStream_t,
                               struct FileServer_UserData> Test_U_UDPConnector_t;

#endif
