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

#ifndef TEST_U_HTTP_DECODER_STREAM_COMMON_H
#define TEST_U_HTTP_DECODER_STREAM_COMMON_H

#include <string>

#include "ace/Synch_Traits.h"

#include "common_configuration.h"

#include "common_timer_manager_common.h"

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

#include "test_u_stream_common.h"

#include "test_u_connection_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;
//template <typename TimerManagerType> // implements Common_ITimer
//class Test_U_Stream_T;

typedef Stream_DataBase_T<struct HTTP_Record> Test_U_MessageData_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_HTTPDecoder_MessageAllocator_t;

struct Test_U_HTTPDecoder_SessionData
 : Test_U_StreamSessionData
{
  Test_U_HTTPDecoder_SessionData ()
   : Test_U_StreamSessionData ()
   , connectionState (NULL)
   , format (STREAM_COMPRESSION_FORMAT_NONE)
   , targetFileName ()
   //, userData (NULL)
  {};
  
  struct Test_U_HTTPDecoder_SessionData& operator= (const struct Test_U_HTTPDecoder_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    connectionState =
        (connectionState ? connectionState : rhs_in.connectionState);
    format = rhs_in.format;
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  };

  struct HTTP_ConnectionState*              connectionState;
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  std::string                               targetFileName; // file writer module

  //struct Net_UserData*                      userData;
};
typedef Stream_SessionData_T<struct Test_U_HTTPDecoder_SessionData> Test_U_HTTPDecoder_SessionData_t;

struct Test_U_Configuration;
//extern const char stream_name_string_[];
struct Test_U_HTTPDecoder_StreamConfiguration;
struct Test_U_HTTPDecoder_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_HTTPDecoder_StreamConfiguration,
                               struct Test_U_HTTPDecoder_ModuleHandlerConfiguration> Test_U_HTTPDecoder_StreamConfiguration_t;
class Test_U_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<Test_U_HTTPDecoder_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  Test_U_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Net_StreamConnectionState,
                          HTTP_Statistic_t> Test_U_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_U_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_U_IConnectionManager_t;
struct Test_U_HTTPDecoder_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  Test_U_HTTPDecoder_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , dumpFileName ()
   , hostName ()
   , printProgressDot (true)
   , streamConfiguration (NULL)
   , targetFileName ()
   , URL ()
  {
    inbound = true;
  }

  struct Test_U_Configuration*              configuration;
  Test_U_IConnection_t*                     connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*           connectionConfigurations;
  Test_U_IConnectionManager_t*              connectionManager; // TCP IO module
  std::string                               dumpFileName; // file writer module (HTTP)
  std::string                               hostName; // net source module
  bool                                      printProgressDot; // file writer module
  Test_U_HTTPDecoder_StreamConfiguration_t* streamConfiguration;
  std::string                               targetFileName; // file writer module (HTML)
  std::string                               URL;
};

struct Test_U_HTTPDecoder_StreamConfiguration
 : HTTP_StreamConfiguration
{
  Test_U_HTTPDecoder_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
   //, userData (NULL)
  {}

  //struct Net_UserData* userData;
};

struct Test_U_HTTPDecoder_StreamState
 : Test_U_StreamState
{
  Test_U_HTTPDecoder_StreamState ()
   : Test_U_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_U_HTTPDecoder_SessionData* sessionData;
};

#endif
