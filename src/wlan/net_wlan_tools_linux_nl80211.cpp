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

#include <sys/capability.h>
#include <linux/capability.h>
//#include <linux/ieee80211.h>
// *TODO*: contains IW_ESSID_MAX_SIZE; should not include this
#include <linux/wireless.h>
#include <net/if.h> /* for IFNAMSIZ and co... */
//#include <net/if_arp.h>
// *TODO*: remove ASAP
#include "ifaddrs.h"

#include "netlink/attr.h"
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"
#include "netlink/msg.h"
#include "netlink/socket.h"

#include "ace/Handle_Set.h"
#include "ace/Netlink_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_macros.h"

#include "net_wlan_defines.h"

int
network_wlan_nl80211_error_cb (struct sockaddr_nl* address_in,
                               struct nlmsgerr* message_in,
                               void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_error_cb"));

  // initialize return value(s)
  int return_value = NL_SKIP;

  // sanity check(s)
  ACE_ASSERT (address_in);
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  Net_Netlink_Addr netlink_address (address_in,
                                    sizeof (struct sockaddr_nl));
  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (&message_in->msg));
  ACE_ASSERT (genlmsghdr_p);
  int* result_p = static_cast<int*> (argument_in);

  struct nlattr* nlattr_a[NLMSGERR_ATTR_MAX + 1];
  int result = nla_parse (nlattr_a,
                          NLMSGERR_ATTR_MAX,
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
  const char* string_p = NULL;
  if (likely (nlattr_a[NLMSGERR_ATTR_MSG]))
    string_p = nla_get_string (nlattr_a[NLMSGERR_ATTR_MSG]);
  else
    string_p = nl_geterror (message_in->error);

  // decide severity
  // *NOTE*: the kernel returns negative errnos
  switch (genlmsghdr_p->cmd)
  {
    case NL80211_CMD_TRIGGER_SCAN:
    {
      if ((message_in->error == -EBUSY) || // 16: scan in progress ?
          (message_in->error == -ENOTSUP)) // 95: AP unreachable ?
        return_value = NL_OK;
      break;
    }
    default:
      break;
  } // end SWITCH

  if (likely (return_value == NL_SKIP))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": nl80211 error: \"%s\" (was: command: %d; errno: \"%s\" (%d)), skipping\n"),
                ACE_TEXT (Net_Common_Tools::NetlinkAddressToString (netlink_address).c_str ()),
                ACE_TEXT (string_p),
                genlmsghdr_p->cmd,
                ACE_TEXT (ACE_OS::strerror (-message_in->error)),
                message_in->error));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": nl80211 error: \"%s\" (was: command: %d; errno: \"%s\" (%d)), continuing\n"),
                ACE_TEXT (Net_Common_Tools::NetlinkAddressToString (netlink_address).c_str ()),
                ACE_TEXT (string_p),
                genlmsghdr_p->cmd,
                ACE_TEXT (ACE_OS::strerror (-message_in->error)),
                message_in->error));
#endif // _DEBUG

  *result_p = message_in->error;

  return return_value;
}

int
network_wlan_nl80211_interface_cb (struct nl_msg* message_in,
                                   void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_interface_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_INTERFACE);
  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  int result = nla_parse (nlattr_a, NL80211_ATTR_MAX,
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
  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX] ||
                !nlattr_a[NL80211_ATTR_WIPHY]   ||
                !nlattr_a[NL80211_ATTR_IFTYPE]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), continuing\n")));
    return NL_STOP;
  } // end IF
  struct Net_WLAN_nl80211_InterfaceConfigurationCBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_InterfaceConfigurationCBData*> (argument_in);
  ACE_ASSERT (cb_data_p->index);
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
    return NL_SKIP;
  cb_data_p->type =
      static_cast<enum nl80211_iftype> (nla_get_u32 (nlattr_a[NL80211_ATTR_IFTYPE]));

  return NL_STOP;
}

int
network_wlan_nl80211_bssid_cb (struct nl_msg* message_in,
                               void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_bssid_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_SCAN_RESULTS);
  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  struct nlattr* nlattr_2[NL80211_BSS_MAX + 1];
  enum nl80211_bss_status status_e;
  int result = nla_parse (nlattr_a, NL80211_ATTR_MAX,
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
  if (unlikely (!nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss' attribute, continuing\n")));
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
  if (unlikely (!nlattr_2[NL80211_BSS_STATUS] ||
                !nlattr_2[NL80211_BSS_BSSID]))
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("missing 'bss status/bssid' attribute(s), continuing\n")));
    return NL_SKIP;
  } // end IF
  status_e =
    static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]));
  switch (status_e)
  {
    case NL80211_BSS_STATUS_AUTHENTICATED:
    case NL80211_BSS_STATUS_ASSOCIATED:
    case NL80211_BSS_STATUS_IBSS_JOINED:
      break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("invalid/unknown bss status (was: %d), returning\n"),
                  static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]))));
      return NL_SKIP;
    }
  } // end SWITCH
  struct ether_addr* ap_mac_address_p =
      static_cast<struct ether_addr*> (argument_in);
  result = nla_memcpy (ap_mac_address_p->ether_addr_octet,
                       nlattr_2[NL80211_BSS_BSSID],
                       ETH_ALEN);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nla_memcpy(%d): \"%m\", aborting\n"),
                NL80211_BSS_BSSID));
    return NL_SKIP;
  } // end IF

  return NL_STOP;
}

int
network_wlan_nl80211_ssid_cb (struct nl_msg* message_in,
                              void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_ssid_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_SCAN_RESULTS);
  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  struct nlattr* nlattr_2[NL80211_BSS_MAX + 1];
  enum nl80211_bss_status status_e;
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
  if (unlikely (!nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss' attribute, continuing\n")));
    return NL_SKIP;
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
  if (unlikely (!nlattr_2[NL80211_BSS_STATUS] ||
                !nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]))
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("missing 'bss status/information elements' attribute(s), continuing\n")));
    return NL_SKIP;
  } // end IF
  status_e =
    static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]));
  switch (status_e)
  {
    case NL80211_BSS_STATUS_AUTHENTICATED:
    case NL80211_BSS_STATUS_ASSOCIATED:
    case NL80211_BSS_STATUS_IBSS_JOINED:
      break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("invalid/unknown bss status (was: %d), returning\n"),
                  static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]))));
      return NL_SKIP;
    }
  } // end SWITCH
  // *NOTE*: information elements are encoded like so: id[1]len[1]/data[len]
  //         (see also: http://standards.ieee.org/findstds/standard/802.11-2016.html)
  void* information_elements_p = nla_data (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]);
  struct Net_WLAN_IEEE802_11_InformationElement* information_element_p = NULL;
  int offset = 0;
  do
  {
    if (offset >= nla_len (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]))
      break; // no more 'IE's
    information_element_p =
          reinterpret_cast<struct Net_WLAN_IEEE802_11_InformationElement*> ((uint8_t*)information_elements_p + offset);
    // *NOTE*: see aforementioned document "Table 7-26—Element IDs"
    if (information_element_p->id == 0)
      break;
    offset += (1 + 1 + information_element_p->length);
  } while (true);
  if (unlikely (!information_element_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'SSID' information element, continuing\n")));
    return NL_SKIP;
  } // end IF
  ACE_ASSERT (information_element_p->length <= IW_ESSID_MAX_SIZE);
  std::string* ssid_p = static_cast<std::string*> (argument_in);
  ssid_p->assign (reinterpret_cast<char*> (&information_element_p->data),
                  information_element_p->length);

  return NL_STOP;
}

int
network_wlan_nl80211_ssids_cb (struct nl_msg* message_in,
                               void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_ssids_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_SCAN_RESULTS);
  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  struct nlattr* nlattr_2[NL80211_BSS_MAX + 1];
  std::string ssid_string;
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
  if (unlikely (!nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss' attribute, continuing\n")));
    return NL_SKIP;
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
  if (unlikely (!nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss information elements' attribute, continuing\n")));
    return NL_SKIP;
  } // end IF
  ssid_string.assign (reinterpret_cast<char*> (nla_data (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS])),
                      nla_len (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]));
  Net_WLAN_SSIDs_t* ssids_p = static_cast<Net_WLAN_SSIDs_t*> (argument_in);
  ssids_p->push_back (ssid_string);

  return NL_OK;
}

int
network_wlan_nl80211_feature_cb (struct nl_msg* message_in,
                                 void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_feature_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_WIPHY);
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
  ACE_ASSERT (nlattr_a[NL80211_ATTR_WIPHY] && nlattr_a[NL80211_ATTR_WIPHY_NAME]);
  if (unlikely (!nlattr_a[NL80211_ATTR_FEATURE_FLAGS] ||
                !nlattr_a[NL80211_ATTR_EXT_FEATURES]))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s [%u]: missing 'features'/'ext. features' attribute(s), continuing\n"),
                ACE_TEXT (nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME])),
                nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY])));
  struct Net_WLAN_nl80211_InterfaceFeaturesCBData* features_p =
      static_cast<struct Net_WLAN_nl80211_InterfaceFeaturesCBData*> (argument_in);
  if (likely (nlattr_a[NL80211_ATTR_FEATURE_FLAGS]))
    features_p->features = nla_get_u32 (nlattr_a[NL80211_ATTR_FEATURE_FLAGS]);
  if (likely (nlattr_a[NL80211_ATTR_EXT_FEATURES]))
  {
    uint8_t* data_p =
        reinterpret_cast<uint8_t*> (nla_data (nlattr_a[NL80211_ATTR_EXT_FEATURES]));
    ACE_ASSERT (data_p);
    for (unsigned int i = 0;
         i < static_cast<unsigned int> (nla_len (nlattr_a[NL80211_ATTR_EXT_FEATURES]));
         ++i)
      for (unsigned int j = 0;
           j < 8;
           ++j)
        if (*(data_p + i) & (1 << j))
          features_p->extendedFeatures.insert (static_cast<enum nl80211_ext_feature_index> ((i * 8) + j));
  } // end IF

  return NL_OK;
}

//////////////////////////////////////////

Net_InterfaceIdentifiers_t
Net_WLAN_Tools::getInterfaces (struct nl_sock* handle_in,
                               int driverFamilyId_in,
                               int addressFamily_in,
                               int flags_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  // initialize return value(s)
  Net_InterfaceIdentifiers_t result;

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
    if (!Net_WLAN_Tools::isInterface (ifaddrs_2->ifa_name,
                                      handle_in,
                                      driverFamilyId_in))
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

  return result;
}

bool
Net_WLAN_Tools::isInterface (const std::string& interfaceIdentifier_in,
                             struct nl_sock* handle_in,
                             int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::isInterface"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

  bool result = false;
  struct nl_sock* socket_handle_p = handle_in;
  bool release_handle = false;
  int driver_family_id_i = driverFamilyId_in;
  int result_2 = -1;
  struct Net_WLAN_nl80211_InterfaceConfigurationCBData cb_data_s;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  struct nl_msg* message_p = NULL;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc (): \"%m\", aborting\n")));
      return result; // *TODO*: avoid false negatives
    } // end IF
    release_handle = true;

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF

    if (unlikely (driver_family_id_i <= 0))
    {
      driver_family_id_i =
          genl_ctrl_resolve (socket_handle_p,
                             ACE_TEXT_ALWAYS_CHAR (NL80211_GENL_NAME));
      if (driver_family_id_i <= 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to genl_ctrl_resolve(0x%@,\"%s\"): \"%s\", returning\n"),
                    socket_handle_p,
                    ACE_TEXT (NL80211_GENL_NAME),
                    ACE_TEXT (nl_geterror (driver_family_id_i))));
        goto clean;
      } // end IF
    } // end IF

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc (): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          &result_2);
    ACE_ASSERT (result_2 >= 0);
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p);
    callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);
  ACE_ASSERT (driver_family_id_i > 0);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  ACE_ASSERT (callback_p);
  callback_2 = nl_cb_clone (callback_p);
  ACE_ASSERT (callback_2);
  result_2 = nl_cb_set (callback_2,
                        NL_CB_VALID,
                        NL_CB_CUSTOM,
                        network_wlan_nl80211_interface_cb,
                        &cb_data_s);
  ACE_ASSERT (!result_2);
//  nl_socket_set_cb (handle_in, callback_2);

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto clean;
  } // end IF
  // *NOTE*: "...When getting scan results without triggering scan first, you'll
  //         always get the information about currently associated BSS..."
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,              // port #
                              NL_AUTO_SEQ,               // sequence #
                              driver_family_id_i,        // family id
                              0,                         // (user-) hdrlen
                              NLM_F_DUMP,                // flags
                              NL80211_CMD_GET_INTERFACE, // command id
                              0)))                       // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto clean;
  } // end IF
  cb_data_s.index = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!cb_data_s.index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean;
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (cb_data_s.index));

  result_2 = nl_send_auto (socket_handle_p, message_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean;
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  while (result_2 > 0)
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean;
  } // end IF

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);
  if (callback_2)
    nl_cb_put (callback_2);
  if (release_handle)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF

  return ((cb_data_s.type == NL80211_IFTYPE_ADHOC) ||
          (cb_data_s.type == NL80211_IFTYPE_STATION));
}

bool
Net_WLAN_Tools::associate (const std::string& interfaceIdentifier_in,
                           const struct ether_addr& APMACAddress_in,
                           const std::string& SSID_in,
                           struct nl_sock* handle_in,
                           int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associate"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (!SSID_in.empty ());
  ACE_ASSERT (SSID_in.size () <= IW_ESSID_MAX_SIZE);
  ACE_ASSERT (driverFamilyId_in > 0);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);

  ACE_NOTREACHED (return result;)
}

bool
Net_WLAN_Tools::disassociate (const std::string& interfaceIdentifier_in,
                              struct nl_sock* handle_in,
                              int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disassociate"));

  bool result = false;

  // sanity check(s)
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (interfaceIdentifier_in.empty ()))
  {
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (handle_in,
                                                             driverFamilyId_in,
                                                             AF_UNSPEC,
                                                             0);
    if (interface_identifiers_a.empty ())
      return true;
  } // end IF
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);
  ACE_ASSERT (driverFamilyId_in > 0);

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
  } // end FOR

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);

  ACE_NOTREACHED (return result;)
}

void
Net_WLAN_Tools::scan (const std::string& interfaceIdentifier_in,
                      const std::string& ESSID_in,
                      struct nl_sock* handle_in,
                      int driverFamilyId_in,
                      bool dispatchResults_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::scan"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (ESSID_in.size () <= IW_ESSID_MAX_SIZE);
  ACE_ASSERT (driverFamilyId_in > 0);

//  int result = -1;
#if defined (ACE_LINUX)
  bool handle_capabilities = false;
#endif // ACE_LINUX

#if defined (ACE_LINUX)
  // (temporarily) elevate privileges to start scan
  if (!Common_Tools::hasCapability (CAP_NET_ADMIN,
                                    CAP_EFFECTIVE))
  {
    if (unlikely (!Common_Tools::setCapability (CAP_NET_ADMIN,
                                                CAP_EFFECTIVE)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setCapability(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::capabilityToString (CAP_NET_ADMIN).c_str ())));
      goto error;
    } // end IF
    handle_capabilities = true;
  } // end IF
#endif // ACE_LINUX

error:
#if defined (ACE_LINUX)
  if (handle_capabilities)
    if (!Common_Tools::dropCapability (CAP_NET_ADMIN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::dropCapability(CAP_NET_ADMIN): \"%m\", continuing\n")));
#endif // ACE_LINUX
}

struct ether_addr
Net_WLAN_Tools::associatedBSSID (const std::string& interfaceIdentifier_in,
                                 struct nl_sock* handle_in,
                                 int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedBSSID"));

  // initialize return value(s)
  struct ether_addr return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  struct nl_sock* socket_handle_p = handle_in;
  bool release_handle = false;
  int result_2 = -1;
  unsigned int if_index_i = 0;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  struct nl_msg* message_p = NULL;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc (): \"%m\", aborting\n")));
      return return_value;
    } // end IF
    release_handle = true;

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc (): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          &result_2);
    ACE_ASSERT (result_2 >= 0);
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p);
    callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);
  ACE_ASSERT (driverFamilyId_in);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  ACE_ASSERT (callback_p);
  callback_2 = nl_cb_clone (callback_p);
  ACE_ASSERT (callback_2);
  result_2 = nl_cb_set (callback_2,
                        NL_CB_VALID,
                        NL_CB_CUSTOM,
                        network_wlan_nl80211_bssid_cb,
                        &return_value);
  ACE_ASSERT (!result_2);
//  nl_socket_set_cb (handle_in, callback_2);

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto clean;
  } // end IF
  // *NOTE*: "...When getting scan results without triggering scan first, you'll
  //         always get the information about currently associated BSS..."
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,         // port #
                              NL_AUTO_SEQ,          // sequence #
                              driverFamilyId_in,    // family id
                              0,                    // (user-) hdrlen
                              NLM_F_DUMP,           // flags
                              NL80211_CMD_GET_SCAN, // command id
                              0)))                  // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto clean;
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean;
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           ap_mac_address_s.ether_addr_octet);

  result_2 = nl_send_auto (socket_handle_p, message_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean;
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  while (result_2 > 0)
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean;
  } // end IF
  ACE_ASSERT (!Net_Common_Tools::isAny (return_value));

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);
  if (callback_2)
    nl_cb_put (callback_2);
//  if (callback_p)
//    nl_socket_set_cb (handle_in, callback_p);
  if (release_handle)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF

  return return_value;
}

std::string
Net_WLAN_Tools::associatedSSID (const std::string& interfaceIdentifier_in,
                                struct nl_sock* handle_in,
                                int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  std::string return_value;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  struct nl_sock* socket_handle_p = handle_in;
  bool release_handle = false;
  int result_2 = -1;
  unsigned int if_index_i = 0;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  struct nl_msg* message_p = NULL;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc (): \"%m\", aborting\n")));
      return return_value;
    } // end IF
    release_handle = true;

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc (): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          &result_2);
    ACE_ASSERT (result_2 >= 0);
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p);
    callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);
  ACE_ASSERT (driverFamilyId_in);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  ACE_ASSERT (callback_p);
  callback_2 = nl_cb_clone (callback_p);
  ACE_ASSERT (callback_2);
  result_2 = nl_cb_set (callback_2,
                        NL_CB_VALID,
                        NL_CB_CUSTOM,
                        network_wlan_nl80211_ssid_cb,
                        &return_value);
  ACE_ASSERT (!result_2);
//  nl_socket_set_cb (handle_in, callback_2);

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,         // port #
                              NL_AUTO_SEQ,          // sequence #
                              driverFamilyId_in,    // family id
                              0,                    // (user-) hdrlen
                              NLM_F_DUMP,           // flags
                              NL80211_CMD_GET_SCAN, // command id
                              0)))                  // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           ap_mac_address_s.ether_addr_octet);

  result_2 = nl_send_auto (socket_handle_p, message_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  while (result_2 > 0)
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);
  if (callback_2)
    nl_cb_put (callback_2);
//  if (callback_p)
//    nl_socket_set_cb (handle_in, callback_p);
  if (release_handle)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF

  return return_value;
}

Net_WLAN_SSIDs_t
Net_WLAN_Tools::getSSIDs (const std::string& interfaceIdentifier_in,
                          struct nl_sock* handle_in,
                          int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getSSIDs"));

  // initialize return value(s)
  Net_WLAN_SSIDs_t return_value;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  struct nl_sock* socket_handle_p = handle_in;
  bool release_handle = false;
  int result_2 = -1;
  unsigned int if_index_i = 0;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  struct nl_msg* message_p = NULL;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc (): \"%m\", aborting\n")));
      return return_value;
    } // end IF
    release_handle = true;

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc (): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          &result_2);
    ACE_ASSERT (result_2 >= 0);
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p);
    callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);
  ACE_ASSERT (driverFamilyId_in);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  ACE_ASSERT (callback_p);
  callback_2 = nl_cb_clone (callback_p);
  ACE_ASSERT (callback_2);
  result_2 = nl_cb_set (callback_2,
                        NL_CB_VALID,
                        NL_CB_CUSTOM,
                        network_wlan_nl80211_ssids_cb,
                        &return_value);
  ACE_ASSERT (!result_2);
//  nl_socket_set_cb (handle_in, callback_2);

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,         // port #
                              NL_AUTO_SEQ,          // sequence #
                              driverFamilyId_in,    // family id
                              0,                    // (user-) hdrlen
                              NLM_F_DUMP,           // flags
                              NL80211_CMD_GET_SCAN, // command id
                              0)))                  // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));

  result_2 = nl_send_auto (socket_handle_p, message_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  while (result_2 > 0)
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);
  if (callback_2)
    nl_cb_put (callback_2);
//  if (callback_p)
//    nl_socket_set_cb (handle_in, callback_p);
  if (release_handle)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF

  return return_value;
}

bool
Net_WLAN_Tools::getFeatures (const std::string& interfaceIdentifier_in,
                             struct nl_sock* handle_in,
                             int driverFamilyId_in,
                             ACE_UINT32& features_out,
                             Net_WLAN_nl80211_ExtendedFeatures_t& extendedFeatures_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getFeatures"));

  // initialize return values
  features_out = 0;
  extendedFeatures_out.clear ();
  bool result = false;

  int result_2 = -1;
  struct Net_WLAN_nl80211_InterfaceFeaturesCBData features_s;
  struct nl_msg* message_p = NULL;
  unsigned int if_index_i = 0;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return result; // *TODO*: avoid false negatives
  } // end IF

  // sanity check(s)
  struct nl_sock* socket_handle_p = handle_in;
  bool release_handle = false;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc (): \"%m\", aborting\n")));
      return result;
    } // end IF
    release_handle = true;

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc (): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          &result_2);
    ACE_ASSERT (result_2 >= 0);
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p);
    callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);
  ACE_ASSERT (driverFamilyId_in);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  ACE_ASSERT (callback_p);
  callback_2 = nl_cb_clone (callback_p);
  ACE_ASSERT (callback_2);
  result_2 = nl_cb_set (callback_2,
                        NL_CB_VALID,
                        NL_CB_CUSTOM,
                        network_wlan_nl80211_feature_cb,
                        &features_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_cb_set(0x%@), aborting\n"),
                callback_2));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
//  nl_socket_set_cb (handle_in, callback_2);

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,          // port #
                              NL_AUTO_SEQ,           // sequence #
                              driverFamilyId_in,     // family id
                              0,                     // (user-) hdrlen
                              0,                     // flags
                              NL80211_CMD_GET_WIPHY, // command id
                              0)))                   // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));

  result_2 = nl_send_auto (socket_handle_p, message_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  while (result_2 > 0)
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF

  features_out = features_s.features;
  extendedFeatures_out = features_s.extendedFeatures;

  result = true;

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);
  if (callback_2)
    nl_cb_put (callback_2);
//  if (callback_p)
//    nl_socket_set_cb (handle_in, callback_p);
  if (release_handle)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF

  return result;
}
