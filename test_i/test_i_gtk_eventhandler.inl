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

#include "common_ui_common.h"

#include "stream_common.h"
#include "stream_macros.h"

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::Test_I_GTK_EventHandler_T (struct Common_UI_GTK_EventConfiguration* configuration_in,
                                                                        CallbackDataType* CBData_in)
 : inherited (configuration_in,
              CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::Test_I_GTK_EventHandler_T"));

}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::start (SessionIdType sessionId_in,
                                                    const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (inherited::CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::CBData_->lock);
    inherited::CBData_->eventStack.push_back (COMMON_UI_EVENT_STARTED);
  } // end lock scope
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const SessionEventType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::notify"));

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
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::end (SessionIdType sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (inherited::CBData_);
  ACE_ASSERT (inherited::hooks_);

  guint event_source_id = 0;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::CBData_->lock);
    if (inherited::hooks_->finiHook)
    {
      event_source_id = g_idle_add (inherited::hooks_->finiHook,
                                    inherited::CBData_);
      if (!event_source_id)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(finiHook): \"%m\", continuing\n")));
      else
        inherited::CBData_->eventSourceIds.insert (event_source_id);
    } // end IF
    inherited::CBData_->eventStack.push_back (COMMON_UI_EVENT_STOPPED);
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
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const MessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (inherited::CBData_);
  ACE_ASSERT (inherited::hooks_);

  guint event_source_id = 0;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::CBData_->lock);
    if (inherited::hooks_->dataHook)
    {
      event_source_id = g_idle_add (inherited::hooks_->dataHook,
                                    inherited::CBData_);
      if (!event_source_id)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(dataHook): \"%m\", continuing\n")));
      //else
      //  inherited::CBData_->eventSourceIds.insert (event_source_id);
    } // end IF
    inherited::CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_GTK_EventHandler_T<SessionIdType,
                          SessionDataType,
                          SessionEventType,
                          MessageType,
                          SessionMessageType,
                          CallbackDataType>::notify (SessionIdType sessionId_in,
                                                    const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_GTK_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (inherited::CBData_);

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
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::CBData_->lock);
        inherited::CBData_->progressData.statistic = sessionData_->statistic;
      } // end lock scope
      //inherited::CBData_->progressData.statistic.bytes = current_bytes;

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

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::CBData_->lock);
    inherited::CBData_->eventStack.push (event_e);
  } // end lock scope
}
