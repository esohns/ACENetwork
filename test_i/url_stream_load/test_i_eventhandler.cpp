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
#include "test_i_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "stream_session_message_base.h"

#include "net_macros.h"

#include "gtk/gtk.h"

#include "test_i_callbacks.h"
#include "test_i_defines.h"

Test_I_EventHandler::Test_I_EventHandler (Test_I_URLStreamLoad_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

}

Test_I_EventHandler::~Test_I_EventHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::~Test_I_EventHandler"));

}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionID_in,
                            const Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionID_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionID_in,
                                          &const_cast<Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

//  CBData_->progressData.transferred = 0;
  CBData_->eventStack.push_back (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);

  sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->progressData.transferred += message_in.total_length ();
  CBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
}
void
Test_I_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionID_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  enum Common_UI_Event event = COMMON_UI_EVENT_INVALID;
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
}
