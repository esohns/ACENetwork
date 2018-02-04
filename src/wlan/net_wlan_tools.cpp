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
#include "ifaddrs.h"
#include "iwlib.h"

#if defined (DBUS_SUPPORT)
#include "common_dbus_tools.h"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_wlan_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_xml_defines.h"

#include "net_wlan_profile_xml_handler.h"
#endif

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
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    clientHandle_out = ACE_INVALID_HANDLE;
  } // end IF

  DWORD maximum_client_version =
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA) // see wlanapi.h:55
#if defined (WINXP_SUPPORT)
    WLAN_API_VERSION_1_0;
#pargma message ("compiling for WLAN API version " WLAN_API_VERSION_1_0)
#else
    WLAN_API_VERSION; // use SDK-native
#pragma message ("compiling for WLAN API version " COMMON_STRINGIZE(WLAN_API_VERSION))
#endif // WINXP_SUPPORT
#else
    WLAN_API_VERSION; // use SDK-native
#pargma message ("compiling for WLAN API version " WLAN_API_VERSION)
#endif // (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (clientHandle_out != ACE_INVALID_HANDLE);

  return true;
}
void
Net_WLAN_Tools::finalize (HANDLE clientHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::finalize"));

  // sanity check(s)
  ACE_ASSERT (clientHandle_in != ACE_INVALID_HANDLE);

  DWORD result = WlanCloseHandle (clientHandle_in,
                                  NULL);
  if (unlikely (result != ERROR_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
}

bool
Net_WLAN_Tools::getDeviceSettingBool (HANDLE clientHandle_in,
                                      REFGUID interfaceIdentifier_in,
                                      enum _WLAN_INTF_OPCODE parameter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getDeviceSettingBool"));

  bool result = false;

  // sanity check(s)
  if  (unlikely ((clientHandle_in == ACE_INVALID_HANDLE) ||
                 InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT)
  switch (parameter_in)
  {
    case wlan_intf_opcode_autoconf_enabled:
    //case wlan_intf_opcode_bss_type:
    //case wlan_intf_opcode_interface_state:
    //case wlan_intf_opcode_current_connection:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %d), aborting\n"),
                  parameter_in));
      return false;
    }
  } // end SWITCH
#endif

  enum _WLAN_OPCODE_VALUE_TYPE value_type = wlan_opcode_value_type_invalid;
  DWORD data_size = 0;
  PVOID data_p = NULL;
  DWORD result_2 = WlanQueryInterface (clientHandle_in,
                                       &interfaceIdentifier_in,
                                       parameter_in,
                                       NULL,
                                       &data_size,
                                       &data_p,
                                       &value_type);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanQueryInterface(0x%@,%d): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_in, parameter_in,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p && (data_size == sizeof (BOOL)));
  result = *static_cast<BOOL*> (data_p);

  WlanFreeMemory (data_p);

  return result;
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
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT)
  switch (parameter_in)
  {
    case wlan_intf_opcode_autoconf_enabled:
    case wlan_intf_opcode_bss_type:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %d), aborting\n"),
                  parameter_in));
      return false;
    }
  } // end SWITCH
#endif
  if (Net_WLAN_Tools::getDeviceSettingBool (client_handle,
                                            interfaceIdentifier_in,
                                            parameter_in) == enable_in)
    return true; // nothing to do

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
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  result = true;
#if defined (_DEBUG)
  std::string opcode_string;
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
      return result;
    }
  } // end SWITCH
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": %s '%s' setting\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              (enable_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled")),
              ACE_TEXT (opcode_string.c_str ())));
#endif // _DEBUG
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);

  return result;
}
#endif

Net_InterfaceIdentifiers_t
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::getInterfaces (HANDLE clientHandle_in)
#else
Net_WLAN_Tools::getInterfaces (int addressFamily_in,
                               int flags_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  // initialize return value(s)
  Net_InterfaceIdentifiers_t result;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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

#if defined (_DEBUG)
  //std::string interface_state_string;
#endif
  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD result_2 = WlanEnumInterfaces (client_handle,
                                       NULL,
                                       &interface_list_p);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanEnumInterfaces(0x%@): \"%s\", aborting\n"),
                client_handle,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (interface_list_p);
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("found %u WLAN interface(s)\n"),
  //            interface_list_p->dwNumberOfItems));

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
#endif
    result.push_back (interface_list_p->InterfaceInfo[i].InterfaceGuid);
  } // end FOR
  WlanFreeMemory (interface_list_p);

clean:
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);
#else
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
    if (!Net_WLAN_Tools::isWireless (ifaddrs_2->ifa_name))
      continue;
    if (unlikely (addressFamily_in != AF_UNSPEC))
    { ACE_ASSERT (ifaddrs_2->ifa_addr);
//      if (!ifaddrs_2->ifa_addr)
//        continue;
      if (addressFamily_in == AF_MAX)
      {
        if ((ifaddrs_2->ifa_addr->sa_family != AF_INET) &&
            (ifaddrs_2->ifa_addr->sa_family != AF_INET6))
          continue;
      } // end IF
      else if (ifaddrs_2->ifa_addr->sa_family != addressFamily_in)
        continue;
    } // end IF
//    else
//    { ACE_ASSERT (ifaddrs_2->ifa_addr);
//      if (unlikely (ifaddrs_2->ifa_addr->sa_family == AF_PACKET))
//      continue;
//    } // end IF
    if (unlikely (flags_in &&
                  !(ifaddrs_2->ifa_flags & flags_in)))
      continue;

    result.push_back (ifaddrs_2->ifa_name);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);

  // *NOTE*: there may be duplicate entries (e.g. one for each address family)
  //         --> remove duplicates
  std::sort (result.begin (), result.end ());
  Net_InterfacesIdentifiersIterator_t iterator =
      std::unique (result.begin (), result.end ());
  result.erase (iterator, result.end ());
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ether_addr
Net_WLAN_Tools::getAccessPointAddress (HANDLE clientHandle_in,
                                       REFGUID interfaceIdentifier_in,
                                       const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getAccessPointAddress"));

  // initialize return value(s)
  struct ether_addr result;
  ACE_OS::memset (&result, 0, sizeof (struct ether_addr));

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
  Net_InterfaceIdentifiers_t interface_identifiers;
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
    interface_identifiers =
      Net_WLAN_Tools::getInterfaces (client_handle);
  else
    interface_identifiers.push_back (interfaceIdentifier_in);
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
#if defined (_DEBUG)
  //std::string bss_network_type_string;
  //std::string phy_type_string;
#endif
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers.begin ();
       iterator != interface_identifiers.end ();
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
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
#endif
      if (done)
        break;
    } // end FOR
    if (done)
      break;
    WlanFreeMemory (wlan_bss_list_p);
    wlan_bss_list_p = NULL;
  } // end FOR

//clean:
  if (wlan_bss_list_p)
    WlanFreeMemory (wlan_bss_list_p);
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);

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
  Net_InterfaceIdentifiers_t interface_identifiers;
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
    interface_identifiers = Net_WLAN_Tools::getInterfaces (clientHandle_in);
  else
    interface_identifiers.push_back (interfaceIdentifier_in);
  DWORD result_2 = 0;
  struct _WLAN_PROFILE_INFO_LIST* profile_list_p = NULL;
  struct _WLAN_PROFILE_INFO* profile_p = NULL;
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers.begin ();
       iterator != interface_identifiers.end ();
       ++iterator)
  {
    ACE_ASSERT (!profile_list_p);
    result_2 = WlanGetProfileList (clientHandle_in,
                                   &(*iterator),
                                   NULL,
                                   &profile_list_p);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetProfileList(0x%@): \"%s\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
#if ((_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT))
  for (Net_WLAN_ProfilesIterator_t iterator != profiles_a.begin ();
       iterator != profiles_a.end ();
       ++iterator)
    if (!ACE_OS::strcmp ((*iterator).c_str (),
                         SSID_in.c_str ()))
    {
      result = SSID_in;
      break;
    } // end IF
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
  std::string profile_string;
  struct Net_WLAN_Profile_ParserContext parser_context;
  struct Common_XML_ParserConfiguration parser_configuration;
  parser_configuration.SAXFeatures.push_back (std::make_pair (ACE_TEXT_ALWAYS_CHAR (COMMON_XML_PARSER_FEATURE_VALIDATION),
                                                              false));
  Net_WLAN_Profile_Parser_t parser (&parser_context);
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
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      profile_name_string_p = NULL;
      continue;
    } // end IF
    ACE_ASSERT (profile_string_p);

    profile_string =
      ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (profile_string_p));
    ACE_ASSERT (parser_context.SSIDs.empty ());
    parser.parseString (profile_string);
    ACE_ASSERT (!parser_context.SSIDs.empty ());
    if (!ACE_OS::strcmp (parser_context.SSIDs.front ().c_str (),
                         SSID_in.c_str ()))
    {
      result = *iterator;

      WlanFreeMemory (profile_string_p);

      break;
    } // end IF
    parser_context.SSIDs.clear ();
    profile_name_string_p = NULL;
    WlanFreeMemory (profile_string_p);
    profile_string_p = NULL;
  } // end FOR
#endif // ((_WIN32_WINNT <= _WIN32_WINNT_WINXP) || WINXP_SUPPORT)

  return result;
}
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
bool
Net_WLAN_Tools::isWireless (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::isWireless"));

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
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
  std::string profile_name_string =
    Net_WLAN_Tools::getProfile (client_handle,
                                interfaceIdentifier_in,
                                SSID_in);
  ACE_ASSERT (profile_name_string.size () <= WLAN_MAX_NAME_LENGTH);
  ACE_Ascii_To_Wide converter (profile_name_string.c_str ());
  // *TODO*: this specification is not precise enough
#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP)
  wlan_connection_parameters_s.wlanConnectionMode =
    wlan_connection_mode_profile;
  ACE_ASSERT (!profile_name_string.empty ());
  wlan_connection_parameters_s.strProfile = converter.wchar_rep ();
#else
  // *TODO*: do the research here
  if (!profile_name_string.empty ())
  {
    wlan_connection_parameters_s.wlanConnectionMode =
      wlan_connection_mode_profile;
    wlan_connection_parameters_s.strProfile = converter.wchar_rep ();
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": associating with SSID %s using profile \"%s\"\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (profile_name_string.c_str ())));
#endif
  } // end IF
  else
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("\"%s\": no profile found (SSID was: %s), trying 'wlan_connection_mode_auto'\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (profile_name_string.c_str ())));
    // *TODO*: do the research here
    wlan_connection_parameters_s.wlanConnectionMode =
      wlan_connection_mode_auto;
  } // end ELSE
#endif
  wlan_connection_parameters_s.pDot11Ssid = &ssid_s;
  //wlan_connection_parameters_s.pDesiredBssidList = NULL;
  wlan_connection_parameters_s.dot11BssType =
    dot11_BSS_type_infrastructure;
    //wlan_connection_parameters_s.dwFlags = 0;
  // *NOTE*: this returns immediately
  DWORD result_2 = WlanConnect (client_handle,
                                &interfaceIdentifier_in,
                                &wlan_connection_parameters_s,
                                NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to ::WlanConnect(0x%@,%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                client_handle,
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF

  result = true;

clean:
  if (unlikely (release_handle))
    Net_WLAN_Tools::finalize (client_handle);
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

  // step0: set mode ?
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWMODE,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWMODE): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF
  if (unlikely (iwreq_s.u.mode != IW_MODE_INFRA))
  {
    iwreq_s.u.mode = IW_MODE_INFRA;
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWMODE,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWMODE): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);

  // step1: set BSSID (AP MAC) ?
  iwreq_s.u.ap_addr.sa_family = ARPHRD_ETHER;
  if (Net_Common_Tools::isAny (APMACAddress_in))
    goto continue_;
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

continue_:
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

  // step3: commit pending changes
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWCOMMIT,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSUP) // 95
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWCOMMIT): \"%m\", aborting\n"),
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

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::disassociate (HANDLE clientHandle_in,
                              REFGUID interfaceIdentifier_in)
#else
Net_WLAN_Tools::disassociate (const std::string& interfaceIdentifier_in,
                              ACE_HANDLE handle_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disassociate"));

  bool result = false;

  // sanity check(s)
  bool release_handle = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
#else
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_HANDLE socket_handle = handle_in;
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
    release_handle = true;
  } // end IF
#endif
  Net_InterfaceIdentifiers_t interface_identifiers_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
#else
  if (unlikely (interfaceIdentifier_in.empty ()))
#endif
  {
    interface_identifiers_a =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Net_WLAN_Tools::getInterfaces (clientHandle_in);
#else
      Net_WLAN_Tools::getInterfaces ();
#endif
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      goto clean;
    } // end IF
  } // end IF
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *NOTE*: this returns immediately
    result_2 = WlanDisconnect (client_handle,
                               &(*iterator),
                               NULL);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanDisconnect(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      goto clean;
    } // end IF
#else
    // step1: reset BSSID (AP MAC)
    ACE_ASSERT ((*iterator).size () <= IFNAMSIZ);
    ACE_OS::strncpy (iwreq_s.ifr_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    iwreq_s.u.ap_addr.sa_family = ARPHRD_ETHER;
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

    // step2: commit pending changes
    ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
    ACE_OS::strncpy (iwreq_s.ifr_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWCOMMIT,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != ENOTSUP) // 95
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWCOMMIT): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF
#endif
  } // end FOR

  result = true;

clean:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (release_handle))
    Net_WLAN_Tools::finalize (client_handle);
#else
  if (unlikely (release_handle))
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
  // *TODO*: support attaching an information element
  //struct _WLAN_RAW_DATA raw_data_s;
  //ACE_OS::memset (&raw_data_s, 0, sizeof (struct _WLAN_RAW_DATA));
  // *NOTE*: this returns immediately
  DWORD result_2 = WlanScan (clientHandle_in,
                             &interfaceIdentifier_in,
#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT)
                             NULL, // *NOTE*: support WinXP
#else
                             (ESSID_in.empty () ? NULL : &ssid_s),
#endif
#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT)
                             NULL, // *NOTE*: support WinXP
#else
                             NULL,
                             //&raw_data_s,
#endif
                             NULL);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    if (result_2 != ERROR_BUSY) // 170: The requested resource is in use
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@,%s): \"%s\", returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (ESSID_in.c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
#if defined (_DEBUG)
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@,%s): \"%s\", returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  clientHandle_in,
                  ACE_TEXT (ESSID_in.c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
#endif
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("scanning for ESSID %s...\n"),
                ACE_TEXT (ESSID_in.c_str ())));
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
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (unlikely (interfaceIdentifier_in.empty ()))
#endif
  {
    Net_InterfaceIdentifiers_t interface_identifiers_a =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Net_WLAN_Tools::getInterfaces (clientHandle_in);
#else
      Net_WLAN_Tools::getInterfaces ();
#endif
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      return result;
    } // end IF
    interface_identifier = interface_identifiers_a.front ();
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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

  DWORD data_size = 0;
  PVOID data_p = NULL;
  struct _WLAN_CONNECTION_ATTRIBUTES* wlan_connection_attributes_p = NULL;
  DWORD result_2 =
    WlanQueryInterface (client_handle,
                        &interface_identifier,
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
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  if (result_2 == ERROR_INVALID_STATE) // <-- not connected
    return result;
  ACE_ASSERT (data_p && (data_size == sizeof (struct _WLAN_CONNECTION_ATTRIBUTES)));
  wlan_connection_attributes_p =
    static_cast<struct _WLAN_CONNECTION_ATTRIBUTES*> (data_p);
  result.assign (reinterpret_cast<char*> (wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.ucSSID),
                 wlan_connection_attributes_p->wlanAssociationAttributes.dot11Ssid.uSSIDLength);

clean:
  if (data_p)
    WlanFreeMemory (data_p);
  if (release_handle)
    Net_WLAN_Tools::finalize (client_handle);
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
                         interface_identifier.c_str (),
                         SIOCGIWESSID,
                         &iwreq_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_ext(\"%s\",SIOCGIWESSID): \"%m\", aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: the length iwreq_s.u.essid.length is wrong
//  ACE_ASSERT (iwreq_s.u.essid.length && (iwreq_s.u.essid.length <= IW_ESSID_MAX_SIZE));
//  result.assign (essid, iwreq_s.u.essid.length);
  result = essid;

error:
  if (unlikely (close_socket))
    iw_sockets_close (socket_handle);

//continue_:
#endif

  return result;
}

Net_WLAN_SSIDs_t
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_WLAN_Tools::getSSIDs (HANDLE clientHandle_in,
                          REFGUID interfaceIdentifier_in)
#else
Net_WLAN_Tools::getSSIDs (const std::string& interfaceIdentifier_in,
                          ACE_HANDLE handle_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getSSIDs"));

  Net_WLAN_SSIDs_t result;

  Net_InterfaceIdentifiers_t interface_identifiers_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
#else
  if (unlikely (interfaceIdentifier_in.empty ()))
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();
#endif
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  DWORD result_2 = 0;
  DWORD flags =
#if (_WIN32_WINNT <= _WIN32_WINNT_WINXP) || defined (WINXP_SUPPORT)
    0;
#else
    (WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES        |
     WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES);
#endif
  struct _WLAN_AVAILABLE_NETWORK_LIST* wlan_network_list_p = NULL;
#if defined (_DEBUG)
  std::string SSID_string;
  std::string bss_network_type_string;
#endif
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    result_2 =
      WlanGetAvailableNetworkList (client_handle,
                                   &(*iterator),
                                   flags,
                                   NULL,
                                   &wlan_network_list_p);
    if (unlikely (result_2 != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetAvailableNetworkList(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  client_handle,
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
#endif
      result.push_back (SSID_string);
    } // end FOR
    WlanFreeMemory (wlan_network_list_p);
    wlan_network_list_p = NULL;
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
#else
  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
//  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  int result_2 = -1;
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  struct iw_range iw_range_s;
  ACE_OS::memset (&iw_range_s, 0, sizeof (struct iw_range));
  struct stream_descr stream_descr_s;
  ACE_OS::memset (&stream_descr_s, 0, sizeof (struct stream_descr));
  struct iw_event iw_event_s;
  ACE_OS::memset (&iw_event_s, 0, sizeof (struct iw_event));
  int error = 0;
  size_t buffer_size = IW_SCAN_MAX_DATA;
  struct ether_addr ap_mac_address;
  ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
  std::string essid_string;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return result;
    } // end IF
    close_handle = true;
  } // end IF

  result_2 = iw_get_range_info (socket_handle,
                                interfaceIdentifier_in.c_str (),
                                &iw_range_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_range_info(%d,\"%s\"): \"%m\", aborting\n"),
                socket_handle,
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean;
  } // end IF

  iwreq_s.u.data.pointer = ACE_OS::malloc (buffer_size);
  if (unlikely (!iwreq_s.u.data.pointer))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", aborting\n"),
                buffer_size));
    goto clean;
  } // end IF
  iwreq_s.u.data.length = buffer_size;

fetch_scan_result_data:
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWSCAN,
                            &iwreq_s);
  if (unlikely (result_2 < 0))
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7
        (iw_range_s.we_version_compiled > 16))
    { // buffer too small --> retry
      if (iwreq_s.u.data.length > buffer_size)
        buffer_size = iwreq_s.u.data.length;
      else
        buffer_size *= 2; // grow dynamically
      goto retry;
    } // end IF

    if (unlikely (error == EAGAIN)) // 11
    { // result(s) not available yet
      goto clean;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWSCAN): \"%m\", aborting\n"),
                handle_in));
    result.clear ();
    goto clean;
  } // end IF

  // the driver may have reported the required buffer size
  if (unlikely (iwreq_s.u.data.length > buffer_size))
  { // --> grow the buffer and retrys
    buffer_size = iwreq_s.u.data.length;
retry:
    iwreq_s.u.data.pointer = ACE_OS::realloc (iwreq_s.u.data.pointer,
                                              buffer_size);
    if (unlikely (!iwreq_s.u.data.pointer))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to reallocate memory (%u byte(s)): \"%m\", aborting\n"),
                  buffer_size));
      result.clear ();
      goto clean;
    } // end IF
    iwreq_s.u.data.length = buffer_size;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= buffer_size));

  // process the result data
  iw_init_event_stream (&stream_descr_s,
                        static_cast<char*> (iwreq_s.u.data.pointer),
                        static_cast<int> (iwreq_s.u.data.length));
  do
  {
    result_2 = iw_extract_event_stream (&stream_descr_s,
                                        &iw_event_s,
                                        iw_range_s.we_version_compiled);
    if (result_2 <= 0)
      break; // done

    switch (iw_event_s.cmd)
    {
      case SIOCGIWAP:
      {
        ACE_OS::memcpy (&ap_mac_address,
                        reinterpret_cast<void*> (iw_event_s.u.ap_addr.sa_data),
                        sizeof (struct ether_addr));
        break;
      }
      case SIOCGIWNWID:
      case SIOCGIWFREQ:
      case SIOCGIWMODE:
      case SIOCGIWNAME:
      case SIOCGIWENCODE:
      case SIOCGIWRATE:
      case SIOCGIWMODUL:
      case IWEVQUAL:
#ifndef WE_ESSENTIAL
      case IWEVGENIE:
#endif /* WE_ESSENTIAL */
      case IWEVCUSTOM:
        break;
      case SIOCGIWESSID:
      {
        ACE_ASSERT (iw_event_s.u.essid.pointer);
        ACE_ASSERT (iw_event_s.u.essid.length && (iw_event_s.u.essid.length <= IW_ESSID_MAX_SIZE));
        essid_string.assign (reinterpret_cast<char*> (iw_event_s.u.essid.pointer),
                             iw_event_s.u.essid.length);
        //        if (iw_event_s.u.essid.flags)
        //        {
        //          /* Does it have an ESSID index ? */
        //          if((iw_event_s.u.essid.flags & IW_ENCODE_INDEX) > 1)
        //            printf("                    ESSID:\"%s\" [%d]\n", essid,
        //                   (iw_event_s.u.essid.flags & IW_ENCODE_INDEX));
        //          else
        //            printf("                    ESSID:\"%s\"\n", essid);
        //        } // end IF
        //        else
        //          printf("                    ESSID:off/any/hidden\n");
        result.push_back (essid_string);
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("\"%s\": found wireless access point (MAC address: %s, ESSID: %s)...\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address),
                                                                          NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (essid_string.c_str ())));
#endif
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("invalid/unknown WE event type (was: %d), continuing\n"),
                    iw_event_s.cmd));
        break;
      }
    } // end SWITCH
  } while (true);

clean:
  if (iwreq_s.u.data.pointer)
    ACE_OS::free (iwreq_s.u.data.pointer);
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
    ACE_UNUSED_ARG (error);
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
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  int socket_handle = -1;
  struct iw_range iw_range_s;
  struct wireless_scan_head wireless_scan_head_s;
  struct wireless_scan* wireless_scan_p = NULL;
  bool done = false;

  // step1: select interface(s)
  if (likely (!interfaceIdentifier_in.empty ()))
  {
    interface_identifiers_a.push_back (interfaceIdentifier_in);
    goto continue_;
  } // end IF
  interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();

continue_:
  socket_handle = iw_sockets_open ();
  if (unlikely (socket_handle == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_sockets_open(): \"%m\", aborting\n")));
    return false;
  } // end IF

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       (iterator != interface_identifiers_a.end ()) && !done;
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
      { // found SSID on current interface
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
#if defined (DBUS_SUPPORT)
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
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
