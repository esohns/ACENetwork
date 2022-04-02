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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_macros.h"

#include "test_i_av_stream_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
#if defined (GUI_SUPPORT)
                             CallbackDataType>::Test_I_AVStream_Client_EventHandler_T (CallbackDataType* CBData_in)
#else
                             CallbackDataType>::Test_I_AVStream_Client_EventHandler_T ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
#else
 : sessionData_ (NULL)
#endif // GUI_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::Test_I_AVStream_Client_EventHandler_T"));

}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::start (Stream_SessionId_t sessionId_in,
                                                       const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
  ACE_ASSERT (!sessionData_);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
    CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                        const SessionEventType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    event_source_id = g_idle_add (idle_end_source_UI_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_end_source_UI_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                        const MessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_->progressData.transferred += message_in.total_length ();
    event_source_id = g_idle_add (idle_update_video_display_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_video_display_cb): \"%m\", continuing\n")));
    //else
      //  CBData_->eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_AVStream_Client_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                        const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      guint event_source_id = 0;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
        event_source_id = g_idle_add (idle_end_source_UI_cb,
                                      CBData_);
        if (!event_source_id)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add(idle_end_source_UI_cb): \"%m\", continuing\n")));
        else
          state_r.eventSourceIds.insert (event_source_id);
        state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
      } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
    
      event_e = COMMON_UI_EVENT_ABORT;

      break;
    }
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT
        CBData_->progressData.statistic = sessionData_->statistic;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}
