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

#ifndef TEST_U_MODULE_EVENTHANDLER_H
#define TEST_U_MODULE_EVENTHANDLER_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_stream_common.h"

// forward declarations
class Stream_IMessageQueue;

class Test_U_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_U_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Test_U_StreamSessionData>
{
 public:
  Test_U_Module_EventHandler ();
  virtual ~Test_U_Module_EventHandler ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const struct Test_U_ModuleHandlerConfiguration&,
                           Stream_IAllocator* = NULL); // report cache usage ?

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ACE_Message_Block&);

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_U_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Test_U_StreamSessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_EventHandler (const Test_U_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_EventHandler& operator= (const Test_U_Module_EventHandler&))

  Stream_IMessageQueue* outboundQueue_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_IStreamNotify_t,                   // stream notification interface type
                              Test_U_Module_EventHandler);              // writer type

#endif
