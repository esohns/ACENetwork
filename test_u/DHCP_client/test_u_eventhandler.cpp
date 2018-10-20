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

#include "ace/Synch.h"
#include "test_u_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "stream_session_message_base.h"

#include "net_macros.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_u_defines.h"

#if defined (GUI_SUPPORT)
Test_U_EventHandler::Test_U_EventHandler (struct DHCPClient_UI_CBData* CBData_in)
#else
Test_U_EventHandler::Test_U_EventHandler ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
#else
 : sessionDataMap_ ()
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct DHCPClient_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->UIState);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct DHCPClient_SessionData&> (sessionData_in)));

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_UI_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_send_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->UIState);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);

  guint event_source_id = g_idle_add (idle_end_UI_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionDataMap_.erase (iterator);
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->UIState);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);

  CBData_->progressData.transferred += message_in.total_length ();
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GUI_SUPPORT
}
void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->UIState);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ());
#endif // GTK_USE
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
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

      CBData_->progressData.statistic = (*iterator).second->statistic;

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
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
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}
