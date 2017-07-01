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

#include "ace/Synch.h"
#include "test_u_module_eventhandler.h"

#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "stream_imessagequeue.h"

#include "net_macros.h"

Test_U_Module_EventHandler::Test_U_Module_EventHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
 //, connection_ (NULL)
 //, outboundQueue_ (NULL)
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
  //outboundQueue_ = configuration_in.outboundQueue;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
Test_U_Module_EventHandler::handleControlMessage (Test_U_ControlMessage_t& controlMessage_in)
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

      //// step1: wait for upstream to dispatch the outbound data
      //if (outboundQueue_)
      //{
      //  try { // *NOTE*: wait for the upstream head module queue to idle
      //    outboundQueue_->waitForIdleState ();
      //  } catch (...) {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to Stream_IMessageQueue::waitForIdleState(), returning\n"),
      //                inherited::mod_->name ()));
      //    return;
      //  }
      //} // end IF

      //// step2: wait for the connection to schedule all outbound data
      //if (connection_)
      //{
      //  try {
      //    connection_->waitForIdleState ();
      //  } catch (...) {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to Net_IStreamConnection_T::waitForIdleState() (id was: %u), returning\n"),
      //                inherited::mod_->name (),
      //                connection_->id ()));
      //    return;
      //  }
      //} // end IF

      // step3: send a 'disconnect' command upstream to sever the connection
      if (!inherited::putControlMessage (STREAM_CONTROL_DISCONNECT,
                                         true))
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

//void
//Test_U_Module_EventHandler::handleSessionMessage (Test_U_SessionMessage*& message_inout,
//                                                  bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_EventHandler::handleSessionMessage"));
//
//  // don't care (implies yes per default, when part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//
//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_MESSAGE_BEGIN:
//    {
//      const Test_U_FileServer_SessionData_t& session_data_container_r =
//          message_inout->get ();
//      const Test_U_FileServer_SessionData& session_data_r =
//          session_data_container_r.get ();
//
//      // sanity check(s)
//      ACE_ASSERT (!connection_);
//
//      Net_ConnectionId_t connection_id =
//        static_cast<Net_ConnectionId_t> (session_data_r.sessionID);
//      Test_U_IConnection_t* connection_p =
//        TEST_U_CONNECTIONMANAGER_SINGLETON::instance ()->get (connection_id);
//      if (!connection_p)
//      { // *TODO*: remove type inference
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to retrieve connection handle (id was: %u), aborting\n"),
//                    inherited::mod_->name (),
//                    connection_id));
//        goto error;
//      } // end IF
//      connection_ = dynamic_cast<Test_U_IStreamConnection_t*> (connection_p);
//      if (!connection_)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T*>(0x%@), aborting\n"),
//                    inherited::mod_->name (),
//                    connection_p));
//        goto error;
//      } // end IF
//
//      break;
//
//error:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT);
//
//      if (connection_p)
//        connection_p->decrease ();
//
//      break;
//    }
//    case STREAM_SESSION_MESSAGE_END:
//    {
//      // clean up
//      if (connection_)
//      {
//        connection_->decrease ();
//        connection_ = NULL;
//      } // end IF
//
//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}

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
