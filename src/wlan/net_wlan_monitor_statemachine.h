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
#include "common_statemachine_base.h"

enum Net_WLAN_MonitorState
{
  NET_WLAN_MONITOR_STATE_INVALID = -1,
  NET_WLAN_MONITOR_STATE_INITIAL = 0,  // initialized
  // -------------------------------------
  NET_WLAN_MONITOR_STATE_SCAN,         // scanning
  NET_WLAN_MONITOR_STATE_ASSOCIATE,    // associating to AP
  NET_WLAN_MONITOR_STATE_CONNECT,      // request IP address (e.g. DHCP)
  NET_WLAN_MONITOR_STATE_DISCONNECT,   // release IP address (e.g. DHCP)
  NET_WLAN_MONITOR_STATE_DISASSOCIATE, // disassociate from AP
  ////////////////////////////////////////
  NET_WLAN_MONITOR_STATE_ASSOCIATED,   // associated to AP
  NET_WLAN_MONITOR_STATE_CONNECTED,    // 'online': interface is associated to AP and has a valid IP address
  ////////////////////////////////////////
  NET_WLAN_MONITOR_STATE_MAX
};

class Net_WLAN_MonitorStateMachine
 : public Common_StateMachine_Base_T<ACE_NULL_SYNCH,
                                     enum Net_WLAN_MonitorState>
{
 public:
  Net_WLAN_MonitorStateMachine ();
  inline virtual ~Net_WLAN_MonitorStateMachine () {}

  // implement (part of) Common_IStateMachine_T
  virtual void initialize ();
  inline virtual void reset () { initialize (); };
  virtual std::string stateToString (enum Net_WLAN_MonitorState) const;

 protected:
   ACE_SYNCH_NULL_MUTEX lock_;

  // implement (part of) Common_IStateMachine_T
  // *NOTE*: only derived classes can change state
  virtual bool change (enum Net_WLAN_MonitorState); // new state

 private:
  typedef Common_StateMachine_Base_T<ACE_NULL_SYNCH,
                                     enum Net_WLAN_MonitorState> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine (const Net_WLAN_MonitorStateMachine&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_MonitorStateMachine& operator= (const Net_WLAN_MonitorStateMachine&))
};

// convenient types
typedef Common_IStateMachine_T<enum Net_WLAN_MonitorState> Net_WLAN_Monitor_IStateMachine_t;

#endif
