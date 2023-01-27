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

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if defined (WLANAPI_USE)
//typedef NET_WLAN_WLANAPIMONITOR_SINGLETON NET_WLAN_MONITOR_SINGLETON;
//#endif // WLANAPI_USE
//#elif defined (ACE_LINUX)
//#if defined (WEXT_USE)
//typedef NET_WLAN_WEXTMONITOR_SINGLETON NET_WLAN_MONITOR_SINGLETON;
//#elif defined (NL80211_USE)
//typedef NET_WLAN_NL80211MONITOR_SINGLETON NET_WLAN_MONITOR_SINGLETON;
//#elif defined (DBUS_USE)
//typedef NET_WLAN_DBUSMONITOR_SINGLETON NET_WLAN_MONITOR_SINGLETON;
//#endif
//#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

enum WLANMMMonitor_EventType
{
#if defined (GUI_SUPPORT)
  WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG = COMMON_UI_EVENT_OTHER_USER_BASE,
#else
  WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG = 0,
#endif // GUI_SUPPORT
  WLAN_MONITOR_EVENT_INTERFACE_REMOVE,
  WLAN_MONITOR_EVENT_SCAN_START,
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

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
struct WLANMonitor_UI_GTK_State
 : Common_UI_GTK_State_t
{
  WLANMonitor_UI_GTK_State ()
   : Common_UI_GTK_State_t ()
   , eventStack ()
  {}

  WLANMMMonitor_Events_t eventStack;
};

struct WLANMonitor_GTK_ProgressData
 : Test_U_GTK_ProgressData
{
  WLANMonitor_GTK_ProgressData ()
   : Test_U_GTK_ProgressData ()
   , state (NULL)
  {}

  struct WLANMonitor_UI_GTK_State* state;
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
#endif // GTK_USE

struct WLANMonitor_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#else
 : Test_U_UI_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  WLANMonitor_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#else
   : Test_U_UI_CBData ()
#endif // GTK_USE || WXWIDGETS_USE
   , configuration (NULL)
   , monitor (NULL)
#if defined (GTK_USE)
   , progressData ()
   , UIState (NULL)
#elif defined (WXWIDGETS_USE)
   , UIState (NULL)
#endif // GTK_USE || WXWIDGETS_USE
  {
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
    progressData.state = UIState;
#endif // GTK_USE || WXWIDGETS_USE
  }

  struct WLANMonitor_Configuration*      configuration;
  Net_WLAN_IMonitor_t*                   monitor;
#if defined (GTK_USE)
  struct WLANMonitor_GTK_ProgressData    progressData;
  struct WLANMonitor_UI_GTK_State*       UIState;
#elif defined (WXWIDGETS_USE)
  struct WLANMonitor_UI_WxWidgets_State* UIState;
#endif // GTK_USE || WXWIDGETS_USE
};

#if defined (GTK_USE)
typedef Common_UI_GtkBuilderDefinition_T<struct WLANMonitor_UI_GTK_State> WLANMonitor_GtkBuilderDefinition_t;

//////////////////////////////////////////

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct WLANMonitor_UI_GTK_Configuration,
                                struct WLANMonitor_UI_GTK_State,
                                gpointer> WLANMonitor_UI_GTK_Manager_t;
typedef ACE_Singleton<WLANMonitor_UI_GTK_Manager_t,
                      ACE_MT_SYNCH::MUTEX> WLANMONITOR_UI_GTK_MANAGER_SINGLETON;
#endif // GTK_USE
#endif // GUI_SUPPORT

//////////////////////////////////////////

#endif
