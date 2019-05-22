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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "net_macros.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_u_stream.h"

Test_U_EventHandler::Test_U_EventHandler (
#if defined (GUI_SUPPORT)
                                          struct FileServer_UI_CBData* CBData_in
#endif // GUI_SUPPORT
                                         )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
#else
 : sessionData_ (NULL)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct FileServer_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  sessionData_ =
    &const_cast<struct FileServer_SessionData&> (sessionData_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
  ACE_ASSERT (CBData_->UIState);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_session_start_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_session_start_cb): \"%m\", returning\n")));
    return;
  } // end IF
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    //CBData_->eventSourceIds.insert (event_source_id);
    CBData_->UIState->eventStack.push (COMMON_UI_EVENT_CONNECT);
  } // end lock scope
#endif // GUI_SUPPORT
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& sessionEvent_in)
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

  ACE_UNUSED_ARG (sessionId_in);

  sessionData_ = NULL;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
  ACE_ASSERT (CBData_->UIState);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (COMMON_UI_EVENT_DISCONNECT);
  } // end lock scope
#endif // GUI_SUPPORT
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
  ACE_ASSERT (CBData_->UIState);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    //CBData_->progressData.statistic.bytes += message_in.total_length ();
    CBData_->UIState->eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GUI_SUPPORT
}
void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
  ACE_ASSERT (CBData_->UIState);
#endif // GUI_SUPPORT

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT;
      return;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
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

#if defined (GUI_SUPPORT)
      CBData_->progressData.statistic.streamStatistic = sessionData_->statistic;
#endif // GUI_SUPPORT

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
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (event_e);
  } // end lock scope
#endif // GUI_SUPPORT
}
