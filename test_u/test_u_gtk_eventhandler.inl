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

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "stream_macros.h"

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::Test_U_GTK_EventHandler_T (CallbackDataType* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::Test_U_GTK_EventHandler_T"));

}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::start (SessionIdType sessionId_in,
                                                    const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push_back (COMMON_UI_EVENT_STARTED);
  } // end lock scope
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const SessionEventType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::end (SessionIdType sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  guint event_source_id = 0;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    event_source_id = g_idle_add (idle_session_end_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
    else
      CBData_->eventSourceIds.insert (event_source_id);
    CBData_->eventStack.push_back (COMMON_UI_EVENT_STOPPED);
  } // end lock scope

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const MessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  guint event_source_id = 0;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    event_source_id = g_idle_add (idle_data_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_data_cb): \"%m\", continuing\n")));
    //else
      //  CBData_->eventSourceIds.insert (event_source_id);
    CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_U_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_GTK_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
//      float current_bytes = 0.0F;

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      // *NOTE*: the byte counter is more current than what is received here
      //         (see above) --> do not update
      //current_bytes = CBData_->progressData.statistic.bytes;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
        CBData_->progressData.statistic = sessionData_->statistic;
      } // end lock scope
      //CBData_->progressData.statistic.bytes = current_bytes;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push (event_e);
  } // end lock scope
}
