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

#ifndef Test_U_Module_EventHandler_H
#define Test_U_Module_EventHandler_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "test_u_configuration.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "net_client_stream_common.h"

class Test_U_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct ClientServer_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Test_U_StreamSessionData,
                                         struct Stream_UserData>
{
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct ClientServer_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Test_U_StreamSessionData,
                                         struct Stream_UserData> inherited;

 public:
  Test_U_Module_EventHandler (ISTREAM_T*); // stream handle
  inline virtual ~Test_U_Module_EventHandler () {}

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_EventHandler (const Test_U_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Module_EventHandler& operator= (const Test_U_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
                              enum Stream_SessionMessageType,                 // session event type
                              struct ClientServer_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                               // stream notification interface type
                              Test_U_Module_EventHandler);              // writer type

#endif
