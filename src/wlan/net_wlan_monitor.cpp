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
#include "net_wlan_monitor.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
#include "iwlib.h"
#endif  // WEXT_SUPPORT

#if defined (NL80211_SUPPORT)
#include "netlink/errno.h"
#endif  // NL80211_SUPPORT

#if defined (DBUS_SUPPORT)
#include "NetworkManager.h"
#endif // DBUS_SUPPORT

#include "common_timer_tools.h"

#include "net_configuration.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (DBUS_SUPPORT)
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
//    ACE_DEBUG ((LM_ERROR,nl_geterror
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
  Net_WLAN_IMonitorBase* imonitor_p =
      static_cast<Net_WLAN_IMonitorBase*> (userData_in);
  Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
      dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (imonitor_p);
  ACE_ASSERT (istate_machine_p);

  bool result = false;
  struct DBusError error_s;
  dbus_error_init (&error_s);
  std::string object_path_string, interface_identifier_string;
  const char* object_path_p = NULL;

  if (dbus_message_is_signal (message_in,
                              ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE),
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
    interface_identifier_string =
        Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_in,
                                                          object_path_string);
    if (interface_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF

    // add to cache
    Common_ISet2R_T<std::string,
                    std::string>* iset_p =
        dynamic_cast<Common_ISet2R_T<std::string,
                                     std::string>*> (imonitor_p);
    ACE_ASSERT (iset_p);
    iset_p->set2R (interface_identifier_string,
                   object_path_string);

    try {
      imonitor_p->onHotPlug (interface_identifier_string,
                             true);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onHotPlug(), continuing\n")));
      goto continue_;
    }
  } // end IF
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE),
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
    Common_IGet1RR_2_T<std::string,
                       std::string>* iget_p =
        dynamic_cast<Common_IGet1RR_2_T<std::string,
                                        std::string>*> (imonitor_p);
    ACE_ASSERT (iget_p);
    // *NOTE*: this fails, the device has gone away
//    interface_identifier_string =
//        Net_Common_Tools::deviceDBusObjectPathToIdentifier (connection_in,
//                                                      object_path_string);
    interface_identifier_string = iget_p->get1RR_2 (object_path_string);
    if (interface_identifier_string.empty ())
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("failed to Net_Common_Tools::deviceDBusObjectPathToIdentifier(\"%s\"), aborting\n"),
//                  ACE_TEXT (object_path_string.c_str ())));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to retrieve device identifier (DBus object path was: \"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    try {
      imonitor_p->onHotPlug (interface_identifier_string,
                             false);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB;;onHotPlug(), continuing\n")));
      goto continue_;
    }
  } // end ELSE IF
  ////////////////////////////////////////
  else if (dbus_message_is_signal (message_in,
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE),
                                   ACE_TEXT_ALWAYS_CHAR ("StateChanged")))
  {
    NMDeviceState state_previous, state_current;
    NMDeviceStateReason reason;
    std::string active_access_point_path_string, SSID_string;
    Common_IGet1RR_2_T<std::string,
                       std::string>* iget_p = NULL;

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
        dynamic_cast<Common_IGet1RR_2_T<std::string,
                                        std::string>*> (imonitor_p);
    ACE_ASSERT (iget_p);
    interface_identifier_string = iget_p->get1RR_2 (object_path_string);
    if (interface_identifier_string.empty ())
      interface_identifier_string =
          Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_in,
                                                      object_path_string);
    if (interface_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    if (ACE_OS::strcmp (interface_identifier_string.c_str (),
                        imonitor_p->interfaceIdentifier ().c_str ()))
      goto continue_; // --> not interested
    // *NOTE*: this may fail if the device has-/is- disconnect-ed/-ing
    if ((state_current == NM_DEVICE_STATE_IP_CONFIG) ||
        (state_current == NM_DEVICE_STATE_ACTIVATED))
    {
      active_access_point_path_string =
          Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath (connection_in,
                                                                     object_path_string);
      if (active_access_point_path_string.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath(\"%s\"), aborting\n"),
                    ACE_TEXT (object_path_string.c_str ())));
        goto continue_;
      } // end IF
      SSID_string =
          Net_WLAN_Tools::accessPointDBusObjectPathToSSID (connection_in,
                                                           active_access_point_path_string);
      if (SSID_string.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusObjectPathToSSID(\"%s\"), aborting\n"),
                    ACE_TEXT (active_access_point_path_string.c_str ())));
        goto continue_;
      } // end IF
    } // end IF
    switch (state_current)
    {
      case NM_DEVICE_STATE_IP_CONFIG:
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_ASSOCIATED), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_ACTIVATED:
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_CONNECTED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_CONNECTED), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_DEACTIVATING:
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_DISCONNECTED), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_DEVICE_STATE_DISCONNECTED:
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_SCANNED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_SCANNED), continuing\n")));
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
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE),
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
        Net_WLAN_Tools::activeConnectionDBusObjectPathToDeviceDBusObjectPath (connection_in,
                                                                              object_path_string);
    if (device_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLANs_Tools::activeConnectionDBusObjectPathToDeviceDBusObjectPath(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    interface_identifier_string =
        Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_in,
                                                          device_path_string);
    if (interface_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (device_path_string.c_str ())));
      goto continue_;
    } // end IF
    active_access_point_path_string =
        Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath (connection_in,
                                                                               device_path_string);
    if (active_access_point_path_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath(\"%s\"), aborting\n"),
                  ACE_TEXT (device_path_string.c_str ())));
      goto continue_;
    } // end IF
    SSID_string =
        Net_WLAN_Tools::accessPointDBusObjectPathToSSID (connection_in,
                                                         active_access_point_path_string);
    if (SSID_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusObjectPathToSSID(\"%s\"), aborting\n"),
                  ACE_TEXT (active_access_point_path_string.c_str ())));
      goto continue_;
    } // end IF
    switch (state_current)
    {
//      case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
//      {
//        try {
//          imonitorbase_p->onAssociate (interface_identifier_string,
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
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_CONNECTED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_CONNECTED), continuing\n")));
          goto continue_;
        }
        break;
      }
      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATING:
      { ACE_ASSERT (istate_machine_p);
        try {
          istate_machine_p->change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_ASSOCIATED), continuing\n")));
          goto continue_;
        }
        break;
      }
//      case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
//      {
//        try {
//          imonitorbase_p->onAssociate (interface_identifier_string,
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
                                   ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE),
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
    interface_identifier_string =
        Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_in,
                                                          object_path_string);

    if (interface_identifier_string.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToIdentifier(\"%s\"), aborting\n"),
                  ACE_TEXT (object_path_string.c_str ())));
      goto continue_;
    } // end IF
    ACE_ASSERT (istate_machine_p);
    try {
      istate_machine_p->change (NET_WLAN_MONITOR_STATE_SCANNED);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_SCANNED), continuing\n")));
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
//    SSID_string = Net_Common_Tools::accessPointDBusObjectPathToSSID (connection_in,
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
#endif // DBUS_SUPPORT

#if defined (NL80211_SUPPORT)
int
network_wlan_nl80211_no_seq_check_cb (struct nl_msg* message_in,
                                      void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_no_seq_check_cb"));

  ACE_UNUSED_ARG (message_in);

//  // sanity check(s)
//  ACE_ASSERT (argument_in);

//  struct Net_WLAN_nl80211_CBData* cb_data_p =
//      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);

  return NL_OK;
}

int
network_wlan_nl80211_finish_cb (struct nl_msg* message_in,
                                void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_finish_cb"));

  ACE_UNUSED_ARG (message_in);

  // sanity check(s)
  ACE_ASSERT (argument_in);

  struct nlmsghdr* nlmsghdr_p = nlmsg_hdr (message_in);
  ACE_ASSERT (nlmsghdr_p);
  ACE_ASSERT (nlmsghdr_p->nlmsg_type == NLMSG_DONE);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);

  if (cb_data_p->dumping)
    cb_data_p->dumping = false;

  if (cb_data_p->scanning)
  { // received the final scan result
    cb_data_p->scanning = false;

#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": scanning...DONE in %s\n"),
                ACE_TEXT (cb_data_p->monitor->interfaceIdentifier ()).c_str (),
                ACE_TEXT (Common_Timer_Tools::periodToString (COMMON_TIME_NOW - cb_data_p->timestamp).c_str ())));
#endif // _DEBUG

    ACE_ASSERT (cb_data_p->monitor);
    Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
        dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
    ACE_ASSERT (istate_machine_p);
    try {
      istate_machine_p->change (NET_WLAN_MONITOR_STATE_SCANNED);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_SCANNED), aborting\n")));
      return NL_STOP;
    }
  } // end IF

  if (cb_data_p->done)
    *(cb_data_p->done) = 1;

  return NL_OK;
}

int
network_wlan_nl80211_ack_cb (struct nl_msg* message_in,
                             void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_ack_cb"));

  ACE_UNUSED_ARG (message_in);

//  // sanity check(s)
//  ACE_ASSERT (argument_in);

//  struct Net_WLAN_nl80211_CBData* cb_data_p =
//      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);

  return NL_OK;
}

int
network_wlan_nl80211_multicast_groups_cb (struct nl_msg* message_in,
                                          void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_multicast_groups_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->map);

  struct nlattr* nlattr_a[CTRL_ATTR_MAX + 1];
  struct nlattr* nlattr_p = NULL;
  int rem_mcgrp = 0;
  Net_WLAN_nl80211_MulticastGroupIdsIterator_t iterator;

  int result = nla_parse (nlattr_a,
                          CTRL_ATTR_MAX,
                          genlmsg_attrdata (genlmsghdr_p, 0),
                          genlmsg_attrlen (genlmsghdr_p, 0),
                          NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nla_parse(): \"%s\", aborting\n"),
                ACE_TEXT (nl_geterror (result))));
    return NL_STOP;
  } // end IF
  if (!nlattr_a[CTRL_ATTR_MCAST_GROUPS])
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'multicast groups' attribute, aborting\n")));
    return NL_STOP;
  } // end IF

  nla_for_each_nested (nlattr_p, nlattr_a[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp)
  { ACE_ASSERT (nlattr_p);
    struct nlattr* nlattr_2[CTRL_ATTR_MCAST_GRP_MAX + 1];
    result = nla_parse (nlattr_2,
                        CTRL_ATTR_MCAST_GRP_MAX,
                        static_cast<struct nlattr*> (nla_data (nlattr_p)),
                        nla_len (nlattr_p),
                        NULL);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nla_parse(): \"%s\", aborting\n"),
                  ACE_TEXT (nl_geterror (result))));
      return NL_STOP;
    } // end IF
    if (unlikely (!nlattr_2[CTRL_ATTR_MCAST_GRP_NAME] ||
                  !nlattr_2[CTRL_ATTR_MCAST_GRP_ID]))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("missing 'name'/'id' attribute(s) from 'multicast groups' set item, continuing\n")));
      continue;
    } // end IF
    iterator =
        cb_data_p->map->find (nla_get_string (nlattr_2[CTRL_ATTR_MCAST_GRP_NAME]));
    if (unlikely (iterator == cb_data_p->map->end ()))
    {
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("unknown/invalid multicast group (was name: \"%s\"; id: %u), continuing\n"),
                  ACE_TEXT (nla_get_string (nlattr_2[CTRL_ATTR_MCAST_GRP_NAME])),
                  nla_get_u32 (nlattr_2[CTRL_ATTR_MCAST_GRP_ID])));
#endif // _DEBUG
      continue;
    } // end IF
    (*iterator).second =
        static_cast<int> (nla_get_u32 (nlattr_2[CTRL_ATTR_MCAST_GRP_ID]));
  } // end FOREACH

  return NL_OK;
}

int
network_wlan_nl80211_default_handler_cb (struct nl_msg* message_in,
                                         void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_default_handler_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  // Called by the kernel when the scan is done or has been aborted.
#if defined (_DEBUG)
//  nl_msg_dump (message_in, stderr);
#endif // _DEBUG

  int return_value = NL_OK;

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->error);
//  ACE_ASSERT (!(*cb_data_p->error));
//  ACE_ASSERT (cb_data_p->monitor);

  // step1: parse message attributes
  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  int result = nla_parse (nlattr_a,
                          NL80211_ATTR_MAX,
                          genlmsg_attrdata (genlmsghdr_p, 0),
                          genlmsg_attrlen (genlmsghdr_p, 0),
                          NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nla_parse(): \"%s\", aborting\n"),
                ACE_TEXT (nl_geterror (result))));
    return NL_STOP;
  } // end IF

  switch (genlmsghdr_p->cmd)
  {
    case NL80211_CMD_GET_WIPHY:
    {
      break;
    }
    case NL80211_CMD_NEW_WIPHY:
    {
      break;
    }
    case NL80211_CMD_NEW_STATION:
    {
      break;
    }
    case NL80211_CMD_DEL_STATION:
    {
      break;
    }
    case NL80211_CMD_TRIGGER_SCAN:
    {
      break;
    }
    case NL80211_CMD_NEW_SCAN_RESULTS:
    {
      // sanity check(s)
//      ACE_ASSERT (cb_data_p->index);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      // *NOTE*: (somewhat confusingly,) control flow reaches here in two cases:
      //         - when the scan operation has completed (i.e. comand
      //           NL80211_CMD_TRIGGER_SCAN) has been processed)
      //           --> send NL80211_CMD_GET_SCAN to retrieve the result(s)
      //         - when scan results are being notified (i.e. command
      //           NL80211_CMD_GET_SCAN is being processed)
      //           --> update cache entries
      //         The cases may be distinguished by the fact that case 1 is a
      //         broadcast event sent to the "scan" multicast group, whereas
      //         case 2 are a unicast message(s) forwarded to the sender of the
      //         NL80211_CMD_GET_SCAN command
      struct sockaddr_nl* source_address_p = nlmsg_get_src (message_in);
      ACE_ASSERT (source_address_p);
      if (source_address_p->nl_groups)
      {
        // scan complete --> fetch result(s)
        int result = -1;
        int driver_family_id_i = cb_data_p->monitor->get_3 ();
        ACE_ASSERT (driver_family_id_i);
        const struct nl_sock* socket_handle_p = cb_data_p->monitor->getP ();
        ACE_ASSERT (socket_handle_p);
        struct nl_msg* message_p = nlmsg_alloc ();
        if (unlikely (!message_p))
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
          return NL_STOP;
        } // end IF
        if (unlikely (!genlmsg_put (message_p,
                                    NL_AUTO_PORT,         // port #
                                    NL_AUTO_SEQ,          // sequence #
                                    driver_family_id_i,   // family id
                                    0,                    // (user-) hdrlen
                                    NLM_F_DUMP,           // flags
                                    NL80211_CMD_GET_SCAN, // command id
                                    0)))                  // interface version
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
          nlmsg_free (message_p);
          return NL_STOP;
        } // end IF
        NLA_PUT_U32 (message_p,
                     NL80211_ATTR_IFINDEX,
                     if_index_i);
        result =
            nl_send_auto_complete (const_cast<struct nl_sock*> (socket_handle_p),
                                   message_p);
        if (unlikely (result < 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to nl_send_auto_complete(%@): \"%s\", aborting\n"),
                      socket_handle_p,
                      ACE_TEXT (nl_geterror (result))));
          nlmsg_free (message_p);
          return NL_STOP;
        } // end IF
nla_put_failure:
        nlmsg_free (message_p);
        break;
      } // end IF

      // scan result(s)
      struct nlattr* nlattr_2[NL80211_BSS_MAX + 1];
      uint8_t length_i = 0;
      uint8_t* data_p = NULL;
      int offset_i = 0;
      struct Net_WLAN_IEEE802_11_InformationElement* information_element_p =
          NULL;
      std::string ssid_string;
#if defined (_DEBUG)
      char buffer_a[IF_NAMESIZE + 1];
#endif // _DEBUG
      struct Net_WLAN_AccessPointState state_s;

      if (//!nlattr_a[NL80211_ATTR_SCAN_FREQUENCIES] ||
          //!nlattr_a[NL80211_ATTR_SCAN_SSIDS] ||
          !nlattr_a[NL80211_ATTR_BSS])
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("'bss' attribute missing, aborting\n")));
        return NL_STOP;
      } // end IF
      result = nla_parse_nested (nlattr_2,
                                 NL80211_BSS_MAX,
                                 nlattr_a[NL80211_ATTR_BSS],
                                 NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to nla_parse_nested(NL80211_ATTR_BSS): \"%s\", aborting\n"),
                    ACE_TEXT (nl_geterror (result))));
        return NL_STOP;
      } // end IF
      if (!nlattr_2[NL80211_BSS_FREQUENCY]            ||
          !nlattr_2[NL80211_BSS_SIGNAL_MBM]           ||
          !nlattr_2[NL80211_BSS_BSSID]                ||
          !nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS] ||
          !nlattr_2[NL80211_BSS_SEEN_MS_AGO])
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("'bss freq/signal/bssid/IEs/age' attribute(s) missing, aborting\n")));
        return NL_STOP;
      } // end IF

#if defined (_DEBUG)
      ACE_OS::memset (buffer_a, 0, sizeof (char[IF_NAMESIZE + 1]));
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF
#endif // _DEBUG

      state_s.frequency = nla_get_u32 (nlattr_2[NL80211_BSS_FREQUENCY]);
      state_s.lastSeen = nla_get_u32 (nlattr_2[NL80211_BSS_SEEN_MS_AGO]);
      ACE_OS::memcpy (&state_s.linkLayerAddress.ether_addr_octet,
                      nla_data (nlattr_2[NL80211_BSS_BSSID]),
                      ETH_ALEN);
      state_s.signalQuality = nla_get_u32 (nlattr_2[NL80211_BSS_SIGNAL_MBM]);
      length_i =
          static_cast<uint8_t> (nla_len (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]));
      data_p =
          reinterpret_cast<uint8_t*> (nla_data (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]));
      // extract SSID (IE id 0)
      // *NOTE*: information elements are encoded like so: id[1]len[1]/data[len]
      //         (see also: http://standards.ieee.org/findstds/standard/802.11-2016.html)
      do
      {
        if (offset_i >= length_i)
          break; // no more 'IE's
        information_element_p =
              reinterpret_cast<struct Net_WLAN_IEEE802_11_InformationElement*> (data_p + offset_i);
        // *NOTE*: see aforementioned document "Table 7-26—Element IDs"
        if (information_element_p->id == NET_WLAN_IEEE80211_INFORMATION_ELEMENT_ID_SSID)
          break;
        offset_i += (1 + 1 + information_element_p->length);
        information_element_p = NULL;
      } while (true);
      if (unlikely (!information_element_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": 'bss' missing information element (id was: %u), aborting\n"),
                    ACE_TEXT (buffer_a),
                    NET_WLAN_IEEE80211_INFORMATION_ELEMENT_ID_SSID));
        return NL_STOP;
      } // end IF
      // *TODO*: the returned SSID IE does not seem to contain any data in some
      //         cases; find out why
      if (unlikely (!information_element_p->length ||
                    (information_element_p->length > NET_WLAN_ESSID_MAX_SIZE)))
      {
        // *NOTE*: "...A 0 length information field is used within Probe Request
        //         management frames to indicate the wildcard SSID. ..."
        ACE_DEBUG (((!information_element_p->length ? LM_ERROR : LM_WARNING),
                    ACE_TEXT ("\"%s\": (MAC: %s) 'bss' information element (id was: %u) has invalid length (was: %u), aborting\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (state_s.linkLayerAddress.ether_addr_octet, NET_LINKLAYER_802_11).c_str ()),
                    NET_WLAN_IEEE80211_INFORMATION_ELEMENT_ID_SSID,
                    information_element_p->length));
        return NL_STOP;
      } // end IF
      ssid_string = Net_WLAN_Tools::decodeSSID (information_element_p->data,
                                                information_element_p->length);
      ACE_ASSERT (!ssid_string.empty ());
      cb_data_p->monitor->set3R (ssid_string,
                                 buffer_a,
                                 state_s);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": detected access point (frequency (MHz): %u, signal strength (dBm): %u.%.2u, MAC: %s, SSID: %s, age (ms): %u)...\n"),
                  ACE_TEXT (buffer_a),
                  state_s.frequency,
                  state_s.signalQuality / 100, state_s.signalQuality % 100,
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (state_s.linkLayerAddress.ether_addr_octet, NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (ssid_string.c_str ()),
                  state_s.lastSeen));
#endif // _DEBUG

      break;
    }
    case NL80211_CMD_SCAN_ABORTED:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": scan failed, continuing\n"),
                  ACE_TEXT (buffer_a)));

      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (NET_WLAN_MONITOR_STATE_SCANNED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_SCANNED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_REG_CHANGE:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_REG_INITIATOR]);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_REG_TYPE]);

      enum nl80211_reg_initiator initiator_e =
          static_cast<enum nl80211_reg_initiator> (nla_get_u32 (nlattr_a[NL80211_ATTR_REG_INITIATOR]));
      ACE_UNUSED_ARG (initiator_e);
      enum nl80211_reg_type reg_type_e =
          static_cast<enum nl80211_reg_type> (nla_get_u32 (nlattr_a[NL80211_ATTR_REG_TYPE]));
      std::string reg_domain_string;
      switch (reg_type_e)
      {
        case NL80211_REGDOM_TYPE_COUNTRY:
        { ACE_ASSERT (nlattr_a[NL80211_ATTR_REG_ALPHA2]);
          reg_domain_string =
              nla_get_string (nlattr_a[NL80211_ATTR_REG_ALPHA2]);
          break;
        }
        case NL80211_REGDOM_TYPE_WORLD:
          reg_domain_string = ACE_TEXT_ALWAYS_CHAR ("world");
          break;
        case NL80211_REGDOM_TYPE_CUSTOM_WORLD:
          reg_domain_string = ACE_TEXT_ALWAYS_CHAR ("world (custom)");
          break;
        case NL80211_REGDOM_TYPE_INTERSECTION:
          reg_domain_string = ACE_TEXT_ALWAYS_CHAR ("intersection");
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("unknown/invalid regulatory domain type (was: %d), aborting\n"),
                      reg_type_e));
          return NL_STOP;
        }
      } // end SWITCH

#if defined (_DEBUG)
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_WIPHY]);
      ACE_ASSERT (cb_data_p->monitor);
      std::string interface_identifier_string =
          cb_data_p->monitor->interfaceIdentifier ();
      unsigned int wiphy_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY]);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\"/\"%s\" [%u]: switched regulatory domain: %s\n"),
                  ACE_TEXT (interface_identifier_string.c_str ()),
                  ACE_TEXT (Net_WLAN_Tools::wiPhyIndexToWiPhyNameString (interface_identifier_string, NULL, cb_data_p->monitor->get_3 (), wiphy_index_i).c_str ()),
                  wiphy_index_i,
                  ACE_TEXT (reg_domain_string.c_str ())));
#endif // _DEBUG

      break;
    }
    case NL80211_CMD_AUTHENTICATE:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      uint16_t status_i = 0;
      uint8_t* data_p = NULL;
      size_t length_i = 0;
      if (nlattr_a[NL80211_ATTR_TIMED_OUT])
      { ACE_ASSERT (nlattr_a[NL80211_ATTR_MAC]);
        ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                        nla_data (nlattr_a[NL80211_ATTR_MAC]),
                        ETH_ALEN);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to authenticate to access point (MAC was: %s), timed out, retrying\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ())));
        *(cb_data_p->error) = ETIMEDOUT;
        status_i = ETIMEDOUT;
        return_value = NL_STOP;
        goto update_state;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_FRAME]);

      data_p =
          reinterpret_cast<uint8_t*> (nla_data (nlattr_a[NL80211_ATTR_FRAME]));
      length_i = nla_len (nlattr_a[NL80211_ATTR_FRAME]);

      // sanity check(s)
      ACE_ASSERT ((data_p[0] & 0xFC) == 0xB0); // auth
      ACE_ASSERT (length_i >= 30);
      ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                      data_p + 10,
                      ETH_ALEN);
      status_i = (data_p[29] << 8) + data_p[28];
#if defined (_DEBUG)
      ACE_DEBUG (((status_i ? LM_ERROR : LM_DEBUG),
                  ACE_TEXT ("\"%s\": authenticated with access point (was: MAC: %s): %u\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
//                  ACE_TEXT (get_status_str (status_i))));
                  status_i));
#endif // _DEBUG

update_state:
      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        // *TODO*: retry only if the nature of the error is transient
        istate_machine_p->change (status_i ? NET_WLAN_MONITOR_STATE_AUTHENTICATE // retry
                                           : NET_WLAN_MONITOR_STATE_AUTHENTICATED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_AUTHENTICATED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_ASSOCIATE:
    {
      // sanity check(s)
//      ACE_ASSERT (nlattr_a[NL80211_ATTR_FRAME]);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      uint16_t status_i = 0;
      uint8_t* data_p = NULL;
      size_t length_i = 0;
      if (nlattr_a[NL80211_ATTR_TIMED_OUT])
      { ACE_ASSERT (nlattr_a[NL80211_ATTR_MAC]);
        ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                        nla_data (nlattr_a[NL80211_ATTR_MAC]),
                        ETH_ALEN);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to associate to access point (MAC was: %s), timed out, retrying\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ())));
        *(cb_data_p->error) = ETIMEDOUT;
        status_i = ETIMEDOUT;
        return_value = NL_STOP;
        goto update_state_2;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_FRAME]);

      data_p =
          reinterpret_cast<uint8_t*> (nla_data (nlattr_a[NL80211_ATTR_FRAME]));
      length_i = nla_len (nlattr_a[NL80211_ATTR_FRAME]);

      // sanity check(s)
      ACE_ASSERT ((data_p[0] & 0xFC) == 0x10); // assoc
      ACE_ASSERT (length_i >= 124);
      ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                      data_p + 10,
                      ETH_ALEN);
      status_i = (data_p[27] << 8) + data_p[26];
#if defined (_DEBUG)
      ACE_DEBUG (((status_i ? LM_ERROR : LM_DEBUG),
                  ACE_TEXT ("\"%s\": associated with access point (was: MAC: %s): %u\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
//                  ACE_TEXT (get_status_str (status_i))));
                  status_i));
#endif // _DEBUG

update_state_2:
      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (status_i ? NET_WLAN_MONITOR_STATE_ASSOCIATE // retry
                                           : NET_WLAN_MONITOR_STATE_ASSOCIATED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_ASSOCIATED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_DEAUTHENTICATE:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      uint16_t reason_i = 0;
      uint8_t* data_p = NULL;
      size_t length_i = 0;

      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_FRAME]);

      data_p =
          reinterpret_cast<uint8_t*> (nla_data (nlattr_a[NL80211_ATTR_FRAME]));
      length_i = nla_len (nlattr_a[NL80211_ATTR_FRAME]);

      // sanity check(s)
      ACE_ASSERT ((data_p[0] & 0xFC) == 0xC0); // deauth
      ACE_ASSERT (length_i >= 26);
      ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                      data_p + 10,
                      ETH_ALEN);
      reason_i = (data_p[25] << 8) + data_p[24];
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": deauthenticated with access point (was: MAC: %s; SSID: %s): %d\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (cb_data_p->monitor->SSID ().c_str ()),
                  reason_i));
#endif // _DEBUG
      ACE_UNUSED_ARG (reason_i);

      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (NET_WLAN_MONITOR_STATE_SCANNED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_SCANNED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_DISASSOCIATE:
    {
      // sanity check(s)
//      ACE_ASSERT (nlattr_a[NL80211_ATTR_FRAME]);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);
//      ACE_ASSERT (nlattr_a[NL80211_ATTR_WIPHY]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF
      std::string ssid_string = cb_data_p->monitor->SSID ();
      try {
        cb_data_p->monitor->onDisassociate (buffer_a,
                                            ssid_string,
                                            true);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onDisassociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (ssid_string.c_str ())));
      }
#if defined (_DEBUG)
      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": disassociated with access point (was: MAC: %s; SSID: %s)\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (ssid_string.c_str ())));
#endif // _DEBUG

      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (NET_WLAN_MONITOR_STATE_AUTHENTICATED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_AUTHENTICATED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_DISCONNECT:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);
//      ACE_ASSERT (nlattr_a[NL80211_ATTR_WIPHY]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF
      std::string ssid_string = cb_data_p->monitor->SSID ();
      try {
        cb_data_p->monitor->onDisconnect (buffer_a,
                                          ssid_string,
                                          true);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onDisconnect(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (ssid_string.c_str ())));
      }
#if defined (_DEBUG)
      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": disconnected from access point (was: MAC: %s; SSID: %s)\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (ssid_string.c_str ())));
#endif // _DEBUG

      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_ASSOCIATED), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_CONNECT:
    {
      // sanity check(s)
      ACE_ASSERT (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_MAC]);
      ACE_ASSERT (nlattr_a[NL80211_ATTR_STATUS_CODE]);

      unsigned int if_index_i = nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]);
      ACE_ASSERT (if_index_i);
      if (static_cast<unsigned int> (cb_data_p->index) != if_index_i)
        return NL_SKIP;

      char buffer_a[IF_NAMESIZE + 1];
      if (unlikely (!::if_indextoname (if_index_i,
                                       buffer_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to if_indextoname(%u): \"%m\", aborting\n"),
                    if_index_i));
        return NL_STOP;
      } // end IF

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      ACE_OS::memcpy (&(ap_mac_address_s.ether_addr_octet),
                      nla_data (nlattr_a[NL80211_ATTR_MAC]),
                      ETH_ALEN);
      ACE_UINT16 status_i = nla_get_u16 (nlattr_a[NL80211_ATTR_STATUS_CODE]);
      if (nlattr_a[NL80211_ATTR_TIMED_OUT])
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to associate to access point (MAC was: %s), timed out: %u, aborting\n"),
                    ACE_TEXT (buffer_a),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                    status_i));
        *(cb_data_p->error) = ETIMEDOUT;
        return NL_STOP;
      } // end IF
#if defined (_DEBUG)
      ACE_DEBUG (((status_i ? LM_ERROR : LM_DEBUG),
                  ACE_TEXT ("\"%s\": connected to access point (is: MAC: %s; SSID: %s): %u\n"),
                  ACE_TEXT (buffer_a),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (cb_data_p->monitor->SSID ().c_str ()),
                  status_i));
#endif // _DEBUG

      ACE_ASSERT (cb_data_p->monitor);
      Net_WLAN_Monitor_IStateMachine_t* istate_machine_p =
          dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
      ACE_ASSERT (istate_machine_p);
      try {
        istate_machine_p->change (NET_WLAN_MONITOR_STATE_CONNECT);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_CONNECT), aborting\n")));
        return NL_STOP;
      }

      break;
    }
    case NL80211_CMD_NOTIFY_CQM:
    {
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown nl80211 command (was: %u), continuing\n"),
                  static_cast<unsigned int> (genlmsghdr_p->cmd)));
      break;
    }
  } // end SWITCH

  return return_value;
}
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
