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

#include <sstream>

#include <ace/Log_Msg.h>

#include "common_ui_common.h"

#include "net_macros.h"

#include "bittorrent_defines.h"
#include "bittorrent_isession.h"
#include "bittorrent_tools.h"

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::BitTorrent_Module_PeerHandler_T ()
 : inherited ()
 , CBData_ (NULL)
 , session_ (NULL)
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::BitTorrent_Module_PeerHandler_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::~BitTorrent_Module_PeerHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::~BitTorrent_Module_PeerHandler_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
bool
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

//    {
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//      connectionIsAlive_ = false;
//    } // end lock scope

//    { // synch access to state machine
//      ACE_GUARD (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited2::stateLock_);

//      inherited2::state_ = REGISTRATION_STATE_NICK;
//    } // end lock scope

    CBData_ = NULL;
    session_ = NULL;
  } // end IF

  CBData_ = configuration_in.CBData;
  session_ = configuration_in.session;

  return inherited::initialize (configuration_in);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::handleDataMessage (MessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);

  const struct BitTorrent_Record& data_r = message_inout->get ();
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::Record2String (data_r).c_str ())));
#endif

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      if (!inherited::subscribe (this))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ISubscribe_T::subscribe(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

//      const SessionDataType& session_data_container_r =
//          message_inout->get ();
//      const typename SessionDataType::DATA_T& session_data_r =
//          session_data_container_r.get ();
//      ACE_ASSERT (session_data_r.connectionState);
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, session_data_r.connectionState->lock);
//        session_data_r.connectionState->nickName =
//            inherited::configuration_->protocolConfiguration->loginOptions.nickName;
//      } // end lock scope

//      // step1: remember connection has been opened...
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//        connectionIsAlive_ = true;

//        // signal any waiter(s)
//        result = condition_.broadcast ();
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
//      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (!inherited::unsubscribe (this))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ISubscribe_T::unsubscribe(), continuing\n"),
                    inherited::mod_->name ()));

//      // remember connection has been closed
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//        connectionIsAlive_ = false;

//        // signal any waiter(s)
//        result = condition_.broadcast ();
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
//      } // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH

  inherited::handleSessionMessage (message_inout,
                                   passMessageDownstream_out);
}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//bool
//BitTorrent_Module_PeerHandler_T::wait (const ACE_Time_Value* timeout_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::wait"));

//  int result = -1;

//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

//  result = condition_.wait (timeout_in);
//  if (result == -1)
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ETIME)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Condition::wait(): \"%m\", continuing\n")));
//  } // end IF
//  if (initialRegistration_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("registration timeout, aborting\n")));
//    return false;
//  } // end IF

//  return true;
//}

//void
//BitTorrent_Module_PeerHandler_T::subscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::subscribe"));

//  // sanity check(s)
//  ACE_ASSERT (interfaceHandle_in);

//  // synch access to subscribers
//  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

//  subscribers_.push_back (interfaceHandle_in);
//}

//void
//BitTorrent_Module_PeerHandler_T::unsubscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::unsubscribe"));

//  // sanity check(s)
//  ACE_ASSERT (interfaceHandle_in);

//  // synch access to subscribers
//  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

//  SubscribersIterator_t iterator = subscribers_.begin ();
//  for (;
//       iterator != subscribers_.end ();
//       iterator++)
//    if ((*iterator) == interfaceHandle_in)
//      break;

//  if (iterator != subscribers_.end ())
//    subscribers_.erase (iterator);
//  else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid argument (was: %@), aborting\n"),
//                interfaceHandle_in));
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//void
//BitTorrent_Module_PeerHandler_T::onChange (IRC_RegistrationState newState_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::allocateMessage"));

//  int result = -1;

//  if (newState_in == REGISTRATION_STATE_FINISHED)
//  {
//    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

//    result = condition_.broadcast ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Condition::broadcast(): \"%m\", continuing\n")));
//  } // end IF
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//IRC_Message*
//BitTorrent_Module_PeerHandler_T::allocateMessage (unsigned int requestedSize_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::allocateMessage"));

//  // initialize return value(s)
//  IRC_Message* message_p = NULL;

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);

//  if (inherited::configuration_->streamConfiguration->messageAllocator)
//  {
//allocate:
//    try {
//      message_p =
//        static_cast<IRC_Message*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                  requestedSize_in));
//      return NULL;
//    }

//    // keep retrying ?
//    if (!message_p &&
//        !inherited::configuration_->streamConfiguration->messageAllocator->block ())
//      goto allocate;
//  } // end IF
//  else
//    ACE_NEW_NORETURN (message_p,
//                      IRC_Message (requestedSize_in));
//  if (!message_p)
//  {
//    if (inherited::configuration_->streamConfiguration->messageAllocator)
//    {
//      if (inherited::configuration_->streamConfiguration->messageAllocator->block ())
//        ACE_DEBUG ((LM_CRITICAL,
//                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//    } // end IF
//    else
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//  } // end IF

//  return message_p;
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//IRC_Record*
//BitTorrent_Module_PeerHandler_T::allocateMessage (IRC_Record::CommandType type_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::allocateMessage"));

//  // initialize return value(s)
//  IRC_Record* message_p = NULL;

//  ACE_NEW_NORETURN (message_p,
//                    IRC_Record ());
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return NULL;
//  } // end IF
//  ACE_NEW_NORETURN (message_p->command_.string,
//                    std::string (IRC_Message::CommandType2String (type_in)));
//  if (!message_p->command_.string)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

//    // clean up
//    message_p->decrease ();

//    return NULL;
//  } // end IF
//  message_p->command_.discriminator = IRC_Record::Command::STRING;

//  return message_p;
//}

//void
//BitTorrent_Module_PeerHandler_T::sendMessage (IRC_Record*& record_inout)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::sendMessage"));

//  int result = -1;

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//  ACE_ASSERT (record_inout);

//  // step1: get a message buffer
//  IRC_Message* message_p =
//      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to BitTorrent_Module_PeerHandler_T::allocateMessage(%u), returning\n"),
//                inherited::configuration_->streamConfiguration->bufferSize));

//    // clean up
//    record_inout->decrease ();
//    record_inout = NULL;

//    return;
//  } // end IF

//  // step2: attach the command
//  // *NOTE*: message assumes control over command_in
//  message_p->initialize (*record_inout,
//                         NULL);
//  // --> bye bye...
//  record_inout = NULL;

//  // step3: send it upstream

//  // *NOTE*: there is NO way to prevent asynchronous closure of the connection;
//  //         this protect against closure of the stream while the message is
//  //         propagated... (see line 614)
//  //         --> grab lock and check connectionIsAlive_
//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);
//  // sanity check
//  if (!connectionIsAlive_)
//  {
//    //ACE_DEBUG ((LM_DEBUG,
//    //            ACE_TEXT ("connection has been closed/lost - cannot send message, returning\n")));

//    // clean up
//    message_p->release ();

//    return;
//  } // end IF

//  result = inherited::reply (message_p, NULL);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

//    // clean up
//    message_p->release ();

//    return;
//  } // end IF

////   ACE_DEBUG((LM_DEBUG,
////              ACE_TEXT("pushed message...\n")));
//}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::clone"));

  // initialize return value(s)
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    OWN_TYPE_T ());
  if (!task_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
  else
  {
    inherited* inherited_p = dynamic_cast<inherited*> (task_p);
    ACE_ASSERT (task_p);
    inherited_p->initialize (inherited::subscribers_,
                             inherited::lock_);
  } // end ELSE

  return task_p;
}

//----------------------------------------

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::start (Stream_SessionId_t sessionID_in,
                                                    const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator == sessionData_.end ());

  sessionData_.insert (std::make_pair (sessionID_in,
                                       &const_cast<SessionDataType&> (sessionData_in)));

  try {
    session_->peerConnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::peerConnect (), returning\n")));
    return;
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    //  CBData_->progressData.transferred = 0;
    CBData_->eventStack.push_back (COMMON_UI_EVENT_CONNECT);

//    guint event_source_id = g_idle_add (idle_start_UI_cb,
//                                        CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_send_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                     const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionData_.end ());

  try {
    session_->peerDisconnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::peerDisconnect (), returning\n")));
    return;
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (COMMON_UI_EVENT_DISCONNECT);

//    guint event_source_id = g_idle_add (idle_end_UI_cb,
//                                        CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_end_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope

  sessionData_.erase (iterator);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                     const MessageType& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);

  const struct BitTorrent_Record& record_r = message_in.get ();
  try {
    session_->notify (record_r,
                      ((record_r.type == BITTORRENT_MESSAGETYPE_PIECE) ? &const_cast<MessageType&> (message_in)
                                                                       : NULL));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::notify(), returning\n")));
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->progressData.transferred += message_in.total_length ();
    CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_PeerHandler_T<AddressType,
                                ConfigurationType,
                                StatisticContainerType,
                                ControlMessageType,
                                MessageType,
                                SessionMessageType,
                                SessionDataType,
                                StreamType,
                                StreamStatusType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                ConnectionStateType,
                                SessionStateType,
                                CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                     const SessionMessageType& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerHandler_T::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionData_.end ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  enum Common_UI_Event event = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      CBData_->progressData.statistic = (*iterator).second->currentStatistic;

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
  CBData_->eventStack.push_back (event);
}

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::BitTorrent_Module_TrackerHandler_T ()
 : inherited ()
 , CBData_ (NULL)
 , session_ (NULL)
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::BitTorrent_Module_TrackerHandler_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::~BitTorrent_Module_TrackerHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::~BitTorrent_Module_TrackerHandler_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
bool
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

//    {
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//      connectionIsAlive_ = false;
//    } // end lock scope

//    { // synch access to state machine
//      ACE_GUARD (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited2::stateLock_);

//      inherited2::state_ = REGISTRATION_STATE_NICK;
//    } // end lock scope

    CBData_ = NULL;
    session_ = NULL;
  } // end IF

  CBData_ = configuration_in.CBData;
  session_ = configuration_in.session;

  return inherited::initialize (configuration_in);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::handleDataMessage (MessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);

  const struct BitTorrent_Record& data_r = message_inout->get ();
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::Record2String (data_r).c_str ())));
#endif

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      if (!inherited::subscribe (this))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ISubscribe_T::subscribe(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

//      const SessionDataType& session_data_container_r =
//          message_inout->get ();
//      const typename SessionDataType::DATA_T& session_data_r =
//          session_data_container_r.get ();
//      ACE_ASSERT (session_data_r.connectionState);
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, session_data_r.connectionState->lock);
//        session_data_r.connectionState->nickName =
//            inherited::configuration_->protocolConfiguration->loginOptions.nickName;
//      } // end lock scope

//      // step1: remember connection has been opened...
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//        connectionIsAlive_ = true;

//        // signal any waiter(s)
//        result = condition_.broadcast ();
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
//      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (!inherited::unsubscribe (this))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ISubscribe_T::unsubscribe(), continuing\n"),
                    inherited::mod_->name ()));

//      // remember connection has been closed
//      {
//        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, conditionLock_);

//        connectionIsAlive_ = false;

//        // signal any waiter(s)
//        result = condition_.broadcast ();
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
//      } // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH

  inherited::handleSessionMessage (message_inout,
                                   passMessageDownstream_out);
}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//bool
//BitTorrent_Module_TrackerHandler_T::wait (const ACE_Time_Value* timeout_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::wait"));

//  int result = -1;

//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

//  result = condition_.wait (timeout_in);
//  if (result == -1)
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ETIME)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Condition::wait(): \"%m\", continuing\n")));
//  } // end IF
//  if (initialRegistration_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("registration timeout, aborting\n")));
//    return false;
//  } // end IF

//  return true;
//}

//void
//BitTorrent_Module_TrackerHandler_T::subscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::subscribe"));

//  // sanity check(s)
//  ACE_ASSERT (interfaceHandle_in);

//  // synch access to subscribers
//  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

//  subscribers_.push_back (interfaceHandle_in);
//}

//void
//BitTorrent_Module_TrackerHandler_T::unsubscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::unsubscribe"));

//  // sanity check(s)
//  ACE_ASSERT (interfaceHandle_in);

//  // synch access to subscribers
//  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

//  SubscribersIterator_t iterator = subscribers_.begin ();
//  for (;
//       iterator != subscribers_.end ();
//       iterator++)
//    if ((*iterator) == interfaceHandle_in)
//      break;

//  if (iterator != subscribers_.end ())
//    subscribers_.erase (iterator);
//  else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid argument (was: %@), aborting\n"),
//                interfaceHandle_in));
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//void
//BitTorrent_Module_TrackerHandler_T::onChange (IRC_RegistrationState newState_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::allocateMessage"));

//  int result = -1;

//  if (newState_in == REGISTRATION_STATE_FINISHED)
//  {
//    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

//    result = condition_.broadcast ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Condition::broadcast(): \"%m\", continuing\n")));
//  } // end IF
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//IRC_Message*
//BitTorrent_Module_TrackerHandler_T::allocateMessage (unsigned int requestedSize_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::allocateMessage"));

//  // initialize return value(s)
//  IRC_Message* message_p = NULL;

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);

//  if (inherited::configuration_->streamConfiguration->messageAllocator)
//  {
//allocate:
//    try {
//      message_p =
//        static_cast<IRC_Message*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                  requestedSize_in));
//      return NULL;
//    }

//    // keep retrying ?
//    if (!message_p &&
//        !inherited::configuration_->streamConfiguration->messageAllocator->block ())
//      goto allocate;
//  } // end IF
//  else
//    ACE_NEW_NORETURN (message_p,
//                      IRC_Message (requestedSize_in));
//  if (!message_p)
//  {
//    if (inherited::configuration_->streamConfiguration->messageAllocator)
//    {
//      if (inherited::configuration_->streamConfiguration->messageAllocator->block ())
//        ACE_DEBUG ((LM_CRITICAL,
//                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//    } // end IF
//    else
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//  } // end IF

//  return message_p;
//}

//template <typename ConfigurationType,
//          typename ControlMessageType,
//          typename SessionDataType>
//IRC_Record*
//BitTorrent_Module_TrackerHandler_T::allocateMessage (IRC_Record::CommandType type_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::allocateMessage"));

//  // initialize return value(s)
//  IRC_Record* message_p = NULL;

//  ACE_NEW_NORETURN (message_p,
//                    IRC_Record ());
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return NULL;
//  } // end IF
//  ACE_NEW_NORETURN (message_p->command_.string,
//                    std::string (IRC_Message::CommandType2String (type_in)));
//  if (!message_p->command_.string)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

//    // clean up
//    message_p->decrease ();

//    return NULL;
//  } // end IF
//  message_p->command_.discriminator = IRC_Record::Command::STRING;

//  return message_p;
//}

//void
//BitTorrent_Module_TrackerHandler_T::sendMessage (IRC_Record*& record_inout)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::sendMessage"));

//  int result = -1;

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//  ACE_ASSERT (record_inout);

//  // step1: get a message buffer
//  IRC_Message* message_p =
//      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to BitTorrent_Module_TrackerHandler_T::allocateMessage(%u), returning\n"),
//                inherited::configuration_->streamConfiguration->bufferSize));

//    // clean up
//    record_inout->decrease ();
//    record_inout = NULL;

//    return;
//  } // end IF

//  // step2: attach the command
//  // *NOTE*: message assumes control over command_in
//  message_p->initialize (*record_inout,
//                         NULL);
//  // --> bye bye...
//  record_inout = NULL;

//  // step3: send it upstream

//  // *NOTE*: there is NO way to prevent asynchronous closure of the connection;
//  //         this protect against closure of the stream while the message is
//  //         propagated... (see line 614)
//  //         --> grab lock and check connectionIsAlive_
//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);
//  // sanity check
//  if (!connectionIsAlive_)
//  {
//    //ACE_DEBUG ((LM_DEBUG,
//    //            ACE_TEXT ("connection has been closed/lost - cannot send message, returning\n")));

//    // clean up
//    message_p->release ();

//    return;
//  } // end IF

//  result = inherited::reply (message_p, NULL);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

//    // clean up
//    message_p->release ();

//    return;
//  } // end IF

////   ACE_DEBUG((LM_DEBUG,
////              ACE_TEXT("pushed message...\n")));
//}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::clone"));

  // initialize return value(s)
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    OWN_TYPE_T ());
  if (!task_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
  else
  {
    inherited* inherited_p = dynamic_cast<inherited*> (task_p);
    ACE_ASSERT (task_p);
    inherited_p->initialize (inherited::subscribers_,
                             inherited::lock_);
  } // end ELSE

  return task_p;
}

//----------------------------------------

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::start (Stream_SessionId_t sessionID_in,
                                                       const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator == sessionData_.end ());

  sessionData_.insert (std::make_pair (sessionID_in,
                                       &const_cast<SessionDataType&> (sessionData_in)));

  try {
    session_->peerConnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::peerConnect (), returning\n")));
    return;
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    //  CBData_->progressData.transferred = 0;
    CBData_->eventStack.push_back (COMMON_UI_EVENT_CONNECT);

//    guint event_source_id = g_idle_add (idle_start_UI_cb,
//                                        CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_send_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                        const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionData_.end ());

  try {
    session_->peerDisconnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::peerDisconnect (), returning\n")));
    return;
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (COMMON_UI_EVENT_DISCONNECT);

//    guint event_source_id = g_idle_add (idle_end_UI_cb,
//                                        CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_end_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope

  sessionData_.erase (iterator);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                        const MessageType& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (session_);

  const struct BitTorrent_Record& record_r = message_in.get ();
  try {
    session_->notify (record_r,
                      ((record_r.type == BITTORRENT_MESSAGETYPE_PIECE) ? &const_cast<MessageType&> (message_in)
                                                                       : NULL));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_ISession_T::notify(), returning\n")));
  }

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->progressData.transferred += message_in.total_length ();
    CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
void
BitTorrent_Module_TrackerHandler_T<AddressType,
                                   ConfigurationType,
                                   StatisticContainerType,
                                   ControlMessageType,
                                   MessageType,
                                   SessionMessageType,
                                   SessionDataType,
                                   StreamType,
                                   StreamStatusType,
                                   SocketConfigurationType,
                                   HandlerConfigurationType,
                                   ConnectionStateType,
                                   SessionStateType,
                                   CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                        const SessionMessageType& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerHandler_T::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionData_.end ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  enum Common_UI_Event event = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      CBData_->progressData.statistic = (*iterator).second->currentStatistic;

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
  CBData_->eventStack.push_back (event);
}
