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

#include "stream_defines.h"
#include "stream_session_message_base.h"

#include "net_macros.h"

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::Net_Module_TCPSocketHandler_T (ISTREAM_T* stream_in,
                                                                            bool generateSessionMessages_in)
 : inherited (stream_in,
              false,
              STREAM_HEADMODULECONCURRENCY_CONCURRENT,
              generateSessionMessages_in)
 , currentBuffer_ (NULL)
 , currentMessage_ (NULL)
 , currentMessageLength_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::Net_Module_TCPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::~Net_Module_TCPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::~Net_Module_TCPSocketHandler_T"));

  if (currentMessage_)
    currentMessage_->release ();
}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
bool
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::initialize"));

  if (inherited::isInitialized_)
  {
    currentMessageLength_ = 0;
    if (currentMessage_)
      currentMessage_->release ();
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
void
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::handleDataMessage"));

  int result = -1;

  // initialize return value(s), default behavior is to pass all messages along
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

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
      // --> push it downstream
      result = inherited::put_next (message_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

        // clean up
        message_p->release ();
      } // end IF
      message_p = NULL;
    } // end IF
  } // end WHILE
}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
void
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // schedule regular statistic collection ?
      // *TODO*: remove type inference
      if (inherited::configuration_->statisticReportingInterval !=
          ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL, 0);
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
            itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler handle
                                              NULL,                            // asynchronous completion token
                                              COMMON_TIME_NOW + interval,      // first wakeup time
                                              interval);                       // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T\n"),
//                    timerId_,
//                    &interval));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
bool
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

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
                ACE_TEXT ("%s: failed to Net_Module_TCPSocketHandler_T::putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ProtocolHeaderType>
//void
//Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ProtocolHeaderType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename ProtocolHeaderType,
          typename UserDataType>
bool
Net_Module_TCPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              ProtocolHeaderType,
                              UserDataType>::bisectMessages (DataMessageType*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_TCPSocketHandler_T::bisectMessages"));

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
                  ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
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

/////////////////////////////////////////

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename UserDataType>
Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              UserDataType>::Net_Module_UDPSocketHandler_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                            bool autoStart_in,
                                                                            bool generateSessionMessages_in)
 : inherited (lock_in,
              autoStart_in,
              generateSessionMessages_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::Net_Module_UDPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename UserDataType>
bool
Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::initialize"));

  if (inherited::initialized_)
  {
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
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

//template <ACE_SYNCH_DECL,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//void
//Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
//                              ControlMessageType,
//                              DataMessageType,
//                              SessionMessageType,
//                              ConfigurationType,
//                              StreamControlType,
//                              StreamNotificationType,
//                              StreamStateType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              StatisticContainerType,
//                              TimerManagerType,
//                              UserDataType>::handleDataMessage (DataMessageType*& message_inout,
//                                                                bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::handleDataMessage"));

////  // init return value(s), default behavior is to pass all messages along...
////  // --> don't want that !
////  passMessageDownstream_out = false;

////  // sanity check(s)
////  ACE_ASSERT (message_inout);
////  ACE_ASSERT (isInitialized_);

////  // --> push it downstream...
////  if (inherited::put_next (message_inout, NULL) == -1)
////  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

////    // clean up
////    message_inout->release ();
////  } // end IF
//}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename UserDataType>
void
Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::handleSessionMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->getType ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      // schedule regular statistic collection ?
      if (inherited::configuration_->streamConfiguration->statisticReportingInterval)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
            itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler handle
                                              NULL,                            // asynchronous completion token
                                              COMMON_TIME_NOW + interval,      // first wakeup time
                                              interval);                       // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    timerID_,
//                    &interval));
      } // end IF

//      // start profile timer...
//      profile_.start ();

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerId_ != -1)
      {
        int result = -1;
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerID_));
        inherited::timerId_ = -1;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
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
          typename TimerManagerType,
          typename UserDataType>
bool
Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
                              TimerManagerType,
                              UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_UDPSocketHandler_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out,               // data container
                                       COMMON_TIME_NOW)) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
//   try {
//     message_out = static_cast<Net_Message*> (//inherited::allocator_->malloc (requestedSize_in));
//   } catch (...) {
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

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//Net_Module_UDPSocketHandler_T<ACE_SYNCH_USE,
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
