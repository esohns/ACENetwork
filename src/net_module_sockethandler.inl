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

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_session_message_base.h"

#include "net_macros.h"

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::Net_Module_SocketHandler_T (LockType* lock_in)
 : inherited (lock_in, // lock handle
              false,   // auto-start ?
              true)    // generate sesssion messages ?
 , currentBuffer_ (NULL)
 , currentMessage_ (NULL)
 , currentMessageLength_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::Net_Module_SocketHandler_T"));

}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::~Net_Module_SocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::~Net_Module_SocketHandler_T"));

  if (currentMessage_)
    currentMessage_->release ();
}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::initialize"));

  bool result = false;

  if (inherited::initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    currentMessageLength_ = 0;
    if (currentMessage_)
      currentMessage_->release ();
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
  } // end IF

  result = inherited::initialize (configuration_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
  // *NOTE*: data is fed into the stream from outside, as it arrives
  //         --> do not run svc() on start()
  inherited::runSvcOnStart_ = false;

  return result;
}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
void
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::handleDataMessage"));

  int result = -1;

  // initialize return value(s), default behavior is to pass all messages along
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // perhaps part of this message has already arrived ?
  if (currentBuffer_)
  { // chain the inbound buffer
    currentBuffer_->cont (message_inout);
  } // end IF
  else
    currentBuffer_ = message_inout;

  DataMessageType* message_p = NULL;
  while (bisectMessages (message_p))
  {
    // full message available ?
    if (message_p)
    {
      // --> push it downstream...
      result = inherited::put_next (message_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        message_p->release ();
      } // end IF
      message_p = NULL;
    } // end IF
  } // end WHILE
}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
void
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::initialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      if (inherited::configuration_->streamConfiguration->statisticReportingInterval)
      {
        // schedule regular statistic collection
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL, 0);
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p = &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    timerID_,
//                    &interval));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited::timerID_));
        inherited::timerID_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Module_SocketHandler_T::putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ProtocolHeaderType>
//void
//Net_Module_SocketHandler_T<LockType,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ProtocolHeaderType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ProtocolHeaderType>
bool
Net_Module_SocketHandler_T<LockType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ProtocolHeaderType>::bisectMessages (DataMessageType*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::bisectMessages"));

  // initialize result
  message_out = NULL;

  if (currentMessageLength_ == 0)
  {
    // --> evaluate the incoming message header

    // perhaps part of the header has already arrived ?
    if (currentMessage_ == NULL)
    {
      // really don't know anything
      // --> if possible, use the current buffer as head
      if (currentBuffer_)
        currentMessage_ = currentBuffer_;
      else
        return false; // don't have data --> cannot proceed
    } // end IF

    // OK, perhaps start interpreting the message header

    // check: received the full header yet ?...
    if (currentMessage_->total_length () < sizeof (ProtocolHeaderType))
    {
      // no, so keep what there is (default behavior)

      // ... and wait for some more data
      return false;
    } // end IF

    // OK, start parsing this message

    ProtocolHeaderType message_header = currentMessage_->get ();
    // *PORTABILITY*: handle endianness && type issues !
    // see also net_remote_comm.h
    currentMessageLength_ =
        message_header.messageLength + sizeof (unsigned int);
  } // end IF

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: received %u bytes [current: %u, total: %u]...\n"),
//               connectionID_,
//               currentBuffer_->length (),
//               currentMessage_->total_length (),
//               currentMessageLength_));

  // check if we received the whole message yet
  if (currentMessage_->total_length () < currentMessageLength_)
  {
    // no, so keep what there is (default behavior)

    // ... and wait for some more data
    return false;
  } // end IF

  // OK, message complete !
  message_out = currentMessage_;

  // check if (part of) the next message has arrived
  if (currentMessage_->total_length () > currentMessageLength_)
  {
    // adjust write pointer of (current) buffer so (total_-)length() returns
    // the proper size...
    unsigned int offset = currentMessageLength_;
    // in order to find the correct offset in the tail buffer, the total size
    // of the preceding continuation(s) may need to be retrieved
    ACE_Message_Block* tail_p = currentMessage_;
    do
    {
      if (offset < tail_p->length ())
        break;

      offset -= tail_p->length ();
      if (offset == 0) // <-- no overlap
      {
        // set new message head
        currentMessage_ =
          dynamic_cast<DataMessageType*> (tail_p->cont ());
        ACE_ASSERT (currentMessage_);

        // unchain the rest of the buffer
        tail_p->cont (NULL);

        // don't know anything about the next message
        currentMessageLength_ = 0;

        return true;
      } // end IF

      tail_p = tail_p->cont ();
    } while (tail_p != currentBuffer_);

    // create a shallow copy
    DataMessageType* new_head_p =
        dynamic_cast<DataMessageType*> (tail_p->duplicate ());
    if (!new_head_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to DataMessageType::duplicate(): \"%m\", aborting\n")));
      return false;
    } // end IF
    // ...and adjust rd_ptr to point to the beginning of the next message
    new_head_p->rd_ptr (offset);

    // adjust wr_ptr to make total_length() work
    tail_p->wr_ptr (tail_p->rd_ptr () + offset);

    // set new message head/current buffer
    currentBuffer_ = currentMessage_ = new_head_p;
  } // end IF
  else
  {
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
  } // end ELSE

  // don't know anything about the next message
  currentMessageLength_ = 0;

  return true;
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename DataMessageType>
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

//template <typename LockType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ProtocolHeaderType>
//bool
//Net_Module_SocketHandler_T<LockType,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ProtocolHeaderType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_SocketHandler_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  // *TODO*: remove type inferences
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
//  // step1: update session state
//  // *TODO*: remove type inferences
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  session_data_r.currentStatistic = statisticData_in;
//
//  // *TODO*: attach stream state information to the session data
//
////  // step2: create session data object container
////  SessionDataContainerType* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    SessionDataContainerType (inherited::sessionData_,
////                                              false));
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
////    return false;
////  } // end IF
//
//  // step3: send the statistic data downstream
//  // *NOTE*: fire-and-forget session_data_p here
//  // *TODO*: remove type inference
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}

/////////////////////////////////////////

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::Net_Module_UDPSocketHandler_T (LockType* lock_in)
 : inherited (lock_in, // lock handle
              false,   // auto-start ?
              true)    // generate sesssion messages ?
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::Net_Module_UDPSocketHandler_T"));

}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::~Net_Module_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::~Net_Module_UDPSocketHandler_T"));

}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::initialize"));

  bool result = false;

  if (inherited::initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));
  } // end IF

  result = inherited::initialize (configuration_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return result;
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename DataMessageType>
//unsigned int
//Net_Module_UDPSocketHandler_T::getSessionID () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::getSessionID"));

//  return inherited::sessionID_;
//}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
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

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::handleSessionMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);
  ACE_ASSERT (inherited::initialized_);

  switch (message_inout->getType ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      if (inherited::configuration_.streamConfiguration->statisticReportingInterval)
      {
        // schedule regular statistics collection...
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p =
            &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n")));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
//                    timerID_,
//                    &interval));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerID_ != -1)
      {
        int result = -1;
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited::timerID_));
        inherited::timerID_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Net_Module_UDPSocketHandler_T<LockType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticsMessage (data_out,               // data container
                                        COMMON_TIME_NOW)) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Net_Module_UDPSocketHandler_T<LockType,
//                              SessionMessageType,
//                              DataMessageType,
//                              ConfigurationType,
//                              StreamStateType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              StatisticContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename DataMessageType>
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

//template <typename LockType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//Net_Module_UDPSocketHandler_T<LockType,
//                              SessionMessageType,
//                              DataMessageType,
//                              ConfigurationType,
//                              StreamStateType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              StatisticContainerType>::putStatisticsMessage (const StatisticContainerType& statistic_in) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::putStatisticsMessage"));
//
//  // step1: initialize information object
//  inherited::state_->sessionData.currentStatistic = statistic_in;
//
//  // *TODO*: attach stream state information to the session data
//
//  // step2: create session data object container
//  SessionDataContainerType* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    SessionDataContainerType (inherited::sessionData_,
//                                              false));
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
//    return false;
//  } // end IF
//
//  // step3: send the data downstream
//  // *NOTE*: this is a "fire-and-forget" API, so don't worry about
//  //         session_data_p
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       session_data_p,
//                                       inherited::allocator_);
//}
