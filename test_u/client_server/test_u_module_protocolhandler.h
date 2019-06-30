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

#ifndef Test_U_Module_ProtocolHandler_H
#define Test_U_Module_ProtocolHandler_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"

#include "common_itimerhandler.h"
#include "common_time_common.h"
#include "common_timer_handler.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

#include "test_u_stream_common.h"

// forward declaration(s)
class Stream_IAllocator;
class Test_U_Message;
class Test_U_SessionMessage;

extern const char libacenetwork_default_test_u_protocolhandler_module_name_string[];

class Test_U_Module_ProtocolHandler
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 Common_ILock_T<ACE_MT_SYNCH>,
                                 struct Test_U_ModuleHandlerConfiguration,
                                 Test_U_ControlMessage_t,
                                 Test_U_Message,
                                 Test_U_SessionMessage,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Common_ITimerHandler
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 Common_ILock_T<ACE_MT_SYNCH>,
                                 struct Test_U_ModuleHandlerConfiguration,
                                 Test_U_ControlMessage_t,
                                 Test_U_Message,
                                 Test_U_SessionMessage,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Test_U_Module_ProtocolHandler (ISTREAM_T*); // stream handle
  virtual ~Test_U_Module_ProtocolHandler ();

  // initialization
  virtual bool initialize (const struct Test_U_ModuleHandlerConfiguration&, // configuration
                           Stream_IAllocator* = NULL);                            // allocator

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_U_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_SessionMessage*&, // session message handle
                                     bool&);                  // return value: pass message downstream ?

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_ProtocolHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_ProtocolHandler (const Test_U_Module_ProtocolHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_ProtocolHandler& operator= (const Test_U_Module_ProtocolHandler&))

  // implement Common_ITimerHandler
  inline virtual const long get () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  virtual void handle (const void*); // asynchronous completion token

  // timer
  Common_Timer_Handler pingHandler_;
  ACE_Time_Value       pingInterval_;
  long                 pingTimerId_;

  bool                 automaticPong_;
  unsigned int         counter_;
  bool                 printPongDot_;
  Stream_SessionId_t   sessionId_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
                              enum Stream_SessionMessageType,                 // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_default_test_u_protocolhandler_module_name_string,
                              Stream_INotify_t,                               // stream notification interface type
                              Test_U_Module_ProtocolHandler);           // writer type

#endif
