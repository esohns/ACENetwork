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

#ifndef WLAN_MONITOR_CONFIGURATION_H
#define WLAN_MONITOR_CONFIGURATION_H

#include "common_timer_common.h"

#include "net_common.h"
#include "net_configuration.h"

#include "net_wlan_monitor_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

struct WLANMonitor_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  WLANMonitor_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , monitor (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Net_WLAN_IMonitor_t*     monitor;
  Net_IStatisticHandler_t* statisticReportingHandler;
  long                     statisticReportingTimerId;
};

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
typedef Common_UI_IDefinition_T<struct WLANMonitor_UI_GTK_State> WLANMonitor_UI_GTK_IDefinition_t;
struct WLANMonitor_UI_GTK_Configuration
 : Common_UI_GTK_Configuration_t
{
  WLANMonitor_UI_GTK_Configuration ()
   : Common_UI_GTK_Configuration_t ()
   , definition (NULL)
  {}

  WLANMonitor_UI_GTK_IDefinition_t* definition;
};
#endif // GTK_USE
#endif // GUI_SUPPORT

struct WLANMonitor_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#else
 : Test_U_Configuration
#endif // GUI_SUPPORT
{
  WLANMonitor_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#else
   : Test_U_Configuration ()
#endif // GUI_SUPPORT
 #if defined (GUI_SUPPORT)
 #if defined (GTK_USE)
   , GTKConfiguration ()
 #endif // GTK_USE
 #endif // GUI_SUPPORT
   , handle (ACE_INVALID_HANDLE)
   , signalHandlerConfiguration ()
   , timerConfiguration ()
   , WLANMonitorConfiguration ()
  {}

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  struct WLANMonitor_UI_GTK_Configuration       GTKConfiguration;
#endif // GTK_USE
#endif // GUI_SUPPORT
  ACE_HANDLE                                    handle;
  struct WLANMonitor_SignalHandlerConfiguration signalHandlerConfiguration;
  struct Common_TimerConfiguration              timerConfiguration;
  struct Net_WLAN_MonitorConfiguration          WLANMonitorConfiguration;
};

#endif
