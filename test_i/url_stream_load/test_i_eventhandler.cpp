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

#include "test_i_eventhandler.h"

#if defined (RAPIDJSON_SUPPORT)
#include "document.h"
#endif // RAPIDJSON_SUPPORT

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include <regex>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_common.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "stream_session_message_base.h"
#include "stream_tools.h"

#include "stream_html_tools.h"

#include "net_macros.h"

#include "test_i_url_stream_load_defines.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT

//////////////////////////////////////////

Test_I_EventHandler::Test_I_EventHandler (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& sessionEvent_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  CBData_->progressData.transferred += message_in.total_length ();
  CBData_->progressData.statistic.bytes += message_in.total_length ();

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GTK_USE)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

      guint event_source_id = g_idle_add (idle_end_session_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

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
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}

//////////////////////////////////////////

Test_I_EventHandler_1b::Test_I_EventHandler_1b (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::Test_I_EventHandler_1b"));

}

void
Test_I_EventHandler_1b::start (Stream_SessionId_t sessionId_in,
                               const struct Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  //  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const enum Stream_SessionMessageType& sessionEvent_in,
                                bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_1b::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  CBData_->progressData.transferred += message_in.total_length ();
  CBData_->progressData.statistic.bytes += message_in.total_length ();

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GTK_USE)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

      guint event_source_id = g_idle_add (idle_end_session_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

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
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}

//////////////////////////////////////////

Test_I_EventHandler_2::Test_I_EventHandler_2 (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::Test_I_EventHandler_2"));

}

void
Test_I_EventHandler_2::start (Stream_SessionId_t sessionId_in,
                              const struct Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  //  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const enum Stream_SessionMessageType& sessionEvent_in,
                               bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_2::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  CBData_->progressData.transferred += message_in.total_length ();
  CBData_->progressData.statistic.bytes += message_in.total_length ();

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_FINISHED;
      break;
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
      event_e = COMMON_UI_EVENT_STEP;
      break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

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
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}
