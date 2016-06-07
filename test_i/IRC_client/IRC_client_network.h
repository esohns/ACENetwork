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

#include "ace/Global_Macros.h"

#include "irc_network.h"

#include "IRC_client_common.h"
#include "IRC_client_configuration.h"
#include "IRC_client_curses.h"
#include "IRC_client_stream.h"
#include "IRC_client_stream_common.h"

/////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<IRC_Client_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ///////
                                  ACE_INET_Addr,
                                  IRC_Client_Configuration,
                                  IRC_Client_SessionState,
                                  IRC_RuntimeStatistic_t,
                                  IRC_Client_Stream,
                                  ///////
                                  IRC_Client_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  IRC_Client_ModuleHandlerConfiguration> IRC_Client_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<IRC_Client_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        IRC_Client_Configuration,
                                        IRC_Client_SessionState,
                                        IRC_RuntimeStatistic_t,
                                        IRC_Client_Stream,

                                        IRC_Client_UserData,

                                        Stream_ModuleConfiguration,
                                        IRC_Client_ModuleHandlerConfiguration> IRC_Client_AsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<IRC_Client_TCPHandler_t,
                                /////////
                                IRC_Client_Configuration,
                                IRC_Client_SessionState,
                                IRC_RuntimeStatistic_t,
                                IRC_Client_Stream,
                                /////////
                                IRC_Client_SocketHandlerConfiguration,
                                /////////
                                IRC_Client_UserData> IRC_Client_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<IRC_Client_AsynchTCPHandler_t,
                                      ///
                                      IRC_Client_Configuration,
                                      IRC_Client_SessionState,
                                      IRC_RuntimeStatistic_t,
                                      IRC_Client_Stream,
                                      ///
                                      IRC_Client_SocketHandlerConfiguration,
                                      ///
                                      IRC_Client_UserData> IRC_Client_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          IRC_Client_Configuration,
                          IRC_Client_SessionState,
                          IRC_RuntimeStatistic_t> IRC_Client_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                IRC_Client_Configuration,
                                IRC_Client_SessionState,
                                IRC_RuntimeStatistic_t,
                                IRC_Client_Stream,
                                Stream_StateMachine_ControlState,
                                /////////
                                Net_SocketConfiguration,
                                /////////
                                IRC_Client_SocketHandlerConfiguration> IRC_Client_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       Net_SocketConfiguration,
//                       IRC_Client_Configuration,
//                       IRC_Client_ConnectionState,
//                       IRC_Client_RuntimeStatistic_t,
//                       IRC_Client_Stream> IRC_Client_ISession_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         IRC_Client_SocketHandlerConfiguration> IRC_Client_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         IRC_Client_ConnectorConfiguration> IRC_Client_IConnector_t;
typedef Net_Client_Connector_T<IRC_Client_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               //////////
                               ACE_INET_Addr,
                               IRC_Client_Configuration,
                               IRC_Client_SessionState,
                               IRC_RuntimeStatistic_t,
                               IRC_Client_Stream,
                               //////////
                               IRC_Client_SocketHandlerConfiguration,
                               //////////
                               IRC_Client_UserData> IRC_Client_Connector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_AsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     IRC_Client_Configuration,
                                     IRC_Client_SessionState,
                                     IRC_RuntimeStatistic_t,
                                     IRC_Client_Stream,
                                     ////
                                     IRC_Client_SocketHandlerConfiguration,
                                     ////
                                     IRC_Client_UserData> IRC_Client_AsynchConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 IRC_Client_Configuration,
                                 IRC_Client_SessionState,
                                 IRC_RuntimeStatistic_t,
                                 ////////
                                 IRC_Client_UserData> IRC_Client_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 IRC_Client_Configuration,
                                 IRC_Client_SessionState,
                                 IRC_RuntimeStatistic_t,
                                 ////////
                                 IRC_Client_UserData> IRC_Client_Connection_Manager_t;

typedef ACE_Singleton<IRC_Client_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> IRC_CLIENT_CONNECTIONMANAGER_SINGLETON;

#endif
