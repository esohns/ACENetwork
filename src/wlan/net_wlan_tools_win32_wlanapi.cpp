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

#include <cguid.h>
#include <guiddef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <wlanapi.h>

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "common_xml_defines.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_wlan_defines.h"
#include "net_wlan_profile_xml_handler.h"

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
  ACE_ASSERT (!SSID_in.empty ());

  bool result = false;
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
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
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
    interface_identifier = interface_identifiers_a.front ();
  } // end IF

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
    interface_identifiers_a.push_back (interfaceIdentifier_in);

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
