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

#include <ace/Guard_T.h>
#include <ace/Log_Msg.h>
#include <ace/Synch_Traits.h>

#include "net_macros.h"

#include "bittorrent_message.h"
#include "bittorrent_sessionmessage.h"

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::BitTorrent_PeerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                            CBDataType* CBData_in)
 : CBData_ (CBData_in)
 , session_ (interfaceHandle_in)
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::BitTorrent_PeerStreamHandler_T"));

  // sanity check(s)
  ACE_ASSERT (session_);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::~BitTorrent_PeerStreamHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::~BitTorrent_PeerStreamHandler_T"));

}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::start (Stream_SessionId_t sessionID_in,
                                                   const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::start"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (iterator == sessionData_.end ());
  ACE_ASSERT (session_);

  sessionData_.insert (std::make_pair (sessionID_in,
                                       &const_cast<SessionDataType&> (sessionData_in)));

  try {
    session_->connect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::connect(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    //  CBData_->progressData.transferred = 0;
    CBData_->eventStack.push_back (COMMON_UI_EVENT_STARTED);

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

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                    const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::end"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());
  ACE_ASSERT (session_);

  sessionData_.erase (iterator);

  try {
    session_->disconnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::disconnect(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (COMMON_UI_EVENT_FINISHED);

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
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                    const BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                                               UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (session_);

  const typename BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                      UserDataType>::DATA_T& data_container_r =
      message_in.get ();
  const struct BitTorrent_Record& record_r = data_container_r.get ();
  try {
    session_->notify (record_r,
                      (record_r.type == BITTORRENT_MESSAGETYPE_PIECE) ? &const_cast<BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                                                                         UserDataType>&> (message_in)
                                                                      : NULL);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::notify(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->progressData.transferred += message_in.total_length ();
    CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType,
                               CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                    const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                      UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  int result = -1;

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    Common_UI_Event event = COMMON_UI_EVENT_INVALID;
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
  } // end IF
}

//////////////////////////////////////////

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::BitTorrent_TrackerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                                  CBDataType* CBData_in)
 : CBData_ (CBData_in)
 , session_ (interfaceHandle_in)
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::BitTorrent_TrackerStreamHandler_T"));

  // sanity check(s)
  ACE_ASSERT (session_);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::~BitTorrent_TrackerStreamHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::~BitTorrent_TrackerStreamHandler_T"));

}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::start (Stream_SessionId_t sessionID_in,
                                                      const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::start"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (iterator == sessionData_.end ());
  ACE_ASSERT (session_);

  sessionData_.insert (std::make_pair (sessionID_in,
                                       &const_cast<SessionDataType&> (sessionData_in)));

  try {
    session_->trackerConnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::trackerConnect(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    //  CBData_->progressData.transferred = 0;
    CBData_->eventStack.push_back (COMMON_UI_EVENT_STARTED);

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

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                       const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::end"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());
  ACE_ASSERT (session_);

  sessionData_.erase (iterator);

  try {
    session_->trackerDisconnect (sessionID_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::trackerDisconnect(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (COMMON_UI_EVENT_FINISHED);

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
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                       const BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                                                                         UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (session_);

  const typename BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                             UserDataType>::DATA_T& data_container_r =
      message_in.get ();
  const struct HTTP_Record& record_r = data_container_r.get ();
  try {
    session_->notify (record_r);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::notify(), continuing\n")));
  }

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->progressData.transferred += message_in.total_length ();
    CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType,
                                  CBDataType>::notify (Stream_SessionId_t sessionID_in,
                                                       const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                         UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionID_in);
  int result = -1;

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());

  if (CBData_)
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    Common_UI_Event event = COMMON_UI_EVENT_INVALID;
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
  } // end IF
}
