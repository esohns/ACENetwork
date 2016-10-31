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

#include <ace/Guard_T.h>
#include <ace/Log_Msg.h>
#include <ace/Synch_Traits.h>

#include "net_macros.h"

#include "test_u_stream.h"

Test_U_EventHandler::Test_U_EventHandler (Test_U_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

Test_U_EventHandler::~Test_U_EventHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::~Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionID_in,
                            const Test_U_StreamSessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionData_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_CONNECT);
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::end"));

  ACE_UNUSED_ARG (sessionID_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_DISCONNECT);
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_DATA);
}
void
Test_U_EventHandler::notify (Stream_SessionId_t sessionID_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  Test_U_GTK_Event event =
    ((sessionMessage_in.type () == STREAM_SESSION_MESSAGE_STATISTIC) ? TEST_U_GTKEVENT_STATISTIC
                                                                     : TEST_U_GTKEVENT_INVALID);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (event);
}
