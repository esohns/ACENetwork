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

#ifndef NET_WLAN_MONITOR_STATEMACHINE_H
#define NET_WLAN_MONITOR_STATEMACHINE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_istatemachine.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_statemachine_base.h"
#else
#include "common_ilock.h"
#include "common_statemachine.h"
#include "common_time_common.h"
#endif

enum Net_WLAN_MonitorState
{
  NET_WLAN_MONITOR_STATE_INVALID     = -1,
  // -------------------------------------
  // 'transitional' states (task-/timeout-oriented, i.e. may return to previous
  // state upon completion)
  NET_WLAN_MONITOR_STATE_IDLE        = 0,  // idle (i.e. waiting between scans/connection attempts/...)
  NET_WLAN_MONITOR_STATE_SCAN,             // scanning
  NET_WLAN_MONITOR_STATE_ASSOCIATE,        // associating to AP
  NET_WLAN_MONITOR_STATE_CONNECT,          // requesting IP address (e.g. DHCP)
  NET_WLAN_MONITOR_STATE_DISCONNECT,       // releasing IP address (e.g. DHCP)
  NET_WLAN_MONITOR_STATE_DISASSOCIATE,     // disassociating from AP
  ////////////////////////////////////////
  // 'static' states (discrete/persisting, i.e. long(er)-term)
  NET_WLAN_MONITOR_STATE_INITIALIZED,      // initialized
  NET_WLAN_MONITOR_STATE_SCANNED,          // scanning complete
  NET_WLAN_MONITOR_STATE_ASSOCIATED,       // associated to AP
  NET_WLAN_MONITOR_STATE_CONNECTED,        // 'online': interface is associated to AP and has a valid IP address
  ////////////////////////////////////////
  NET_WLAN_MONITOR_STATE_MAX
};

extern const char network_wlan_statemachine_monitor_name_string_[];

class Net_WLAN_MonitorStateMachine
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 : public Common_StateMachine_T<network_wlan_statemachine_monitor_name_string_,
                                ACE_NULL_SYNCH,
                                enum Net_WLAN_MonitorState,
                                Common_IStateMachine_T<enum Net_WLAN_MonitorState> >
#else
 // *TODO*: only the NET_WLAN_MONITOR_API_IOCTL specialization really warrants
 //         an asynchronous state machine; the other implementations just
 //         require dispatching threads, or should use the reactor/proactor
 //         instead
 : public Common_StateMachineAsynch_T<network_wlan_statemachine_monitor_name_string_,
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Common_ILock_T<ACE_MT_SYNCH>,
                                      enum Net_WLAN_MonitorState>
#endif
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef Common_StateMachine_T<network_wlan_statemachine_monitor_name_string_,
                                ACE_NULL_SYNCH,
                                enum Net_WLAN_MonitorState,
                                Common_IStateMachine_T<enum Net_WLAN_MonitorState> > inherited;
#else
  typedef Common_StateMachineAsynch_T<network_wlan_statemachine_monitor_name_string_,
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Common_ILock_T<ACE_MT_SYNCH>,
                                      enum Net_WLAN_MonitorState> inherited;
#endif

 public:
  Net_WLAN_MonitorStateMachine ();
  inline virtual ~Net_WLAN_MonitorStateMachine () {}

  // implement (part of) Common_IStateMachine_T
  inline virtual bool initialize () { change (NET_WLAN_MONITOR_STATE_INITIALIZED); return true; }
  inline virtual void reset () { initialize (); }
  virtual std::string stateToString (enum Net_WLAN_MonitorState) const;

 protected:
  // convenient types
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typename ACE_NULL_SYNCH::MUTEX MUTEX_T;

  ACE_SYNCH_NULL_MUTEX lock_;
#else
  typename ACE_MT_SYNCH::MUTEX MUTEX_T;

  bool                 dispatchStarted_;
  ACE_SYNCH_MUTEX      lock_;
#endif // ACE_WIN32 || ACE_WIN64

  // implement (part of) Common_IStateMachine_T
  // *NOTE*: only derived classes can change state
  virtual bool change (enum Net_WLAN_MonitorState); // new state

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine (const Net_WLAN_MonitorStateMachine&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine& operator= (const Net_WLAN_MonitorStateMachine&))
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_IStateMachine_T<enum Net_WLAN_MonitorState> Net_WLAN_Monitor_IStateMachine_t;
#else
typedef Common_IStateMachine_2<enum Net_WLAN_MonitorState> Net_WLAN_Monitor_IStateMachine_t;
#endif

#endif
