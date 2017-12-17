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

#include <map>
#include <string>

#include "ace/Synch_Traits.h"

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

#include "test_u_HTTP_decoder_stream_common.h"

//struct Test_U_StreamConfiguration;
//struct Test_U_HTTPDecoder_UserData
// : Test_U_UserData
//{
//  Test_U_HTTPDecoder_UserData ()
//   : Test_U_UserData ()
//  {};
//};

struct Test_U_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  Test_U_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    paddingBytes = NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Test_U_HTTPDecoder_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_HTTPDecoder_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_U_ConnectionConfiguration;
typedef std::map<std::string,
                 struct Test_U_ConnectionConfiguration> Test_U_ConnectionConfigurations_t;
//extern const char stream_name_string_[];
struct Test_U_AllocatorConfiguration;
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_AllocatorConfiguration,
                               struct Test_U_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct Test_U_HTTPDecoder_Configuration
 : Test_U_Configuration
{
  Test_U_HTTPDecoder_Configuration ()
   : Test_U_Configuration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** socket data **********************************
  Test_U_ConnectionConfigurations_t                    connectionConfigurations;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration                    parserConfiguration;
  // **************************** signal data **********************************
  struct Test_U_HTTPDecoder_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Test_U_StreamConfiguration_t                         streamConfiguration;
  // *************************** protocol data *********************************
  bool                                                 useReactor;
};

#endif
