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

#include <ace/Synch.h>
#include "test_u_module_eventhandler.h"

#include <ace/Log_Msg.h>
#include <ace/OS_Memory.h>

#include "stream_imessagequeue.h"

#include "net_macros.h"

Test_U_Module_EventHandler::Test_U_Module_EventHandler ()
 : inherited ()
 , outboundQueue_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::Test_U_Module_EventHandler"));

}

Test_U_Module_EventHandler::~Test_U_Module_EventHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::~Test_U_Module_EventHandler"));

}

bool
Test_U_Module_EventHandler::initialize (const struct Test_U_ModuleHandlerConfiguration& configuration_in,
                                        Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::initialize"));

  // *TODO*: remove type inference
  outboundQueue_ = configuration_in.outboundQueue;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
Test_U_Module_EventHandler::handleControlMessage (ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::handleControlMessage"));

  switch (messageBlock_in.msg_type ())
  {
    case STREAM_CONTROL_STEP:
    {
      // finished reading the source file
      // step1: wait for upstream to process the outbound data
      if (outboundQueue_)
      {
        try { // *NOTE*: wait for the queue to idle
          outboundQueue_->waitForIdleState ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::waitForIdleState(), returning\n")));
          return;
        }
      } // end IF

      // step2: send a disconnect command upstream to sever the connection
      if (!inherited::putControlMessage (STREAM_CONTROL_DISCONNECT,
                                         false))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_TaskBase_T::putControlMessage(%d), returning\n"),
                    STREAM_CONTROL_DISCONNECT));
        return;
      } // end IF

      // step2: wait for 

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control message type (was: %d), aborting\n"),
                  messageBlock_in.msg_type ()));
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
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    Test_U_Module_EventHandler ());
  if (!task_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
  else
  {
    inherited* inherited_p = dynamic_cast<inherited*> (task_p);
    ACE_ASSERT (!inherited_p);
    ACE_ASSERT (inherited::configuration_);
    inherited_p->initialize (*inherited::configuration_,
                             inherited::allocator_);
  } // end ELSE

  return task_p;
}
