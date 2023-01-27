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

#include "common_configuration.h"

#include "stream_configuration.h"

#include "net_defines.h"

#include "test_u_common.h"

#include "test_u_connection_common.h"
#include "test_u_HTTP_decoder_stream_common.h"

//struct Test_U_StreamConfiguration;
//struct Test_U_HTTPDecoder_UserData
// : Test_U_UserData
//{
//  Test_U_HTTPDecoder_UserData ()
//   : Test_U_UserData ()
//  {};
//};

struct Test_U_HTTPDecoder_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_HTTPDecoder_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingInterval (0)
  {};

  unsigned int statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

//extern const char stream_name_string_[];
struct Test_U_HTTPDecoder_StreamConfiguration;
struct Test_U_HTTPDecoder_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_HTTPDecoder_StreamConfiguration,
                               struct Test_U_HTTPDecoder_ModuleHandlerConfiguration> Test_U_HTTPDecoder_StreamConfiguration_t;
struct Test_U_HTTPDecoder_Configuration
 : Test_U_Configuration
{
  Test_U_HTTPDecoder_Configuration ()
   : Test_U_Configuration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
  {};

  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                       connectionConfigurations;
  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                      parserConfiguration;
  // **************************** signal data **********************************
  struct Test_U_HTTPDecoder_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Test_U_HTTPDecoder_StreamConfiguration_t             streamConfiguration;
  // *************************** protocol data *********************************

  enum Common_EventDispatchType                        dispatch;
};

#endif
