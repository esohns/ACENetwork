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

//#include "ace/Synch.h"
//#include "test_u_stream.h"
//#include "test_u_sessionmessage.h"
#include "test_u_module_protocolhandler.h"

#include <iostream>

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "test_u_defines.h"
#include "test_u_message.h"

Test_U_Module_ProtocolHandler::Test_U_Module_ProtocolHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , pingHandler_ (this,  // dispatch ourselves
                 false) // ping peer at regular intervals...
 , pingInterval_ (ACE_Time_Value::zero) // [0: --> OFF]
 , pingTimerId_ (-1)
 , automaticPong_ (true)
 , counter_ (1)
 , printPongDot_ (false)
 , sessionId_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::Test_U_Module_ProtocolHandler"));

}

Test_U_Module_ProtocolHandler::~Test_U_Module_ProtocolHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::~Test_U_Module_ProtocolHandler"));

  int result = -1;

  // clean up timer if necessary
  if (pingTimerId_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerId_,
                                                                  &act_p);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: session %u: failed to cancel \"ping\" timer (id: %d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  sessionId_,
                  pingTimerId_));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: session %u: cancelled \"ping\" timer (id: %d) in dtor --> check implementation !\n"),
                  inherited::mod_->name (),
                  sessionId_,
                  pingTimerId_));
  } // end IF
}

bool
Test_U_Module_ProtocolHandler::initialize (const struct ClientServer_ModuleHandlerConfiguration& configuration_in,
                                           Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::initialize"));

  int result = -1;

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.protocolConfiguration);

  if (inherited::isInitialized_)
  {
    // reset state
    if (pingTimerId_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerId_,
                                                                    &act_p);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to cancel \"ping\" timer (id: %d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    pingTimerId_));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("%s: cancelled \"ping\" timer (id: %d)\n"),
      //              inherited::mod_->name (),
      //              pingTimerId_));
      pingTimerId_ = -1;
    } // end IF
    pingInterval_ = ACE_Time_Value::zero;

    automaticPong_ = true;
    counter_ = 1;
    printPongDot_ = false;
    sessionId_ = 0;
  } // end IF

  // *TODO*: remove type inferences
  pingInterval_ = configuration_in.protocolConfiguration->pingInterval;

  automaticPong_ = configuration_in.protocolConfiguration->pingAutoAnswer;
  //if (automaticPong)
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("auto-answering \"ping\" messages\n")));
  //printPongDot_ = configuration_in.printProgressDot;
  printPongDot_ = configuration_in.protocolConfiguration->printPongMessages;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
Test_U_Module_ProtocolHandler::handleDataMessage (Test_U_Message*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::handleDataMessage"));

  int result = -1;

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // retrieve type of message and other details...
  Net_MessageHeader_t message_header = message_inout->get ();
  switch (message_header.type)
  {
    case Net_Remote_Comm::NET_MESSAGE_PING:
    {
      // auto-answer ?
      if (automaticPong_)
      {
        // --> reply with a "PONG"

        // step0: create reply structure
        // --> get a message buffer
        Test_U_Message* message_p =
            inherited::allocateMessage (sizeof (Net_Remote_Comm::PongMessage));
        if (!message_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to allocate reply message(%u), returning\n"),
                      inherited::mod_->name (),
                      sizeof (Net_Remote_Comm::PongMessage)));
          return;
        } // end IF
        // step1: initialize reply
        ACE_ASSERT (message_p->capacity () >= sizeof (Net_Remote_Comm::PongMessage));
        Net_Remote_Comm::PongMessage* pong_p =
          reinterpret_cast<Net_Remote_Comm::PongMessage*> (message_p->wr_ptr ());
        //ACE_OS::memset (pong_p, 0, sizeof (Net_Remote_Comm::PongMessage));
        pong_p->header.length =
          sizeof (Net_Remote_Comm::PongMessage) - sizeof (pong_p->header.length);
        pong_p->header.type =
          Net_Remote_Comm::NET_MESSAGE_PONG;
        message_p->wr_ptr (sizeof (Net_Remote_Comm::PongMessage));
        // step2: send it upstream
        result = inherited::reply (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          message_p->release (); message_p = NULL;
          return;
        } // end IF
      } // end IF

      break;
    }
    case Net_Remote_Comm::NET_MESSAGE_PONG:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: received PONG (connection id: %u)\n"),
      //            inherited::mod_->name (),
      //            sessionId_));

      if (printPongDot_)
      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT (".")));
        std::cout << '.';
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: [%u/%u]: unknown/invalid message type (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  message_inout->sessionId (), message_inout->id (),
                  ACE_TEXT (Test_U_Message::CommandTypeToString (message_header.type).c_str ())));
      return;
    }
  } // end SWITCH
}

void
Test_U_Module_ProtocolHandler::handleSessionMessage (Test_U_SessionMessage*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::handleSessionMessage"));

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::isInitialized_);

  int result = -1;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // retain session id for reporting
      const Test_U_StreamSessionData_t& session_data_container_r =
          message_inout->getR ();
      const struct Test_U_StreamSessionData& session_data_r =
          session_data_container_r.getR ();
      sessionId_ = session_data_r.sessionId;

      if (pingInterval_ != ACE_Time_Value::zero)
      {
        // schedule ping interval timer
        ACE_ASSERT (pingTimerId_ == -1);
        pingTimerId_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (&pingHandler_,                   // event handler handle
                                                                      NULL,                            // asynchronous completion token
                                                                      COMMON_TIME_NOW + pingInterval_, // first wakeup time
                                                                      pingInterval_);                  // interval
        if (pingTimerId_ == -1)
        {
           ACE_DEBUG ((LM_ERROR,
                       ACE_TEXT ("%s: session %u: failed to schedule \"ping\" timer: \"%m\", returning\n"),
                       inherited::mod_->name (),
                       sessionId_));
           return;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u: scheduled \"ping\" timer (id: %d), interval: %#T\n"),
                    inherited::mod_->name (),
                    sessionId_,
                    pingTimerId_,
                    &pingInterval_));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (pingTimerId_ != -1)
      {
        const void* act_p = NULL;
        result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerId_,
                                                                    &act_p);
        if (result <= 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: session %u: failed to cancel \"ping\" timer (id: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      sessionId_,
                      pingTimerId_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: session %u: cancelled \"ping\" timer (id: %d)\n"),
                      inherited::mod_->name (),
                      sessionId_,
                      pingTimerId_));
        pingTimerId_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

void
Test_U_Module_ProtocolHandler::handle (const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::handle"));

  ACE_UNUSED_ARG (arg_in);

  // step0: get a message buffer
  Test_U_Message* message_p =
      inherited::allocateMessage (sizeof (Net_Remote_Comm::PingMessage));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to allocate ping message(%u), returning\n"),
                inherited::mod_->name (),
                sizeof (Net_Remote_Comm::PingMessage)));
    return;
  } // end IF

  // step1: initialize message data
  ACE_ASSERT (message_p->capacity () >= sizeof (Net_Remote_Comm::PingMessage));
  // *TODO*: handle endianness consistently !
  Net_Remote_Comm::PingMessage* ping_p =
    reinterpret_cast<Net_Remote_Comm::PingMessage*> (message_p->wr_ptr ());
  //ACE_OS::memset (ping_p, 0, sizeof (Net_Remote_Comm::PingMessage));
  ping_p->header.length =
    (sizeof (Net_Remote_Comm::PingMessage) - sizeof (unsigned int));
  ping_p->header.type = Net_Remote_Comm::NET_MESSAGE_PING;
  ping_p->counter = counter_++;
  message_p->wr_ptr (sizeof (Net_Remote_Comm::PingMessage));

  // step2: send it upstream
  int result = inherited::reply (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return;
  } // end IF
}

void
Test_U_Module_ProtocolHandler::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::dump_state"));

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\n"),
//               inherited::mod_->name ()));
//
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\\END\n"),
//               inherited::mod_->name ()));
}

//////////////////////////////////////////

//Net_Export const char libacenetwork_default_test_u_protocolhandler_module_name_string[] =
//  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_MODULE_PROTOCOLHANDLER_NAME);
const char libacenetwork_default_test_u_protocolhandler_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_MODULE_PROTOCOLHANDLER_NAME);
