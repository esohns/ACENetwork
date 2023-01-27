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

#ifndef TEST_I_MODULE_HTTPGET_H
#define TEST_I_MODULE_HTTPGET_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_source_http_get.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Module_HTTPGet
 : public Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
                                              Common_TimePolicy_t,
                                              struct Test_I_WebTV_ModuleHandlerConfiguration,
                                              Stream_ControlMessage_t,
                                              Test_I_Message,
                                              Test_I_SessionMessage>
{
  typedef Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
                                              Common_TimePolicy_t,
                                              struct Test_I_WebTV_ModuleHandlerConfiguration,
                                              Stream_ControlMessage_t,
                                              Test_I_Message,
                                              Test_I_SessionMessage> inherited;

 public:
  Test_I_Module_HTTPGet (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_Module_HTTPGet () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_I_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_HTTPGet ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_HTTPGet (const Test_I_Module_HTTPGet&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_HTTPGet& operator= (const Test_I_Module_HTTPGet&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_WebTV_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_WebTV_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_http_get_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Module_HTTPGet);                                 // writer type

#endif
