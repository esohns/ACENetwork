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

#include "net_defines.h"

#include "net_wlan_defines.h"

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
   , useReactor (NET_EVENT_USE_REACTOR)
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
  bool                       useReactor;
};

#endif
