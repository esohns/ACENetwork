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
#include "NetworkManager/NetworkManager.h"

#include "net_configuration.h"
#endif

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
            iwlanmonitor_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
          }
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_acm_scan_complete");
          break;
        }
        case wlan_notification_acm_scan_fail:
        {
          try {
            iwlanmonitor_cb_p->onScanComplete (data_in->InterfaceGuid);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
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
            iwlanmonitor_cb_p->onAssociate (data_in->InterfaceGuid,
                                            SSID_string,
                                            (wlan_connection_notification_data_p->wlanReasonCode == WLAN_REASON_CODE_SUCCESS));
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onAssociate(%s), continuing\n"),
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
            iwlanmonitor_cb_p->onHotPlug (data_in->InterfaceGuid,
                                          true);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n"),
                        ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ())));
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
                        ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n"),
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
          notification_string =
            ACE_TEXT_ALWAYS_CHAR ("wlan_notification_msm_signal_quality_change");
          break;
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
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: received notification \"%s\"\n"),
              ACE_TEXT (Common_Tools::GUIDToString (data_in->InterfaceGuid).c_str ()),
              ACE_TEXT (notification_string.c_str ())));
}
#else
//void
//network_wlan_dbus_main_wakeup_cb (void* userData_in)
//{
//  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_dbus_main_wakeup_cb "));

//  // sanity check(s)
//  ACE_ASSERT (userData_in);

//  struct DBusConnection* connection_p =
//      static_cast<struct DBusConnection*> (userData_in);
//  ACE_ASSERT (connection_p);

//  int socket_fd = -1;
//  if (!dbus_connection_get_socket (connection_p,
//                                   &socket_fd))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dbus_connection_get_socket(0x%@): \"%m\", returning\n"),
//                connection_p));
//    return;
//  } // end IF
//  ACE_ASSERT (socket_fd != -1);

//  int result = ACE_OS::raise (SIGPOLL); // *TODO*: prefer SIGIO
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_OS::raise(SIGPOLL): \"%m\", continuing\n")));
//}

DBusHandlerResult
network_wlan_dbus_default_filter_cb (struct DBusConnection* connection_in,
                                     struct DBusMessage* message_in,
                                     void* userData_in)
{
//  NETWORK_TRACE (ACE_TEXT ("network_wlan_dbus_default_filter_cb"));

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (message_in);
  if (dbus_message_get_type (message_in) != DBUS_MESSAGE_TYPE_SIGNAL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  ACE_ASSERT (userData_in);
  Net_WLAN_IMonitorBase* imonitorbase_p =
      static_cast<Net_WLAN_IMonitorBase*> (userData_in);
  ACE_ASSERT (imonitorbase_p);

  bool result = false;
  struct DBusError error_s;
  dbus_error_init (&error_s);
  std::string object_path_string, device_identifier_string;
  const char* object_path_p = NULL;

  if (dbus_message_is_signal (message_in,
                              ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_INTERFACE),
                              ACE_TEXT_ALWAYS_CHAR ("DeviceAdded")))
  {
    if (!dbus_message_get_args (message_in,
                                &error_s,
                                DBUS_TYPE_OBJECT_PATH,
                                &object_path_p,
                                DBUS_TYPE_INVALID))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_args(): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      goto continue_;
    } // end IF
    ACE_ASSERT (object_path_p);

    object_path_string = object_path_p;
    device_identifier_string =
        Net_WLAN_Tools::deviceDBusPathToIdentifier (connection_in,
                                                    object_path_string);
    if (device_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF

    // add to cache
    Common_ISet2R_T<std::string>* iset_p =
        dynamic_cast<Common_ISet2R_T<std::string>*> (imonitorbase_p);
    ACE_ASSERT (iset_p);
    iset_p->set2R (device_identifier_string, object_path_string);

    try {
      imonitorbase_p->onHotPlug (device_identifier_string,
                                     true);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onHotPlug(), continuing\n")));
      goto continue_;
    }
  } // end IF
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_INTERFACE),
                                   ACE_TEXT_ALWAYS_CHAR ("DeviceRemoved")))
  {
    if (!dbus_message_get_args (message_in,
                                &error_s,
                                DBUS_TYPE_OBJECT_PATH,
                                &object_path_p,
                                DBUS_TYPE_INVALID))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_args(): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      goto continue_;
    } // end IF
    ACE_ASSERT (object_path_p);

    object_path_string = object_path_p;
    // check cache first
    Common_IGet1R_T<std::string>* iget_p =
        dynamic_cast<Common_IGet1R_T<std::string>*> (imonitorbase_p);
    ACE_ASSERT (iget_p);
    // *NOTE*: this fails, the device has gone away
//    device_identifier_string =
//        Net_Common_Tools::deviceDBusPathToIdentifier (connection_in,
//                                                      object_path_string);
    device_identifier_string = iget_p->get1R (object_path_string);
    if (device_identifier_string.empty ())
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("failed to Net_Common_Tools::deviceDBusPathToIdentifier(\"%s\"), aborting\n"),
//                  ACE_TEXT (object_path_string.c_str ())));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to retrieve device identifier (DBus object path was: \"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    try {
      imonitorbase_p->onHotPlug (device_identifier_string,
                                     false);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onHotPlug(), continuing\n")));
      goto continue_;
    }
  } // end ELSE IF
  ////////////////////////////////////////
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_INTERFACE),
                                   ACE_TEXT_ALWAYS_CHAR ("StateChanged")))
  {
    NMDeviceState state_previous, state_current;
    NMDeviceStateReason reason;
    std::string active_access_point_path_string, SSID_string;
    Common_IGet1R_T<std::string>* iget_p = NULL;

    if (!dbus_message_get_args (message_in,
                                &error_s,
                                DBUS_TYPE_UINT32,
                                &state_current,
                                DBUS_TYPE_UINT32,
                                &state_previous,
                                DBUS_TYPE_UINT32,
                                &reason,
                                DBUS_TYPE_INVALID))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_args(): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      goto continue_;
    } // end IF
    switch (state_current)
    {
      case NM_DEVICE_STATE_IP_CONFIG:
      case NM_DEVICE_STATE_ACTIVATED:
      case NM_DEVICE_STATE_DEACTIVATING:
      case NM_DEVICE_STATE_DISCONNECTED:
        break;
      default:
        goto continue_;
    } // end SWITCH

    object_path_p = dbus_message_get_path (message_in);
    ACE_ASSERT (object_path_p);
    object_path_string = object_path_p;
    if (object_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_path(), aborting\n")));
      goto continue_;
    } // end IF
    // check cache first
    iget_p =
        dynamic_cast<Common_IGet1R_T<std::string>*> (imonitorbase_p);
    ACE_ASSERT (iget_p);
    device_identifier_string = iget_p->get1R (object_path_string);
    if (device_identifier_string.empty ())
      device_identifier_string =
          Net_WLAN_Tools::deviceDBusPathToIdentifier (connection_in,
                                                      object_path_string);
    if (device_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    if (ACE_OS::strcmp (device_identifier_string.c_str (),
                        imonitorbase_p->interfaceIdentifier ().c_str ()))
      goto continue_; // --> not interested
    // *NOTE*: this may fail if the device has-/is- disconnect-ed/-ing
    if ((state_current == NM_DEVICE_STATE_IP_CONFIG) ||
        (state_current == NM_DEVICE_STATE_ACTIVATED))
    {
      active_access_point_path_string =
          Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath (connection_in,
                                                                     object_path_string);
      if (active_access_point_path_string.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath(\"%s\"), aborting\n"),
                    ACE_TEXT (object_path_string.c_str ())));
        goto continue_;
      } // end IF
      SSID_string =
          Net_WLAN_Tools::accessPointDBusPathToSSID (connection_in,
                                                     active_access_point_path_string);
      if (SSID_string.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusPathToSSID(\"%s\"), aborting\n"),
                    ACE_TEXT (active_access_point_path_string.c_str ())));
        goto continue_;
      } // end IF
    } // end IF
    switch (state_current)
    {
      case NM_DEVICE_STATE_IP_CONFIG:
      {
        try {
          imonitorbase_p->onAssociate (device_identifier_string,
                                       SSID_string,
                                       true);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_ACTIVATED:
      {
        try {
          imonitorbase_p->onConnect (device_identifier_string,
                                     SSID_string,
                                     true);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_DEACTIVATING:
      {
        try {
          imonitorbase_p->onConnect (device_identifier_string,
                                     SSID_string,
                                     false);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_DISCONNECTED:
      {
        try {
          imonitorbase_p->onAssociate (device_identifier_string,
                                       SSID_string,
                                       false);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(), continuing\n")));
          goto continue_;
        }
        break;
      }
      default:
        break;
    } // end SWITCH
  } // end ELSE IF
  ////////////////////////////////////////
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE),
                                   ACE_TEXT_ALWAYS_CHAR ("StateChanged")))
  {
    NMActiveConnectionState state_current;
    ACE_UINT32 reason;
    std::string active_access_point_path_string, SSID_string;
    std::string device_path_string;

    if (!dbus_message_get_args (message_in,
                                &error_s,
                                DBUS_TYPE_UINT32,
                                &state_current,
                                DBUS_TYPE_UINT32,
                                &reason,
                                DBUS_TYPE_INVALID))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_args(): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      goto continue_;
    } // end IF
    switch (state_current)
    {
//      case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
      case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATING:
//      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
        break;
      default:
        goto continue_;
    } // end SWITCH

    object_path_p = dbus_message_get_path (message_in);
    ACE_ASSERT (object_path_p);
    object_path_string = object_path_p;
    if (object_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_path(), aborting\n")));
      goto continue_;
    } // end IF
    device_path_string =
        Net_WLAN_Tools::activeConnectionDBusPathToDeviceDBusPath (connection_in,
                                                                  object_path_string);
    if (device_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLANs_Tools::activeConnectionDBusPathToDeviceDBusPath(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    device_identifier_string =
        Net_WLAN_Tools::deviceDBusPathToIdentifier (connection_in,
                                                    device_path_string);
    if (device_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (device_path_string.c_str ())));
      goto continue_;
    } // end IF
    active_access_point_path_string =
        Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath (connection_in,
                                                                   device_path_string);
    if (active_access_point_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath(\"%s\"), aborting\n"),
                  ACE_TEXT (device_path_string.c_str ())));
      goto continue_;
    } // end IF
    SSID_string =
        Net_WLAN_Tools::accessPointDBusPathToSSID (connection_in,
                                                   active_access_point_path_string);
    if (SSID_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusPathToSSID(\"%s\"), aborting\n"),
                  ACE_TEXT (active_access_point_path_string.c_str ())));
      goto continue_;
    } // end IF
    switch (state_current)
    {
//      case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
//      {
//        try {
//          imonitorbase_p->onAssociate (device_identifier_string,
//                                           SSID_string,
//                                           true);
//        } catch (...) {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onAssociate(), continuing\n")));
//          goto continue_;
//        }
//        break;
//      }
      case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
      {
        try {
          imonitorbase_p->onConnect (device_identifier_string,
                                         SSID_string,
                                         true);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onConnect(), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATING:
      {
        try {
          imonitorbase_p->onConnect (device_identifier_string,
                                         SSID_string,
                                         false);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onConnect(), continuing\n")));
          goto continue_;
        }
        break;
      }
//      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
//      {
//        try {
//          imonitorbase_p->onAssociate (device_identifier_string,
//                                           SSID_string,
//                                           false);
//        } catch (...) {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onAssociate(), continuing\n")));
//          goto continue_;
//        }
//        break;
//      }
      default:
        break;
    } // end SWITCH
  } // end ELSE IF
  ////////////////////////////////////////
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE),
                                   ACE_TEXT_ALWAYS_CHAR ("AccessPointAdded")))
  {
    object_path_p = dbus_message_get_path (message_in);
    ACE_ASSERT (object_path_p);
    object_path_string = object_path_p;
    if (object_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_get_path(), aborting\n")));
      goto continue_;
    } // end IF
    device_identifier_string =
        Net_WLAN_Tools::deviceDBusPathToIdentifier (connection_in,
                                                    object_path_string);

    if (device_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    try {
      imonitorbase_p->onScanComplete (device_identifier_string);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
    }
  } // end ELSE IF
//  else if (dbus_message_is_signal (message_in,
//                                   ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_DBUS_NETWORKMANAGER_WIRELESS_INTERFACE),
//                                   ACE_TEXT_ALWAYS_CHAR ("AccessPointRemoved")))
//  {
//    if (!dbus_message_get_args (message_in,
//                                &error_s,
//                                DBUS_TYPE_OBJECT_PATH,
//                                &object_path_p,
//                                DBUS_TYPE_INVALID))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to dbus_message_get_args(): \"%s\", aborting\n"),
//                  ACE_TEXT (error_s.message)));

//      dbus_error_free (&error_s);

//      goto continue_;
//    } // end IF
//    ACE_ASSERT (object_path_p);
//    SSID_string = Net_Common_Tools::accessPointDBusPathToSSID (connection_in,
//                                                               object_path_p);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("lost access point (SSID was: %s)...\n"),
//                ACE_TEXT (SSID_string.c_str ())));
//  } // end ELSE IF
  else
  {
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("invalid/unknown dbus signal (was: \"%s\":\"%s\"), continuing\n"),
//                ACE_TEXT (dbus_message_get_interface (message_in)),
//                ACE_TEXT ("")));
    goto continue_;
  } // end IF

  result = true;

continue_:
  if (dbus_error_is_set (&error_s))
    dbus_error_free (&error_s);

  return (result ? DBUS_HANDLER_RESULT_HANDLED
                 : DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}
//void
//network_wlan_networkmanager_state_changed_cb (struct DBusGProxy* proxy_in,
//                                              guint32 state_in,
//                                              gpointer userData_in)
//{
//  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_networkmanager_state_changed_cb"));

//  ACE_UNUSED_ARG (proxy_in);

//  // sanity check(s)
//  ACE_ASSERT (userData_in);

//  Net_WLAN_IMonitorCB* iwlan_cb_p = static_cast<Net_WLAN_IMonitorCB*> (userData_in);
//}
#endif

//////////////////////////////////////////
