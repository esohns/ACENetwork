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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "guiddef.h"
#include "Ks.h"
#include "wlanapi.h"
#else
#if defined (DBUS_NM_SUPPORT)
#include "dbus/dbus.h"
#endif // DBUS_NM_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include <string>

#include "ace/Basic_Types.h"

#include "common_event_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "net_defines.h"

#include "net_wlan_defines.h"
#include "net_wlan_imonitor.h"

struct Net_WLAN_MonitorConfiguration
{
  Net_WLAN_MonitorConfiguration ()
   : autoAssociate (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
   , enableBackgroundScans (NET_WLAN_MONITOR_WIN32_DEFAULT_BACKGROUNDSCANS ? TRUE : FALSE)
   , enableAutoConf (NET_WLAN_MONITOR_WIN32_DEFAULT_AUTOCONF ? TRUE : FALSE)
   , enableMediaStreamingMode (NET_WLAN_MONITOR_WIN32_DEFAULT_MEDIASTREAMINGMODE ? TRUE : FALSE)
   , interfaceIdentifier (GUID_NULL)
   , notificationCB (NULL)
   , notificationCBData (NULL)
   , timerInterface (NULL)
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
   , enableBackgroundScans (NET_WLAN_MONITOR_UNIX_DEFAULT_BACKGROUNDSCANS)
   , interfaceIdentifier ()
#if defined (NL80211_SUPPORT)
#if defined (_DEBUG)
   , debug (false)
#endif // _DEBUG
   , authenticationType (NL80211_AUTHTYPE_AUTOMATIC)
   , defaultBufferSize (NET_STREAM_DEFAULT_MESSAGE_DATA_BUFFER_SIZE)
   , flushCacheBeforeScans (NET_WLAN_MONITOR_NL80211_DEFAULT_FLUSHCACHEBEFORESCANS)
   , frequency (0)
   , lowPriorityScans (NET_WLAN_MONITOR_NL80211_DEFAULT_LOWPRIORITYSCANS)
   , randomizeMACAddressForScans (NET_WLAN_MONITOR_NL80211_DEFAULT_RANDOMIZEMACADDRESSFORSCANS)
   , wiPhyIdentifier ()
#endif // NL80211_SUPPORT
#if defined (DBUS_NM_SUPPORT)
   , notificationCB (NULL)
   , notificationCBData (NULL)
#endif // DBUS_NM_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , SSID ()
   , subscriber (NULL)
   , userData (NULL)
  {}

  bool                          autoAssociate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
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
  Common_ITimerCB_t*            timerInterface;
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
  bool                          enableBackgroundScans;
  std::string                   interfaceIdentifier;
#if defined (NL80211_SUPPORT)
#if defined (_DEBUG)
  bool                          debug;
#endif // _DEBUG
  // *TODO*: conceive of an ADT for authentication credentials
  enum nl80211_auth_type        authenticationType;
  unsigned int                  defaultBufferSize;
  bool                          flushCacheBeforeScans;
  ACE_UINT32                    frequency;
  bool                          lowPriorityScans;
  bool                          randomizeMACAddressForScans;
  std::string                   wiPhyIdentifier;
#endif // NL80211_SUPPORT
#if defined (DBUS_NM_SUPPORT)
  DBusHandleMessageFunction     notificationCB;
  void*                         notificationCBData;
#endif // DBUS_NM_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  enum Common_EventDispatchType dispatch;
  std::string                   SSID;
  Net_WLAN_IMonitorCB*          subscriber;

  struct Net_UserData*          userData;
};

#endif
