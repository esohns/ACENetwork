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

#ifndef TEST_I_STREAM_H
#define TEST_I_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "stream_net_source.h"

#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "http_module_parser.h"

#include "test_i_common.h"
#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_module_htmlparser.h"
#include "test_i_module_httpget.h"
#include "test_i_module_spreadsheetwriter.h"
#include "test_i_session_message.h"

#include "test_i_trending_common.h"
#include "test_i_trending_connection_common.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

template <typename ConnectorType>
class Test_I_Trending_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_Trending_StreamState,
                        struct Test_I_Trending_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_Trending_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_Trending_StreamState,
                        struct Test_I_Trending_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_Trending_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage> inherited;

 public:
  Test_I_Trending_Stream_T ();
  inline virtual ~Test_I_Trending_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Trending_StreamConfiguration_t&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Module_Net_Source_Writer_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_Trending_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_Stream_Message,
                                            Test_I_Stream_SessionMessage,
                                            ConnectorType> SOURCE_WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                                     // task synch type
                                         Common_TimePolicy_t,                              // time policy
                                         struct Test_I_Trending_SessionData,                // session data type
                                         enum Stream_SessionMessageType,                   // session event type
                                         struct Stream_ModuleConfiguration,                // module configuration type
                                         struct Test_I_Trending_ModuleHandlerConfiguration, // module handler configuration type
                                         libacestream_default_net_source_module_name_string,
                                         Stream_INotify_t,                                 // stream notification interface type
                                         SOURCE_WRITER_T> SOURCE_MODULE_T;                 // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_Trending_Stream_T (const Test_I_Trending_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Trending_Stream_T& operator= (const Test_I_Trending_Stream_T&))
};

// include template definition
#include "test_i_stream.inl"

//////////////////////////////////////////

typedef Test_I_Trending_Stream_T<Test_I_Trending_Connector_t> Test_I_Trending_Stream_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_Trending_ConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_NetStream_t,
                                struct Net_UserData> Test_I_SSLConnection_t;
typedef Net_Client_SSL_Connector_T<Test_I_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_Trending_ConnectionConfiguration_t,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_I_NetStream_t,
                                   struct Net_UserData> Test_I_Trending_SSLConnector_t;
typedef Test_I_Trending_Stream_T<Test_I_Trending_SSLConnector_t> Test_I_Trending_SSL_Stream_t;
#endif // SSL_SUPPORT
typedef Test_I_Trending_Stream_T<Test_I_Trending_AsynchConnector_t> Test_I_Trending_AsynchStream_t;

#endif
