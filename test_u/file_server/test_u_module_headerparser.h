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

#ifndef TEST_U_MODULE_HEADERPARSER_H
#define TEST_U_MODULE_HEADERPARSER_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

//#include "net_exports.h"

#include "test_u_defines.h"
#include "test_u_stream_common.h"
#include "test_u_sessionmessage.h"

//extern Net_Export const char libacenetwork_default_test_u_headerparser_module_name_string[];
extern const char libacenetwork_default_test_u_headerparser_module_name_string[];

// forward declaration(s)
class Test_U_Message;
//class Test_U_SessionMessage;

class Test_U_Module_HeaderParser
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct FileServer_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_U_Message,
                                 Test_U_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct FileServer_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_U_Message,
                                 Test_U_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Test_U_Module_HeaderParser (ISTREAM_T*); // stream handle
  inline virtual ~Test_U_Module_HeaderParser () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_U_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_HeaderParser ());
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_HeaderParser (const Test_U_Module_HeaderParser&));
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_HeaderParser& operator= (const Test_U_Module_HeaderParser&));
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct FileServer_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_default_test_u_headerparser_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_U_Module_HeaderParser);              // writer type

#endif
