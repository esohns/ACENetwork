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

#include "ace/Log_Msg.h"

#include "common_timer_manager.h"

#include "stream_session_message_base.h"

#include "net_macros.h"

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::Net_Module_SocketHandler_T ()
 : inherited (false, // inactive by default
              false) // DON'T auto-start !
 , currentMessageLength_ (0)
 , currentMessage_ (NULL)
 , currentBuffer_ (NULL)
 , isInitialized_ (false)
 , statCollectionInterval_ (0)
 , statCollectHandler_ (ACTION_COLLECT,
                        this,
                        false)
 , statCollectHandlerID_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::Net_Module_SocketHandler_T"));

}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::~Net_Module_SocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::~Net_Module_SocketHandler_T"));

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
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  statCollectHandlerID_));
  } // end IF

  // clean up any unprocessed (chained) buffer(s)
  if (currentMessage_)
    currentMessage_->release ();
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::initialize (StreamStateType* state_in,
                                                            Stream_IAllocator* allocator_in,
                                                            bool isActive_in,
                                                            unsigned int statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::initialize"));

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
  inherited::state_ = state_in;

  // OK: all's well...
  isInitialized_ = true;

  return isInitialized_;
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//unsigned int
//Net_Module_SocketHandler_T::getSessionID () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::getSessionID"));

//  return inherited::sessionID_;
//}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
void
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::handleDataMessage"));

  // init return value(s), default behavior is to pass all messages along...
  // --> we don't want that !
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  // perhaps part of this message has already arrived ?
  if (currentBuffer_)
  { // chain the inbound buffer
    currentBuffer_->cont (message_inout);
  } // end IF
  else
    currentBuffer_ = message_inout;

  ProtocolMessageType* message_p = NULL;
  while (bisectMessages (message_p))
  {
    // full message available ?
    if (message_p)
    {
      // --> push it downstream...
      if (inherited::put_next (message_p, NULL) == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        message_p->release ();
      } // end IF

      // reset state
      message_p = NULL;
    } // end IF
  } // end WHILE
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
void
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::handleSessionMessage"));

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
                                                                  COMMON_TIME_NOW + interval, // first wakeup time
                                                                  interval);                        // interval
        if (statCollectHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule(), aborting\n")));

          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                    statCollectHandlerID_,
//                    statCollectionInterval_));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;
    }
    case SESSION_END:
    {
      if (statCollectHandlerID_ != -1)
      {
        int result = -1;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      statCollectHandlerID_));
        statCollectHandlerID_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::collect (Stream_Statistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: init container
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticsMessage (data_out,               // data container
                             COMMON_TIME_NOW)) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));

    return false;
  } // end IF

  return true;
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
void
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::bisectMessages (ProtocolMessageType*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::bisectMessages"));

  // init result
  message_out = NULL;

  if (currentMessageLength_ == 0)
  {
    // --> evaluate the incoming message header

    // perhaps part of the header has already arrived ?
    if (currentMessage_ == NULL)
    {
      // really don't know anything
      // --> if possible, use the current buffer as head...
      if (currentBuffer_)
        currentMessage_ = currentBuffer_;
      else
        return false; // don't have data --> cannot proceed
    } // end IF

    // OK, perhaps start interpreting the message header...

    // check: received the full header yet ?...
    if (currentMessage_->total_length () < sizeof (ProtocolHeaderType))
    {
      // no, so keep what there is (default behavior) ...

      // ... and wait for some more data
      return false;
    } // end IF

    // OK, start interpreting this message...

    // ensure enough CONTIGUOUS data is available
    if (!currentMessage_->crunchForHeader (sizeof (ProtocolHeaderType)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ProtocolMessageType::crunchForHeader(%u), aborting\n")));
      return false;
    } // end IF

    ProtocolHeaderType* message_header_p =
        reinterpret_cast<ProtocolHeaderType*> (currentMessage_->rd_ptr ());
    // *PORTABILITY*: handle endianness && type issues !
    currentMessageLength_ =
        message_header_p->messageLength + sizeof (unsigned int);
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
    // no, so keep what there is (default behavior) ...

    // ... and wait for some more data
    return false;
  } // end IF

  // OK, message header complete !
  message_out = currentMessage_;

  // check if (part of) the next message has arrived
  if (currentMessage_->total_length () > currentMessageLength_)
  {
    // remember overlapping bytes
//     size_t overlap = currentMessage_->total_length () - currentMessageLength_;

    // adjust write pointer of current buffer so (total_-)length() reflects the
    // proper size...
    unsigned int offset = currentMessageLength_;
    // in order to find the correct offset in currentBuffer_, the total size of
    // the preceding continuation MAY need to be retrieved... :-(
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
//       ACE_DEBUG ((LM_CRITICAL,
//                   ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
//                   NET_DEF_NETWORK_BUFFER_SIZE));
//
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
//       return true;
//     } // end IF

    // [instead], use copy ctor and just reference the same data block...
    ProtocolMessageType* new_head =
        dynamic_cast<ProtocolMessageType*> (currentBuffer_->duplicate ());

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

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename ProtocolMessageType>
// Net_Message*
// Net_Module_SocketHandler_T::allocateMessage (unsigned int requestedSize_in)
// {
//NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::allocateMessage"));
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

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ProtocolHeaderType>::putStatisticsMessage (const Stream_Statistic_t& statistic_in,
                                                                      const ACE_Time_Value& collectionTime_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::putStatisticsMessage"));

  // step1: init information object
  inherited::state_->currentStatistics = statistic_in;
  inherited::state_->lastCollectionTimestamp = collectionTime_in;

  // *TODO*: attach stream state information to the session data

  // step2: create session data object container
  SessionDataContainerType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataContainerType (inherited::sessionData_,
                                              false));
  if (!session_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));

    return false;
  } // end IF

  // step3: send the data downstream
  // *NOTE*: this is a "fire-and-forget" API, so don't worry about session_data_p
  return inherited::putSessionMessage (SESSION_STATISTICS,
                                       session_data_p,
                                       inherited::allocator_);
}

/////////////////////////////////////////

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::Net_Module_UDPSocketHandler_T ()
 : inherited (false, // inactive by default
              false) // DON'T auto-start !
 , isInitialized_ (false)
 , statCollectHandler_ (ACTION_COLLECT,
                        this,
                        false)
 , statCollectHandlerID_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::Net_Module_UDPSocketHandler_T"));

}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::~Net_Module_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::~Net_Module_UDPSocketHandler_T"));

  // clean up timer if necessary
  if (statCollectHandlerID_ != -1)
  {
    const void* act = NULL;
    int result = -1;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_,
                                                            &act);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statCollectHandlerID_));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  statCollectHandlerID_));
  } // end IF
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::initialize (StreamStateType* state_in,
                                                                Stream_IAllocator* allocator_in,
                                                                bool isActive_in,
                                                                unsigned int statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::initialize"));

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (statCollectHandlerID_ != -1)
    {
      const void* act = NULL;
      int result = -1;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_,
                                                              &act);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    statCollectHandlerID_));
    } // end IF
    statCollectHandlerID_ = -1;
    isInitialized_ = false;
  } // end IF

  statCollectionInterval_ = statisticsCollectionInterval_in;

  inherited::allocator_ = allocator_in;
  inherited::isActive_ = isActive_in;
  inherited::state_ = state_in;

  // OK: all's well...
  isInitialized_ = true;

  return isInitialized_;
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//unsigned int
//Net_Module_UDPSocketHandler_T::getSessionID () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::getSessionID"));

//  return inherited::sessionID_;
//}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::handleDataMessage"));

//  // init return value(s), default behavior is to pass all messages along...
//  // --> don't want that !
//  passMessageDownstream_out = false;

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);

//  // --> push it downstream...
//  if (inherited::put_next (message_inout, NULL) == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

//    // clean up
//    message_inout->release ();
//  } // end IF
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::handleSessionMessage"));

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
                                                                  COMMON_TIME_NOW + interval, // first wakeup time
                                                                  interval);                        // interval
        if (statCollectHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule(), aborting\n")));

          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                    statCollectHandlerID_,
//                    statCollectionInterval_));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;
    }
    case SESSION_END:
    {
      if (statCollectHandlerID_ != -1)
      {
        int result = -1;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      statCollectHandlerID_));
        statCollectHandlerID_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::collect (Stream_Statistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: init container
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticsMessage (data_out,               // data container
                             COMMON_TIME_NOW)) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename ProtocolMessageType>
// Net_Message*
// Net_Module_UDPSocketHandler_T::allocateMessage (unsigned int requestedSize_in)
// {
//NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::allocateMessage"));
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

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Net_Module_UDPSocketHandler_T<StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              SessionMessageType,
                              ProtocolMessageType>::putStatisticsMessage (const Stream_Statistic_t& statistic_in,
                                                                          const ACE_Time_Value& collectionTime_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::putStatisticsMessage"));

  // step1: init information object
  inherited::state_->currentStatistics = statistic_in;
  inherited::state_->lastCollectionTimestamp = collectionTime_in;

  // *TODO*: attach stream state information to the session data

  // step2: create session data object container
  SessionDataContainerType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataContainerType (inherited::sessionData_,
                                              false));
  if (!session_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
    return false;
  } // end IF

  // step3: send the data downstream
  // *NOTE*: this is a "fire-and-forget" API, so don't worry about
  //         session_data_p
  return inherited::putSessionMessage (SESSION_STATISTICS,
                                       session_data_p,
                                       inherited::allocator_);
}
