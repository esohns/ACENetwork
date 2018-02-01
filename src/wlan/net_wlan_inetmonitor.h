/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef NET_WLAN_INETMONITOR_T_H
#define NET_WLAN_INETMONITOR_T_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#endif

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_wlan_monitor.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          ////////////////////////////////
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_InetMonitor_T
 : public Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             MonitorAPI_e,
                             UserDataType>
{
  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_InetMonitor_T<ACE_SYNCH_USE,
                                                    TimePolicyType,
                                                    ConfigurationType,
                                                    MonitorAPI_e,
                                                    UserDataType>,
                             ACE_SYNCH_MUTEX>;

  typedef Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             MonitorAPI_e,
                             UserDataType> inherited;

 public:
  inline virtual ~Net_WLAN_InetMonitor_T () {};

 private:
  Net_WLAN_InetMonitor_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_InetMonitor_T (const Net_WLAN_InetMonitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_InetMonitor_T& operator= (const Net_WLAN_InetMonitor_T&))

  // override (part of) Net_WLAN_IMonitor_T
  ////////////////////////////////////////

  // *IMPORTANT NOTE*: addresses() may not return significant data before this,
  //                   as the link layer configuration may not have been
  //                   established (e.g. DHCP handshake, ...)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onConnect (REFGUID,            // device identifier
#else
  virtual void onConnect (const std::string&, // device identifier
#endif
                          const std::string&, // SSID
                          bool);              // success ?
};

// include template definition
#include "net_wlan_inetmonitor.inl"

#endif