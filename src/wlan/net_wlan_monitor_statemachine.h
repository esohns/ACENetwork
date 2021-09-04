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
#else
#include "common_ilock.h"
#include "common_time_common.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "common_statemachine.h"

#include "net_wlan_common.h"

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
                                      enum Net_WLAN_MonitorState>
#endif // ACE_WIN32 || ACE_WIN64
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
                                      enum Net_WLAN_MonitorState> inherited;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  Net_WLAN_MonitorStateMachine ();
  inline virtual ~Net_WLAN_MonitorStateMachine () {}

  // implement (part of) Common_IStateMachine_T
  inline virtual bool initialize () { change (NET_WLAN_MONITOR_STATE_INITIALIZED); return true; }
  inline virtual void reset () { initialize (); }
  virtual bool change (enum Net_WLAN_MonitorState); // new state
  virtual std::string stateToString (enum Net_WLAN_MonitorState) const;

 protected:
  // convenient types
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef ACE_NULL_SYNCH::MUTEX MUTEX_T;
#else
  typedef ACE_MT_SYNCH::MUTEX MUTEX_T;

  bool    dispatchStarted_;
#endif // ACE_WIN32 || ACE_WIN64
  MUTEX_T lock_;

  // implement (part of) Common_IStateMachine_T
  // *NOTE*: only derived classes can change state

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine (const Net_WLAN_MonitorStateMachine&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine& operator= (const Net_WLAN_MonitorStateMachine&))
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_IStateMachine_T<enum Net_WLAN_MonitorState> Net_WLAN_Monitor_IStateMachine_t;
#else
typedef Common_IStateMachine_2<enum Net_WLAN_MonitorState> Net_WLAN_Monitor_IStateMachine_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
