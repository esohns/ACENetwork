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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_macros.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

template <typename CallbackDataType>
Test_U_EventHandler_T<CallbackDataType>::Test_U_EventHandler_T (
#if defined (GUI_SUPPORT)
                                                                CallbackDataType* CBData_in
#endif // GUI_SUPPORT
                                                               )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_T::Test_U_EventHandler_T"));

}

template <typename CallbackDataType>
void
Test_U_EventHandler_T<CallbackDataType>::start (Stream_SessionId_t sessionId_in,
                                                const struct Test_U_StreamSessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (!event_source_id)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_CONNECT);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename CallbackDataType>
void
Test_U_EventHandler_T<CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                 const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename CallbackDataType>
void
Test_U_EventHandler_T<CallbackDataType>::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  //sessionData_ = NULL;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (!event_source_id)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
#endif // GTK_USE

#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    //state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename CallbackDataType>
void
Test_U_EventHandler_T<CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                 const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (message_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename CallbackDataType>
void
Test_U_EventHandler_T<CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                 const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT
  const Test_U_StreamSessionData_t& session_data_container_r =
    sessionMessage_in.getR ();
  const struct Test_U_StreamSessionData& session_data_r =
    session_data_container_r.getR ();

#if defined (GUI_SUPPORT)
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
#endif // GUI_SUPPORT
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
#if defined (GUI_SUPPORT)
      //event_e = COMMON_UI_EVENT_CONNECT;
#endif // GUI_SUPPORT
      break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
#if defined (GUI_SUPPORT)
      event_e = COMMON_UI_EVENT_DISCONNECT;
#endif // GUI_SUPPORT
      break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
#if defined (GUI_SUPPORT)
      event_e = COMMON_UI_EVENT_STATISTIC;
      CBData_->progressData.statistic.streamStatistic = session_data_r.statistic;
#endif // GUI_SUPPORT
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}
