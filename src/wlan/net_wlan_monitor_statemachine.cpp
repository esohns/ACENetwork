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

#include "net_wlan_defines.h"

const char network_wlan_statemachine_monitor_name_string_[] =
    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_STATEMACHINE_MONITOR_NAME);

Net_WLAN_MonitorStateMachine::Net_WLAN_MonitorStateMachine ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 : inherited (&lock_,                         // lock handle
              NET_WLAN_MONITOR_STATE_INVALID) // (initial) state
#else
 : inherited (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_THREAD_NAME), // thread name
              NET_WLAN_MONITOR_THREAD_GROUP_ID,                    // group id
              &lock_,                                              // lock handle
              NET_WLAN_MONITOR_STATE_INVALID)                      // (initial) state
 , dispatchStarted_ (false)
#endif // ACE_WIN32 || ACE_WIN64
  , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::Net_WLAN_MonitorStateMachine"));

}

bool
Net_WLAN_MonitorStateMachine::change (enum Net_WLAN_MonitorState newState_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_MonitorStateMachine::change"));

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);

  // synchronize access to state machine
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_GUARD_RETURN (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited::stateLock_, false);
#else
  // *NOTE*: the state machine is asynchronous; idle first
  inherited::idle ();

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *inherited::stateLock_, false);
#endif // ACE_WIN32 || ACE_WIN64

  switch (inherited::state_)
  {
    case NET_WLAN_MONITOR_STATE_INVALID:
    {
      switch (newState_in)
      {
        //case NET_WLAN_MONITOR_STATE_INVALID:      // initialization failed
        // good case
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // initialized successfully
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_IDLE:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:
        case NET_WLAN_MONITOR_STATE_SCAN:         // not configured || configured SSID unknown (i.e. not cached yet || auto-associate disabled)
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:    // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_SCANNED:      // (background) scan has completed
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:   // already connected (initially)
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        //////////////////////////////////
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        // *NOTE*: these transitions is necessary due to the usage of a timer
        //         and the fact that the Wlan API dispatches events from
        //         multiple threads
        case NET_WLAN_MONITOR_STATE_SCAN:
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_SCANNED:      // scan completed
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:         // association failed (gave up)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:    // association failed (retrying)
#endif // ACE_WIN32 || ACE_WIN64
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:   // association succeeded
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:         // not configured || configured SSID unknown (i.e. not cached)
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_SCANNED:      // configured && configured SSID known (i.e. cached)
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_CONNECT:      // connection failed (e.g. failed to obtain DHCP lease) (retrying)
#endif // ACE_WIN32 || ACE_WIN64
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:   // connection failed (e.g. failed to obtain DHCP lease) (gave up)
        case NET_WLAN_MONITOR_STATE_CONNECTED:    // connection succeeded (e.g. DHCP lease obtained)
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE: // disconnect completed (e.g. DHCP lease relinquished)
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    //////////////////////////////////////
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:         // monitor started (!) && not connected
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // re-initialized
        case NET_WLAN_MONITOR_STATE_CONNECTED:    // monitor started (!) && already connected (--> monitor link activity)
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case NET_WLAN_MONITOR_STATE_SCANNED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:         // ELSE
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:    // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
        //////////////////////////////////
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_SCAN:         // monitor link quality
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:      // (background) scan has completed
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:   // association completed ||
                                                  // already connected (monitoring link quality)
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_CONNECT:      // association succeeded
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE: // disassociate after disconnect completed
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
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_SCAN:         // monitor link quality
        case NET_WLAN_MONITOR_STATE_DISCONNECT:   // (event/user intervention || (configured && not connected to configured SSID))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_SCANNED:
#else
        case NET_WLAN_MONITOR_STATE_IDLE:
#endif // ACE_WIN32 || ACE_WIN64
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
    case NET_WLAN_MONITOR_STATE_IDLE:
      result = ACE_TEXT_ALWAYS_CHAR ("IDLE"); break;
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
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIALIZED"); break;
    case NET_WLAN_MONITOR_STATE_SCANNED:
      result = ACE_TEXT_ALWAYS_CHAR ("SCANNED"); break;
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
