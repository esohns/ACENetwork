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

#include <map>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Dgram_Mcast.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

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
#include "test_u_stream_common.h"

#include "file_server_defines.h"

// forward declarations
class Test_U_Stream;
class Test_U_UDPStream;
struct FileServer_ConnectionConfiguration;
struct FileServer_ConnectionState;
struct FileServer_UserData;

struct FileServer_ConnectionConfiguration;
struct FileServer_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  FileServer_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   , socketConfiguration_2 ()
   , socketConfiguration_3 ()
   , connectionConfiguration (NULL)
   , userData (NULL)
  {
    socketConfiguration = &socketConfiguration_2;
  };

  struct Net_TCPSocketConfiguration          socketConfiguration_2;
  struct Net_UDPSocketConfiguration          socketConfiguration_3;
  struct FileServer_ConnectionConfiguration* connectionConfiguration;

  struct FileServer_UserData*                userData;
};

//extern const char stream_name_string_[];
struct FileServer_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Net_AllocatorConfiguration,
                               struct FileServer_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_ModuleHandlerConfiguration> FileServer_StreamConfiguration_t;
struct FileServer_ConnectionConfiguration;
typedef Net_ConnectionConfiguration_T<struct FileServer_ConnectionConfiguration,
                                      struct Net_AllocatorConfiguration,
                                      FileServer_StreamConfiguration_t> FileServer_ConnectionConfiguration_t;
struct FileServer_ConnectionState
 : Net_ConnectionState
{
  FileServer_ConnectionState ()
   : Net_ConnectionState ()
   , userData (NULL)
  {};

  struct FileServer_UserData* userData;
};
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_ConnectionConfiguration_t,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct FileServer_UserData> FileServer_IInetConnectionManager_t;
struct FileServer_ConnectionConfiguration
 : Test_U_ConnectionConfiguration
{
  FileServer_ConnectionConfiguration ()
   : Test_U_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , userData (NULL)
  {
    PDUSize = FILE_SERVER_DEFAULT_MESSAGE_DATA_BUFFER_SIZE;
  };

  FileServer_IInetConnectionManager_t*         connectionManager;
  struct FileServer_SocketHandlerConfiguration socketHandlerConfiguration;

  struct FileServer_UserData*                  userData;
};
typedef std::map<std::string,
                 FileServer_ConnectionConfiguration_t> FileServer_ConnectionConfigurations_t;
typedef FileServer_ConnectionConfigurations_t::iterator FileServer_ConnectionConfigurationIterator_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_ConnectionConfiguration_t,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct FileServer_UserData> FileServer_InetConnectionManager_t;

struct FileServer_ListenerConfiguration
 : Net_ListenerConfiguration
{
  FileServer_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionConfiguration (NULL)
   , connectionManager (NULL)
  {};

  FileServer_ConnectionConfiguration_t* connectionConfiguration;
  FileServer_IInetConnectionManager_t*  connectionManager;
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          FileServer_ConnectionConfiguration_t,
                          struct FileServer_ConnectionState,
                          Net_Statistic_t> FileServer_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FileServer_ConnectionConfiguration_t,
                                struct FileServer_ConnectionState,
                                Net_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_Stream,
                                enum Stream_StateMachine_ControlState> FileServer_IStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FileServer_ConnectionConfiguration_t,
                                struct FileServer_ConnectionState,
                                Net_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_UDPStream,
                                enum Stream_StateMachine_ControlState> FileServer_UDPIStreamConnection_t;

//////////////////////////////////////////

extern const char stream_name_string_[];

struct FileServer_SessionData;
typedef Stream_SessionData_T<struct FileServer_SessionData> FileServer_SessionData_t;
class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct FileServer_StreamState,
                                      struct FileServer_StreamConfiguration,
                                      Net_Statistic_t,
                                      Common_Timer_Manager_t,
                                      struct Net_AllocatorConfiguration,
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

typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram,
                               struct FileServer_SocketHandlerConfiguration> Test_U_UDPSocketHandler_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Mcast,
                               struct FileServer_SocketHandlerConfiguration> Test_U_UDPIPMulticastSocketHandler_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Bcast,
                               struct FileServer_SocketHandlerConfiguration> Test_U_UDPIPBroadcastSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram,
                                     struct FileServer_SocketHandlerConfiguration> Test_U_UDPAsynchSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Mcast,
                                     struct FileServer_SocketHandlerConfiguration> Test_U_UDPAsynchIPMulticastSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
                                     struct FileServer_SocketHandlerConfiguration> Test_U_UDPAsynchIPBroadcastSocketHandler_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Test_U_UDPSocketHandler_t,
                                FileServer_ConnectionConfiguration_t,
                                struct FileServer_ConnectionState,
                                Net_Statistic_t,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_NetStream_t,
                                Common_Timer_Manager_t,
                                struct FileServer_UserData> Test_U_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_U_UDPAsynchSocketHandler_t,
                                      FileServer_ConnectionConfiguration_t,
                                      struct FileServer_ConnectionState,
                                      Net_Statistic_t,
                                      struct FileServer_SocketHandlerConfiguration,
                                      Test_U_NetStream_t,
                                      Common_Timer_Manager_t,
                                      struct FileServer_UserData> Test_U_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         FileServer_ConnectionConfiguration_t> Test_U_IInetConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     FileServer_ConnectionConfiguration_t,
                                     struct FileServer_ConnectionState,
                                     Net_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct FileServer_SocketHandlerConfiguration,
                                     Test_U_NetStream_t,
                                     struct FileServer_UserData> Test_U_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               FileServer_ConnectionConfiguration_t,
                               struct FileServer_ConnectionState,
                               Net_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct FileServer_SocketHandlerConfiguration,
                               Test_U_NetStream_t,
                               struct FileServer_UserData> Test_U_UDPConnector_t;

#endif
