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
#include "net_wlanmonitor.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void WINAPI
network_wlan_default_notification_cb (PWLAN_NOTIFICATION_DATA data_in,
                                      PVOID context_in)
{
  NETWORK_TRACE (ACE_TEXT ("network_wlan_default_notification_cb"));

  // sanity check(s)
  ACE_ASSERT (data_in);
  ACE_ASSERT (context_in);

  Net_IWLANCB* iwlan_cb_p = static_cast<Net_IWLANCB*> (context_in);

  WLAN_REASON_CODE reason_i = 0;
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
            iwlan_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_IWLANCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_complete");
          break;
        }
        case wlan_notification_acm_scan_fail:
        {
          try {
            iwlan_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_IWLANCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
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
            iwlan_cb_p->onAssociate (data_in->InterfaceGuid,
                                     SSID_string,
                                     (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_IWLANCB::onAssociate(%s), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ()),
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
          ACE_UNUSED_ARG (wlan_connection_notification_data_p);
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
            iwlan_cb_p->onHotPlug (data_in->InterfaceGuid,
                                   true);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_IWLANCB::onHotPlug(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
          }
          break;
        }
        case wlan_notification_acm_interface_removal:
        {
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_interface_removal");
          try {
            iwlan_cb_p->onHotPlug (data_in->InterfaceGuid,
                                   false);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_IWLANCB::onHotPlug(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
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
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_network_available");
          break;
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
          ACE_UNUSED_ARG (wlan_connection_notification_data_p);
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
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_list_refresh");
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown notification code (was: %d), returning\n"),
                      data_in->NotificationCode));
          return;
        }
      } // end SWITCH

      break;
    }
    case WLAN_NOTIFICATION_SOURCE_MSM:
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
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: received notification \"%s\"\n"),
              ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ()),
              ACE_TEXT (notification_string.c_str ())));
}
#endif

//////////////////////////////////////////
