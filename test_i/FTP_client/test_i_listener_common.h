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

#ifndef TEST_I_LISTENER_COMMON_H
#define TEST_I_LISTENER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Acceptor.h"
#endif // SSL_SUPPORT

#include "net_common.h"

#include "net_client_asynchconnector.h"
#include "net_server_asynchlistener.h"
#include "net_server_common.h"
#include "net_client_connector.h"
#include "net_server_listener.h"

#include "test_i_connection_common.h"
#include "test_i_stream_common.h"

typedef Net_IListener_T<FTP_Client_ConnectionConfiguration> FTP_Client_IListener_t;

typedef Net_Server_AsynchListener_T<FTP_Client_AsynchConnection_t,
                                    ACE_INET_Addr,
                                    FTP_Client_ConnectionConfiguration,
                                    struct Net_StreamConnectionState,
                                    FTP_Client_AsynchConnection_t::STREAM_T,
                                    struct Net_UserData> FTP_Client_AsynchListener_t;
typedef Net_Server_Listener_T<FTP_Client_Connection_t,
                              ACE_SOCK_ACCEPTOR,
                              ACE_INET_Addr,
                              FTP_Client_ConnectionConfiguration,
                              struct Net_StreamConnectionState,
                              FTP_Client_Connection_t::STREAM_T,
                              struct Net_UserData> FTP_Client_Listener_t;
#if defined (SSL_SUPPORT)
typedef Net_Server_Listener_T<FTP_Client_SSLConnection_t,
                              ACE_SSL_SOCK_Acceptor,
                              ACE_INET_Addr,
                              FTP_Client_ConnectionConfiguration,
                              struct Net_StreamConnectionState,
                              FTP_Client_SSLConnection_t::STREAM_T,
                              struct Net_UserData> FTP_Client_SSLListener_t;
#endif // SSL_SUPPORT

typedef ACE_Singleton<FTP_Client_AsynchListener_t,
                      ACE_SYNCH_MUTEX> FTP_CLIENT_ASYNCH_LISTENER_SINGLETON;
typedef ACE_Singleton<FTP_Client_Listener_t,
                      ACE_SYNCH_MUTEX> FTP_CLIENT_LISTENER_SINGLETON;
#if defined (SSL_SUPPORT)
typedef ACE_Singleton<FTP_Client_SSLListener_t,
                      ACE_SYNCH_MUTEX> FTP_CLIENT_SSL_LISTENER_SINGLETON;
#endif // SSL_SUPPORT

#endif
