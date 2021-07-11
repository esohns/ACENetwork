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

//#include "ace/Synch.h"
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

  enum Net_WLAN_MonitorState state_e = NET_WLAN_MONITOR_STATE_INVALID;

  // synchronize access to state machine
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_GUARD_RETURN (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited::stateLock_, false);
  state_e = inherited::state_;
#else
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *inherited::stateLock_, false);
  // *NOTE*: the state machine is asynchronous; the 'current' state may very
  //         well be at the tail end of the transitions stack
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, inherited::msg_queue_->lock (), false);
    inherited::MESSAGE_QUEUE_ITERATOR_T iterator (*inherited::msg_queue_);
    if (iterator.done ())
      state_e = inherited::state_;
    else
    {
      ACE_Message_Block* message_block_p = NULL;
      for (;
           iterator.next (message_block_p);
           iterator.advance ())
        if (!message_block_p->next ())
          break;
      ACE_ASSERT (message_block_p);
      state_e =
          static_cast<enum Net_WLAN_MonitorState> (message_block_p->msg_type ());
    } // end ELSE
  } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64

  switch (state_e)
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
        case NET_WLAN_MONITOR_STATE_INVALID:       // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:          // *TODO*: restarted ?
        case NET_WLAN_MONITOR_STATE_SCAN:          // not configured || configured SSID unknown (i.e. not cached yet || auto-associate disabled)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_CONNECT:
#elif defined (ACE_LINUX)
        case NET_WLAN_MONITOR_STATE_AUTHENTICATE:  // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
#if defined (NL80211_SUPPORT)
        // *NOTE*: the scan mechanism is asynchronous, so 'idle' gets enqueued
        //         at any stage and must therefore support any intermediate
        //         state
        case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE:
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:     // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
        case NET_WLAN_MONITOR_STATE_CONNECT:
        case NET_WLAN_MONITOR_STATE_DISCONNECT:
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:   // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:       // (background) scan has completed
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_CONNECTED:     // already connected (initially)
#elif defined (ACE_LINUX)
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED: // already connected (initially)
#if defined (NL80211_SUPPORT)
        // *NOTE*: the scan mechanism is asynchronous, so 'idle' gets enqueued
        //         at any stage and must therefore support any intermediate
        //         state
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
#endif // NL80211_SUPPORT
        case NET_WLAN_MONITOR_STATE_CONNECTED:     // already connected (initially)
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
    case NET_WLAN_MONITOR_STATE_SCAN:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_IDLE:         // scan aborted
        case NET_WLAN_MONITOR_STATE_SCAN:         // *TODO*: restarted ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
//        case NET_WLAN_MONITOR_STATE_SCAN:
        case NET_WLAN_MONITOR_STATE_CONNECT:
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATE:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:        // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:           // authentication failed (gave up)
        case NET_WLAN_MONITOR_STATE_AUTHENTICATE:   // authentication failed (retrying)
        case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE: // already authenticated with a different access point
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:    // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:        // scan completed
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED:  // authentication succeeded
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
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
        // *NOTE*: the scan mechanism is asynchronous, so 'scan' gets enqueued
        //         at any stage and must therefore support any intermediate
        //         state
        case NET_WLAN_MONITOR_STATE_SCAN:
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:    // association failed (retrying)
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE: // already associated with a different access point
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:      // scan completed
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
    case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:     // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:        // not configured || configured SSID unknown (i.e. not cached)
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED: // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:     // configured && configured SSID known (i.e. cached) || scan completed
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
        case NET_WLAN_MONITOR_STATE_INVALID:       // reset
        // good case
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED: // configured && configured SSID known (i.e. cached)
        case NET_WLAN_MONITOR_STATE_IDLE:          // not configured || configured SSID unknown (i.e. not cached)
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:   // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:       // scan completed
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_CONNECT:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:     // reset
        // good case
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_CONNECT:     // connection failed (e.g. failed to obtain DHCP lease) (retrying)
        case NET_WLAN_MONITOR_STATE_DISCONNECT:  // connected to different SSID
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
        // *NOTE*: the 'connected' notification arrives automatically, shortly
        //         after the kernel sends the 'associated' event notification.
        //         Therefore the 'idle' state change enqueued by the 'connected'
        //         event processing "overtakes" the 'connect' processing
        //         triggered by the 'associated' event
        // *TODO*: this is a mess
        case NET_WLAN_MONITOR_STATE_IDLE:
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED: // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:     // scan completed
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:  // connection failed (e.g. failed to obtain DHCP lease) (gave up)
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_CONNECTED:   // connection succeeded (e.g. DHCP lease obtained)
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_IDLE:         // disconnect completed
#else
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE: // disconnect completed (e.g. DHCP lease relinquished)
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_SCANNED:      // *TODO*: this is a bug; when the monitor is stopped, the scan timer
                                                  //         is cancelled; however, the timer heap implementation is buggy
                                                  //         and fails to cancel the timer, so this state-change happens
#endif // ACE_WIN32 || ACE_WIN64
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
        case NET_WLAN_MONITOR_STATE_INVALID:       // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:          // ELSE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_SCAN:          // monitor link quality (timer elapsed)
        case NET_WLAN_MONITOR_STATE_CONNECT:       // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
#else
        case NET_WLAN_MONITOR_STATE_AUTHENTICATE:  // not connected && (configured && configured SSID known (i.e. cached) && auto-associate enabled)
#endif // ACE_WIN32 || ACE_WIN64
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:   // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:       // (background) scan has completed
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED:
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_CONNECTED:     // association completed ||
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:        // reset
        // good case
        case NET_WLAN_MONITOR_STATE_SCAN:           // monitor link quality
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:      // authentication succeeded, proceed
        case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE: // deauthenticate after disassociation completed
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:    // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:        // (background) scan has completed
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:     // already associated
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
        // *NOTE*: the 'connected' notification arrives automatically, shortly
        //         after the kernel sends the 'associated' event notification.
        //         Therefore the 'idle' state change enqueued by the 'connected'
        //         event processing "overtakes" the 'connect' processing
        //         triggered by the 'associated' event
        // *TODO*: this is a mess
        case NET_WLAN_MONITOR_STATE_IDLE:
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_SCAN:          // monitor link quality
        case NET_WLAN_MONITOR_STATE_CONNECT:       // association succeeded, proceed
        case NET_WLAN_MONITOR_STATE_DISASSOCIATE:  // disassociate after disconnect completed
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:   // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:       // (background) scan has completed
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED: // deauthenticate after disassociation complete
        case NET_WLAN_MONITOR_STATE_CONNECTED:     // already associated (*NOTE*: automatic)
        {
          inherited::change (newState_in);
          return true;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_CONNECTED:
    {
      switch (newState_in)
      {
        case NET_WLAN_MONITOR_STATE_INVALID:      // reset
        // good case
        case NET_WLAN_MONITOR_STATE_IDLE:         //
        case NET_WLAN_MONITOR_STATE_SCAN:         // monitor link quality
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_CONNECT:
#else
        case NET_WLAN_MONITOR_STATE_CONNECT:      // *NOTE*: NL80211_CMD_CONNECT event arrives automatically after successful association
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_DISCONNECT:   // (event/user intervention || (configured && not connected to configured SSID))
        //////////////////////////////////
        case NET_WLAN_MONITOR_STATE_INITIALIZED:  // monitor stopped
        case NET_WLAN_MONITOR_STATE_SCANNED:      // (background) scan has completed
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
              ACE_TEXT (stateToString (state_e).c_str ()),
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATE:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATE"); break;
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
      result = ACE_TEXT_ALWAYS_CHAR ("ASSOCIATE"); break;
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_CONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECT"); break;
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("DISCONNECT"); break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
      result = ACE_TEXT_ALWAYS_CHAR ("DISASSOCIATE"); break;
    case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE:
      result = ACE_TEXT_ALWAYS_CHAR ("DEAUTHENTICATE"); break;
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIALIZED"); break;
    case NET_WLAN_MONITOR_STATE_SCANNED:
      result = ACE_TEXT_ALWAYS_CHAR ("SCANNED"); break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATED:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATED"); break;
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
      result = ACE_TEXT_ALWAYS_CHAR ("ASSOCIATED"); break;
#endif // ACE_WIN32 || ACE_WIN64
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
