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

#ifndef NET_WLAN_MONITOR_COMMON_H
#define NET_WLAN_MONITOR_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "net_common.h"

#include "net_wlan_common.h"
#include "net_wlan_configuration.h"
#include "net_wlan_inetmonitor.h"
#include "net_wlan_monitor.h"

//////////////////////////////////////////

typedef Net_WLAN_IMonitor_T<ACE_INET_Addr,
                            struct Net_WLAN_MonitorConfiguration> Net_WLAN_IMonitor_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
typedef Net_WLAN_Monitor_T<ACE_INET_Addr,
                           struct Net_WLAN_MonitorConfiguration,
                           NET_WLAN_MONITOR_API_WLANAPI,
                           struct Net_UserData> Net_WLAN_WlanAPIMonitor_t;
typedef ACE_Singleton<Net_WLAN_WlanAPIMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_WLANAPIMONITOR_SINGLETON;
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_WEXT,
                               struct Net_UserData> Net_WLAN_WExtMonitor_t;
typedef ACE_Singleton<Net_WLAN_WExtMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_WEXTMONITOR_SINGLETON;
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_NL80211,
                               struct Net_UserData> Net_WLAN_Nl80211Monitor_t;
typedef ACE_Singleton<Net_WLAN_Nl80211Monitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_NL80211MONITOR_SINGLETON;
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_DBUS,
                               struct Net_UserData> Net_WLAN_DBusMonitor_t;
typedef ACE_Singleton<Net_WLAN_DBusMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_DBUSMONITOR_SINGLETON;
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

typedef Net_WLAN_Monitor_T<ACE_INET_Addr,
                           struct Net_WLAN_MonitorConfiguration,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           NET_WLAN_MONITOR_API_WLANAPI,
#else
                           ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           NET_WLAN_MONITOR_API_NL80211,
#endif // ACE_WIN32 || ACE_WIN64
                           struct Net_UserData> Net_WLANMonitor_t;
typedef ACE_Singleton<Net_WLANMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_MONITOR_SINGLETON;

#endif
