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

#include "net_defines.h"

#include "net_wlan_configuration.h"
#include "net_wlan_defines.h"
#include "net_wlan_imonitor.h"
#include "net_wlan_inetmonitor.h"

struct Net_WLAN_MonitorConfiguration
{
  Net_WLAN_MonitorConfiguration ()
   : autoAssociate (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , enableBackgroundScans (NET_WLAN_MONITOR_DEFAULT_BACKGROUNDSCANS)
   , enableStreamingMode (NET_WLAN_MONITOR_DEFAULT_STREAMINGMODE)
   , interfaceIdentifier (GUID_NULL)
   , notificationCB (NULL)
   , notificationCBData (NULL)
#else
   , interfaceIdentifier ()
#endif
   , SSID ()
   , subscriber (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData (NULL)
  {};

  bool                       autoAssociate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  BOOL                       enableBackgroundScans;
  BOOL                       enableStreamingMode;
  struct _GUID               interfaceIdentifier;
  WLAN_NOTIFICATION_CALLBACK notificationCB;
  PVOID                      notificationCBData;
#else
  std::string                interfaceIdentifier;
  DBusHandleMessageFunction  notificationCB;
  void*                      notificationCBData;
#endif
  std::string                SSID;
  Net_WLAN_IMonitorCB*       subscriber;
  bool                       useReactor;

  struct Net_UserData*       userData;
};

//////////////////////////////////////////

typedef Net_WLAN_IMonitor_T<ACE_INET_Addr,
                            struct Net_WLAN_MonitorConfiguration> Net_WLAN_IInetMonitor_t;

typedef Net_WLAN_InetMonitor_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Net_WLAN_MonitorConfiguration,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               NET_WLAN_MONITOR_API_WLANAPI,
#else
                               NET_WLAN_MONITOR_API_IOCTL,
#endif
                               struct Net_UserData> Net_WLAN_InetMonitor_t;
typedef ACE_Singleton<Net_WLAN_InetMonitor_t,
                      ACE_SYNCH_MUTEX> NET_WLAN_INETMONITOR_SINGLETON;

#endif
