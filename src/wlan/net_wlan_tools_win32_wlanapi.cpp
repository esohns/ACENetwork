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

#include "net_wlan_tools.h"

#include "guiddef.h"
#include "iphlpapi.h"
#include "Ks.h"
#include "l2cmn.h"
#include "mstcpip.h"
#define __CGUID_H__
#include "OleAuto.h"
#include "shlwapi.h"
#include "wlanapi.h"
#include "WinInet.h"

#if defined (MSXML_SUPPORT)
#include "msxml2.h"
#endif // MSXML_SUPPORT
#if defined (LIBXML2_SUPPORT)
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#endif // LIBXML2_SUPPORT

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"
#include "common_string_tools.h"

#include "common_error_tools.h"

#include "common_xml_defines.h"
#include "common_xml_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_wlan_defines.h"
#include "net_wlan_imonitor.h"
#include "net_wlan_monitor_statemachine.h"
#include "net_wlan_profile_xml_handler.h"

#if defined (WLANAPI_SUPPORT)
void WINAPI
network_wlan_default_notification_cb (struct _L2_NOTIFICATION_DATA* data_in,
                                      PVOID userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("network_wlan_default_notification_cb"));

  // sanity check(s)
  ACE_ASSERT (data_in);
  ACE_ASSERT (userData_in);

  Net_WLAN_IMonitorBase* imonitor_p =
      static_cast<Net_WLAN_IMonitorBase*> (userData_in);
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
          // *NOTE*: invoke the callback in wlan_notification_acm_scan_list_refresh (see below)
          //try {
          //  imonitor_p->onScanComplete (data_in->InterfaceGuid);
          //} catch (...) {
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
          //              ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          //}
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_complete");
          break;
        }
        case wlan_notification_acm_scan_fail:
        { ACE_ASSERT (data_in->pData);
          reason_i = *static_cast<WLAN_REASON_CODE*> (data_in->pData);
          ACE_UNUSED_ARG (reason_i);
          try {
            imonitor_p->onScanComplete (data_in->InterfaceGuid);
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
            imonitor_p->onConnect (data_in->InterfaceGuid,
                                   SSID_string,
                                   (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
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
          ACE_UNUSED_ARG (reason_i);
          try {
            imonitor_p->onConnect (data_in->InterfaceGuid,
                                   SSID_string,
                                   false);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
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
            imonitor_p->onHotPlug (data_in->InterfaceGuid,
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
            imonitor_p->onHotPlug (data_in->InterfaceGuid,
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
            imonitor_p->onDisconnect (data_in->InterfaceGuid,
                                      SSID_string,
                                      (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onDisconnect(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
        case wlan_notification_acm_scan_list_refresh:
        {
          // *NOTE*: the list of SSIDs has been refreshed --> invoke the callback now (see above)
          try {
            imonitor_p->onScanComplete (data_in->InterfaceGuid);
          }
          catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_list_refresh");
          break;
        }
        //case wlan_notification_acm_operational_state_change:
        //  notification_string =
        //    ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_operational_state_change");
        //  break;
        case wlan_notification_acm_end:
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
            imonitor_p->onSignalQualityChange (data_in->InterfaceGuid,
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
  WCHAR buffer_a[BUFSIZ];
  if (unlikely (reason_i != WLAN_REASON_CODE_SUCCESS))
  {
    DWORD result = WlanReasonCodeToString (reason_i,
                                           sizeof (WCHAR[BUFSIZ]),
                                           buffer_a,
                                           NULL);
    if (unlikely (result != ERROR_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to WlanReasonCodeToString(%d): %s, continuing\n"),
                  reason_i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": received notification %s: %s\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
                ACE_TEXT (notification_string.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (buffer_a)));
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": received notification %s%s%s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
              ACE_TEXT (notification_string.c_str ()),
              ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT (": ")   : ACE_TEXT ("")),
              ((reason_i != WLAN_REASON_CODE_SUCCESS) ? ACE_TEXT_WCHAR_TO_TCHAR (buffer_a) : ACE_TEXT (""))));
#endif // _DEBUG
}
#endif // WLANAPI_SUPPORT

//void WINAPI
//network_wlan_notification_cb (PWLAN_NOTIFICATION_DATA data_in,
//                              PVOID context_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("network_wlan_notification_cb"));
//
//  // sanity check(s)
//  ACE_ASSERT (data_in);
//  ACE_ASSERT (context_in);
//
//  ACE_SYNCH_CONDITION* condition_p =
//    static_cast<ACE_SYNCH_CONDITION*> (context_in);
//
//  std::string notification_string;
//  switch (data_in->NotificationSource)
//  {
//    case WLAN_NOTIFICATION_SOURCE_ACM:
//    {
//      switch (data_in->NotificationCode)
//      {
//        case wlan_notification_acm_autoconf_enabled:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_autoconf_enabled");
//          break;
//        case wlan_notification_acm_autoconf_disabled:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_autoconf_disabled");
//          break;
//        case wlan_notification_acm_background_scan_enabled:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_background_scan_enabled");
//          break;
//        case wlan_notification_acm_background_scan_disabled:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_background_scan_disabled");
//          break;
//        case wlan_notification_acm_bss_type_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_bss_type_change");
//          break;
//        case wlan_notification_acm_power_setting_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_power_setting_change");
//          break;
//        case wlan_notification_acm_scan_complete:
//        {
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_complete");
//          int result = condition_p->broadcast ();
//          if (result == -1)
//            ACE_DEBUG ((LM_ERROR,
//                        ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\"\n")));
//          break;
//        }
//        case wlan_notification_acm_scan_fail:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_fail");
//          break;
//        case wlan_notification_acm_connection_start:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_start");
//          break;
//        case wlan_notification_acm_connection_complete:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_complete");
//          break;
//        case wlan_notification_acm_connection_attempt_fail:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_connection_attempt_fail");
//          break;
//        case wlan_notification_acm_filter_list_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_filter_list_change");
//          break;
//        case wlan_notification_acm_interface_arrival:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_interface_arrival");
//          break;
//        case wlan_notification_acm_interface_removal:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_interface_removal");
//          break;
//        case wlan_notification_acm_profile_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_change");
//          break;
//        case wlan_notification_acm_profile_name_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_name_change");
//          break;
//        case wlan_notification_acm_profiles_exhausted:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profiles_exhausted");
//          break;
//        case wlan_notification_acm_network_not_available:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_network_not_available");
//          break;
//        case wlan_notification_acm_network_available:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_network_available");
//          break;
//        case wlan_notification_acm_disconnecting:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_disconnecting");
//          break;
//        case wlan_notification_acm_disconnected:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_disconnected");
//          break;
//        case wlan_notification_acm_adhoc_network_state_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_adhoc_network_state_change");
//          break;
//        case wlan_notification_acm_profile_unblocked:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_unblocked");
//          break;
//        case wlan_notification_acm_screen_power_change:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_screen_power_change");
//          break;
//        case wlan_notification_acm_profile_blocked:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_profile_blocked");
//          break;
//        case wlan_notification_acm_scan_list_refresh:
//          notification_string =
//            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_list_refresh");
//          break;
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown notification code (was: %d), returning\n"),
//                      data_in->NotificationCode));
//          return;
//        }
//      } // end SWITCH
//
//      break;
//    }
//    case WLAN_NOTIFICATION_SOURCE_MSM:
//    case WLAN_NOTIFICATION_SOURCE_SECURITY:
//    case WLAN_NOTIFICATION_SOURCE_IHV:
//    case WLAN_NOTIFICATION_SOURCE_HNWK:
//    case WLAN_NOTIFICATION_SOURCE_ONEX:
//      break;
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown notification source (was: %d), returning\n"),
//                  data_in->NotificationSource));
//      return;
//    }
//  } // end SWITCH
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": received notification \"%s\"\n"),
//              ACE_TEXT (Net_WLAN_Tools::interfaceToString (data_in->InterfaceGuid).c_str ()),
//              ACE_TEXT (notification_string.c_str ())));
//}

//////////////////////////////////////////

//std::string
//Net_WLAN_Tools::WLANInterfaceToString (HANDLE clientHandle_in,
//                                       REFGUID interfaceIdentifier_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::WLANInterfaceToString"));
//
//  // initialize return value(s)
//  std::string result;
//
//  // sanity check(s)
//  if  (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
//                 InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid argument, aborting\n")));
//    return result;
//  } // end IF
//
//  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
//  DWORD result_2 = WlanEnumInterfaces (clientHandle_in,
//                                       NULL,
//                                       &interface_list_p);
//  if (unlikely (result_2 != ERROR_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::WlanEnumInterfaces(0x%@): \"%s\", aborting\n"),
//                clientHandle_in,
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (interface_list_p);
//
//  for (DWORD i = 0;
//       i < interface_list_p->dwNumberOfItems;
//       ++i)
//  {
//    if (!InlineIsEqualGUID (interface_list_p->InterfaceInfo[i].InterfaceGuid,
//                            interfaceIdentifier_in))
//      continue;
//
//    result =
//      ACE_TEXT_WCHAR_TO_TCHAR (interface_list_p->InterfaceInfo[i].strInterfaceDescription);
//    break;
//  } // end FOR
//  if (unlikely (result.empty ()))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("device not found (id was: %s), aborting\n"),
//                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
//    goto error;
//  } // end IF
//
//error:
//  if (interface_list_p)
//    WlanFreeMemory (interface_list_p);
//
//  return result;
//}

bool
Net_WLAN_Tools::initialize (HANDLE& clientHandle_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::initialize"));

  DWORD result = 0;

  // initialize return value(s)
  if (clientHandle_out != ACE_INVALID_HANDLE)
  {
    result = WlanCloseHandle (clientHandle_out,
                              NULL);
    if (result != ERROR_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    clientHandle_out = ACE_INVALID_HANDLE;
  } // end IF

  DWORD maximum_client_version =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA; see wlanapi.h:55
#if defined (WINXP_SUPPORT)
    WLAN_API_VERSION_1_0;
#pragma message ("compiling for WLAN API version " WLAN_API_VERSION_1_0)
#else
    WLAN_API_VERSION; // use SDK-native
#pragma message ("compiling for WLAN API version " COMMON_STRINGIZE(WLAN_API_VERSION))
#endif // WINXP_SUPPORT
#else
    WLAN_API_VERSION; // use SDK-native
#pragma message ("compiling for WLAN API version " COMMON_STRINGIZE(WLAN_API_VERSION))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  DWORD negotiated_version = 0;
  result = WlanOpenHandle (maximum_client_version,
                           NULL,
                           &negotiated_version,
                           &clientHandle_out);
  if (unlikely (result != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanOpenHandle(%u): \"%s\", aborting\n"),
                maximum_client_version,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (clientHandle_out != ACE_INVALID_HANDLE);

  Common_XML_Tools::initialize ();

  return true;
}

void
Net_WLAN_Tools::finalize (HANDLE clientHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::finalize"));

  Common_XML_Tools::finalize ();

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);

  DWORD result = WlanCloseHandle (clientHandle_in,
                                  NULL);
  if (unlikely (result != ERROR_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}

bool
Net_WLAN_Tools::getDeviceSettingBool (HANDLE clientHandle_in,
                                      REFGUID interfaceIdentifier_in,
                                      enum _WLAN_INTF_OPCODE parameter_in,
                                      BOOL& value_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getDeviceSettingBool"));

  // initialize return value(s)
  value_out = FALSE;

  // sanity check(s)
  if  (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                 InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
  switch (parameter_in)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
    case wlan_intf_opcode_autoconf_enabled:
    case wlan_intf_opcode_bss_type:
    case wlan_intf_opcode_interface_state:
    case wlan_intf_opcode_current_connection:
      break;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    case wlan_intf_opcode_background_scan_enabled:
    case wlan_intf_opcode_radio_state:
    case wlan_intf_opcode_channel_number:
    case wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs:
    case wlan_intf_opcode_supported_adhoc_auth_cipher_pairs:
    case wlan_intf_opcode_supported_country_or_region_string_list:
    case wlan_intf_opcode_media_streaming_mode:
    case wlan_intf_opcode_statistics:
    case wlan_intf_opcode_rssi:
    case wlan_intf_opcode_current_operation_mode:
    case wlan_intf_opcode_supported_safe_mode:
    case wlan_intf_opcode_certified_safe_mode:
      break;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
//    case wlan_intf_opcode_hosted_network_capable:
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
//    case wlan_intf_opcode_management_frame_protection_capable:
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
      //break;
    //case wlan_intf_opcode_autoconf_start:
    //case wlan_intf_opcode_autoconf_end:
    //case wlan_intf_opcode_msm_start:
    //case wlan_intf_opcode_msm_end:
    //case wlan_intf_opcode_security_start:
    //case wlan_intf_opcode_security_end:
    //case wlan_intf_opcode_ihv_start:
    //case wlan_intf_opcode_ihv_end:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %d), aborting\n"),
                  parameter_in));
      return false;
    }
  } // end SWITCH

  enum _WLAN_OPCODE_VALUE_TYPE value_type = wlan_opcode_value_type_invalid;
  DWORD data_size = 0;
  PVOID data_p = NULL;
  DWORD result = WlanQueryInterface (clientHandle_in,
                                     &interfaceIdentifier_in,
                                     parameter_in,
                                     NULL,
                                     &data_size,
                                     &data_p,
                                     &value_type);
  if (unlikely (result != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanQueryInterface(0x%@,%d): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_in, parameter_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p && (data_size == sizeof (BOOL)));
  value_out = *static_cast<BOOL*> (data_p);

  WlanFreeMemory (data_p); data_p = NULL;

  return true;
}
bool
Net_WLAN_Tools::setDeviceSettingBool (HANDLE clientHandle_in,
                                      REFGUID interfaceIdentifier_in,
                                      enum _WLAN_INTF_OPCODE parameter_in,
                                      bool enable_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::setDeviceSettingBool"));

  bool result = false;
  HANDLE client_handle = clientHandle_in;
  bool release_handle = false;
  BOOL value_b = FALSE;
#if defined (_DEBUG)
  std::string opcode_string;
#endif // _DEBUG
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  // sanity check(s)
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
  switch (parameter_in)
  {
    case wlan_intf_opcode_autoconf_enabled:
    case wlan_intf_opcode_background_scan_enabled:
    case wlan_intf_opcode_media_streaming_mode:
    case wlan_intf_opcode_radio_state:
    case wlan_intf_opcode_bss_type:
    case wlan_intf_opcode_current_operation_mode:
      break;
    case wlan_intf_opcode_autoconf_start:
    case wlan_intf_opcode_interface_state:
    case wlan_intf_opcode_current_connection:
    case wlan_intf_opcode_channel_number:
    case wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs:
    case wlan_intf_opcode_supported_adhoc_auth_cipher_pairs:
    case wlan_intf_opcode_supported_country_or_region_string_list:
    case wlan_intf_opcode_supported_safe_mode:
    case wlan_intf_opcode_certified_safe_mode:
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    case wlan_intf_opcode_hosted_network_capable:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
    case wlan_intf_opcode_management_frame_protection_capable:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
    case wlan_intf_opcode_autoconf_end:
    case wlan_intf_opcode_msm_start:
    case wlan_intf_opcode_statistics:
    case wlan_intf_opcode_rssi:
    case wlan_intf_opcode_msm_end:
    case wlan_intf_opcode_security_start:
    case wlan_intf_opcode_security_end:
    case wlan_intf_opcode_ihv_start:
    case wlan_intf_opcode_ihv_end:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %d), aborting\n"),
                  parameter_in));
      goto clean;
    }
  } // end SWITCH
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
  if (!Net_WLAN_Tools::getDeviceSettingBool (client_handle,
                                             interfaceIdentifier_in,
                                             parameter_in,
                                             value_b))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::getDeviceSettingBool(%d), continuing\n"),
                parameter_in));
    goto continue_;
  } // end IF
  if (static_cast<bool> (value_b) == enable_in) 
    return true; // nothing to do

continue_:
  BOOL data_b = (enable_in ? TRUE : FALSE);
  DWORD result_2 = WlanSetInterface (client_handle,
                                     &interfaceIdentifier_in,
                                     parameter_in,
                                     sizeof (BOOL),
                                     &data_b,
                                     NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanSetInterface(0x%@,%d): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                client_handle, parameter_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  result = true;
#if defined (_DEBUG)
  switch (parameter_in)
  {
    case wlan_intf_opcode_autoconf_enabled:
      opcode_string =
        ACE_TEXT_ALWAYS_CHAR ("wlan_intf_opcode_autoconf_enabled");
      break;
    case wlan_intf_opcode_background_scan_enabled:
      opcode_string =
        ACE_TEXT_ALWAYS_CHAR ("wlan_intf_opcode_background_scan_enabled");
      break;
    case wlan_intf_opcode_media_streaming_mode:
      opcode_string =
        ACE_TEXT_ALWAYS_CHAR ("wlan_intf_opcode_media_streaming_mode");
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid opcode (was: %d), aborting\n"),
                  parameter_in));
      goto clean;
    }
  } // end SWITCH
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": %s '%s' setting\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              (enable_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled")),
              ACE_TEXT (opcode_string.c_str ())));
#endif // _DEBUG
clean:
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);

  return result;
}

Net_InterfaceIdentifiers_t
Net_WLAN_Tools::getInterfaces (HANDLE clientHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  // initialize return value(s)
  Net_InterfaceIdentifiers_t result;

  HANDLE client_handle = clientHandle_in;
  bool release_handle = false;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return result;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);

  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD result_2 = WlanEnumInterfaces (client_handle,
                                       NULL,
                                       &interface_list_p);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanEnumInterfaces(0x%@): \"%s\", aborting\n"),
                client_handle,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (interface_list_p);

  for (DWORD i = 0;
       i < interface_list_p->dwNumberOfItems;
       ++i)
  {
#if defined (_DEBUG)
    //switch (interface_list_p->InterfaceInfo[i].isState)
    //{
    //  case wlan_interface_state_not_ready:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("not ready"); break;
    //  case wlan_interface_state_connected:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("connected"); break;
    //  case wlan_interface_state_ad_hoc_network_formed:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("first node in a ad hoc network"); break;
    //  case wlan_interface_state_disconnecting:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("disconnecting"); break;
    //  case wlan_interface_state_disconnected:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("not connected"); break;
    //  case wlan_interface_state_associating:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("attempting to associate with a network"); break;
    //  case wlan_interface_state_discovering:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("auto configuration is discovering settings for the network"); break;
    //  case wlan_interface_state_authenticating:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("in process of authenticating"); break;
    //  default:
    //    interface_state_string = ACE_TEXT_ALWAYS_CHAR ("unknown state"); break;
    //} // end SWITCH
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("[#%u] %s: \"%s\": %s\n"),
    //            i + 1,
    //            ACE_TEXT (Common_Tools::GUIDToString (interface_list_p->InterfaceInfo[i].InterfaceGuid).c_str ()),
    //            ACE_TEXT_WCHAR_TO_TCHAR (interface_list_p->InterfaceInfo[i].strInterfaceDescription),
    //            ACE_TEXT (interface_state_string.c_str ())));
#endif // _DEBUG
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    result.push_back (interface_list_p->InterfaceInfo[i].InterfaceGuid);
#else
    result.push_back (Net_Common_Tools::interfaceToString (interface_list_p->InterfaceInfo[i].InterfaceGuid));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  } // end FOR
  WlanFreeMemory (interface_list_p); interface_list_p = NULL;

clean:
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);

  return result;
}

struct ether_addr
Net_WLAN_Tools::getAccessPointAddress (HANDLE clientHandle_in,
                                       REFGUID interfaceIdentifier_in,
                                       const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getAccessPointAddress"));

  // initialize return value(s)
  struct ether_addr result;
  ACE_OS::memset (&result, 0, sizeof (struct ether_addr));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
  // sanity check(s)
  ACE_UNUSED_ARG (clientHandle_in);
  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
#else
  HANDLE client_handle = clientHandle_in;
  bool release_handle = false;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return result;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  // sanity check(s)
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
  Common_Identifiers_t Net_Common_Tools::interfaceToString (data_in->InterfaceGuid)s;
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
    Net_Common_Tools::interfaceToString (data_in->InterfaceGuid)s = Net_WLAN_Tools::getInterfaces (client_handle);
  else
    Net_Common_Tools::interfaceToString (data_in->InterfaceGuid)s.push_back (interfaceIdentifier_in);
  ACE_ASSERT (!SSID_in.empty ());
  ACE_ASSERT (SSID_in.size () <= DOT11_SSID_MAX_LENGTH);

  DWORD result_2 = 0;
  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = SSID_in.size ();
  ACE_OS::memcpy (ssid_s.ucSSID,
                  SSID_in.c_str (),
                  SSID_in.size ());
  struct _WLAN_BSS_LIST* wlan_bss_list_p = NULL;
  struct _WLAN_BSS_ENTRY* wlan_bss_list_entry_p = NULL;
  unsigned int offset = 0;
  struct Net_WLAN_IEEE802_11_InformationElement* information_element_p = NULL;
  std::string SSID_string;
  bool done = false;
  for (Common_IdentifiersIterator_t iterator = Net_Common_Tools::interfaceToString (data_in->InterfaceGuid)s.begin ();
       iterator != Net_Common_Tools::interfaceToString (data_in->InterfaceGuid)s.end ();
       ++iterator)
  {
    ACE_ASSERT (!wlan_bss_list_p);
    result_2 =
      WlanGetNetworkBssList (client_handle,
                             &(*iterator),
                             //(SSID_in.empty () ? NULL : &ssid_s),
                             //(SSID_in.empty () ? dot11_BSS_type_any : dot11_BSS_type_infrastructure), // *TODO*: iff SSID is specified, this must be specified as well
                             NULL,
                             dot11_BSS_type_any,
                             FALSE,                                                                   // *TODO*: iff SSID is specified, this must be specified as well
                             NULL,
                             &wlan_bss_list_p);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetNetworkBssList(0x%@): \"%s\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      continue;
    } // end IF
    ACE_ASSERT (wlan_bss_list_p);
    for (DWORD i = 0;
         i < wlan_bss_list_p->dwNumberOfItems;
         ++i)
    {
      wlan_bss_list_entry_p = &wlan_bss_list_p->wlanBssEntries[i];
      SSID_string.assign (reinterpret_cast<CHAR*> (wlan_bss_list_entry_p->dot11Ssid.ucSSID),
                          static_cast<std::string::size_type> (wlan_bss_list_entry_p->dot11Ssid.uSSIDLength));
      if (SSID_in.empty () ||
          !ACE_OS::strcmp (SSID_in.c_str (),
                           SSID_string.c_str ()))
      {
        ACE_OS::memcpy (&result,
                        wlan_bss_list_entry_p->dot11Bssid,
                        ETH_ALEN);
        done = true;
      } // end IF
#if defined (_DEBUG)
      //switch (wlan_bss_list_entry_p->dot11BssType)
      //{
      //  case dot11_BSS_type_infrastructure:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("infrastructure");
      //    break;
      //  case dot11_BSS_type_independent:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("ad-hoc");
      //    break;
      //  case dot11_BSS_type_any:
      //  default:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("unknown type");
      //    break;
      //} // end SWITCH
      //switch (wlan_bss_list_entry_p->dot11BssPhyType)
      //{
      //  case dot11_phy_type_fhss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("FHSS"); break;
      //  case dot11_phy_type_dsss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("DSSS"); break;
      //  case dot11_phy_type_irbaseband:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("IR baseband"); break;
      //  case dot11_phy_type_ofdm:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("OFDM"); break;
      //  case dot11_phy_type_hrdsss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("high-rate DSSS"); break;
      //  case dot11_phy_type_erp:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("ERP"); break;
      //  case dot11_phy_type_ht:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("802.11n PHY"); break;
      //  case dot11_phy_type_vht:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("802.11ac PHY"); break;
      //  case dot11_phy_type_IHV_start:
      //  case dot11_phy_type_IHV_end:
      //  case dot11_phy_type_unknown:
      //  //case dot11_phy_type_any:
      //  default:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("unknown PHY type"); break;
      //} // end SWITCH
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("[#%u] PHY id: %u, type: %s; AP MAC: %s, AP type: %s; RSSI: %d (dBm); link quality %u%%; in region domain: %s; beacon interval (us): %u; timestamp(/host): %Q/%Q; capability: 0x%x; channel center frequency (kHz): %u\n"),
      //            i + 1,
      //            wlan_bss_list_entry_p->uPhyId,
      //            ACE_TEXT (phy_type_string.c_str ()),
      //            ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (wlan_bss_list_entry_p->dot11Bssid, NET_LINKLAYER_802_3).c_str ()),
      //            ACE_TEXT (bss_network_type_string.c_str ()),
      //            wlan_bss_list_entry_p->lRssi, wlan_bss_list_entry_p->uLinkQuality,
      //            (wlan_bss_list_entry_p->bInRegDomain ? ACE_TEXT ("true") : ACE_TEXT ("false")),
      //            static_cast<unsigned int> (wlan_bss_list_entry_p->usBeaconPeriod) * 1024, // us
      //            wlan_bss_list_entry_p->ullTimestamp, wlan_bss_list_entry_p->ullHostTimestamp,
      //            static_cast<unsigned int> (wlan_bss_list_entry_p->usCapabilityInformation),
      //            wlan_bss_list_entry_p->ulChCenterFrequency));
      //for (unsigned int j = 0;
      //      j < (wlan_bss_list_entry_p->wlanRateSet.uRateSetLength / sizeof (USHORT));
      //      ++j)
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("[#%u] supported rate [#%u]: %s%u Mbps\n"),
      //              i + 1, j + 1,
      //              ((wlan_bss_list_entry_p->wlanRateSet.usRateSet[j] & 0x8000) ? ACE_TEXT ("*") : ACE_TEXT ("")),
      //              static_cast<unsigned int> ((wlan_bss_list_entry_p->wlanRateSet.usRateSet[j] & 0x7fff) * 0.5)));
      //// *TODO*: report all available information here
      //for (unsigned int j = 0, offset = 0;
      //     offset < wlan_bss_list_entry_p->ulIeSize;
      //     ++j, offset += (information_element_p->length + 2))
      //{
      //  information_element_p =
      //    reinterpret_cast<struct Net_WLAN_IEEE802_11_InformationElement*> (reinterpret_cast<char*> (wlan_bss_list_entry_p) + (wlan_bss_list_entry_p->ulIeOffset + offset));
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("[#%u/#%u]: id: %u, length: %u byte(s)\n"),
      //              i + 1, j + 1,
      //              static_cast<unsigned int> (information_element_p->id),
      //              static_cast<unsigned int> (information_element_p->length)));
      //} // end FOR
#endif // _DEBUG
      if (done)
        break;
    } // end FOR
    if (done)
      break;
    WlanFreeMemory (wlan_bss_list_p); wlan_bss_list_p = NULL;
  } // end FOR

//clean:
  if (wlan_bss_list_p)
    WlanFreeMemory (wlan_bss_list_p);
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)

  return result;
}

Net_WLAN_Profiles_t
Net_WLAN_Tools::getProfiles (HANDLE clientHandle_in,
                             REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getProfiles"));

  Net_WLAN_Profiles_t result;

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (clientHandle_in);
  else
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifiers_a.push_back (interfaceIdentifier_in);
#else
    interface_identifiers_a.push_back (Net_Common_Tools::indexToInterface (Net_Common_Tools::interfaceToIndex_2 (interfaceIdentifier_in)));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  DWORD result_2 = 0;
  struct _WLAN_PROFILE_INFO_LIST* profile_list_p = NULL;
  struct _WLAN_PROFILE_INFO* profile_p = NULL;
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    ACE_ASSERT (!profile_list_p);
    result_2 = WlanGetProfileList (clientHandle_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                   &(*iterator),
#else
                                   &Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (*iterator)),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                   NULL,
                                   &profile_list_p);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetProfileList(0x%@): \"%s\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetProfileList(0x%@): \"%s\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (*iterator))).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      continue;
    } // end IF
    ACE_ASSERT (profile_list_p);
    for (unsigned int i = 0;
         i < profile_list_p->dwNumberOfItems;
         ++i)
    { ACE_ASSERT (!profile_p);
      profile_p =
        reinterpret_cast<struct _WLAN_PROFILE_INFO*> (&profile_list_p->ProfileInfo[i]);
      ACE_ASSERT (profile_p);
      result.push_back (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (profile_p->strProfileName)));
      profile_p = NULL;
    } // end FOR
  } // end FOR

  if (profile_list_p)
    WlanFreeMemory (profile_list_p);

  return result;
}

std::string
Net_WLAN_Tools::getProfile (HANDLE clientHandle_in,
                            REFGUID interfaceIdentifier_in,
                            const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getProfile"));

  std::string result;

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);

  Net_WLAN_Profiles_t profiles_a =
    Net_WLAN_Tools::getProfiles (clientHandle_in,
                                 interfaceIdentifier_in);

  // *NOTE*: "... Windows XP with SP3 and Wireless LAN API for Windows XP with
  //          SP2:  The name of the profile is derived automatically from the
  //          SSID of the wireless network. For infrastructure network profiles,
  //          the name of the profile is the SSID of the network. For ad hoc
  //          network profiles, the name of the profile is the SSID of the ad
  //          hoc network followed by -adhoc. ..."
  // *TODO*: this specification is not precise enough (see above)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
  for (Net_WLAN_ProfilesIterator_t iterator = profiles_a.begin ();
       iterator != profiles_a.end ();
       ++iterator)
  {
    if (!ACE_OS::strcmp ((*iterator).c_str (),
                         SSID_in.c_str ()))
    {
      result = SSID_in;
      break;
    } // end IF
  } // end FOR
#else
  // *NOTE*: the WLAN profiles used by the "Native Wifi AutoConfig" service are
  //         XML documents containing "WLANProfile" root elements specified in
  //         http://www.microsoft.com/networking/WLAN/profile/v1 (see also:
  //         https://msdn.microsoft.com/en-us/library/windows/desktop/ms707341(v=vs.85).aspx).
  DWORD result_2 = 0;
  LPCWSTR profile_name_string_p = NULL;
  LPWSTR profile_string_p = NULL;
  DWORD flags = 0;
  DWORD granted_access = 0;
  struct Net_WLAN_Profile_ParserContext parser_context;
  struct Common_XML_ParserConfiguration parser_configuration;
  parser_configuration.SAXFeatures.push_back (std::make_pair (ACE_TEXT_ALWAYS_CHAR (COMMON_XML_PARSER_FEATURE_VALIDATION),
                                                              false));
  Net_WLAN_Profile_ListParser_t parser (&parser_context);
  parser.initialize (parser_configuration);
  for (Net_WLAN_ProfilesIterator_t iterator = profiles_a.begin ();
       iterator != profiles_a.end ();
       ++iterator)
  { ACE_ASSERT (!profile_name_string_p); ACE_ASSERT (!profile_string_p);
    ACE_Ascii_To_Wide converter ((*iterator).c_str ());
    profile_name_string_p = converter.wchar_rep ();
    result_2 = WlanGetProfile (clientHandle_in,
                               &interfaceIdentifier_in,
                               profile_name_string_p,
                               NULL,
                               &profile_string_p,
                               &flags,
                               &granted_access);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetProfile(0x%@,\"%s\"): \"%s\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  clientHandle_in,
                  ACE_TEXT ((*iterator).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      profile_name_string_p = NULL;
      continue;
    } // end IF
    ACE_ASSERT (profile_string_p);

    ACE_ASSERT (parser_context.SSIDs.empty ());
    parser.parseString (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (profile_string_p)));
    ACE_ASSERT (!parser_context.SSIDs.empty ());
    if (!ACE_OS::strcmp (parser_context.SSIDs.front ().c_str (),
                         SSID_in.c_str ()))
    {
      result = *iterator;
      WlanFreeMemory (profile_string_p); profile_string_p = NULL;
      break;
    } // end IF
    parser_context.SSIDs.clear ();
    profile_name_string_p = NULL;
    WlanFreeMemory (profile_string_p); profile_string_p = NULL;
  } // end FOR
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)

  return result;
}
bool
Net_WLAN_Tools::setProfile (HANDLE clientHandle_in,
                            REFGUID interfaceIdentifier_in,
                            const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::setProfile"));

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  ACE_ASSERT (!SSID_in.empty () && SSID_in.size () <= DOT11_SSID_MAX_LENGTH);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
#else
  DWORD dwFlags = 0;
  LPCWSTR strAllUserProfileSecurity = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
  LPCWSTR strProfileXml = NULL;
  DWORD   dwReasonCode = 0;

  std::string profile_template_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (NET_MODULE_WLAN_DIRECTORY_STRING),
                                                      true);
  profile_template_path += ACE_DIRECTORY_SEPARATOR_STR;
  profile_template_path +=
    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_TEMPLATE_FILENAME);
  DWORD result_3 = ERROR_INVALID_DATA;
#if defined (MSXML_SUPPORT)
  // load/populate profile template
  HRESULT result = CoInitializeEx (NULL,
                                   (COINIT_MULTITHREADED    |
                                    COINIT_DISABLE_OLE1DDE  |
                                    COINIT_SPEED_OVER_MEMORY));
  if (unlikely (FAILED (result))) // 0x80010106: RPC_E_CHANGED_MODE
  {
    if (result != RPC_E_CHANGED_MODE) // already initialized
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      return false;
    } // end IF
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
  } // end IF
  IXMLDOMDocument* document_p = NULL;
  IXMLDOMNode* node_p = NULL;
  struct tagVARIANT variant_s;
  VariantInit (&variant_s);
  VARIANT_BOOL result_2 = VARIANT_TRUE;
  BSTR string_p = NULL;
  BSTR xpath_query_p =
    SysAllocString (ACE_TEXT_ALWAYS_WCHAR (NET_WLAN_PROFILE_WLANCONFIG_SSIDCONFIG_SSID_XPATH_STRING));
  ACE_ASSERT (xpath_query_p);
  result = CoCreateInstance (CLSID_DOMDocument30, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&document_p));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_DOMDocument30): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (document_p);
  result = document_p->put_async (VARIANT_FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = document_p->put_validateOnParse (VARIANT_FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = document_p->put_resolveExternals (VARIANT_FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = document_p->put_preserveWhiteSpace (VARIANT_FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: find a way to pass a valid path URL to IXMLDOMDocument::load()
  //         (already tried UrlCreateFromPath() and Common_File_Tools::escape()
  //         to no avail)
  V_VT (&variant_s) = VT_BSTR;
  
  uint8_t* data_p = NULL;
  ACE_UINT64 file_size_i = 0;
  if (unlikely (!Common_File_Tools::load (profile_template_path,
                                          data_p,
                                          file_size_i,
                                          0)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (data_p);
  V_BSTR (&variant_s) =
    Common_String_Tools::to (reinterpret_cast<char*> (data_p));
  ACE_ASSERT (V_BSTR (&variant_s));
  delete [] data_p; data_p = NULL;
  result =
    document_p->load (variant_s,
                      &result_2);
    document_p->loadXML (V_BSTR (&variant_s),
                         &result_2);
  if (unlikely ((result == S_FALSE) || (result_2 == VARIANT_FALSE)))
  {
    IXMLDOMParseError* error_p = NULL;
    BSTR error_2 = NULL, line_p = NULL;
    long error_i = 0, line_i = 0, column_i = 0;
    HRESULT result_4 = document_p->get_parseError (&error_p);
    ACE_ASSERT (SUCCEEDED (result_4) && error_p);
    result_4 = error_p->get_errorCode (&error_i);
    ACE_ASSERT (SUCCEEDED (result_4));
    result_4 = error_p->get_reason (&error_2);
    ACE_ASSERT (SUCCEEDED (result_4) && error_2);
    result_4 = error_p->get_srcText (&line_p);
    ACE_ASSERT (SUCCEEDED (result_4));// && line_p);
    result_4 = error_p->get_line (&line_i);
    ACE_ASSERT (SUCCEEDED (result_3));
    result_4 = error_p->get_linepos (&column_i);
    ACE_ASSERT (SUCCEEDED (result_4));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IXMLDOMDocument::load(\"%s\"): \"%s\" (\"%s\"), aborting\n"),
                ACE_TEXT (profile_template_path.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (HRESULT_CODE (error_i), false, true).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (error_2)));
    SysFreeString (error_2); error_2 = NULL;
    SysFreeString (line_p); line_p = NULL;
    error_p->Release (); error_p = NULL;
    goto clean;
  } // end IF
  result = VariantClear (&variant_s);
  ACE_ASSERT (SUCCEEDED (result));
  result = document_p->selectSingleNode (xpath_query_p,
                                         &node_p);
  if (unlikely (FAILED (result) || !node_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IXMLDOMDocument::selectSingleNode(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (NET_WLAN_PROFILE_WLANCONFIG_SSIDCONFIG_SSID_XPATH_STRING),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
    goto clean;
  } // end IF
  V_BSTR (&variant_s) =
    SysAllocStringByteLen (SSID_in.c_str (),
                           static_cast<UINT> (SSID_in.size ()));
  ACE_ASSERT (V_BSTR (&variant_s));
  result = node_p->put_nodeValue (variant_s);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IXMLDOMNode::put_nodeValue(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
    goto clean;
  } // end IF
  result = document_p->get_xml (&string_p);
  if (unlikely (FAILED (result) || !string_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IXMLDOMDocument::get_xml(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto clean;
  } // end IF
#elif defined (LIBXML2_SUPPORT)
  xmlChar* data_p = NULL;
  int size_i = 0;
  xmlXPathContextPtr xpath_context_p = NULL;
  std::string xpath_query_string =
    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_WLANCONFIG_SSIDCONFIG_SSID_XPATH_STRING);
  xpath_query_string =
    Common_XML_Tools::applyNsPrefixToXPathQuery (xpath_query_string,
                                                 ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_NAMESPACE_PREFIX_STRING));
  xmlXPathObjectPtr xpath_object_p = NULL;
  xmlNodePtr node_p = NULL, node_2 = NULL;
  xmlDocPtr document_p = xmlParseFile (profile_template_path.c_str ());
  if (unlikely (!document_p))
  {
    xmlErrorPtr error_p = xmlGetLastError ();
    ACE_ASSERT (error_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlParseFile(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (profile_template_path.c_str ()),
                ACE_TEXT (error_p->message)));
    xmlResetError (error_p); error_p = NULL;
    goto clean;
  } // end IF
  xpath_context_p = xmlXPathNewContext (document_p);
  if (unlikely (!xpath_context_p))
  {
    xmlErrorPtr error_p = xmlGetLastError ();
    ACE_ASSERT (error_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathNewContext(0x%@): \"%s\", aborting\n"),
                document_p,
                ACE_TEXT (error_p->message)));
    xmlResetError (error_p); error_p = NULL;
    goto clean;
  } // end IF
  if (unlikely (xmlXPathRegisterNs (xpath_context_p,
                                    BAD_CAST (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_NAMESPACE_PREFIX_STRING)),
                                    BAD_CAST (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_NAMESPACE_HREF_STRING)))))
  {
    xmlErrorPtr error_p = xmlGetLastError ();
    ACE_ASSERT (error_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathRegisterNs(\"%s\":\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (NET_WLAN_PROFILE_NAMESPACE_PREFIX_STRING),
                ACE_TEXT (NET_WLAN_PROFILE_NAMESPACE_HREF_STRING),
                ACE_TEXT (error_p->message)));
    xmlResetError (error_p); error_p = NULL;
    goto clean;
  } // end IF
  xpath_object_p =
    xmlXPathEvalExpression (BAD_CAST (xpath_query_string.c_str ()),
                            xpath_context_p);
  if (unlikely (!xpath_object_p))
  {
    xmlErrorPtr error_p = xmlGetLastError ();
    ACE_ASSERT (error_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathEvalExpression(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_WLANCONFIG_SSIDCONFIG_SSID_XPATH_STRING),
                ACE_TEXT (error_p->message)));
    xmlResetError (error_p); error_p = NULL;
    goto clean;
  } // end IF
  ACE_ASSERT (xpath_object_p->type == XPATH_NODESET);
  ACE_ASSERT (xpath_object_p->nodesetval);
  ACE_ASSERT (!xmlXPathNodeSetIsEmpty (xpath_object_p->nodesetval));
  ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[0]);
  ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE);
  node_p = xpath_object_p->nodesetval->nodeTab[0]; // SSID
  ACE_ASSERT (node_p);
  for (node_2 = node_p->children;
       node_2;
       node_2 = node_2->next)
  {
    if (node_2->type != XML_ELEMENT_NODE)
      continue;
    if (!ACE_OS::strcmp (reinterpret_cast<const char*> (node_2->name),
                         ACE_TEXT_ALWAYS_CHAR (NET_WLAN_PROFILE_NAME_ELEMENT_STRING)))
      break;
  } // end FOR
  if (!node_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid XML schema --> check implementation !, aborting\n")));
    goto clean;
  } // end IF
  ACE_ASSERT (node_2->children);
  ACE_ASSERT (!node_2->children->next);
  ACE_ASSERT (node_2->children->type == XML_TEXT_NODE);
  node_2 = node_2->children;
  xmlNodeSetContent (node_2,
                     BAD_CAST (SSID_in.c_str ()));
  xmlDocDumpMemory (document_p,
                    &data_p,
                    &size_i);
  ACE_ASSERT (data_p && size_i);
  strProfileXml =
    Common_String_Tools::to (reinterpret_cast<char*> (data_p));
  ACE_ASSERT (strProfileXml);
#else
  ACE_ASSERT (false); // *TODO* (see above)
#endif // LIBXML2_SUPPORT
  result_3 = WlanSetProfile (clientHandle_in,
                             &interfaceIdentifier_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
                             0,
#else
                             dwFlags,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
                             strProfileXml,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
                             NULL,
#else
                             strAllUserProfileSecurity,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
                             FALSE, // bOverwrite
                             NULL,
                             &dwReasonCode);
  if (unlikely (result_3 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanSetProfile(0x%@,\"%s\",%s): \"%s\" (reason code: %d), aborting\n"),
                clientHandle_in,
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result_3, false).c_str ()),
                dwReasonCode));
    goto clean;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": created profile (SSID was: %s), returning\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG

clean:
#if defined (MSXML_SUPPORT)
  if (xpath_query_p)
    SysFreeString (xpath_query_p);
  if (string_p)
    SysFreeString (string_p);
  result = VariantClear (&variant_s);
  ACE_ASSERT (SUCCEEDED (result));
  if (node_p)
    node_p->Release ();
  if (document_p)
    document_p->Release ();
  CoUninitialize ();
#elif defined (LIBXML2_SUPPORT)
  if (xpath_context_p)
    xmlXPathFreeContext (xpath_context_p);
  if (document_p)
    xmlFreeDoc (document_p);
#endif

  return (result_3 == ERROR_SUCCESS);
}

bool
Net_WLAN_Tools::associate (HANDLE clientHandle_in,
                           REFGUID interfaceIdentifier_in,
                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associate"));

  // sanity check(s)
  bool release_handle = false;
  HANDLE client_handle = clientHandle_in;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  ACE_ASSERT (SSID_in.size () <= DOT11_SSID_MAX_LENGTH);

  bool result = false;
  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = static_cast<ULONG> (SSID_in.size ());
  ACE_OS::memcpy (ssid_s.ucSSID,
                  SSID_in.c_str (),
                  SSID_in.size ());
  struct _WLAN_CONNECTION_PARAMETERS wlan_connection_parameters_s;
  ACE_OS::memset (&wlan_connection_parameters_s,
                  0,
                  sizeof (struct _WLAN_CONNECTION_PARAMETERS));
  std::string profile_name_string =
    Net_WLAN_Tools::getProfile (client_handle,
                                interfaceIdentifier_in,
                                SSID_in);
  ACE_ASSERT (profile_name_string.size () <= WLAN_MAX_NAME_LENGTH);
  // *TODO*: do the research here
  if (likely (!profile_name_string.empty ()))
  {
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": found profile (SSID was: %s): \"%s\", applying\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (profile_name_string.c_str ())));
#endif // _DEBUG
    wlan_connection_parameters_s.strProfile =
      ACE_Ascii_To_Wide::convert (profile_name_string.c_str ());
    wlan_connection_parameters_s.wlanConnectionMode =
      wlan_connection_mode_profile;
  } // end IF
  else
  {
    // *TODO*: this specification is not precise enough
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": no applicable profile found (SSID was: %s), generating\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG
    if (!Net_WLAN_Tools::setProfile (client_handle,
                                     interfaceIdentifier_in,
                                     SSID_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setProfile(0x%@,%s), aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  client_handle,
                  ACE_TEXT (SSID_in.c_str ())));
      return false;
    } // end IF
    wlan_connection_parameters_s.strProfile =
      ACE_Ascii_To_Wide::convert (SSID_in.c_str ());
    wlan_connection_parameters_s.wlanConnectionMode =
      wlan_connection_mode_profile;
#else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("\"%s\": no applicable profile found (SSID was: %s), trying 'wlan_connection_mode_auto'\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (profile_name_string.c_str ())));
    // *TODO*: do the research here
    wlan_connection_parameters_s.wlanConnectionMode =
      wlan_connection_mode_auto;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
  } // end ELSE
  wlan_connection_parameters_s.pDot11Ssid = &ssid_s;
  //wlan_connection_parameters_s.pDesiredBssidList = NULL;
  wlan_connection_parameters_s.dot11BssType =
    dot11_BSS_type_infrastructure;
    //wlan_connection_parameters_s.dwFlags = 0;
  // *NOTE*: this returns immediately
  DWORD result_2 =
    (SSID_in.empty () ? WlanDisconnect (client_handle,
                                        &interfaceIdentifier_in,
                                        NULL)
                      : WlanConnect (client_handle,
                                     &interfaceIdentifier_in,
                                     &wlan_connection_parameters_s,
                                     NULL));
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::%s(0x%@,%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                (SSID_in.empty () ? ACE_TEXT ("WlanDisconnect") : ACE_TEXT ("WlanConnect")),
                client_handle,
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF

  result = true;

clean:
  if (unlikely (release_handle))
    Net_WLAN_Tools::finalize (client_handle);

  return result;
}

bool
Net_WLAN_Tools::disassociate (HANDLE clientHandle_in,
                              REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disassociate"));

  bool result = false;

  // sanity check(s)
  bool release_handle = false;
  HANDLE client_handle = clientHandle_in;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
  DWORD result_2 = 0;
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (clientHandle_in);
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      goto clean;
    } // end IF
  } // end IF
  else
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifiers_a.push_back (interfaceIdentifier_in);
#else
    interface_identifiers_a.push_back (Net_Common_Tools::interfaceToString (interfaceIdentifier_in));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    // *NOTE*: this returns immediately
    result_2 = WlanDisconnect (client_handle,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                               &(*iterator),
#else
                               &Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (*iterator)),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                               NULL);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanDisconnect(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanDisconnect(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (*iterator))).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      goto clean;
    } // end IF
  } // end FOR

  result = true;

clean:
  if (unlikely (release_handle))
    Net_WLAN_Tools::finalize (client_handle);

  return result;
}

void
Net_WLAN_Tools::scan (HANDLE clientHandle_in,
                      REFGUID interfaceIdentifier_in,
                      const std::string& ESSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::scan"));

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  ACE_ASSERT (ESSID_in.size () <= DOT11_SSID_MAX_LENGTH);

  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = static_cast<ULONG> (ESSID_in.size ());
  ACE_OS::memcpy (ssid_s.ucSSID,
                  ESSID_in.c_str (),
                  ESSID_in.size ());
  // *TODO*: support attaching an information element
  //struct _WLAN_RAW_DATA raw_data_s;
  //ACE_OS::memset (&raw_data_s, 0, sizeof (struct _WLAN_RAW_DATA));
  // *NOTE*: this returns immediately
  DWORD result_2 = WlanScan (clientHandle_in,
                             &interfaceIdentifier_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
                             NULL, // *NOTE*: support WinXP
#else
                             (ESSID_in.empty () ? NULL : &ssid_s),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
                             NULL, // *NOTE*: support WinXP
#else
                             NULL,
                             //&raw_data_s,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
                             NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    if (result_2 != ERROR_BUSY) // 170: The requested resource is in use
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@,%s): \"%s\", returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (ESSID_in.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#if defined (_DEBUG)
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@,%s): \"%s\", returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (ESSID_in.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#endif // _DEBUG
  } // end IF
}

struct ether_addr
Net_WLAN_Tools::associatedBSSID (HANDLE clientHandle_in,
                                 REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedBSSID"));

  // initialize return value(s)
  struct ether_addr return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct ether_addr));

  // sanity check(s)
  if (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return return_value;
  } // end IF

  DWORD data_size = 0;
  PVOID data_p = NULL;
  struct _WLAN_CONNECTION_ATTRIBUTES* wlan_connection_attributes_p = NULL;
  DWORD result_2 =
    WlanQueryInterface (clientHandle_in,
                        &interfaceIdentifier_in,
                        wlan_intf_opcode_current_connection,
                        NULL,
                        &data_size,
                        &data_p,
                        NULL);
  if (unlikely ((result_2 != ERROR_SUCCESS) &&
                (result_2 != ERROR_INVALID_STATE)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanQueryInterface(\"%s\",wlan_intf_opcode_current_connection): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return return_value;
  } // end IF
  if (result_2 == ERROR_INVALID_STATE) // <-- not connected
    return return_value;
  ACE_ASSERT (data_p && (data_size == sizeof (struct _WLAN_CONNECTION_ATTRIBUTES)));
  wlan_connection_attributes_p =
    static_cast<struct _WLAN_CONNECTION_ATTRIBUTES*> (data_p);
  ACE_OS::memcpy (&return_value,
                  wlan_connection_attributes_p->wlanAssociationAttributes.dot11Bssid,
                  ETH_ALEN);

  // clean up
  WlanFreeMemory (data_p);

  return return_value;
}

std::string
Net_WLAN_Tools::associatedSSID (HANDLE clientHandle_in,
                                REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
  {
    Net_InterfaceIdentifiers_t interface_identifiers_a =
      Net_WLAN_Tools::getInterfaces (clientHandle_in);
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      return result;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifier = interface_identifiers_a.front ();
#else
    interface_identifier =
      Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (interface_identifiers_a.front ()));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    interface_identifier = interface_identifiers_a.front ();
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  ACE_ASSERT (!InlineIsEqualGUID (interface_identifier, GUID_NULL));

  HANDLE client_handle = clientHandle_in;
  bool release_handle = false;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return result;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);

  DWORD data_size = sizeof (struct _WLAN_CONNECTION_ATTRIBUTES);
  PVOID data_p = NULL;
  enum _WLAN_OPCODE_VALUE_TYPE op_code_value_type_e =
    wlan_opcode_value_type_invalid;
  struct _WLAN_CONNECTION_ATTRIBUTES* wlan_connection_attributes_p = NULL;
  DWORD result_2 =
    WlanQueryInterface (client_handle,
                        &interface_identifier,
                        wlan_intf_opcode_current_connection,
                        NULL,
                        &data_size,
                        &data_p,
                        &op_code_value_type_e);
  if (unlikely ((result_2 != ERROR_SUCCESS)          && // 0
                (result_2 != ERROR_CAN_NOT_COMPLETE) && // 1003
                (result_2 != ERROR_INVALID_STATE)))     // 5023
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanQueryInterface(\"%s\",%d): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                wlan_intf_opcode_current_connection,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  if ((result_2 == ERROR_CAN_NOT_COMPLETE) ||
      (result_2 == ERROR_INVALID_STATE))      // <-- not connected
    return result;
  ACE_ASSERT (data_p && (data_size == sizeof (struct _WLAN_CONNECTION_ATTRIBUTES)));
  wlan_connection_attributes_p =
    static_cast<struct _WLAN_CONNECTION_ATTRIBUTES*> (data_p);
  result.assign (reinterpret_cast<char*> (wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.ucSSID),
                 wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.uSSIDLength);

clean:
  if (data_p)
  {
    WlanFreeMemory (data_p); data_p = NULL;
  } // end IF
  if (release_handle)
  {
    Net_WLAN_Tools::finalize (client_handle); client_handle = NULL;
  } // end IF

  return result;
}

Net_WLAN_SSIDs_t
Net_WLAN_Tools::getSSIDs (HANDLE clientHandle_in,
                          REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getSSIDs"));

  Net_WLAN_SSIDs_t result;

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  HANDLE client_handle = clientHandle_in;
  bool release_handle = false;
  if (unlikely (client_handle == ACE_INVALID_HANDLE))
  {
    if (unlikely (!Net_WLAN_Tools::initialize (client_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
      return result;
    } // end IF
    ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);
    release_handle = true;
  } // end IF
  ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);

  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (clientHandle_in);
  else
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifiers_a.push_back (interfaceIdentifier_in);
#else
    interface_identifiers_a.push_back (Net_Common_Tools::interfaceToString (interfaceIdentifier_in));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  DWORD result_2 = 0;
  struct _GUID interface_identifier_s = GUID_NULL;
  DWORD flags_i =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
    0;
#else
    (WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES        |
     WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
  struct _WLAN_AVAILABLE_NETWORK_LIST* wlan_network_list_p = NULL;
#if defined (_DEBUG)
  std::string SSID_string;
  std::string bss_network_type_string;
#endif // _DEBUG
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    interface_identifier_s =
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
      *iterator;
#else
      Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (*iterator));
    ACE_ASSERT (!InlineIsEqualGUID (interface_identifier_s, GUID_NULL));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
    result_2 =
      WlanGetAvailableNetworkList (client_handle,
                                   &interface_identifier_s,
                                   flags_i,
                                   NULL,
                                   &wlan_network_list_p);
    if (unlikely (result_2 != ERROR_SUCCESS)) // ERROR_CAN_NOT_COMPLETE: 1003
    {                                         // ERROR_NDIS_DOT11_POWER_STATE_INVALID: 2150899714
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetAvailableNetworkList(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier_s).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (wlan_network_list_p);
    for (DWORD i = 0;
         i < wlan_network_list_p->dwNumberOfItems;
         ++i)
    {
      SSID_string.assign (reinterpret_cast<CHAR*> (wlan_network_list_p->Network[i].dot11Ssid.ucSSID),
                          static_cast<std::string::size_type> (wlan_network_list_p->Network[i].dot11Ssid.uSSIDLength));
#if defined (_DEBUG)
      //switch (wlan_network_list_p->Network[i].dot11BssType)
      //{
      //  case dot11_BSS_type_infrastructure:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("infrastructure");
      //    break;
      //  case dot11_BSS_type_independent:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("ad-hoc");
      //    break;
      //  case dot11_BSS_type_any:
      //  default:
      //    bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("unknown type");
      //    break;
      //} // end SWITCH
      //// *TODO*: report all available information here
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("[#%u] profile \"%s\"; SSID: %s; type: %s; connectable: \"%s\"%s; signal quality %d%% [RSSI: %d (dBm)]; security enabled: \"%s\"\n"),
      //            i + 1,
      //            (wlan_network_list_p->Network[i].strProfileName ? (ACE_OS::strlen (wlan_network_list_p->Network[i].strProfileName) ? ACE_TEXT_WCHAR_TO_TCHAR (wlan_network_list_p->Network[i].strProfileName) : ACE_TEXT ("N/A")) : ACE_TEXT ("N/A")),
      //            ACE_TEXT (SSID_string.c_str ()),
      //            ACE_TEXT (bss_network_type_string.c_str ()),
      //            (wlan_network_list_p->Network[i].bNetworkConnectable ? ACE_TEXT ("yes") : ACE_TEXT ("no")),
      //            (wlan_network_list_p->Network[i].bNetworkConnectable ? ACE_TEXT ("") : ACE_TEXT (" [reason]")),
      //            wlan_network_list_p->Network[i].wlanSignalQuality, (-100 + static_cast<int> (static_cast<float> (wlan_network_list_p->Network[i].wlanSignalQuality) * ::abs ((-100.0F - -50.0F) / 100.0F))),
      //            (wlan_network_list_p->Network[i].bSecurityEnabled ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#endif // _DEBUG
      result.push_back (SSID_string);
    } // end FOR
    WlanFreeMemory (wlan_network_list_p); wlan_network_list_p = NULL;
  } // end FOR

error:
  if (wlan_network_list_p)
    WlanFreeMemory (wlan_network_list_p);
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);

  // *NOTE*: there may be duplicate entries (e.g. one for each network profile)
  //         --> remove duplicates
  std::sort (result.begin (), result.end ());
  Net_WLAN_SSIDsIterator_t iterator =
    std::unique (result.begin (), result.end ());
  result.erase (iterator, result.end ());

  return result;
}

bool
Net_WLAN_Tools::hasSSID (HANDLE clientHandle_in,
                         REFGUID interfaceIdentifier_in,
                         const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
  if (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return result;
  } // end IF

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return result;)
}
