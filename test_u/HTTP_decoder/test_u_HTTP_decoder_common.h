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

#ifndef TEST_U_HTTP_DECODER_COMMON_H
#define TEST_U_HTTP_DECODER_COMMON_H

#include <ace/Synch_Traits.h>

#include "common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dec_common.h"

#include "net_connection_manager.h"
#include "net_defines.h"
#include "net_iconnection.h"

#include "http_common.h"
#include "http_configuration.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"

//#include "test_u_connection_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;

struct Test_U_StreamConfiguration;
struct Test_U_HTTPDecoder_UserData
 : Test_U_UserData
{
  inline Test_U_HTTPDecoder_UserData ()
   : Test_U_UserData ()
   , streamConfiguration (NULL)
  {};

  struct Test_U_StreamConfiguration* streamConfiguration;
};

typedef Stream_DataBase_T<struct HTTP_Record> Test_U_MessageData_t;

struct Test_U_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_U_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_U_AllocatorConfiguration> Test_U_ControlMessage_t;

//typedef Stream_IModuleHandler_T<Test_U_ModuleHandlerConfiguration> Test_U_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_U_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

struct Test_U_HTTPDecoder_SessionData
 : Test_U_StreamSessionData
{
  inline Test_U_HTTPDecoder_SessionData ()
   : Test_U_StreamSessionData ()
   , connectionState (NULL)
   , format (STREAM_COMPRESSION_FORMAT_NONE)
   , targetFileName ()
   , userData (NULL)
  {};
  inline Test_U_HTTPDecoder_SessionData& operator= (Test_U_HTTPDecoder_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    connectionState =
        (connectionState ? connectionState : rhs_in.connectionState);
    format = rhs_in.format;
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct HTTP_ConnectionState*              connectionState;
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  std::string                               targetFileName; // file writer module
  struct Test_U_UserData*                   userData;
};
typedef Stream_SessionData_T<struct Test_U_HTTPDecoder_SessionData> Test_U_HTTPDecoder_SessionData_t;

struct Test_U_Configuration;
struct Test_U_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_U_ConnectionConfiguration,
                          struct Test_U_ConnectionState,
                          HTTP_RuntimeStatistic_t> Test_U_IConnection_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 HTTP_RuntimeStatistic_t,
                                 struct Test_U_UserData> Test_U_ConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 HTTP_RuntimeStatistic_t,
                                 struct Test_U_UserData> Test_U_IConnectionManager_t;
struct Test_U_SocketHandlerConfiguration;
struct Test_U_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  inline Test_U_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , dumpFileName ()
   , hostName ()
   , inbound (true)
   , printProgressDot (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , targetFileName ()
   , URL ()
  {};

  struct Test_U_Configuration*              configuration;
  Test_U_IConnection_t*                     connection; // TCP target/IO module
  Test_U_IConnectionManager_t*              connectionManager; // TCP IO module
  std::string                               dumpFileName; // file writer module (HTTP)
  std::string                               hostName; // net source module
  bool                                      inbound; // net io module
  bool                                      printProgressDot; // file writer module
  struct Net_SocketConfiguration*           socketConfiguration;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  std::string                               targetFileName; // file writer module (HTML)
  std::string                               URL;
};

struct Test_U_StreamConfiguration
 : HTTP_StreamConfiguration
{
  inline Test_U_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ModuleHandlerConfiguration* moduleHandlerConfiguration; // stream module handler configuration

  struct Test_U_UserData*                   userData;
};

struct Test_U_HTTPDecoder_StreamState
 : Test_U_StreamState
{
  inline Test_U_HTTPDecoder_StreamState ()
   : Test_U_StreamState ()
   , currentSessionData (NULL)
  {};

  struct Test_U_HTTPDecoder_SessionData* currentSessionData;
};

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_U_SocketHandlerConfiguration;
struct Test_U_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline Test_U_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_U_StreamConfiguration*        streamConfiguration;

  struct Test_U_UserData*                   userData;
};

struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , parserConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Test_U_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Net_SocketConfiguration           socketConfiguration;
  struct Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_U_ConnectionConfiguration    connectionConfiguration;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration        parserConfiguration;
  // **************************** stream data **********************************
  struct Stream_ModuleConfiguration        moduleConfiguration;
  struct Test_U_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_U_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  bool                                     useReactor;

  struct Test_U_UserData                   userData;
};

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_U_IStreamNotify_t;

#endif
