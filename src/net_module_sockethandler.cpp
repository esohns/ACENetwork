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

#include "net_module_sockethandler.h"

#include "common.h"
#include "common_timer_manager.h"

#include "net_defines.h"
#include "net_sessionmessage.h"
#include "net_message.h"
#include "net_remote_comm.h"
#include "net_stream_common.h"

Net_Module_SocketHandler::Net_Module_SocketHandler ()
 : inherited (false, // inactive by default
              false) // DON'T auto-start !
 , isInitialized_ (false)
 , statCollectHandler_ (this,
                        ACTION_COLLECT)
 , statCollectHandlerID_ (-1)
 , currentMessageLength_ (0)
 , currentMessage_ (NULL)
 , currentBuffer_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::Net_Module_SocketHandler"));

}

Net_Module_SocketHandler::~Net_Module_SocketHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::~Net_Module_SocketHandler"));

  // clean up timer if necessary
  if (statCollectHandlerID_ != -1)
  {
    const void* act = NULL;
    if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_,
                                                            &act) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statCollectHandlerID_));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  statCollectHandlerID_));
  } // end IF

  // clean up any unprocessed (chained) buffer(s)
  if (currentMessage_)
    currentMessage_->release ();
}

bool
Net_Module_SocketHandler::init (Stream_IAllocator* allocator_in,
                                bool isActive_in,
                                unsigned int statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::init"));

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (statCollectHandlerID_ != -1)
    {
      const void* act = NULL;
      if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_,
                                                              &act) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    statCollectHandlerID_));
    } // end IF
    statCollectHandlerID_ = -1;
    currentMessageLength_ = 0;
    if (currentMessage_)
      currentMessage_->release ();
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
    isInitialized_ = false;
  } // end IF

  statCollectionInterval_ = statisticsCollectionInterval_in;

  inherited::allocator_ = allocator_in;
  inherited::isActive_ = isActive_in;

  // OK: all's well...
  isInitialized_ = true;

  return isInitialized_;
}

//unsigned int
//Net_Module_SocketHandler::getSessionID () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::getSessionID"));

//  return inherited::sessionID_;
//}

void
Net_Module_SocketHandler::handleDataMessage (Net_Message*& message_inout,
                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::handleDataMessage"));

  // init return value(s), default behavior is to pass all messages along...
  // --> we don't want that !
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  // perhaps we already have part of this message ?
  if (currentBuffer_)
  {
    // enqueue the incoming buffer onto our chain
    currentBuffer_->cont (message_inout);
  } // end IF
  else
  {
    currentBuffer_ = message_inout;
  } // end ELSE

  Net_Message* complete_message = NULL;
  while (bisectMessages (complete_message))
  {
    // full message available ?
    if (complete_message)
    {
      // --> push it downstream...
      if (put_next (complete_message, NULL) == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        complete_message->release ();
      } // end IF

      // reset state
      complete_message = NULL;
    } // end IF
  } // end WHILE
}

void
Net_Module_SocketHandler::handleSessionMessage (Net_SessionMessage*& message_inout,
                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::handleSessionMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->getType ())
  {
    case SESSION_BEGIN:
    {
      if (statCollectionInterval_)
      {
        // schedule regular statistics collection...
        ACE_Time_Value interval (statCollectionInterval_, 0);
        ACE_ASSERT (statCollectHandlerID_ == -1);
        ACE_Event_Handler* eh = &statCollectHandler_;
        statCollectHandlerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (eh,                               // event handler
                                                                  NULL,                             // argument
                                                                  COMMON_TIME_POLICY () + interval, // first wakeup time
                                                                  interval);                        // interval
        if (statCollectHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to RPG_Common_Timer_Manager::schedule(), aborting\n")));

          return;
        } // end IF
      //     ACE_DEBUG ((LM_DEBUG,
      //                 ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
      //                 statCollectHandlerID_,
      //                 statCollectionInterval_));
      } // end IF

      // start profile timer...
//       profile_.start ();

      break;
    }
    case SESSION_END:
    {
      if (statCollectHandlerID_ != -1)
        if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_) == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      statCollectHandlerID_));
      statCollectHandlerID_ = -1;

      break;
    }
    default:
      break;
  } // end SWITCH
}

bool
Net_Module_SocketHandler::collect (Stream_Statistic_t& data_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: init container
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticsMessage (data_out,               // data container
                             COMMON_TIME_POLICY ())) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));

    return false;
  } // end IF

  return true;
}

void
Net_Module_SocketHandler::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

bool
Net_Module_SocketHandler::bisectMessages (Net_Message*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::bisectMessages"));

  // init result
  message_out = NULL;

  if (currentMessageLength_ == 0)
  {
    // --> evaluate the incoming message header

    // perhaps we already have part of the header ?
    if (currentMessage_ == NULL)
    {
      // we really don't know anything
      // if possible, use the current buffer as our head...
      if (currentBuffer_)
        currentMessage_ = currentBuffer_;
      else
        return false; // don't have data --> cannot proceed
    } // end IF

    // OK, perhaps we can start interpreting the message header...

    // check if we received the full header yet...
    if (currentMessage_->total_length () < sizeof (Net_Remote_Comm::MessageHeader))
    {
      // we don't, so keep what we have (default behavior) ...

      // ... and wait for some more data
      return false;
    } // end IF

    // OK, we can start interpreting this message...

    // make sure we have enough CONTIGUOUS data
    if (!currentMessage_->crunchForHeader (sizeof (Net_Remote_Comm::MessageHeader)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to RPG_Net_Message::crunchForHeader(%u), aborting\n")));

      // what else can we do ?
      return false;
    } // end IF

    Net_Remote_Comm::MessageHeader* message_header = reinterpret_cast<Net_Remote_Comm::MessageHeader*> (currentMessage_->rd_ptr ());
    // *TODO*: *PORTABILITY*: handle endianness && type issues !
    currentMessageLength_ = message_header->messageLength + sizeof (unsigned int);
  } // end IF

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: received %u bytes [current: %u, total: %u]...\n"),
//               connectionID_,
//               currentBuffer_->length (),
//               currentMessage_->total_length (),
//               currentMessageLength_));

  // check if we received the whole message yet...
  if (currentMessage_->total_length () < currentMessageLength_)
  {
    // we don't, so keep what we have (default behavior) ...

    // ... and wait for some more data
    return false;
  } // end IF

  // OK, we have all of it !
  message_out = currentMessage_;

  // check if we have received (part of) the next message
  if (currentMessage_->total_length () > currentMessageLength_)
  {
    // remember overlapping bytes
//     size_t overlap = currentMessage_->total_length () - currentMessageLength_;

    // adjust write pointer of our current buffer so (total_-)length()
    // reflects the proper size...
    unsigned int offset = currentMessageLength_;
    // in order to find the correct offset in currentBuffer_, we MAY need to
    // count the total size of the preceding continuation... :-(
    ACE_Message_Block* current = currentMessage_;
    while (current != currentBuffer_)
    {
      offset -= current->length ();
      current = current->cont ();
    } // end WHILE

//     currentBuffer_->wr_ptr (currentBuffer_->rd_ptrc() + offset);
//     // --> create a new message head...
//     Net_Message* new_head = allocateMessage (NET_DEF_NETWORK_BUFFER_SIZE);
//     if (new_head == NULL)
//     {
//       ACE_DEBUG ((LM_ERROR,
//                   ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
//                   NET_DEF_NETWORK_BUFFER_SIZE));
//
//       // *TODO*: what else can we do ?
//       return true;
//     } // end IF
//     // ...and copy the overlapping data
//     if (new_head->copy (currentBuffer_->wr_ptr (),
//                        overlap))
//     {
//       ACE_DEBUG ((LM_ERROR,
//                   ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
//
//       // clean up
//       new_head->release ();
//
//       // *TODO*: what else can we do ?
//       return true;
//     } // end IF

    // [instead], use copy ctor and just reference the same data block...
    Net_Message* new_head = dynamic_cast<Net_Message*> (currentBuffer_->duplicate ());

    // adjust wr_ptr to make length() work...
    currentBuffer_->wr_ptr (currentBuffer_->rd_ptr () + offset);
    // sanity check
    ACE_ASSERT (currentMessage_->total_length () == currentMessageLength_);

    // adjust rd_ptr to point to the beginning of the next message
    new_head->rd_ptr (offset);

    // set new message head/current buffer
    currentMessage_ = new_head;
    currentBuffer_ = currentMessage_;
  } // end IF
  else
  {
    // bye bye...
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
  } // end ELSE

  // don't know anything about the next message...
  currentMessageLength_ = 0;

  return true;
}

// Net_Message*
// Net_Module_SocketHandler::allocateMessage (unsigned int requestedSize_in)
// {
//NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::allocateMessage"));
//
//   // init return value(s)
//   Net_Message* message_out = NULL;
//
//   try
//   {
//     message_out = static_cast<Net_Message*> (//inherited::allocator_->malloc (requestedSize_in));
//   }
//   catch (...)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                 requestedSize_in));
//   }
//   if (!message_out)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
//                 requestedSize_in));
//   } // end IF
//
//   return message_out;
// }

bool
Net_Module_SocketHandler::putStatisticsMessage (const Stream_Statistic_t& statistic_in,
                                                const ACE_Time_Value& collectionTime_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler::putStatisticsMessage"));

  // step1: init information object
  inherited::state_->currentStatistics = statistic_in;
  inherited::state_->lastCollectionTimestamp = collectionTime_in;

  // *TODO*: attach stream state information to the session data

  // step2: create session data object container
  Net_StreamSessionData_t* session_data_container_p = NULL;
  ACE_NEW_NORETURN (session_data_container_p,
                    Net_StreamSessionData_t (inherited::sessionData_,
                                             false,
                                             inherited::state_,
                                             ACE_Time_Value::zero,
                                             false));
  if (!session_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));

    return false;
  } // end IF

  // step3: send the data downstream
  // *NOTE*: this is a "fire-and-forget" API, so don't worry about session_data_container_p !
  return inherited::putSessionMessage (SESSION_STATISTICS,
                                       session_data_container_p,
                                       inherited::allocator_);
}
