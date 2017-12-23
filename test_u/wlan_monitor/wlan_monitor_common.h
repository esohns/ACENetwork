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

#include <deque>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_istatistic.h"
#include "common_isubscribe.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "net_common.h"
#include "net_configuration.h"

#include "net_wlan_configuration.h"
#include "net_wlan_imonitor.h"

#include "test_u_common.h"
#include "test_u_gtk_common.h"

#include "wlan_monitor_configuration.h"

// forward declarations
class Net_WLAN_IMonitorCB;

//////////////////////////////////////////

struct WLANMonitor_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  WLANMonitor_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {};

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
   , subscriber (NULL)
   , timerConfiguration ()
   , WLANMonitorConfiguration ()
  {};

  ACE_HANDLE                                    handle;
  struct WLANMonitor_SignalHandlerConfiguration signalHandlerConfiguration;
  Net_WLAN_IMonitorCB*                          subscriber;
  struct Common_TimerConfiguration              timerConfiguration;
  struct Net_WLAN_MonitorConfiguration          WLANMonitorConfiguration;
};

//////////////////////////////////////////

enum WLANMMMonitor_EventType : int
{
  WLAN_MONITOR_EVENT_ASSOCIATE_AP = 0,
  WLAN_MONITOR_EVENT_CONNECT_ESSID,
  WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG,
  WLAN_MONITOR_EVENT_SCAN_COMPLETE,
  ////////////////////////////////////////
  WLAN_MONITOR_EVENT_MAX,
  WLAN_MONITOR_EVENT_INVALID
};
typedef std::deque<enum WLANMMMonitor_EventType> WLANMMMonitor_Events_t;
typedef WLANMMMonitor_Events_t::const_iterator WLANMMMonitor_EventsIterator_t;

struct WLANMonitor_GTK_CBData
 : Test_U_GTK_CBData
{
  WLANMonitor_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , eventSourceId (0)
   , eventStack ()
  {};

  struct WLANMonitor_Configuration* configuration;
  guint                             eventSourceId; // progress bar
  WLANMMMonitor_Events_t            eventStack;
};

typedef Common_UI_GtkBuilderDefinition_T<struct WLANMonitor_GTK_CBData> WLANMonitor_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct WLANMonitor_GTK_CBData> WLANMonitor_GTK_Manager_t;
typedef ACE_Singleton<WLANMonitor_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> WLANMONITOR_UI_GTK_MANAGER_SINGLETON;

#endif
