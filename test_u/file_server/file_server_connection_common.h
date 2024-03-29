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
#include "test_u_stream.h"
#include "test_u_stream_common.h"

#include "file_server_defines.h"

// forward declarations
//class Test_U_Stream;
//class Test_U_UDPStream;

//extern const char stream_name_string_[];
struct FileServer_StreamConfiguration;
struct FileServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct FileServer_StreamConfiguration,
                               struct FileServer_ModuleHandlerConfiguration> FileServer_StreamConfiguration_t;
class FileServer_TCPConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<FileServer_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  FileServer_TCPConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {
    //PDUSize = FILE_SERVER_DEFAULT_MESSAGE_DATA_BUFFER_SIZE;
  }
};
//typedef std::map<std::string,
//                 FileServer_TCPConnectionConfiguration> FileServer_TCPConnectionConfigurations_t;
//typedef FileServer_TCPConnectionConfigurations_t::iterator FileServer_TCPConnectionConfigurationIterator_t;

class FileServer_UDPConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<FileServer_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_UDP>
{
 public:
  FileServer_UDPConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {
    //PDUSize = FILE_SERVER_DEFAULT_MESSAGE_DATA_BUFFER_SIZE;
  }
};
//typedef std::map<std::string,
//                 FileServer_UDPConnectionConfiguration> FileServer_UDPConnectionConfigurations_t;
//typedef FileServer_UDPConnectionConfigurations_t::iterator FileServer_UDPConnectionConfigurationIterator_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_TCPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_UDPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_UDPConnectionManager_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          //FileServer_TCPConnectionConfiguration,
                          struct Net_StreamConnectionState,
                          Net_StreamStatistic_t> FileServer_TCPIConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FileServer_TCPConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_U_Stream,
                                enum Stream_StateMachine_ControlState> FileServer_TCPIStreamConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //FileServer_UDPConnectionConfiguration,
                          struct Net_StreamConnectionState,
                          Net_StreamStatistic_t> FileServer_UDPIConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FileServer_UDPConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Net_UDPSocketConfiguration_t,
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
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct FileServer_ModuleHandlerConfiguration,
                                      struct FileServer_SessionData,
                                      FileServer_SessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      ACE_INET_Addr,
                                      FileServer_TCPConnectionManager_t,
                                      struct Stream_UserData> Test_U_TCPNetStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct FileServer_StreamState,
                                      struct FileServer_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct FileServer_ModuleHandlerConfiguration,
                                      struct FileServer_SessionData,
                                      FileServer_SessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      ACE_INET_Addr,
                                      FileServer_UDPConnectionManager_t,
                                      struct Stream_UserData> Test_U_UDPNetStream_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                FileServer_UDPConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_U_UDPNetStream_t,
                                struct Net_UserData> Test_U_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      FileServer_UDPConnectionConfiguration,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_U_UDPNetStream_t,
                                      struct Net_UserData> Test_U_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         FileServer_TCPConnectionConfiguration> Test_U_TCPIConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         FileServer_UDPConnectionConfiguration> Test_U_UDPIConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     FileServer_UDPConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_UDPNetStream_t,
                                     struct Net_UserData> Test_U_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               FileServer_UDPConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_UDPNetStream_t,
                               struct Net_UserData> Test_U_UDPConnector_t;

#endif
