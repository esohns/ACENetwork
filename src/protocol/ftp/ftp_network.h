/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef FTP_NETWORK_H
#define FTP_NETWORK_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_socket_common.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "ftp_common.h"
#include "ftp_configuration.h"
#include "ftp_stream_common.h"

typedef Net_StreamConnectionConfiguration_T<FTP_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> FTP_ConnectionConfiguration_t;

struct FTP_ConnectionState
 : Net_StreamConnectionState
{
  FTP_ConnectionState ()
   : Net_StreamConnectionState ()
   , protocolState (FTP_STATE_INVALID)
  {}

  enum FTP_ProtocolState protocolState;
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct FTP_ConnectionState,
                          struct Net_StreamStatistic> FTP_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                FTP_ConnectionConfiguration_t,
                                struct FTP_ConnectionState,
                                struct Net_StreamStatistic,
                                struct FTP_SocketHandlerConfiguration> FTP_ISocketConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                FTP_ConnectionConfiguration_t,
                                struct FTP_ConnectionState,
                                struct Net_StreamStatistic,
                                Net_TCPSocketConfiguration_t,
                                FTP_Stream_t,
                                enum Stream_StateMachine_ControlState> FTP_IStreamConnection_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                FTP_ConnectionConfiguration_t,
                                struct FTP_ConnectionState,
                                struct Net_StreamStatistic,
                                FTP_Stream_t,
                                struct Net_UserData> FTP_Connection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                FTP_ConnectionConfiguration_t,
                                struct FTP_ConnectionState,
                                struct Net_StreamStatistic,
                                FTP_Stream_t,
                                struct Net_UserData> FTP_SSLConnection_t;
#endif // SSL_SUPPORT

typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      FTP_ConnectionConfiguration_t,
                                      struct FTP_ConnectionState,
                                      struct Net_StreamStatistic,
                                      FTP_Stream_t,
                                      struct Net_UserData> FTP_AsynchConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         FTP_ConnectionConfiguration_t> FTP_IConnector_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               FTP_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               FTP_ConnectionConfiguration_t,
                               struct FTP_ConnectionState,
                               struct Net_StreamStatistic,
                               Net_TCPSocketConfiguration_t,
                               FTP_Stream_t,
                               struct Net_UserData> FTP_Connector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<FTP_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   FTP_ConnectionConfiguration_t,
                                   struct FTP_ConnectionState,
                                   struct Net_StreamStatistic,
                                   FTP_Stream_t,
                                   struct Net_UserData> FTP_SSLConnector_t;
#endif // SSL_SUPPORT

typedef Net_Client_AsynchConnector_T<FTP_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     FTP_ConnectionConfiguration_t,
                                     struct FTP_ConnectionState,
                                     struct Net_StreamStatistic,
                                     Net_TCPSocketConfiguration_t,
                                     FTP_Stream_t,
                                     struct Net_UserData> FTP_AsynchConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 FTP_ConnectionConfiguration_t,
                                 struct FTP_ConnectionState,
                                 struct Net_StreamStatistic,
                                 struct Net_UserData> FTP_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FTP_ConnectionConfiguration_t,
                                 struct FTP_ConnectionState,
                                 struct Net_StreamStatistic,
                                 struct Net_UserData> FTP_Connection_Manager_t;

typedef ACE_Singleton<FTP_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> FTP_CONNECTIONMANAGER_SINGLETON;

#endif
