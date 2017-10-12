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
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "net_macros.h"

#include "test_u_stream.h"
#include "test_u_callbacks.h"

Test_U_EventHandler::Test_U_EventHandler (struct Test_U_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_U_StreamSessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push_back (TEST_U_GTKEVENT_CONNECT);
  } // end lock scope
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

  guint event_source_id = g_idle_add (idle_end_session_client_cb,
                                      CBData_);
  if (!event_source_id)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_client_cb): \"%m\", returning\n")));
    return;
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (message_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push_back (TEST_U_GTKEVENT_DATA);
  } // end lock scope
}
void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  enum Test_U_GTK_Event event_e = TEST_U_GTKEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    //case STREAM_SESSION_MESSAGE_CONNECT:
    //  return;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = TEST_U_GTKEVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
      event_e = TEST_U_GTKEVENT_STATISTIC; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push_back (event_e);
  } // end lock scope
}
