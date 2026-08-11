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

#ifndef TEST_I_MODULE_HTTPPARSER_H
#define TEST_I_MODULE_HTTPPARSER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "http_module_parser.h"
#include "http_module_streamer.h"
#include "http_parser_driver.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage> Test_I_HTTPStreamer;

typedef HTTP_ParserDriver_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            Test_I_SessionMessage> HTTP_ParserDriver_t;

class Test_I_HTTPParser
 : public HTTP_Module_Parser_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage,
                               HTTP_ParserDriver_t>
{
  typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage,
                               HTTP_ParserDriver_t> inherited;

 public:
  Test_I_HTTPParser (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_HTTPParser () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_I_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser (const Test_I_HTTPParser&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser& operator= (const Test_I_HTTPParser&))
};

// declare module
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                                // session event type
                          struct Test_I_IceCastClient_ModuleHandlerConfiguration,        // module handler configuration type
                          libacenetwork_protocol_default_http_parser_module_name_string,
                          Stream_INotify_t,                                              // stream notification interface type
                          Test_I_HTTPStreamer,                                           // reader type
                          Test_I_HTTPParser,                                             // writer type
                          Test_I_HTTPMarshal);                                           // name

//////////////////////////////////////////

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2> Test_I_HTTPStreamer_2;

typedef HTTP_ParserDriver_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            Test_I_SessionMessage_2> HTTP_ParserDriver_2;

class Test_I_HTTPParser_2
 : public HTTP_Module_Parser_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2,
                               HTTP_ParserDriver_2>
{
  typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2,
                               HTTP_ParserDriver_2> inherited;

 public:
  Test_I_HTTPParser_2 (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_HTTPParser_2 () {}

  // implement (part of) Stream_ITaskBase
  //virtual void handleDataMessage (Test_I_Message*&, // data message handle
  //                                bool&);           // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser_2 ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser_2 (const Test_I_HTTPParser_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPParser_2& operator= (const Test_I_HTTPParser_2&))
};

// declare module
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData_2,                     // session data type
                          enum Stream_SessionMessageType,                                // session event type
                          struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,      // module handler configuration type
                          libacenetwork_protocol_default_http_parser_module_name_string,
                          Stream_INotify_t,                                              // stream notification interface type
                          Test_I_HTTPStreamer_2,                                         // reader type
                          Test_I_HTTPParser_2,                                           // writer type
                          Test_I_HTTPMarshal_2);                                         // name

#endif
