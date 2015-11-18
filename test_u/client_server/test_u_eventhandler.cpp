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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "net_macros.h"

#include "test_u_common.h"

Net_EventHandler::Net_EventHandler (Net_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::Net_EventHandler"));

}

Net_EventHandler::~Net_EventHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::~Net_EventHandler"));

}

void
Net_EventHandler::start (const Net_StreamSessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::start"));

  ACE_UNUSED_ARG (sessionData_in);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (NET_GTKEVENT_CONNECT);
}

void
Net_EventHandler::notify (const Net_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::notify"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (NET_GTKEVENT_DATA);
}
void
Net_EventHandler::notify (const Net_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::notify"));

  Net_GTK_Event event =
    ((sessionMessage_in.type () == STREAM_SESSION_STATISTIC) ? NET_GTKEVENT_STATISTIC
                                                             : NET_GKTEVENT_INVALID);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (event);
}

void
Net_EventHandler::end ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_EventHandler::end"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (NET_GTKEVENT_DISCONNECT);
}
