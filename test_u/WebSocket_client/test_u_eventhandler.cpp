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

#include "test_u_eventhandler.h"

//#include "libxml/xpath.h"
//#include "libxml/xpathInternals.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "net_macros.h"

#include "http_common.h"

#if defined (GTK_SUPPORT)
#include "test_u_gtk_callbacks.h"
#endif // GTK_SUPPORT
#include "test_u_defines.h"

#include "test_u_ws_client_defines.h"

Test_U_EventHandler::Test_U_EventHandler (struct WebSocket_Client_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct WebSocket_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE

  sessionData_ = &const_cast<struct WebSocket_Client_SessionData&> (sessionData_in);

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  guint event_source_id_i = g_idle_add (idle_start_session_cb,
                                        CBData_);
  if (event_source_id_i)
    CBData_->UIState->eventSourceIds.insert (event_source_id_i);
#endif // GTK_USE

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Stream_SessionMessageType& sessionEvent_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  guint event_source_id_i = g_idle_add (idle_end_session_cb,
                                        CBData_);
  if (event_source_id_i)
    CBData_->UIState->eventSourceIds.insert (event_source_id_i);
#endif // GTK_USE

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
#endif // GTK_USE

  sessionData_ = NULL;
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE

  WebSocket_Client_MessageData_t& data_r =
    const_cast<WebSocket_Client_MessageData_t&> (message_in.getR ());
  struct WebSocket_Client_MessageData& record_r =
    const_cast<struct WebSocket_Client_MessageData&> (data_r.getR ());

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  switch (record_r.opcode)
  {
    case WebSocket_Codes::OPCODE_TEXT:
    {
      ACE_Message_Block* message_block_p =
        &const_cast<Test_U_Message&> (message_in);
      CBData_->message = Stream_Tools::toString (message_block_p);

      guint event_source_id_i = g_idle_add (idle_message_received_cb,
                                            CBData_);
      if (event_source_id_i)
        CBData_->UIState->eventSourceIds.insert (event_source_id_i);

      break;
    }
    case WebSocket_Codes::OPCODE_CLOSE:
    {
      CBData_->message = ACE_TEXT_ALWAYS_CHAR ("Close");
      guint event_source_id_i = g_idle_add (idle_message_received_cb,
                                            CBData_);
      if (event_source_id_i)
        CBData_->UIState->eventSourceIds.insert (event_source_id_i);

      break;
    }
    case WebSocket_Codes::OPCODE_PONG:
    {
      CBData_->message = ACE_TEXT_ALWAYS_CHAR ("Pong");
      guint event_source_id_i = g_idle_add (idle_message_received_cb,
                                            CBData_);
      if (event_source_id_i)
        CBData_->UIState->eventSourceIds.insert (event_source_id_i);

      break;
    }
    default:
      break;
  } // end SWITCH

#endif // GTK_USE
  CBData_->progressData.transferred += message_in.total_length ();
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
#endif // GTK_USE
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_CONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_DISCONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    { ACE_ASSERT (sessionData_);
      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      CBData_->progressData.statistic = sessionData_->statistic;
#endif // GTK_USE

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STATISTIC;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STEP;
#endif // GTK_USE
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
}
