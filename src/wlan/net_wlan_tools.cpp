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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <cguid.h>
#include <guiddef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <wlanapi.h>
#else
#include <sys/capability.h>
#include <linux/capability.h>
#include <net/if_arp.h>
#include <ifaddrs.h>
#include <iwlib.h>

#include "common_dbus_tools.h"
#endif

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_wlan_defines.h"

//////////////////////////////////////////

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
//#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
//                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
Net_WLAN_Tools::getDeviceSettingBool (HANDLE clientHandle_in,
                                      REFGUID interfaceIdentifier_in,
                                      enum _WLAN_INTF_OPCODE parameter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getDeviceSettingBool"));

  // sanity check(s)
  if  (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                 InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p && (data_size == sizeof (BOOL)));

  return *static_cast<BOOL*> (data_p);
}
bool
Net_WLAN_Tools::setDeviceSettingBool (HANDLE clientHandle_in,
                                      REFGUID interfaceIdentifier_in,
                                      enum _WLAN_INTF_OPCODE parameter_in,
                                      bool enable_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::setDeviceSettingBool"));

  // sanity check(s)
  if  (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                 InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  BOOL data_b = (enable_in ? TRUE : FALSE);
  PVOID data_p = &data_b;
  DWORD result = WlanSetInterface (clientHandle_in,
                                   &interfaceIdentifier_in,
                                   parameter_in,
                                   sizeof (BOOL),
                                   data_p,
                                   NULL);
  if (unlikely (result != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanSetInterface(0x%@,%d): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_in, parameter_in,
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("\"%s\": %s setting (was: %d)\n"),
  //            ACE_TEXT (Net_WLAN_Tools::interfaceToString (clientHandle_in, interfaceIdentifier_in).c_str ()),
  //            (enable_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled")),
  //            parameter_in));

  return true;
}
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
bool
Net_WLAN_Tools::interfaceIsWLAN (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::interfaceIsWLAN"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

  bool result = false;
  int socket_handle = -1;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  int result_2 = -1;

  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  socket_handle = ACE_OS::socket (AF_INET,
                                  SOCK_STREAM,
                                  0);
  if (unlikely (socket_handle == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
    return false;
  } // end IF
  // verify the presence of Wireless Extensions
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWNAME,
                            &iwreq_s);
  if (!result_2)
    result = true;

  result_2 = ACE_OS::close (socket_handle);
  if (unlikely (socket_handle == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));

  return result;
}
#endif

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::associate (HANDLE clientHandle_in,
                           REFGUID interfaceIdentifier_in,
                           const std::string& SSID_in)
#else
Net_WLAN_Tools::associate (const std::string& interfaceIdentifier_in,
                           const struct ether_addr& APMACAddress_in,
                           const std::string& SSID_in,
                           ACE_HANDLE handle_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associate"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  ACE_ASSERT (SSID_in.size () <= DOT11_SSID_MAX_LENGTH);
#else
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (SSID_in.size () <= IW_ESSID_MAX_SIZE);
#endif
  ACE_ASSERT (!SSID_in.empty ());

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = SSID_in.size ();
  ACE_OS::memcpy (ssid_s.ucSSID,
                  SSID_in.c_str (),
                  SSID_in.size ());
  struct _WLAN_CONNECTION_PARAMETERS wlan_connection_parameters_s;
  ACE_OS::memset (&wlan_connection_parameters_s,
                  0,
                  sizeof (struct _WLAN_CONNECTION_PARAMETERS));
  // *TODO*: do the research here
  wlan_connection_parameters_s.wlanConnectionMode =
    //wlan_connection_mode_profile; // support WinXP
    wlan_connection_mode_discovery_unsecure;
  //wlan_connection_parameters_s.strProfile = NULL;
  wlan_connection_parameters_s.pDot11Ssid = &ssid_s;
  //wlan_connection_parameters_s.pDesiredBssidList = NULL;
  wlan_connection_parameters_s.dot11BssType =
    dot11_BSS_type_infrastructure;
    //wlan_connection_parameters_s.dwFlags = 0;
  // *NOTE*: this returns immediately
  DWORD result_2 = WlanConnect (clientHandle_in,
                                &interfaceIdentifier_in,
                                &wlan_connection_parameters_s,
                                NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanConnect(0x%@,%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_in,
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF

  result = true;
#else
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  int result_2 = -1;
  int error = 0;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return false;
    } // end IF
    close_handle = true;
  } // end IF

  // step1: set BSSID (AP MAC)
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  iwreq_s.u.ap_addr.sa_family = ARPHRD_ETHER;
  ACE_OS::memcpy (iwreq_s.u.ap_addr.sa_data,
                  &APMACAddress_in,
                  sizeof (struct ether_addr));
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWAP,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWAP): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  // step2: set ESSID
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  iwreq_s.u.essid.flags = 1;
  iwreq_s.u.essid.length = SSID_in.size ();
  iwreq_s.u.essid.pointer = const_cast<char*> (SSID_in.c_str ());
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWESSID,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWESSID): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  result = true;

clean:
  if (unlikely (close_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF
#endif

  return result;
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::scan (HANDLE clientHandle_in,
                      REFGUID interfaceIdentifier_in,
                      const std::string& ESSID_in)
#else
Net_WLAN_Tools::scan (const std::string& interfaceIdentifier_in,
                      const std::string& ESSID_in,
                      ACE_HANDLE handle_in,
                      bool wait_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::scan"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  ACE_ASSERT (ESSID_in.size () <= DOT11_SSID_MAX_LENGTH);
#else
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (ESSID_in.size () <= IW_ESSID_MAX_SIZE);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = ESSID_in.size ();
  ACE_OS::memcpy (ssid_s.ucSSID,
                  ESSID_in.c_str (),
                  ESSID_in.size ());
  //struct _WLAN_RAW_DATA raw_data_s;
  //ACE_OS::memset (&raw_data_s, 0, sizeof (struct _WLAN_RAW_DATA));
  // *NOTE*: this returns immediately
  DWORD result_2 = WlanScan (clientHandle_in,
                             &interfaceIdentifier_in,
                             NULL, // *NOTE*: support WinXP
                                   //&ssid_s,
                             NULL, // *NOTE*: support WinXP
                                   //&raw_data_s,
                             NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@): \"%s\", returning\n"),
                clientHandle_in,
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return;
  } // end IF
#else
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  struct iw_scan_req iw_scan_req_s;
  ACE_OS::memset (&iw_scan_req_s, 0, sizeof (struct iw_scan_req));
  //  iwreq_s.u.data.flags = IW_SCAN_DEFAULT;
  iwreq_s.u.data.flags = (IW_SCAN_ALL_ESSID |
                          IW_SCAN_ALL_FREQ  |
                          IW_SCAN_ALL_MODE  |
                          IW_SCAN_ALL_RATE);
  int result = -1;
#if defined (ACE_LINUX)
  bool handle_capabilities = false;
#endif

  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  if (likely (!ESSID_in.empty ()))
  { ACE_ASSERT (ESSID_in.size () <= IW_ESSID_MAX_SIZE);
    iw_scan_req_s.bssid.sa_family = ARPHRD_ETHER;
    ACE_OS::memset (iw_scan_req_s.bssid.sa_data, 0xff, ETH_ALEN);
    iw_scan_req_s.essid_len = ESSID_in.size ();
    ACE_OS::memcpy (iw_scan_req_s.essid,
                    ESSID_in.c_str (),
                    ESSID_in.size ());
    iwreq_s.u.data.flags = IW_SCAN_THIS_ESSID;
    iwreq_s.u.data.length = sizeof (struct iw_scan_req);
    iwreq_s.u.data.pointer = &iw_scan_req_s;
  } // end IF
  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", returning\n")));
      return;
    } // end IF
    close_handle = true;
  } // end IF

#if defined (ACE_LINUX)
  // (temporarily) elevate privileges to start scan
  if (!Common_Tools::hasCapability (CAP_NET_ADMIN))
  {
    if (unlikely (!Common_Tools::setCapability (CAP_NET_ADMIN)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setCapability(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::capabilityToString (CAP_NET_ADMIN).c_str ())));
      goto error;
    } // end IF
    handle_capabilities = true;
  } // end IF
#endif

  result = ACE_OS::ioctl (socket_handle,
                          SIOCSIWSCAN,
                          &iwreq_s);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
//    if (error == EPERM) // 1
    if ((error != EAGAIN) && // 11: scan not complete
        (error != EBUSY))    // 16: another scan is in progress
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWSCAN): \"%m\", returning\n"),
                  socket_handle));
      goto error;
    } // end IF
  } // end IF

  if (unlikely (wait_in))
  {
    // *IMPORTANT NOTE*: the kernel apparently does not signal the ioctl socket
    //                   file descriptor on the arrival of result data
    //                   --> poll
    // *TODO*: why ?
    ACE_ASSERT (false);
    ACE_NOTSUP;

    ACE_NOTREACHED (return;)
//    ACE_Handle_Set handle_set;
//    handle_set.set_bit (socket_handle);
//    // wait (maximum) 250ms between set and (first) result data
//    ACE_Time_Value timeout (0, 250000);
//    result = ACE_OS::select (static_cast<int> (socket_handle) + 1,
//                             static_cast<fd_set*> (handle_set),    // read
//                             NULL,                                 // write
//                             NULL,                                 // exception
////                             &timeout);
//                             NULL);
//    switch (result)
//    {
//      case 0:
//        ACE_OS::last_error (ETIME);
//        // *WARNING*: control falls through here
//      case -1:
//      {
//        if (unlikely (result == -1))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::select(%d): \"%m\", returning\n"),
//                      socket_handle));
//          goto error;
//        } // end IF
//        break;
//      }
//      default:
//        break;
//    } // end SWITCH
  } // end IF

error:
  if (unlikely (close_handle))
  {
    result = ACE_OS::close (socket_handle);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

#if defined (ACE_LINUX)
  if (handle_capabilities)
    if (!Common_Tools::dropCapability (CAP_NET_ADMIN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::dropCapability(CAP_NET_ADMIN): \"%m\", continuing\n")));
#endif
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
#endif

std::string
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::associatedSSID (HANDLE clientHandle_in,
                                REFGUID interfaceIdentifier_in)
#else
Net_WLAN_Tools::associatedSSID (const std::string& interfaceIdentifier_in,
                                ACE_HANDLE handle_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
#else
  if (unlikely (interfaceIdentifier_in.empty ()))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return result;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  if (result_2 == ERROR_INVALID_STATE) // <-- not connected
    return result;
  ACE_ASSERT (data_p && (data_size == sizeof (struct _WLAN_CONNECTION_ATTRIBUTES)));
  wlan_connection_attributes_p =
    static_cast<struct _WLAN_CONNECTION_ATTRIBUTES*> (data_p);
  result.assign (reinterpret_cast<char*> (wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.ucSSID),
                 wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.uSSIDLength);

  // clean up
  WlanFreeMemory (data_p);
#else
//  ACE_ASSERT (connection_in);

//  std::string device_path_string =
//      Net_WLAN_Tools::deviceToDBusPath (connection_in,
//                                          interfaceIdentifier_in);
//  ACE_ASSERT (!device_path_string.empty ());
//  std::string access_point_path_string =
//      Net_WLAN_Tools::deviceDBusPathToAccessPointDBusPath (connection_in,
//                                                             device_path_string);
//  if (access_point_path_string.empty ())
//    goto continue_;

//  result =
//      Net_WLAN_Tools::accessPointDBusPathToSSID (connection_in,
//                                                   access_point_path_string);

  ACE_HANDLE socket_handle = handle_in;
  int result_2 = -1;
  struct iwreq iwreq_s;
  char essid[IW_ESSID_MAX_SIZE + 1];
  bool close_socket = false;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = iw_sockets_open ();
    if (socket_handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_sockets_open(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    close_socket = true;
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::memset (essid, 0, sizeof (IW_ESSID_MAX_SIZE + 1));
  iwreq_s.u.essid.pointer = essid;
  iwreq_s.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  result_2 = iw_get_ext (socket_handle,
                         interfaceIdentifier_in.c_str (),
                         SIOCGIWESSID,
                         &iwreq_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_ext(\"%s\",SIOCGIWESSID): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: the length iwreq_s.u.essid.length is wrong
//  ACE_ASSERT (iwreq_s.u.essid.length && (iwreq_s.u.essid.length <= IW_ESSID_MAX_SIZE));
//  result.assign (essid, iwreq_s.u.essid.length);
  result = essid;

error:
  if (unlikely (close_socket))
    iw_sockets_close (socket_handle);
#endif

//continue_:
  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct ether_addr
Net_WLAN_Tools::associatedBSSID (const std::string& interfaceIdentifier_in,
                                 ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedBSSID"));

  // initialize return value(s)
  struct ether_addr return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  int result_2 = -1;
  int error = 0;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return return_value;
    } // end IF
    close_handle = true;
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWAP,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWAP): \"%m\", aborting\n"),
                socket_handle));
    goto clean;
  } // end IF
  ACE_OS::memcpy (&return_value,
                  iwreq_s.u.ap_addr.sa_data,
                  sizeof (struct ether_addr));

clean:
  if (unlikely (close_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

  return return_value;
}

ACE_INET_Addr
Net_WLAN_Tools::getGateway (const std::string& interfaceIdentifier_in,
                            struct DBusConnection* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getGateway"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (connection_in);

  ACE_INET_Addr result = Net_Common_Tools::getGateway (interfaceIdentifier_in);
  int result_2 = -1;

  // *IMPORTANT NOTE*: (as of kernel 3.16.0,x) dhclient apparently does not add
  //                   wireless gateway information to the routing table
  //                   reliably (i.e. Gateway is '*'). Specifically, when there
  //                   already is a gateway configured on a different interface
  //                   --> try DBus instead
  if (result.is_any ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to retrieve gateway from kernel, trying DBus...\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));

    std::string result_string =
        Net_WLAN_Tools::deviceToDBusPath (connection_in,
                                          interfaceIdentifier_in);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusPath(\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
      return result;
    } // end IF
    result_string =
        Net_WLAN_Tools::deviceDBusPathToIp4ConfigDBusPath (connection_in,
                                                             result_string);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusPathToIp4ConfigDBusPath(\"%s\",\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
    result_string =
        Net_WLAN_Tools::Ip4ConfigDBusPathToGateway (connection_in,
                                                      result_string);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::Ip4ConfigDBusPathToGateway(\"%s\",\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
    result_2 = result.set (0,
                           result_string.c_str (),
                           1,
                           AF_INET);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
  } // end IF

  return result;
}
#endif

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::hasSSID (HANDLE clientHandle_in,
                           REFGUID interfaceIdentifier_in,
#else
Net_WLAN_Tools::hasSSID (const std::string& interfaceIdentifier_in,
#endif
                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
#else
  if (unlikely (interfaceIdentifier_in.empty ()))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return result;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#else
  int result_2 = -1;
  std::vector<std::string> wireless_device_identifiers_a;
  int socket_handle = -1;
  struct iw_range iw_range_s;
  struct wireless_scan_head wireless_scan_head_s;
  struct wireless_scan* wireless_scan_p = NULL;
  bool done = false;
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
#endif

  // step1: retrieve all wireless interfaces
  if (likely (!interfaceIdentifier_in.empty ()))
  {
    wireless_device_identifiers_a.push_back (interfaceIdentifier_in);
    goto continue_;
  } // end IF
#if defined (ACE_HAS_GETIFADDRS)
  result_2 = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return result;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
//    if ((ifaddrs_2->ifa_flags & IFF_UP) == 0)
//      continue;
    if (!ifaddrs_2->ifa_addr)
      continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;
    if (!Net_WLAN_Tools::interfaceIsWLAN (ifaddrs_2->ifa_name))
      continue;

    wireless_device_identifiers_a.push_back (ifaddrs_2->ifa_name);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);

  ACE_NOTREACHED (return result;)
#endif /* ACE_HAS_GETIFADDRS */
continue_:
  socket_handle = iw_sockets_open ();
  if (unlikely (socket_handle == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_sockets_open(): \"%m\", aborting\n")));
    return false;
  } // end IF

  for (std::vector<std::string>::const_iterator iterator = wireless_device_identifiers_a.begin ();
       (iterator != wireless_device_identifiers_a.end ()) && !done;
       ++iterator)
  {
    ACE_OS::memset (&iw_range_s, 0, sizeof (struct iw_range));
    result_2 = iw_get_range_info (socket_handle,
                                  (*iterator).c_str (),
                                  &iw_range_s);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_get_range_info(): \"%m\", aborting\n")));
      goto error;
    } // end IF

    ACE_OS::memset (&wireless_scan_head_s,
                    0,
                    sizeof (struct wireless_scan_head));
    // *TODO*: do not use iwlib; talk to the driver directly
    result_2 = iw_scan (socket_handle,
                        const_cast<char*> ((*iterator).c_str ()),
                        iw_range_s.we_version_compiled,
                        &wireless_scan_head_s);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_scan(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    for (wireless_scan_p = wireless_scan_head_s.result;
         wireless_scan_p;
         wireless_scan_p = wireless_scan_p->next)
    {
      if (!wireless_scan_p->b.essid_on)
        continue;
      if (!ACE_OS::strncmp (SSID_in.c_str (),
                            wireless_scan_p->b.essid,
                            wireless_scan_p->b.essid_len))
      { // --> found SSID on adapter
        result = true;
        done = true;
        break;
      } // end IF
    } // end FOR
  } // end FOR

error:
  // clean up
  iw_sockets_close (socket_handle);
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
bool
Net_WLAN_Tools::activateConnection (struct DBusConnection* connection_in,
                                      const std::string& connectionObjectPath_in,
                                      const std::string& deviceObjectPath_in,
                                      const std::string& accessPointObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activateConnection"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!connectionObjectPath_in.empty ());
  ACE_ASSERT (!deviceObjectPath_in.empty ());
  ACE_ASSERT (!accessPointObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("ActivateConnection"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(ActivateConnection): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  const char* argument_string_p = connectionObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = deviceObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = accessPointObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  dbus_message_unref (reply_p); reply_p = NULL;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: activated connection profile \"%s\" (active connection is: \"%s\")\n"),
//              ACE_TEXT (Net_WLAN_Tools::deviceDBusPathToIdentifier (connection_in, deviceObjectPath_in).c_str ()),
//              ACE_TEXT (connectionObjectPath_in.c_str ()),
//              ACE_TEXT (object_path_p)));

  result = true;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

  return false;

continue_:
  return result;
}

std::string
Net_WLAN_Tools::activeConnectionDBusPathToIp4ConfigDBusPath (struct DBusConnection* connection_in,
                                                               const std::string& activeConnectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activeConnectionDBusPathToIp4ConfigDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!activeConnectionObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    activeConnectionObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(IPv4Config): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ip4Config");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::activeConnectionDBusPathToDeviceDBusPath (struct DBusConnection* connection_in,
                                                            const std::string& activeConnectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activeConnectionDBusPathToDeviceDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!activeConnectionObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    activeConnectionObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Devices): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Devices");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_2, &object_path_p);
    ACE_ASSERT (object_path_p);
    if (unlikely (!result.empty ()))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("active connection \"%s\" uses several devices, returning the first one\n"),
                  ACE_TEXT (activeConnectionObjectPath_in.c_str ())));
      break;
    } // end IF
    result = object_path_p;
  } while (dbus_message_iter_next (&iterator));
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::deviceToDBusPath (struct DBusConnection* connection_in,
                                    const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceToDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetDeviceByIpIface"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetDeviceByIpIface): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  dbus_message_iter_init_append (message_p, &iterator);
  const char* device_identifier_p = interfaceIdentifier_in.c_str ();
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &device_identifier_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::Ip4ConfigDBusPathToGateway (struct DBusConnection* connection_in,
                                              const std::string& Ip4ConfigObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::Ip4ConfigDBusPathToGateway"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!Ip4ConfigObjectPath_in.empty ());

  struct DBusMessage* message_p =
  dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                Ip4ConfigObjectPath_in.c_str (),
                                ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Gateway): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  char* string_p = NULL;
//  char character_c = 0;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_IP4CONFIG_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Gateway");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

//  character_c = dbus_message_iter_get_arg_type (&iterator);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&iterator_2, &string_p);
  ACE_ASSERT (string_p);
  result = string_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath (struct DBusConnection* connection_in,
                                                             const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusPathToActiveAccessPointDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("ActiveAccessPoint");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator_2, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  // *NOTE*: the D-Bus protocol defines "/" to be an invalid/unkown object path
  if (unlikely (!ACE_OS::strcmp (result.c_str (), ACE_TEXT_ALWAYS_CHAR ("/"))))
    result.resize (0);

  return result;
}

std::string
Net_WLAN_Tools::deviceDBusPathToIp4ConfigDBusPath (struct DBusConnection* connection_in,
                                                      const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusPathToIp4ConfigDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ip4Config");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  // *NOTE*: contrary to the documentation, the type is 'vo', not 'o'
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator_2, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  // *NOTE*: the D-Bus protocol defines "/" to be an invalid/unkown object path
  if (unlikely (!ACE_OS::strcmp (result.c_str (), ACE_TEXT_ALWAYS_CHAR ("/"))))
    result.resize (0);

  return result;
}

std::string
Net_WLAN_Tools::deviceDBusPathToIdentifier (struct DBusConnection* connection_in,
                                              const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusPathToIdentifier"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* device_identifier_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Interface");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&iterator_2, &device_identifier_p);
  ACE_ASSERT (device_identifier_p);
  result = device_identifier_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::accessPointDBusPathToSSID (struct DBusConnection* connection_in,
                                             const std::string& accessPointObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::accessPointDBusPathToSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!accessPointObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    accessPointObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char character_c = 0;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_ACCESSPOINT_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ssid");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator_2, &iterator);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_BYTE);
    dbus_message_iter_get_basic (&iterator, &character_c);
    ACE_ASSERT (character_c);
    result += character_c;
  } while (dbus_message_iter_next (&iterator));
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::connectionDBusPathToSSID (struct DBusConnection* connection_in,
                                            const std::string& connectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::connectionDBusPathToSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!connectionObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
  dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                connectionObjectPath_in.c_str (),
                                ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGSCONNECTION_INTERFACE),
                                ACE_TEXT_ALWAYS_CHAR ("GetSettings"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetSettings): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2, iterator_3, iterator_4;
  struct DBusMessageIter iterator_5, iterator_6, iterator_7;
  const char* key_string_p, *key_string_2 = NULL;
  DBusBasicValue value_u;
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
  goto error;
  } // end IF
  // *NOTE*: the schema is a{sa{sv}}
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_ARRAY)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    // connection settings --> wireless settings --> IPv4 configuration --> ...
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_DICT_ENTRY);
    dbus_message_iter_recurse (&iterator_2, &iterator_3);
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_STRING);
    key_string_p = NULL;
    dbus_message_iter_get_basic (&iterator_3, &key_string_p);
    ACE_ASSERT (key_string_p);
    if (ACE_OS::strcmp (key_string_p, ACE_TEXT_ALWAYS_CHAR ("802-11-wireless")))
      continue;
    dbus_message_iter_next (&iterator_3);
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_ARRAY);
    dbus_message_iter_recurse (&iterator_3, &iterator_4);
    do
    {
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_4) == DBUS_TYPE_DICT_ENTRY);
      dbus_message_iter_recurse (&iterator_4, &iterator_5);
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_5) == DBUS_TYPE_STRING);
      key_string_2 = NULL;
      dbus_message_iter_get_basic (&iterator_5, &key_string_2);
      ACE_ASSERT (key_string_2);
      dbus_message_iter_next (&iterator_5);
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_5) == DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse (&iterator_5, &iterator_6);
      switch (dbus_message_iter_get_arg_type (&iterator_6))
      {
        case DBUS_TYPE_BOOLEAN:
        case DBUS_TYPE_BYTE:
        case DBUS_TYPE_INT16:
        case DBUS_TYPE_UINT16:
        case DBUS_TYPE_INT32:
        case DBUS_TYPE_UINT32:
        case DBUS_TYPE_INT64:
        case DBUS_TYPE_UINT64:
        case DBUS_TYPE_DOUBLE:
        case DBUS_TYPE_STRING:
        case DBUS_TYPE_OBJECT_PATH:
        case DBUS_TYPE_SIGNATURE:
        case DBUS_TYPE_UNIX_FD:
        {
          dbus_message_iter_get_basic (&iterator_6, &value_u);
          break;
        }
        case DBUS_TYPE_ARRAY:
        {
          dbus_message_iter_recurse (&iterator_6, &iterator_7);
          if (!ACE_OS::strcmp (key_string_2, ACE_TEXT_ALWAYS_CHAR ("ssid")))
          { ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_7) == DBUS_TYPE_BYTE);
            char character_c = 0;
            do
            {
              dbus_message_iter_get_basic (&iterator_7, &character_c);
              result += character_c;
            } while (dbus_message_iter_next (&iterator_7));

            goto done;
          } // end IF

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown argument type (was: %d), continuing\n"),
                      dbus_message_iter_get_arg_type (&iterator_6)));
          break;
        }
      } // end SWITCH
    } while (dbus_message_iter_next (&iterator_4));
  } while (dbus_message_iter_next (&iterator_2));
done:
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::SSIDToAccessPointDBusPath (struct DBusConnection* connection_in,
                                             const std::string& deviceObjectPath_in,
                                             const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::SSIDToAccessPointDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  std::string device_object_path_string =
      (deviceObjectPath_in.empty () ? Net_WLAN_Tools::SSIDToDeviceDBusPath (connection_in,
                                                                              SSID_in)
                                    : deviceObjectPath_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusPath(0x%@,%s), aborting\n"),
                connection_in,
                ACE_TEXT (SSID_in.c_str ())));
    return result;
  } // end IF

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetAllAccessPoints"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetAllAccessPoints): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  std::string SSID_string;
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_ARRAY)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    if (unlikely (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_INVALID))
      break; // device exists, but no SSIDs found --> radio off ?
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_2, &object_path_p);
    ACE_ASSERT (object_path_p);
    SSID_string = Net_WLAN_Tools::accessPointDBusPathToSSID (connection_in,
                                                               object_path_p);
    if (unlikely (SSID_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusPathToSSID(\"%s\"), continuing\n"),
                  ACE_TEXT (object_path_p)));
      continue;
    } // end IF
    if (SSID_string == SSID_in)
    {
      result = object_path_p;
      break;
    } // end IF
  } while (dbus_message_iter_next (&iterator_2));
  dbus_message_unref (reply_p);
  reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::SSIDToDeviceDBusPath (struct DBusConnection* connection_in,
                                        const std::string& SSID_in)
{
  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  // step1: retrieve all wireless adapaters
  std::vector<std::string> wireless_device_identifiers_a;
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result_2 = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return result;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
//    if ((ifaddrs_2->ifa_flags & IFF_UP) == 0)
//      continue;
    if (!ifaddrs_2->ifa_addr)
      continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;
    if (!Net_WLAN_Tools::interfaceIsWLAN (ifaddrs_2->ifa_name))
      continue;

    wireless_device_identifiers_a.push_back (ifaddrs_2->ifa_name);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);

  ACE_NOTREACHED (return result;)
#endif /* ACE_HAS_GETIFADDRS */

  // step2: retrieve the wireless adapter(s) that can see the SSID
  for (std::vector<std::string>::iterator iterator = wireless_device_identifiers_a.begin ();
       iterator != wireless_device_identifiers_a.end ();
       ++iterator)
    if (!Net_WLAN_Tools::hasSSID (*iterator,
                                    SSID_in))
      wireless_device_identifiers_a.erase (iterator);
  if (wireless_device_identifiers_a.empty ())
    return result;
  else if (wireless_device_identifiers_a.size () > 1)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("found several devices that can see SSID %s, choosing the first one\n"),
                ACE_TEXT (SSID_in.c_str ())));

  return Net_WLAN_Tools::deviceToDBusPath (connection_in,
                                             wireless_device_identifiers_a.front ());
}

std::string
Net_WLAN_Tools::SSIDToConnectionDBusPath (struct DBusConnection* connection_in,
                                            const std::string& deviceObjectPath_in,
                                            const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::SSIDToConnectionDBusPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  std::string device_object_path_string =
      (deviceObjectPath_in.empty () ? Net_WLAN_Tools::SSIDToDeviceDBusPath (connection_in,
                                                                              SSID_in)
                                    : deviceObjectPath_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusPath(0x%@,%s), aborting\n"),
                connection_in,
                ACE_TEXT (SSID_in.c_str ())));
    return result;
  } // end IF

  struct DBusMessage* reply_p = NULL;
  // *NOTE*: a better alternative would be to retrieve the
  //        'AvailableConnections' property of the device; this seems to be
  //        broken at the moment *TODO*
  //        --> retrieve all connections
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE),
//                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGS_OBJECT_PATH),
//                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGS_INTERFACE),
//                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
                                    ACE_TEXT_ALWAYS_CHAR ("ListConnections"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
//  struct DBusMessageIter iterator;
  struct DBusMessageIter iterator_2, iterator_3;
  std::vector<std::string> connection_paths_a;
  std::vector<std::string>::const_iterator iterator_4;
  char* object_path_p = NULL;
  std::string SSID_string;
  //  const char* argument_string_p =
//      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
//  dbus_message_iter_init_append (message_p, &iterator);
//  if (!dbus_message_iter_append_basic (&iterator,
//                                       DBUS_TYPE_STRING,
//                                       &argument_string_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
//    goto error;
//  } // end IF
//  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("AvailableConnections");
//  dbus_message_iter_init_append (message_p, &iterator);
//  if (!dbus_message_iter_append_basic (&iterator,
//                                       DBUS_TYPE_STRING,
//                                       &argument_string_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
//    goto error;
//  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
//  if (!dbus_message_iter_init (reply_p, &iterator))
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
//  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
//  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator_2, &iterator_3);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_3, &object_path_p);
    ACE_ASSERT (object_path_p);
    connection_paths_a.push_back (object_path_p);
  } while (dbus_message_iter_next (&iterator_3));
  dbus_message_unref (reply_p); reply_p = NULL;

  iterator_4 = connection_paths_a.begin ();
  for (;
       iterator_4 != connection_paths_a.end ();
       ++iterator_4)
  {
    SSID_string = Net_WLAN_Tools::connectionDBusPathToSSID (connection_in,
                                                              *iterator_4);
    if (SSID_string == SSID_in)
      break;
  } // end FOR
  if (unlikely (iterator_4 == connection_paths_a.end ()))
    goto error;

  result = *iterator_4;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}
#endif
