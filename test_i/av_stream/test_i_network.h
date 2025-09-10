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

#ifndef TEST_I_CAMSTREAM_NETWORK_H
#define TEST_I_CAMSTREAM_NETWORK_H

#include <map>
#include <string>

#include "ace/config-lite.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"
#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Stream.h"
#endif // SSL_SUPPORT

#include "common_statistic_handler.h"

#include "common_parser_common.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_control_message.h"
#include "stream_session_data.h"
#include "stream_session_manager.h"

#include "stream_net_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_udpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "test_i_connection_common.h"

#include "test_i_av_stream_defines.h"

// forward declarations
extern const char stream_name_string_3[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_ConnectionConfiguration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_DirectShow_StreamSessionData;
struct Test_I_AVStream_Client_DirectShow_StreamConfiguration;
class Test_I_AVStream_Client_DirectShow_Message;
class Test_I_AVStream_Client_DirectShow_SessionMessage;
struct Test_I_AVStream_Client_DirectShow_StreamState;
struct Net_UserData;

struct Test_I_AVStream_Server_DirectShow_ConnectionConfiguration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration;
class Test_I_AVStream_Server_DirectShow_StreamSessionData;
class Test_I_AVStream_Server_DirectShow_TCPStream;
class Test_I_AVStream_Server_DirectShow_UDPStream;
class Test_I_AVStream_Server_DirectShow_Message;
class Test_I_AVStream_Server_DirectShow_SessionMessage;
struct Test_I_AVStream_Server_DirectShow_StreamConfiguration;
struct Test_I_AVStream_Server_DirectShow_StreamState;

typedef Stream_SessionData_T<Test_I_AVStream_Client_DirectShow_StreamSessionData> Test_I_AVStream_Client_DirectShow_StreamSessionData_t;

typedef Stream_SessionData_T<Test_I_AVStream_Server_DirectShow_StreamSessionData> Test_I_AVStream_Server_DirectShow_StreamSessionData_t;

struct Test_I_AVStream_Client_MediaFoundation_Configuration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_MediaFoundation_StreamSessionData;
class Test_I_AVStream_Client_MediaFoundation_Message;
class Test_I_AVStream_Client_MediaFoundation_SessionMessage;
struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration;
struct Test_I_AVStream_Client_MediaFoundation_StreamState;
struct Net_UserData;

struct Test_I_AVStream_Server_MediaFoundation_Configuration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration;
class Test_I_AVStream_Server_MediaFoundation_StreamSessionData;
class Test_I_AVStream_Server_MediaFoundation_TCPStream;
class Test_I_AVStream_Server_MediaFoundation_UDPStream;
class Test_I_AVStream_Server_MediaFoundation_Message;
class Test_I_AVStream_Server_MediaFoundation_SessionMessage;
struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration;
struct Test_I_AVStream_Server_MediaFoundation_StreamState;

typedef Stream_SessionData_T<Test_I_AVStream_Client_MediaFoundation_StreamSessionData> Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t;

typedef Stream_SessionData_T<Test_I_AVStream_Server_MediaFoundation_StreamSessionData> Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t;
#else
struct Test_I_AVStream_Client_ALSA_V4L_ConnectionConfiguration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_ALSA_V4L_StreamSessionData;
struct Test_I_AVStream_Client_ALSA_V4L_SocketHandlerConfiguration;
class Test_I_AVStream_Client_ALSA_V4L_Message;
class Test_I_AVStream_Client_ALSA_V4L_SessionMessage;
struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration;
struct Test_I_AVStream_Client_ALSA_V4L_StreamState;

typedef Stream_SessionData_T<Test_I_AVStream_Client_ALSA_V4L_StreamSessionData> Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Server_DirectShow_ConnectionConfiguration;
struct Test_I_AVStream_Server_MediaFoundation_ConnectionConfiguration;
struct Net_StreamConnectionState;
struct Net_StreamConnectionState;
#else
struct Test_I_AVStream_Server_ConnectionConfiguration;
struct Net_StreamConnectionState;
struct Test_I_AVStream_Server_ModuleHandlerConfiguration;
class Test_I_AVStream_Server_StreamSessionData;
struct Test_I_AVStream_Server_SocketHandlerConfiguration;
class Test_I_AVStream_Server_TCPStream;
class Test_I_AVStream_Server_UDPStream;
class Test_I_AVStream_Server_Stream_Message;
class Test_I_AVStream_Server_SessionMessage;
struct Test_I_AVStream_Server_StreamConfiguration;
struct Test_I_AVStream_Server_StreamState;

typedef Stream_SessionData_T<Test_I_AVStream_Server_StreamSessionData> Test_I_AVStream_Server_StreamSessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_StatisticHandler_T<Net_StreamStatistic_t> Test_I_AVStream_Client_StatisticHandler_t;

//extern const char stream_name_string_[];
struct Test_I_AVStream_Client_DirectShow_StreamConfiguration;
struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                               struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration> Test_I_AVStream_Client_DirectShow_StreamConfiguration_t;

struct Test_I_AVStream_Client_DirectShow_ConnectionConfiguration;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_DirectShow_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_DirectShow_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t;

struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration;
struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                               struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration> Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t;
#else
//extern const char stream_name_string_[];
struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration;
struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                               struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration> Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t;
struct Test_I_AVStream_Client_ALSA_V4L_ConnectionConfiguration;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_IUDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_UDPConnectionManager_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Server_DirectShow_StreamConfiguration;
struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Server_DirectShow_StreamConfiguration,
                               struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration> Test_I_AVStream_Server_DirectShow_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_DirectShow_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_DirectShow_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_DirectShow_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_DirectShow_UDPConnectionManager_t;

struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration;
struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration,
                               struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration> Test_I_AVStream_Server_MediaFoundation_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_MediaFoundation_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_MediaFoundation_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_UDPConnectionManager_t;
#else
struct Test_I_AVStream_Server_StreamConfiguration;
struct Test_I_AVStream_Server_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Server_StreamConfiguration,
                               struct Test_I_AVStream_Server_ModuleHandlerConfiguration> Test_I_AVStream_Server_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_AVStream_Server_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_AVStream_Server_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_AVStream_Server_UDPConnectionConfiguration_t;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_IUDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_UDPConnectionManager_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

extern const char stream_name_string_2[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_DirectShow_StreamState,
                                      struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_DirectShow_Message,
                                      Test_I_AVStream_Client_DirectShow_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Net_TCPStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_DirectShow_StreamState,
                                      struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_DirectShow_Message,
                                      Test_I_AVStream_Client_DirectShow_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Net_UDPStream_t;

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                      struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_MediaFoundation_Message,
                                      Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                      struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_MediaFoundation_Message,
                                      Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Net_UDPStream_t;
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_Client_ALSA_V4L_SessionManager_t;

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                      struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                      Test_I_Client_ALSA_V4L_SessionManager_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_3,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                      struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                      Test_I_Client_ALSA_V4L_SessionManager_t,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_AVStream_Client_ALSA_V4L_UDPConnectionManager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_Net_UDPStream_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_DirectShow_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_DirectShow_IUDPConnection_t;
//
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_DirectShow_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_DirectShow_IUDPConnection_t;
//
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_MediaFoundation_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_MediaFoundation_IUDPConnection_t;
//
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_MediaFoundation_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_MediaFoundation_IUDPConnection_t;
#else
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_ALSA_V4L_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Client_ALSA_V4L_IUDPConnection_t;
//
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_AVStream_Server_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_AVStream_Server_IUDPConnection_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// outbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_DirectShow_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_DirectShow_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
	                                  Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_DirectShow_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_DirectShow_Net_UDPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_DirectShow_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_DirectShow_Net_UDPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_DirectShow_AsynchUDPConnection_t;

typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_MediaFoundation_Net_UDPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_MediaFoundation_Net_UDPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_AsynchUDPConnection_t;
#else
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Client_ALSA_V4L_Net_UDPStream_t,
                                struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Client_ALSA_V4L_Net_UDPStream_t,
                                      struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_AsynchUDPConnection_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t> Test_I_AVStream_Client_DirectShow_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t> Test_I_AVStream_Client_DirectShow_IUDPConnector_t;

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t> Test_I_AVStream_Client_MediaFoundation_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t> Test_I_AVStream_Client_MediaFoundation_IUDPConnector_t;
#else
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t> Test_I_AVStream_Client_ALSA_V4L_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t> Test_I_AVStream_Client_ALSA_V4L_IUDPConnector_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// outbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_DirectShow_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_DirectShow_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_AVStream_Client_DirectShow_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                                   struct Net_UserData> Test_I_AVStream_Client_DirectShow_SSLConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_DirectShow_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_AVStream_Client_DirectShow_Net_TCPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_DirectShow_TCPAsynchConnector_t;

typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_DirectShow_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Client_DirectShow_Net_UDPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_DirectShow_UDPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_DirectShow_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Client_DirectShow_Net_UDPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_DirectShow_UDPAsynchConnector_t;

typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_MediaFoundation_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_AVStream_Client_MediaFoundation_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                                   struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_SSLConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_MediaFoundation_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_AVStream_Client_MediaFoundation_Net_TCPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_TCPAsynchConnector_t;

typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_MediaFoundation_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Client_MediaFoundation_Net_UDPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_UDPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_MediaFoundation_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Client_MediaFoundation_Net_UDPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_MediaFoundation_UDPAsynchConnector_t;
#else
typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_ALSA_V4L_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_AVStream_Client_ALSA_V4L_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                                   struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_SSLConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_ALSA_V4L_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Net_TCPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_TCPAsynchConnector_t;

typedef Net_Client_Connector_T<ACE_NULL_SYNCH,
                               Test_I_AVStream_Client_ALSA_V4L_UDPConnection_t,
                               Net_SOCK_Dgram,
                               ACE_INET_Addr,
                               Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_AVStream_Client_ALSA_V4L_Net_UDPStream_t,
                               struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_UDPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_AVStream_Client_ALSA_V4L_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Net_UDPStream_t,
                                     struct Net_UserData> Test_I_AVStream_Client_ALSA_V4L_UDPAsynchConnector_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// inbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_DirectShow_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_DirectShow_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_DirectShow_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_DirectShow_SSLTCPConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_DirectShow_TCPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_DirectShow_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_DirectShow_UDPStream,
                                struct Net_UserData> Test_I_AVStream_Server_DirectShow_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_DirectShow_UDPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_DirectShow_AsynchUDPConnection_t;

typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_MediaFoundation_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_MediaFoundation_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_SSLTCPConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_MediaFoundation_TCPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_MediaFoundation_UDPStream,
                                struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_MediaFoundation_UDPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_AsynchUDPConnection_t;
#else
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_TCPStream,
                                struct Net_UserData> Test_I_AVStream_Server_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_TCPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_AVStream_Server_UDPStream,
                                struct Net_UserData> Test_I_AVStream_Server_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_AVStream_Server_UDPStream,
                                      struct Net_UserData> Test_I_AVStream_Server_AsynchUDPConnection_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t> Test_I_AVStream_Server_DirectShow_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t> Test_I_AVStream_Server_DirectShow_IUDPConnector_t;

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t> Test_I_AVStream_Server_MediaFoundation_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t> Test_I_AVStream_Server_MediaFoundation_IUDPConnector_t;
#else
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_TCPConnectionConfiguration_t> Test_I_AVStream_Server_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_AVStream_Server_UDPConnectionConfiguration_t> Test_I_AVStream_Server_IUDPConnector_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
