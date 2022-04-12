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

#ifndef TEST_I_M3U_MODULE_PARSER_H
#define TEST_I_M3U_MODULE_PARSER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"

#include "common_parser_common.h"

#include "common_parser_m3u_parser_driver.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_parser.h"

#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_url_stream_load_stream_common.h"

class Test_I_M3U_Module_Parser
 : public Stream_Module_Parser_T <ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage,
                                 Common_Parser_M3U_ParserDriver,
                                 struct Stream_UserData>
{
  typedef Stream_Module_Parser_T <ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_I_Message,
                                  Test_I_SessionMessage,
                                  Common_Parser_M3U_ParserDriver,
                                  struct Stream_UserData> inherited;

 public:
  Test_I_M3U_Module_Parser (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_M3U_Module_Parser () {}

  // override (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (Test_I_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Module_Parser ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Module_Parser (const Test_I_M3U_Module_Parser&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Module_Parser& operator= (const Test_I_M3U_Module_Parser&))

  virtual void record (M3U_Playlist_t*&); // data record
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,  // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_parser_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_M3U_Module_Parser);                // writer type

#endif
