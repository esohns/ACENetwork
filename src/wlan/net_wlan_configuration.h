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

#ifndef NET_WLAN_CONFIGURATION_H
#define NET_WLAN_CONFIGURATION_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <cguid.h>
#include <guiddef.h>
#include <wlanapi.h>
#else
#include "dbus/dbus.h"
#endif

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_common.h"
#endif

#include "net_defines.h"

#include "net_wlan_configuration.h"
#include "net_wlan_defines.h"
#include "net_wlan_imonitor.h"
#include "net_wlan_inetmonitor.h"

struct  Net_WLAN_MonitorConfiguration
{
  Net_WLAN_MonitorConfiguration ()
   : autoAssociate (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , enableBackgroundScans (NET_WLAN_MONITOR_WIN32_DEFAULT_BACKGROUNDSCANS ? TRUE : FALSE)
   , enableAutoConf (NET_WLAN_MONITOR_WIN32_DEFAULT_AUTOCONF ? TRUE : FALSE)
   , enableMediaStreamingMode (NET_WLAN_MONITOR_WIN32_DEFAULT_MEDIASTREAMINGMODE ? TRUE : FALSE)
   , interfaceIdentifier (GUID_NULL)
   , notificationCB (NULL)
   , notificationCBData (NULL)
   , timerInterface (NULL)
#else
   , enableBackgroundScans (NET_WLAN_MONITOR_UNIX_DEFAULT_BACKGROUNDSCANS)
   , interfaceIdentifier ()
#if defined (NL80211_SUPPORT)
#if defined (_DEBUG)
   , debug (false)
#endif // _DEBUG
   , defaultBufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , flushCacheBeforeScans (NET_WLAN_MONITOR_NL80211_DEFAULT_FLUSHCACHEBEFORESCANS)
   , lowPriorityScans (NET_WLAN_MONITOR_NL80211_DEFAULT_LOWPRIORITYSCANS)
   , randomizeMACAddressForScans (NET_WLAN_MONITOR_NL80211_DEFAULT_RANDOMIZEMACADDRESSFORSCANS)
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
   , notificationCB (NULL)
   , notificationCBData (NULL)
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
   , dispatch (NET_EVENT_DEFAULT_DISPATCH)
   , SSID ()
   , subscriber (NULL)
   , userData (NULL)
  {};

  bool                          autoAssociate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  BOOL                          enableAutoConf;
  // *NOTE*: "...Background scan can only be disabled when the interface is in
  //         the connected state. Background scan is disabled if at least one
  //         client disables it. If the interface gets disconnected, background
  //         scan will be enabled automatically. ..."
  BOOL                          enableBackgroundScans;
  // *NOTE*: "...The media streaming mode can only be set when the interface is
  //         in the connected state. The media streaming mode is enabled if at
  //         least one client enables it. If the interface gets disconnected,
  //         the media streaming mode is disabled automatically..."
  BOOL                          enableMediaStreamingMode;
  struct _GUID                  interfaceIdentifier;
  WLAN_NOTIFICATION_CALLBACK    notificationCB;
  PVOID                         notificationCBData;
  Common_ITimer_t*              timerInterface;
#else
  bool                          enableBackgroundScans;
  std::string                   interfaceIdentifier;
#if defined (NL80211_SUPPORT)
#if defined (_DEBUG)
  bool                          debug;
#endif // _DEBUG
  unsigned int                  defaultBufferSize;
  bool                          flushCacheBeforeScans;
  bool                          lowPriorityScans;
  bool                          randomizeMACAddressForScans;
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
  DBusHandleMessageFunction     notificationCB;
  void*                         notificationCBData;
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  enum Common_EventDispatchType dispatch;
  std::string                   SSID;
  Net_WLAN_IMonitorCB*          subscriber;

  struct Net_UserData*          userData;
};

//////////////////////////////////////////

typedef Net_WLAN_IMonitor_T<ACE_INET_Addr,
                            struct Net_WLAN_MonitorConfiguration> Net_WLAN_IInetMonitor_t;

//typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//                               ACE_MT_SYNCH,
//                               Common_TimePolicy_t,
//#endif // ACE_WIN32 || ACE_WIN64
//                               NET_WLAN_MONITOR_DEFAULT_API,
//                               struct Net_UserData> Net_WLAN_InetMonitor_t;
//typedef ACE_Singleton<Net_WLAN_InetMonitor_t,
//                      ACE_SYNCH_MUTEX> NET_WLAN_INETMONITOR_SINGLETON;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               NET_WLAN_MONITOR_API_WLANAPI,
                               struct Net_UserData> Net_WLAN_InetWlanAPIMonitor_t;
typedef ACE_Singleton<Net_WLAN_InetWlanAPIMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_INETWLANAPIMONITOR_SINGLETON;
#endif // WLANAPI_SUPPORT
#else
#if defined (WEXT_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_IOCTL,
                               struct Net_UserData> Net_WLAN_InetIoctlMonitor_t;
typedef ACE_Singleton<Net_WLAN_InetIoctlMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_INETIOCTLMONITOR_SINGLETON;
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_NL80211,
                               struct Net_UserData> Net_WLAN_InetNl80211Monitor_t;
typedef ACE_Singleton<Net_WLAN_InetNl80211Monitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_INETNL80211MONITOR_SINGLETON;
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
typedef Net_WLAN_InetMonitor_T<struct Net_WLAN_MonitorConfiguration,
                               ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               NET_WLAN_MONITOR_API_DBUS,
                               struct Net_UserData> Net_WLAN_InetDBusMonitor_t;
typedef ACE_Singleton<Net_WLAN_InetDBusMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_INETDBUSMONITOR_SINGLETON;
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#endif
