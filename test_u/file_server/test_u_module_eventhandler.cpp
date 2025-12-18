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
#include "stdafx.h"

#include "test_u_module_eventhandler.h"

#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "stream_imessagequeue.h"

#include "net_macros.h"

Test_U_Module_EventHandler::Test_U_Module_EventHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::Test_U_Module_EventHandler"));

}

void
Test_U_Module_EventHandler::handleControlMessage (Stream_ControlMessage_t& controlMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::handleControlMessage"));

  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_STEP:
    {
      // finished reading the source file
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: finished reading source file, scheduling disconnect...\n"),
                  inherited::mod_->name ()));

      // step3: send a 'disconnect' command upstream to sever the connection
      ACE_ASSERT (inherited::sessionData_);

      const FileServer_SessionData& session_data_r =
        inherited::sessionData_->getR ();
      if (!inherited::putControlMessage (session_data_r.sessionId,
                                         STREAM_CONTROL_DISCONNECT,
                                         true)) // send upstream ?
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_TaskBase_T::putControlMessage(%d), returning\n"),
                    STREAM_CONTROL_DISCONNECT));
        return;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control message type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  controlMessage_in.type ()));
      break;
    }
  } // end SWITCH
}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_U_Module_EventHandler::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::clone"));

  // initialize return value(s)
  Test_U_Module_EventHandler* module_handler_p = NULL;

  ACE_NEW_NORETURN (module_handler_p,
                    Test_U_Module_EventHandler (NULL));
  if (!module_handler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return NULL;
  } // end IF

  // sanity check(s)
  if (!inherited::configuration_)
    goto continue_;
  
  if (!module_handler_p->initialize (*inherited::configuration_,
                                     inherited::allocator_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Test_U_Module_EventHandler::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

    // clean up
    delete module_handler_p;

    return NULL;
  } // end IF

continue_:
  return module_handler_p;
}
