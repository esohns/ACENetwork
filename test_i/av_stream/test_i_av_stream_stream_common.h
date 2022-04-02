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

#ifndef TEST_I_AVSTREAM_STREAM_COMMON_H
#define TEST_I_AVSTREAM_STREAM_COMMON_H

#include "test_i_connection_common.h"

#include "test_i_connection_manager_common.h"

#include "test_i_av_stream_client_common.h"
#include "test_i_av_stream_client_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_AVStream_Client_DirectShow_Stream_T<Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                                   Test_I_AVStream_Client_DirectShow_TCPConnector_t> Test_I_AVStream_Client_DirectShow_TCPStream_t;
#if defined (SSL_SUPPORT)
typedef Test_I_AVStream_Client_DirectShow_Stream_T<Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                                   Test_I_AVStream_Client_DirectShow_SSLConnector_t> Test_I_AVStream_Client_DirectShow_SSLStream_t;
#endif // SSL_SUPPORT
typedef Test_I_AVStream_Client_DirectShow_Stream_T<Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                                   Test_I_AVStream_Client_DirectShow_TCPAsynchConnector_t> Test_I_AVStream_Client_DirectShow_AsynchTCPStream_t;

typedef Test_I_AVStream_Client_DirectShow_Stream_T<Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t,
                                                   Test_I_AVStream_Client_DirectShow_UDPConnector_t> Test_I_AVStream_Client_DirectShow_UDPStream_t;
typedef Test_I_AVStream_Client_DirectShow_Stream_T<Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t,
                                                   Test_I_AVStream_Client_DirectShow_UDPAsynchConnector_t> Test_I_AVStream_Client_DirectShow_AsynchUDPStream_t;

typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                                        Test_I_AVStream_Client_MediaFoundation_TCPConnector_t> Test_I_AVStream_Client_MediaFoundation_TCPStream_t;
#if defined (SSL_SUPPORT)
typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                                        Test_I_AVStream_Client_MediaFoundation_SSLConnector_t> Test_I_AVStream_Client_MediaFoundation_SSLTCPStream_t;
#endif // SSL_SUPPORT
typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                                        Test_I_AVStream_Client_MediaFoundation_TCPAsynchConnector_t> Test_I_AVStream_Client_MediaFoundation_AsynchTCPStream_t;

typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t,
                                                        Test_I_AVStream_Client_MediaFoundation_UDPConnector_t> Test_I_AVStream_Client_MediaFoundation_UDPStream_t;
typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t,
                                                        Test_I_AVStream_Client_MediaFoundation_UDPAsynchConnector_t> Test_I_AVStream_Client_MediaFoundation_AsynchUDPStream_t;
#else
typedef Test_I_AVStream_Client_V4L_Stream_T<struct Test_I_AVStream_Client_V4L_StreamState,
                                            struct Test_I_AVStream_Client_V4L_StreamConfiguration,
                                            Common_Timer_Manager_t,
                                            struct Test_I_AVStream_Client_V4L_ModuleHandlerConfiguration,
                                            Test_I_AVStream_Client_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_V4L_StreamSessionData_t,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_V4L_Message,
                                            Test_I_AVStream_Client_V4L_SessionMessage,
                                            Test_I_AVStream_Client_V4L_TCPConnectionManager_t,
                                            Test_I_AVStream_Client_V4L_TCPConnector_t> Test_I_AVStream_Client_V4L_TCPStream_t;
typedef Test_I_AVStream_Client_V4L_Stream_T<struct Test_I_AVStream_Client_V4L_StreamState,
                                            struct Test_I_AVStream_Client_V4L_StreamConfiguration,
                                            Common_Timer_Manager_t,
                                            struct Test_I_AVStream_Client_V4L_ModuleHandlerConfiguration,
                                            Test_I_AVStream_Client_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_V4L_StreamSessionData_t,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_V4L_Message,
                                            Test_I_AVStream_Client_V4L_SessionMessage,
                                            Test_I_AVStream_Client_V4L_InetConnectionManager_t,
                                            Test_I_AVStream_Client_V4L_SSLTCPConnector_t> Test_I_AVStream_Client_V4L_SSLTCPStream_t;
typedef Test_I_AVStream_Client_V4L_Stream_T<struct Test_I_AVStream_Client_V4L_StreamState,
                                            struct Test_I_AVStream_Client_V4L_StreamConfiguration,
                                            Common_Timer_Manager_t,
                                            struct Test_I_AVStream_Client_V4L_ModuleHandlerConfiguration,
                                            Test_I_AVStream_Client_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_V4L_StreamSessionData_t,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_V4L_Message,
                                            Test_I_AVStream_Client_V4L_SessionMessage,
                                            Test_I_AVStream_Client_V4L_TCPConnectionManager_t,
                                            Test_I_AVStream_Client_V4L_TCPAsynchConnector_t> Test_I_AVStream_Client_V4L_AsynchTCPStream_t;
typedef Test_I_AVStream_Client_V4L_Stream_T<struct Test_I_AVStream_Client_V4L_StreamState,
                                            struct Test_I_AVStream_Client_V4L_StreamConfiguration,
                                            Common_Timer_Manager_t,
                                            struct Test_I_AVStream_Client_V4L_ModuleHandlerConfiguration,
                                            Test_I_AVStream_Client_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_V4L_StreamSessionData_t,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_V4L_Message,
                                            Test_I_AVStream_Client_V4L_SessionMessage,
                                            Test_I_AVStream_Client_V4L_UDPConnectionManager_t,
                                            Test_I_AVStream_Client_V4L_UDPConnector_t> Test_I_AVStream_Client_V4L_UDPStream_t;
typedef Test_I_AVStream_Client_V4L_Stream_T<struct Test_I_AVStream_Client_V4L_StreamState,
                                            struct Test_I_AVStream_Client_V4L_StreamConfiguration,
                                            Common_Timer_Manager_t,
                                            struct Test_I_AVStream_Client_V4L_ModuleHandlerConfiguration,
                                            Test_I_AVStream_Client_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_V4L_StreamSessionData_t,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_V4L_Message,
                                            Test_I_AVStream_Client_V4L_SessionMessage,
                                            Test_I_AVStream_Client_V4L_UDPConnectionManager_t,
                                            Test_I_AVStream_Client_V4L_UDPAsynchConnector_t> Test_I_AVStream_Client_V4L_AsynchUDPStream_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
