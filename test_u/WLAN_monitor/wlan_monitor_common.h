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

#ifndef WLAN_MONITOR_COMMON_H
#define WLAN_MONITOR_COMMON_H

#include "ace/Containers_T.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common_istatistic.h"
#include "common_isubscribe.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "net_common.h"

#include "net_wlan_common.h"
#include "net_wlan_configuration.h"
#include "net_wlan_imonitor.h"

#include "test_u_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "wlan_monitor_configuration.h"

// forward declarations
class Net_WLAN_IMonitorCB;

//////////////////////////////////////////

struct WLANMonitor_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  WLANMonitor_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , monitor (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Net_WLAN_IInetMonitor_t* monitor;
  Net_IStatisticHandler_t* statisticReportingHandler;
  long                     statisticReportingTimerId;
};

struct WLANMonitor_Configuration
 : Test_U_Configuration
{
  WLANMonitor_Configuration ()
   : Test_U_Configuration ()
   , handle (ACE_INVALID_HANDLE)
   , signalHandlerConfiguration ()
   , timerConfiguration ()
   , WLANMonitorConfiguration ()
  {}

  ACE_HANDLE                                    handle;
  struct WLANMonitor_SignalHandlerConfiguration signalHandlerConfiguration;
  struct Common_TimerConfiguration              timerConfiguration;
  struct Net_WLAN_MonitorConfiguration          WLANMonitorConfiguration;
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
typedef NET_WLAN_INETWLANAPIMONITOR_SINGLETON NET_WLAN_INETMONITOR_SINGLETON;
#endif // WLANAPI_USE
#elif defined (ACE_LINUX)
#if defined (WEXT_USE)
typedef NET_WLAN_INETIOCTLMONITOR_SINGLETON NET_WLAN_INETMONITOR_SINGLETON;
#elif defined (NL80211_USE)
typedef NET_WLAN_INETNL80211MONITOR_SINGLETON NET_WLAN_INETMONITOR_SINGLETON;
#elif defined (DBUS_USE)
typedef NET_WLAN_INETDBUSMONITOR_SINGLETON NET_WLAN_INETMONITOR_SINGLETON;
#endif
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

enum WLANMMMonitor_EventType
{
#if defined (GUI_SUPPORT)
  WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG = COMMON_UI_EVENT_OTHER_USER_BASE,
#else
  WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG = 0,
#endif // GUI_SUPPORT
  WLAN_MONITOR_EVENT_INTERFACE_REMOVE,
  WLAN_MONITOR_EVENT_SCAN_COMPLETE,
  WLAN_MONITOR_EVENT_ASSOCIATE,
  WLAN_MONITOR_EVENT_DISASSOCIATE,
  WLAN_MONITOR_EVENT_CONNECT,
  WLAN_MONITOR_EVENT_DISCONNECT,
  WLAN_MONITOR_EVENT_SIGNAL_QUALITY_CHANGED,
  ////////////////////////////////////////
  WLAN_MONITOR_EVENT_MAX,
  WLAN_MONITOR_EVENT_INVALID
};
typedef ACE_Unbounded_Stack<enum WLANMMMonitor_EventType> WLANMMMonitor_Events_t;
typedef ACE_Unbounded_Stack<enum WLANMMMonitor_EventType>::ITERATOR WLANMMMonitor_EventsIterator_t;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
struct WLANMonitor_UI_GTK_State
 : Common_UI_GTK_State
{
  WLANMonitor_UI_GTK_State ()
   : Common_UI_GTK_State ()
   , eventStack ()
  {}

  WLANMMMonitor_Events_t eventStack;
};
#elif defined (WXWIDGETS_USE)
struct WLANMonitor_UI_WxWidgets_State
 : Common_UI_wxWidgets_State
{
  WLANMonitor_UI_WxWidgets_State ()
   : Common_UI_wxWidgets_State ()
   , eventStack ()
  {}

  WLANMMMonitor_Events_t eventStack;
};
#endif

struct WLANMonitor_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  WLANMonitor_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#endif
   , configuration (NULL)
   , monitor (NULL)
#if defined (GTK_USE)
   , UIState (NULL)
#elif defined (WXWIDGETS_USE)
   , UIState (NULL)
#endif // GTK_USE
  {}

  struct WLANMonitor_Configuration*      configuration;
  Net_WLAN_IInetMonitor_t*               monitor;
#if defined (GTK_USE)
  struct WLANMonitor_UI_GTK_State*       UIState;
#elif defined (WXWIDGETS_USE)
  struct WLANMonitor_UI_WxWidgets_State* UIState;
#endif
};

#if defined (GTK_USE)
typedef Common_UI_GtkBuilderDefinition_T<struct WLANMonitor_UI_GTK_State,
                                         struct WLANMonitor_UI_CBData> WLANMonitor_GtkBuilderDefinition_t;

//////////////////////////////////////////

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct WLANMonitor_UI_GTK_State> WLANMonitor_UI_GTK_Manager_t;
typedef ACE_Singleton<WLANMonitor_UI_GTK_Manager_t,
                      ACE_MT_SYNCH::MUTEX> WLANMONITOR_UI_GTK_MANAGER_SINGLETON;
#endif // GTK_USE
#endif // GUI_SUPPORT

//////////////////////////////////////////

#endif
