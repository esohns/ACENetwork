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

#ifndef IRC_CLIENT_NETWORK_H
#define IRC_CLIENT_NETWORK_H

#include <map>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_configuration.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "irc_common.h"
#include "irc_network.h"

#include "IRC_client_defines.h"
//#include "IRC_client_stream.h"
#include "IRC_client_stream_common.h"

//////////////////////////////////////////

class IRC_Client_ConnectionConfiguration;
struct IRC_Client_StreamConfiguration;
struct IRC_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct IRC_Client_StreamConfiguration,
                               struct IRC_Client_ModuleHandlerConfiguration> IRC_Client_StreamConfiguration_t;
//class IRC_Client_ConnectionConfiguration
// : public Net_ConnectionConfiguration_T<struct IRC_AllocatorConfiguration,
//                                        IRC_Client_StreamConfiguration_t,
//                                        NET_TRANSPORTLAYER_TCP>
//{
// public:
//  IRC_Client_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration_T ()
//   ///////////////////////////////////////
//   , cursesState (NULL)
//   , protocolConfiguration (NULL)
//   , logToFile (IRC_CLIENT_SESSION_DEFAULT_LOG)
//  {
//    PDUSize = IRC_MAXIMUM_FRAME_SIZE;
//  }

//  struct IRC_Client_CursesState*    cursesState;
//  struct IRC_ProtocolConfiguration* protocolConfiguration;
//  bool                              logToFile;
//};

//struct IRC_Client_ConnectionState
// : IRC_ConnectionState
//{
//  IRC_Client_ConnectionState ()
//   : IRC_ConnectionState ()
//   //, configuration (NULL)
////   , userData (NULL)
//  {}
//
//  //IRC_Client_ConnectionConfiguration* configuration;
//
////  struct Net_UserData*           userData;
//};
//// *TODO*: remove this ASAP
//struct IRC_SessionState
// : IRC_Client_ConnectionState
//{
//  IRC_SessionState ()
//   : IRC_Client_ConnectionState ()
//   , away (false)
//   , channel ()
//   , channelModes ()
//   , isFirstMessage (false)
//   , nickName ()
//   , userModes ()
//  {}
//
//  // *TODO*: remove this
//  bool               away;
//  std::string        channel;
//  IRC_ChannelModes_t channelModes;
//  bool               isFirstMessage;
//  std::string        nickName;
//  IRC_UserModes_t    userModes;
//};

//////////////////////////////////////////
template <typename TimerManagerType> // implements Common_ITimer
class IRC_Client_Stream_T;
typedef IRC_Client_Stream_T<Common_Timer_Manager_t> IRC_Client_Stream_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                IRC_Client_ConnectionConfiguration,
                                struct IRC_SessionState,
                                IRC_Statistic_t,
                                IRC_Client_Stream_t,
                                struct Net_UserData> IRC_Client_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      IRC_Client_ConnectionConfiguration,
                                      struct IRC_SessionState,
                                      IRC_Statistic_t,
                                      IRC_Client_Stream_t,
                                      struct Net_UserData> IRC_Client_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          //IRC_Client_ConnectionConfiguration,
                          struct IRC_SessionState,
                          IRC_Statistic_t> IRC_Client_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                IRC_Client_ConnectionConfiguration,
                                struct IRC_SessionState,
                                IRC_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                IRC_Client_Stream_t,
                                enum Stream_StateMachine_ControlState> IRC_Client_IStreamConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       IRC_Client_ConnectionConfiguration,
//                       struct IRC_Client_ConnectionState,
//                       IRC_Client_Statistic_t,
//                       IRC_Client_Stream> IRC_Client_ISession_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         IRC_Client_ConnectionConfiguration> IRC_Client_IConnector_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               IRC_Client_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               IRC_Client_ConnectionConfiguration,
                               struct IRC_SessionState,
                               IRC_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               IRC_Client_Stream_t,
                               struct Net_UserData> IRC_Client_Connector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     IRC_Client_ConnectionConfiguration,
                                     struct IRC_SessionState,
                                     IRC_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     IRC_Client_Stream_t,
                                     struct Net_UserData> IRC_Client_AsynchConnector_t;

//////////////////////////////////////////

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 IRC_Client_ConnectionConfiguration,
                                 struct IRC_SessionState,
                                 IRC_Statistic_t,
                                 struct Net_UserData> IRC_Client_Connection_Manager_t;
typedef ACE_Singleton<IRC_Client_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> IRC_CLIENT_CONNECTIONMANAGER_SINGLETON;

#endif
