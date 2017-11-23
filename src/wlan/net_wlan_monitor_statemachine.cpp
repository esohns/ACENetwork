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
#include "net_wlan_monitor_statemachine.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "net_macros.h"

Net_WLAN_MonitorStateMachine::Net_WLAN_MonitorStateMachine ()
 : inherited (&lock_,                         // lock handle
              NET_WLAN_MONITOR_STATE_INVALID) // (initial) state
 , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::Net_WLAN_MonitorStateMachine"));

}

void
Net_WLAN_MonitorStateMachine::initialize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::initialize"));

  if (!change (NET_WLAN_MONITOR_STATE_INVALID))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_MonitorStateMachine::change(NET_WLAN_MONITOR_STATE_INVALID), continuing\n")));
}

bool
Net_WLAN_MonitorStateMachine::change (enum Net_WLAN_MonitorState newState_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::change"));

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);

  // synchronize access to state machine
  ACE_GUARD_RETURN (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited::stateLock_, false);

  switch (inherited::state_)
  {
    case NET_WLAN_MONITOR_STATE_INVALID:
    {
      switch (newState_in)
      {
        // good case
        case NET_WLAN_MONITOR_STATE_INITIAL:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_INITIAL:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        // good case
        case NET_WLAN_MONITOR_STATE_SCAN:
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_SCAN:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        // good case
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        case NET_WLAN_MONITOR_STATE_SCAN:
        // good case
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECT:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        case NET_WLAN_MONITOR_STATE_SCAN:
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
        // good case
        case NET_WLAN_MONITOR_STATE_CONNECTED:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        case NET_WLAN_MONITOR_STATE_SCAN:
        case NET_WLAN_MONITOR_STATE_CONNECTED:
        // good case
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
        // good case
        case NET_WLAN_MONITOR_STATE_SCAN:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        // good case
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
        case NET_WLAN_MONITOR_STATE_CONNECT:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECTED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:
        // good case
        case NET_WLAN_MONITOR_STATE_DISCONNECT:
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, aborting\n"),
              ACE_TEXT (stateToString (inherited::state_).c_str ()),
              ACE_TEXT (stateToString (newState_in).c_str ())));

  return false;
}

std::string
Net_WLAN_MonitorStateMachine::stateToString (enum Net_WLAN_MonitorState state_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::stateToString"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (state_in)
  {
    case NET_WLAN_MONITOR_STATE_INVALID:
      break;
    case NET_WLAN_MONITOR_STATE_INITIAL:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIAL"); break;
    case NET_WLAN_MONITOR_STATE_SCAN:
      result = ACE_TEXT_ALWAYS_CHAR ("SCAN"); break;
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
      result = ACE_TEXT_ALWAYS_CHAR ("ASSOCIATE"); break;
    case NET_WLAN_MONITOR_STATE_CONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECT"); break;
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("DISCONNECT"); break;
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
      result = ACE_TEXT_ALWAYS_CHAR ("DISASSOCIATE"); break;
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
      result = ACE_TEXT_ALWAYS_CHAR ("ASSOCIATED"); break;
    case NET_WLAN_MONITOR_STATE_CONNECTED:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECTED"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}
