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

#ifndef TEST_I_CONNECTION_COMMON_H
#define TEST_I_CONNECTION_COMMON_H

#include <map>
#include <string>

#include "ace/INET_Addr.h"
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

#include "ftp_common.h"

//#include "test_i_connection_stream.h"
//#include "test_i_connection_stream_2.h"

// forward declarations
class Test_I_Message;
class FTP_Client_ConnectionConfiguration;
class Test_I_ConnectionStream;
class Test_I_ConnectionStream_2;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FTP_Client_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 FTP_Statistic_t,
                                 struct Net_UserData> FTP_Client_ConnectionManager_t;

struct FTP_Client_StreamConfiguration;
struct FTP_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<struct FTP_Client_StreamConfiguration,
                               struct FTP_Client_ModuleHandlerConfiguration> FTP_Client_StreamConfiguration_t;
class FTP_Client_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<FTP_Client_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  FTP_Client_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Net_StreamConnectionState,
                          FTP_Statistic_t> FTP_Client_IConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_ConnectionStream,
                                enum Stream_StateMachine_ControlState> FTP_Client_IStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_ConnectionStream_2,
                                enum Stream_StateMachine_ControlState> FTP_Client_IStreamConnection_2;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Test_I_ConnectionStream,
                                struct Net_UserData> FTP_Client_Connection_t;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      FTP_Client_ConnectionConfiguration,
                                      struct Net_StreamConnectionState,
                                      FTP_Statistic_t,
                                      Test_I_ConnectionStream,
                                      struct Net_UserData> FTP_Client_AsynchConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Test_I_ConnectionStream,
                                struct Net_UserData> FTP_Client_SSLConnection_t;
#endif // SSL_SUPPORT

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Test_I_ConnectionStream_2,
                                struct Net_UserData> FTP_Client_Connection_2;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      FTP_Client_ConnectionConfiguration,
                                      struct Net_StreamConnectionState,
                                      FTP_Statistic_t,
                                      Test_I_ConnectionStream_2,
                                      struct Net_UserData> FTP_Client_AsynchConnection_2;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                FTP_Client_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                FTP_Statistic_t,
                                Test_I_ConnectionStream_2,
                                struct Net_UserData> FTP_Client_SSLConnection_2;
#endif // SSL_SUPPORT

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         FTP_Client_ConnectionConfiguration> FTP_Client_IConnector_t;

//////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<FTP_Client_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     FTP_Client_ConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     FTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_ConnectionStream,
                                     struct Net_UserData> FTP_Client_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               FTP_Client_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               FTP_Client_ConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               FTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_ConnectionStream,
                               struct Net_UserData> FTP_Client_Connector_t;

typedef Net_Client_AsynchConnector_T<FTP_Client_AsynchConnection_2,
                                     ACE_INET_Addr,
                                     FTP_Client_ConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     FTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_ConnectionStream_2,
                                     struct Net_UserData> FTP_Client_AsynchConnector_2;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               FTP_Client_Connection_2,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               FTP_Client_ConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               FTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_ConnectionStream_2,
                               struct Net_UserData> FTP_Client_Connector_2;

#endif
