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

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_string_tools.h"

#include "common_ui_common.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "stream_session_message_base.h"

#include "net_macros.h"

#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT

Test_I_EventHandler::Test_I_EventHandler (struct Test_I_IceCastClient_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , isFirst_ (true)
 , sessionDataMap_ ()
 , sessionDataMap2_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_I_IceCastClient_SessionData& sessionData_in)
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

#if defined (_DEBUG)
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());
#endif // _DEBUG
  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_IceCastClient_SessionData&> (sessionData_in)));

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

//#if defined (GTK_USE)
//  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
//
//  guint event_source_id = g_idle_add (idle_end_session_cb,
//                                      CBData_);
//  if (event_source_id == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
//    return;
//  } // end IF
//  state_r.eventSourceIds.insert (event_source_id);
//#endif // GTK_USE

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
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE

  if (unlikely (isFirst_))
  { isFirst_ = false;
    Test_I_MessageDataContainer& data_container_r =
      const_cast<Test_I_MessageDataContainer&> (message_in.getR ());
    struct Test_I_IceCastClient_MessageData& data_r =
      const_cast<struct Test_I_IceCastClient_MessageData&> (data_container_r.getR ());
    if (!data_r.M3UPlaylist.ext_inf_elements.empty ())
    { isFirst_ = true; // prepare for second session
      const struct M3U_ExtInf_Element& element_r =
        data_r.M3UPlaylist.ext_inf_elements.front ();

      std::string request_protocol_string =
        Net_Common_Tools::URLToProtocol (CBData_->URL);
      std::string request_2_protocol_string =
        Net_Common_Tools::URLToProtocol (element_r.URL);
      std::string URL_string = element_r.URL;
      if (request_protocol_string != request_2_protocol_string)
      { // *NOTE*: probably misconfigured server...
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("request protocol \"%s\" does not match protocol of the response URL: \"%s\", patching response URL...\n"),
                    ACE_TEXT (request_protocol_string.c_str ()),
                    ACE_TEXT (request_2_protocol_string.c_str ())));
        URL_string.insert (4, 1, 's');
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("loading \"%s\"...\n"),
                  ACE_TEXT (URL_string.c_str ())));
      CBData_->URL = URL_string;

#if defined (GTK_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
        guint event_source_id =
          //g_idle_add (idle_load_segment_cb,
          g_timeout_add (0,
                         idle_load_segment_cb,
                         CBData_);
        if (event_source_id == 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add(idle_load_segment_cb): \"%m\", returning\n")));
          return;
        } // end IF
        state_r.eventSourceIds.insert (event_source_id);
      } // end lock scope
#endif // GTK_USE
    } // end IF
  } // end IF
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
    case STREAM_SESSION_MESSAGE_END:
    {
//#if defined (GTK_USE)
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//
//      guint event_source_id = g_idle_add (idle_end_session_cb,
//                                          CBData_);
//      if (event_source_id == 0)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
//        return;
//      } // end IF
//      state_r.eventSourceIds.insert (event_source_id);
//#endif // GTK_USE

      event_e =
        (sessionMessage_in.type () == STREAM_SESSION_MESSAGE_END) ? COMMON_UI_EVENT_FINISHED
                                                                  : COMMON_UI_EVENT_ABORT;
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
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
}

//////////////////////////////////////////

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_I_IceCastClient_SessionData_2& sessionData_in)
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

  SESSION_DATA_MAP_ITERATOR_2_T iterator = sessionDataMap2_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap2_.end ());

  sessionDataMap2_.insert (std::make_pair (sessionId_in,
                                           &const_cast<struct Test_I_IceCastClient_SessionData_2&> (sessionData_in)));

//  CBData_->progressData.transferred = 0;

#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

    guint event_source_id = 
      //g_idle_add (idle_start_session_cb,
      g_timeout_add (0,
                     idle_start_session_cb,
                     CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
      return;
    } // end IF
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_SessionMessage_2& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_2_T iterator = sessionDataMap2_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap2_.end ());

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
    case STREAM_SESSION_MESSAGE_END:
    {
//#if defined (GTK_USE)
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//
//      guint event_source_id = g_idle_add (idle_end_session_cb,
//                                          CBData_);
//      if (event_source_id == 0)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
//        return;
//      } // end IF
//      state_r.eventSourceIds.insert (event_source_id);
//#endif // GTK_USE

      event_e =
        (sessionMessage_in.type () == STREAM_SESSION_MESSAGE_END) ? COMMON_UI_EVENT_FINISHED
                                                                  : COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
#if defined (GTK_SUPPORT)
      CBData_->fft = NULL;
      CBData_->spectrumAnalyzerCBData.dispatch = NULL;
      CBData_->spectrumAnalyzerCBData.resizeNotification = NULL;
#endif // GTK_SUPPORT
      isFirst_ = true;

#if defined (GTK_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
        guint event_source_id = 
          //g_idle_add (idle_end_session_cb,
          g_timeout_add (0,
                         idle_end_session_cb,
                         CBData_);
        if (event_source_id == 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
          return;
        } // end IF
        state_r.eventSourceIds.insert (event_source_id);
      } // end lock scope
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
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
    case STREAM_SESSION_MESSAGE_FORMAT:
      event_e = COMMON_UI_EVENT_FORMAT;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
}
