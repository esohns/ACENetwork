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

#include "test_u_module_protocolhandler.h"

#include <iostream>

//#include "ace/streams.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"

Net_Module_ProtocolHandler::Net_Module_ProtocolHandler ()
 : inherited ()
 , pingHandler_ (this,  // dispatch ourselves
                 false) // ping peer at regular intervals...
 , pingInterval_ (0) // [0: --> OFF]
 , pingTimerID_ (-1)
 , allocator_ (NULL)
 , automaticPong_ (true)
 , counter_ (1)
 , isInitialized_ (false)
 , printPongDot_ (false)
 , sessionID_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::Net_Module_ProtocolHandler"));

}

Net_Module_ProtocolHandler::~Net_Module_ProtocolHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::~Net_Module_ProtocolHandler"));

  int result = -1;

  // clean up timer if necessary
  if (pingTimerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerID_,
                                                                  &act_p);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("session %u: failed to cancel \"ping\" timer (ID: %d): \"%m\", continuing\n"),
                  sessionID_,
                  pingTimerID_));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("session %u: cancelled \"ping\" timer (ID: %d) --> check implementation !\n"),
                  sessionID_,
                  pingTimerID_));
  } // end IF
}

bool
Net_Module_ProtocolHandler::initialize (Stream_IAllocator* allocator_in,
                                        unsigned int pingInterval_in,
                                        bool autoAnswerPings_in,
                                        bool printPongDot_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::initialize"));

  int result = -1;

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // reset state
    if (pingTimerID_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerID_,
                                                                    &act_p);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel \"ping\" timer (ID: %d): \"%m\", continuing\n"),
                    pingTimerID_));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("cancelled \"ping\" timer (ID: %d)\n"),
      //              pingTimerID_));
      pingTimerID_ = -1;
    } // end IF
    pingInterval_ = 0;

    allocator_ = NULL;
    automaticPong_ = true;
    counter_ = 1;
    printPongDot_ = false;
    sessionID_ = 0;

    isInitialized_ = false;
  } // end IF

  pingInterval_ = pingInterval_in;

  allocator_ = allocator_in;
  automaticPong_ = autoAnswerPings_in;
  //if (automaticPong)
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("auto-answering \"ping\" messages\n")));
  printPongDot_ = printPongDot_in;

  isInitialized_ = true;

  return isInitialized_;
}

void
Net_Module_ProtocolHandler::handleDataMessage (Net_Message*& message_inout,
                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::handleDataMessage"));

  int result = -1;

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // retrieve type of message and other details...
  Net_MessageHeader_t message_header = message_inout->get ();
  switch (message_header.messageType)
  {
    case Net_Remote_Comm::NET_MESSAGE_PING:
    {
      // auto-answer ?
      if (automaticPong_)
      {
        // --> reply with a "PONG"

        // step0: create reply structure
        // --> get a message buffer
        Net_Message* message_p =
            allocateMessage (sizeof (Net_Remote_Comm::PongMessage));
        if (!message_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to allocate reply message(%u), returning\n"),
                      sizeof (Net_Remote_Comm::PongMessage)));
          return;
        } // end IF
        // step1: initialize reply
        Net_Remote_Comm::PongMessage* pong_struct =
          reinterpret_cast<Net_Remote_Comm::PongMessage*> (message_p->wr_ptr ());
        ACE_OS::memset (pong_struct, 0, sizeof (Net_Remote_Comm::PongMessage));
        pong_struct->messageHeader.messageLength =
          sizeof (Net_Remote_Comm::PongMessage) - sizeof (unsigned int);
        pong_struct->messageHeader.messageType =
          Net_Remote_Comm::NET_MESSAGE_PONG;
        message_p->wr_ptr (sizeof (Net_Remote_Comm::PongMessage));
        // step2: send it upstream
        result = inherited::reply (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

          // clean up
          message_p->release ();

          return;
        } // end IF
      } // end IF

      break;
    }
    case Net_Remote_Comm::NET_MESSAGE_PONG:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("received PONG (connection ID: %u)...\n"),
      //            sessionID_));

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
                  ACE_TEXT ("[%u]: unknown/invalid message type (was: \"%s\"), returning\n"),
                  message_inout->getID (),
                  ACE_TEXT (Net_Message::CommandType2String (message_header.messageType).c_str ())));
      return;
    }
  } // end SWITCH
}

void
Net_Module_ProtocolHandler::handleSessionMessage (Net_SessionMessage*& message_inout,
                                                  bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::handleSessionMessage"));

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  int result = -1;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // retain session ID for reporting...
      const Net_StreamSessionData_t& session_data_container_r =
          message_inout->get ();
      const Net_StreamSessionData& session_data_r =
          session_data_container_r.get ();
      sessionID_ = session_data_r.sessionID;

      if (pingInterval_)
      {
        // schedule ourselves...
        ACE_Time_Value interval ((pingInterval_ / 1000),
                                 ((pingInterval_ % 1000) * 1000));
        ACE_ASSERT (pingTimerID_ == -1);
        ACE_Event_Handler* handler_p = &pingHandler_;
        pingTimerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                      NULL,                       // ACT
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                  // interval
        if (pingTimerID_ == -1)
        {
           ACE_DEBUG ((LM_ERROR,
                       ACE_TEXT ("session %u: failed to schedule \"ping\" timer: \"%m\", returning\n"),
                       sessionID_));
           return;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("session %u: scheduled \"ping\" timer (id: %d), interval: %#T...\n"),
                    sessionID_,
                    pingTimerID_,
                    &interval));
      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
      if (pingTimerID_ != -1)
      {
        const void* act_p = NULL;
        result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (pingTimerID_,
                                                                    &act_p);
        if (result <= 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("session %u: failed to cancel \"ping\" timer (id: %d): \"%m\", continuing\n"),
                      sessionID_,
                      pingTimerID_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("session %u: cancelled \"ping\" timer (id: %d)\n"),
                      sessionID_,
                      pingTimerID_));
        pingTimerID_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

void
Net_Module_ProtocolHandler::handleTimeout (const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::handleTimeout"));

  ACE_UNUSED_ARG (arg_in);

  // step0: get a message buffer
  Net_Message* message_p =
      allocateMessage (sizeof (Net_Remote_Comm::PingMessage));
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate ping message(%u), returning\n"),
                sizeof (Net_Remote_Comm::PingMessage)));
    return;
  } // end IF

  // step1: initialize message data
  // *TODO*: handle endianness consistently !
  Net_Remote_Comm::PingMessage* ping_struct =
    reinterpret_cast<Net_Remote_Comm::PingMessage*> (message_p->wr_ptr ());
  ACE_OS::memset (ping_struct, 0, sizeof (Net_Remote_Comm::PingMessage));
  ping_struct->messageHeader.messageLength =
    (sizeof (Net_Remote_Comm::PingMessage) - sizeof (unsigned int));
  ping_struct->messageHeader.messageType = Net_Remote_Comm::NET_MESSAGE_PING;
  ping_struct->counter = counter_++;
  message_p->wr_ptr (sizeof (Net_Remote_Comm::PingMessage));

  // step2: send it upstream
  int result = inherited::reply (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("session %u: scheduled ping message...\n"),
//              sessionID_));
}

void
Net_Module_ProtocolHandler::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::dump_state"));

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\n"),
//               ACE_TEXT (inherited::name ())));
//
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\\END\n"),
//               ACE_TEXT (inherited::name ())));
}

Net_Message*
Net_Module_ProtocolHandler::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_ProtocolHandler::allocateMessage"));

  // initialize return value(s)
  Net_Message* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (allocator_);

  try
  {
    message_p =
      static_cast<Net_Message*> (allocator_->malloc (requestedSize_in));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_p;
}
