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
#include "stdafx.h"

//#include "ace/Synch.h"
#include "test_u_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "net_macros.h"

#include "net_wlan_common.h"

#include "wlan_monitor_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_ui_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

Test_U_EventHandler::Test_U_EventHandler (
#if defined (GUI_SUPPORT)
                                          struct WLANMonitor_UI_CBData* CBData_in
#endif // GUI_SUPPORT
                                         )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onSignalQualityChange (REFGUID interfaceIdentifier_in,
                                            WLAN_SIGNAL_QUALITY signalQuality_in)
#else
Test_U_EventHandler::onSignalQualityChange (const std::string& interfaceIdentifier_in,
                                            unsigned int signalQuality_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onSignalQualityChange"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (signalQuality_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_update_signal_quality_cb,
                                      CBData_);
  if (!event_source_id)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_signal_quality_cb): \"%m\", continuing\n")));
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    //CBData_->eventSourceIds.insert (event_source_id);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_SIGNAL_QUALITY_CHANGED);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onAssociate (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onAssociate (const std::string& interfaceIdentifier_in,
#endif
                                  const std::string& SSID_in,
                                  bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onAssociate"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  if (success_in)
  {
    event_source_id = g_idle_add (idle_update_status_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_status_cb): \"%m\", continuing\n")));
    //else
    //  CBData_->eventSourceIds.insert (event_source_id);
  } // end IF
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_ASSOCIATE);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onDisassociate (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onDisassociate (const std::string& interfaceIdentifier_in,
#endif
                                     const std::string& SSID_in,
                                     bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onDisassociate"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  if (success_in)
  {
    event_source_id = g_idle_add (idle_update_status_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_status_cb): \"%m\", continuing\n")));
    //else
    //  CBData_->eventSourceIds.insert (event_source_id);
  } // end IF
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_DISASSOCIATE);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onConnect (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onConnect (const std::string& interfaceIdentifier_in,
#endif
                                const std::string& SSID_in,
                                bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onConnect"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  if (success_in)
  {
    event_source_id = g_idle_add (idle_update_status_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_status_cb): \"%m\", continuing\n")));
    //else
    //  CBData_->eventSourceIds.insert (event_source_id);
  } // end IF
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_CONNECT);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onDisconnect (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onDisconnect (const std::string& interfaceIdentifier_in,
#endif
                                   const std::string& SSID_in,
                                   bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onDisconnect"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
  if (success_in)
  {
    event_source_id = g_idle_add (idle_update_status_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_status_cb): \"%m\", continuing\n")));
    //else
    //  CBData_->eventSourceIds.insert (event_source_id);
  } // end IF
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_DISCONNECT);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onHotPlug (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onHotPlug (const std::string& interfaceIdentifier_in,
#endif
                                bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onHotPlug"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (enabled_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onRemove (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onRemove (const std::string& interfaceIdentifier_in,
#endif
                               bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onRemove"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (enabled_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_INTERFACE_REMOVE);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onScanComplete (REFGUID interfaceIdentifier_in)
#else
Test_U_EventHandler::onScanComplete (const std::string& interfaceIdentifier_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onScanComplete"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!CBData_)
    return;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_->UIState);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_update_scan_end_cb,
                                      CBData_);
  if (!event_source_id)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_scan_end_cb): \"%m\", continuing\n")));
  //else
  //  CBData_->eventSourceIds.insert (event_source_id);
#endif // GTK_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState->lock);
    CBData_->UIState->eventStack.push (WLAN_MONITOR_EVENT_SCAN_COMPLETE);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}
