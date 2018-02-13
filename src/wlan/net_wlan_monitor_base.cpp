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

#include "ace/Synch.h"
#include "net_wlan_monitor.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
#include "NetworkManager/NetworkManager.h"
#endif // DBUS_SUPPORT

#include "net_configuration.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void WINAPI
network_wlan_default_notification_cb (struct _L2_NOTIFICATION_DATA* data_in,
                                      PVOID context_in)
{
  NETWORK_TRACE (ACE_TEXT ("network_wlan_default_notification_cb"));

  // sanity check(s)
  ACE_ASSERT (data_in);
  ACE_ASSERT (context_in);

  Net_WLAN_IMonitorCB* iwlanmonitor_cb_p =
    static_cast<Net_WLAN_IMonitorCB*> (context_in);

  WLAN_REASON_CODE reason_i = WLAN_REASON_CODE_SUCCESS;
  std::string notification_string;
  switch (data_in->NotificationSource)
  {
    case WLAN_NOTIFICATION_SOURCE_ACM:
    {
      switch (data_in->NotificationCode)
      {
        case wlan_notification_acm_autoconf_enabled:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_autoconf_enabled");
          break;
        case wlan_notification_acm_autoconf_disabled:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_autoconf_disabled");
          break;
        case wlan_notification_acm_background_scan_enabled:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_background_scan_enabled");
          break;
        case wlan_notification_acm_background_scan_disabled:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_background_scan_disabled");
          break;
        case wlan_notification_acm_bss_type_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_bss_type_change");
          break;
        case wlan_notification_acm_power_setting_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_power_setting_change");
          break;
        case wlan_notification_acm_scan_complete:
        {
          try {
            iwlanmonitor_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_complete");
          break;
        }
        case wlan_notification_acm_scan_fail:
        { ACE_ASSERT (data_in->pData);
          reason_i = *static_cast<WLAN_REASON_CODE*> (data_in->pData);
          try {
            iwlanmonitor_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_fail");
          break;
        }
        case wlan_notification_acm_connection_start:
        { ACE_ASSERT (data_in->pData);
          struct _WLAN_CONNECTION_NOTIFICATION_DATA* wlan_connection_notification_data_p =
            static_cast<struct _WLAN_CONNECTION_NOTIFICATION_DATA*> (data_in->pData);
          ACE_UNUSED_ARG (wlan_connection_notification_data_p);
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_start");
          break;
        }
        case wlan_notification_acm_connection_complete:
        { ACE_ASSERT (data_in->pData);
          struct _WLAN_CONNECTION_NOTIFICATION_DATA* wlan_connection_notification_data_p =
            static_cast<struct _WLAN_CONNECTION_NOTIFICATION_DATA*> (data_in->pData);
          std::string SSID_string (reinterpret_cast<char*> (wlan_connection_notification_data_p->dot11Ssid.ucSSID),
                                   wlan_connection_notification_data_p->dot11Ssid.uSSIDLength);
          try {
            iwlanmonitor_cb_p->onAssociate (data_in->InterfaceGuid,
                                            SSID_string,
                                            (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onAssociate(%s), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                        ACE_TEXT (SSID_string.c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_complete");
          break;
        }
        case wlan_notification_acm_connection_attempt_fail:
        { ACE_ASSERT (data_in->pData);
          struct _WLAN_CONNECTION_NOTIFICATION_DATA* wlan_connection_notification_data_p =
            static_cast<struct _WLAN_CONNECTION_NOTIFICATION_DATA*> (data_in->pData);
          std::string SSID_string (reinterpret_cast<char*> (wlan_connection_notification_data_p->dot11Ssid.ucSSID),
                                   wlan_connection_notification_data_p->dot11Ssid.uSSIDLength);
          reason_i = wlan_connection_notification_data_p->wlanReasonCode;
          try {
            iwlanmonitor_cb_p->onAssociate (data_in->InterfaceGuid,
                                            SSID_string,
                                            (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onAssociate(%s), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                        ACE_TEXT (SSID_string.c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_attempt_fail");
          break;
        }
        case wlan_notification_acm_filter_list_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_filter_list_change");
          break;
        case wlan_notification_acm_interface_arrival:
        {
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_interface_arrival");
          try {
            iwlanmonitor_cb_p->onHotPlug (data_in->InterfaceGuid,
                                          true);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          break;
        }
        case wlan_notification_acm_interface_removal:
        {
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_interface_removal");
          try {
            iwlanmonitor_cb_p->onHotPlug (data_in->InterfaceGuid,
                                          false);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          break;
        }
        case wlan_notification_acm_profile_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_change");
          break;
        case wlan_notification_acm_profile_name_change:
        { ACE_ASSERT (data_in->pData);
          WCHAR* string_p = reinterpret_cast<WCHAR*> (data_in->pData);
          ACE_UNUSED_ARG (string_p);
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_name_change");
          break;
        }
        case wlan_notification_acm_profiles_exhausted:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profiles_exhausted");
          break;
        case wlan_notification_acm_network_not_available:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_network_not_available");
          break;
        case wlan_notification_acm_network_available:
        { // *NOTE*: - connectable network is detected, and
          //         - not currently connected to another network, and
          //         - there isn't an automatic connection available.
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_network_available");
          break;
        }
        case wlan_notification_acm_disconnecting:
        { ACE_ASSERT (data_in->pData);
          struct _WLAN_CONNECTION_NOTIFICATION_DATA* wlan_connection_notification_data_p =
            static_cast<struct _WLAN_CONNECTION_NOTIFICATION_DATA*> (data_in->pData);
          ACE_UNUSED_ARG (wlan_connection_notification_data_p);
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_disconnecting");
          break;
        }
        case wlan_notification_acm_disconnected:
        { ACE_ASSERT (data_in->pData);
          struct _WLAN_CONNECTION_NOTIFICATION_DATA* wlan_connection_notification_data_p =
            static_cast<struct _WLAN_CONNECTION_NOTIFICATION_DATA*> (data_in->pData);
          std::string SSID_string (reinterpret_cast<char*> (wlan_connection_notification_data_p->dot11Ssid.ucSSID),
                                   wlan_connection_notification_data_p->dot11Ssid.uSSIDLength);
          try {
            iwlanmonitor_cb_p->onDisassociate (data_in->InterfaceGuid,
                                               SSID_string,
                                               (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onDisassociate(%s), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                        ACE_TEXT (SSID_string.c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_disconnected");
          break;
        }
        case wlan_notification_acm_adhoc_network_state_change:
        { ACE_ASSERT (data_in->pData);
          enum _WLAN_ADHOC_NETWORK_STATE* wlan_adhoc_network_state_p =
            static_cast<enum _WLAN_ADHOC_NETWORK_STATE*> (data_in->pData);
          ACE_UNUSED_ARG (wlan_adhoc_network_state_p);
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_adhoc_network_state_change");
          break;
        }
        case wlan_notification_acm_profile_unblocked:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_unblocked");
          break;
        case wlan_notification_acm_screen_power_change:
        { ACE_ASSERT (data_in->pData);
          BOOL* bool_p = static_cast<BOOL*> (data_in->pData);
          ACE_UNUSED_ARG (bool_p);
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_screen_power_change");
          break;
        }
        case wlan_notification_acm_profile_blocked:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_blocked");
          break;
        case wlan_notification_acm_scan_list_refresh:
        {
          try {
            iwlanmonitor_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_list_refresh");
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown ACM notification code (was: %d), returning\n"),
                      data_in->NotificationCode));
          return;
        }
      } // end SWITCH

      break;
    }
    case WLAN_NOTIFICATION_SOURCE_MSM:
    { ACE_ASSERT (data_in->pData);
      struct _WLAN_MSM_NOTIFICATION_DATA* wlan_msm_notification_data_p =
        static_cast<struct _WLAN_MSM_NOTIFICATION_DATA*> (data_in->pData);

      switch (data_in->NotificationCode)
      {
        case wlan_notification_msm_associating:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_associating");
          break;
        case wlan_notification_msm_associated:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_associated");
          break;
        case wlan_notification_msm_authenticating:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_authenticating");
          break;
        case wlan_notification_msm_connected:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_connected");
          break;
        case wlan_notification_msm_roaming_start:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_roaming_start");
          break;
        case wlan_notification_msm_roaming_end:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_roaming_end");
          break;
        case wlan_notification_msm_radio_state_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_radio_state_change");
          break;
        case wlan_notification_msm_signal_quality_change:
        {
          WLAN_SIGNAL_QUALITY* signal_quality_p =
            static_cast<WLAN_SIGNAL_QUALITY*> (data_in->pData);
          try {
            iwlanmonitor_cb_p->onSignalQualityChange (data_in->InterfaceGuid,
                                                      *signal_quality_p);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onSignalQualityChange(%u), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                        *signal_quality_p));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_signal_quality_change");
          break;
        }
        case wlan_notification_msm_disassociating:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_disassociating");
          break;
        case wlan_notification_msm_disconnected:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_disconnected");
          break;
        case wlan_notification_msm_peer_join:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_peer_join");
          break;
        case wlan_notification_msm_peer_leave:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_peer_leave");
          break;
        case wlan_notification_msm_adapter_removal:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_adapter_removal");
          break;
        case wlan_notification_msm_adapter_operation_mode_change:
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_adapter_operation_mode_change");
          break;
        case 57: // *TODO*: happens on Win10; find out what this means
        case 59: // *TODO*: happens on Win10; find out what this means
          return;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown MSM notification code (was: %d), returning\n"),
                      data_in->NotificationCode));
          return;
        }
      } // end SWITCH

      break;
    }
    case WLAN_NOTIFICATION_SOURCE_SECURITY:
    case WLAN_NOTIFICATION_SOURCE_IHV:
    case WLAN_NOTIFICATION_SOURCE_HNWK:
    case WLAN_NOTIFICATION_SOURCE_ONEX:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown notification source (was: %d), returning\n"),
                  data_in->NotificationSource));
      return;
    }
  } // end SWITCH
#if defined (ACE_USES_WCHAR)
  ACE_TCHAR buffer[BUFSIZ];
#else
  ACE_ANTI_TCHAR buffer[BUFSIZ];
#endif
  if (unlikely (reason_i != WLAN_REASON_CODE_SUCCESS))
  {
    DWORD result = WlanReasonCodeToString (reason_i,
                                           sizeof (buffer),
                                           buffer,
                                           NULL);
    if (unlikely (result != ERROR_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to WlanReasonCodeToString(%d): %s, continuing\n"),
                  reason_i,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": received notification %s%s%s\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                ACE_TEXT (notification_string.c_str ()),
                ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT (": ") : ACE_TEXT ("")),
                ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT_WCHAR_TO_TCHAR (buffer) : ACE_TEXT (""))));
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": received notification %s%s%s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
              ACE_TEXT (notification_string.c_str ()),
              ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT (": ")   : ACE_TEXT ("")),
              ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT_WCHAR_TO_TCHAR (buffer) : ACE_TEXT (""))));
#endif
}
#endif // ACE_WIN32 || ACE_WIN64
