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

#ifndef TEST_I_AVSTREAM_SERVER_LISTENER_COMMON_H
#define TEST_I_AVSTREAM_SERVER_LISTENER_COMMON_H

#include "ace/config-lite.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_server_asynchlistener.h"
#include "net_server_listener.h"
#include "net_sock_acceptor.h"
#if defined (SSL_SUPPORT)
#include "net_server_ssl_listener.h"
#endif // SSL_SUPPORT

#include "test_i_network.h"
#include "test_i_av_stream_server_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_ListenerConfiguration;
class Test_I_AVStream_Server_MediaFoundation_ListenerConfiguration;
#else
class Test_I_AVStream_Server_ListenerConfiguration;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Server_AsynchListener_T<Test_I_AVStream_Server_DirectShow_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                    struct Net_StreamConnectionState,
                                    Test_I_AVStream_Server_DirectShow_TCPStream,
                                    struct Net_UserData> Test_I_AVStream_Server_DirectShow_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_AVStream_Server_DirectShow_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                              struct Net_StreamConnectionState,
                              Test_I_AVStream_Server_DirectShow_TCPStream,
                              struct Net_UserData> Test_I_AVStream_Server_DirectShow_Listener_t;
#if defined (SSL_SUPPORT)
typedef Net_Server_SSL_Listener_T<Test_I_AVStream_Server_DirectShow_SSLTCPConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                  struct Net_StreamConnectionState,
                                  Test_I_AVStream_Server_DirectShow_TCPStream,
                                  struct Net_UserData> Test_I_AVStream_Server_DirectShow_SSLListener_t;
#endif // SSL_SUPPORT

typedef Net_Server_AsynchListener_T<Test_I_AVStream_Server_MediaFoundation_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                    struct Net_StreamConnectionState,
                                    Test_I_AVStream_Server_MediaFoundation_TCPStream,
                                    struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_AVStream_Server_MediaFoundation_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                              struct Net_StreamConnectionState,
                              Test_I_AVStream_Server_MediaFoundation_TCPStream,
                              struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_Listener_t;
#if defined (SSL_SUPPORT)
typedef Net_Server_SSL_Listener_T<Test_I_AVStream_Server_MediaFoundation_SSLTCPConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                  struct Net_StreamConnectionState,
                                  Test_I_AVStream_Server_MediaFoundation_TCPStream,
                                  struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_SSLListener_t;
#endif // SSL_SUPPORT
#else
typedef Net_Server_AsynchListener_T<Test_I_AVStream_Server_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                    struct Net_StreamConnectionState,
                                    Test_I_AVStream_Server_TCPStream,
                                    struct Net_UserData> Test_I_AVStream_Server_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_AVStream_Server_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                              struct Net_StreamConnectionState,
                              Test_I_AVStream_Server_TCPStream,
                              struct Net_UserData> Test_I_AVStream_Server_Listener_t;
#if defined (SSL_SUPPORT)
typedef Net_Server_SSL_Listener_T<Test_I_AVStream_Server_SSLConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                  struct Net_StreamConnectionState,
                                  Test_I_AVStream_Server_TCPStream,
                                  struct Net_UserData> Test_I_AVStream_Server_SSLListener_t;
#endif // SSL_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Server_DirectShow_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Server_DirectShow_UDPStream,
                                     struct Net_UserData> Test_I_AVStream_Server_DirectShow_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Server_DirectShow_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Server_DirectShow_UDPStream,
                               struct Net_UserData> Test_I_AVStream_Server_DirectShow_UDPConnector_t;

typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Server_MediaFoundation_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Server_MediaFoundation_UDPStream,
                                     struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Server_MediaFoundation_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Server_MediaFoundation_UDPStream,
                               struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_UDPConnector_t;
#else
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Server_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Server_UDPStream,
                                     struct Net_UserData> Test_I_AVStream_Server_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Server_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Server_UDPStream,
                               struct Net_UserData> Test_I_AVStream_Server_UDPConnector_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef ACE_Singleton<Test_I_AVStream_Server_DirectShow_AsynchListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_DIRECTSHOW_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_AVStream_Server_DirectShow_Listener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_DIRECTSHOW_LISTENER_SINGLETON;
#if defined (SSL_SUPPORT)
typedef ACE_Singleton<Test_I_AVStream_Server_DirectShow_SSLListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_DIRECTSHOW_SSL_LISTENER_SINGLETON;
#endif // SSL_SUPPORT
typedef ACE_Singleton<Test_I_AVStream_Server_MediaFoundation_AsynchListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_AVStream_Server_MediaFoundation_Listener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_LISTENER_SINGLETON;
#if defined (SSL_SUPPORT)
typedef ACE_Singleton<Test_I_AVStream_Server_MediaFoundation_SSLListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_SSL_LISTENER_SINGLETON;
#endif // SSL_SUPPORT
#else
typedef ACE_Singleton<Test_I_AVStream_Server_AsynchListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_AVStream_Server_Listener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_LISTENER_SINGLETON;
#if defined (SSL_SUPPORT)
typedef ACE_Singleton<Test_I_AVStream_Server_SSLListener_t,
                      ACE_SYNCH_MUTEX> TEST_I_AVSTREAM_SERVER_SSL_LISTENER_SINGLETON;
#endif // SSL_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
