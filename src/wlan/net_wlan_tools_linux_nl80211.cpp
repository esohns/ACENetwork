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

#include <cerrno>

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

#include "common_os_tools.h"

#include "net_common.h"
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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->error);
//  ACE_ASSERT (!*(cb_data_p->error));

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
      if (unlikely ((message_in->error != -EBUSY)   && //  16: scan in progress ?
                    (message_in->error != -ENOTSUP) && //  95: access point unreachable ?
                    (message_in->error != -ENETDOWN))) // 100: network down ?
        return_value = NL_STOP;
      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_DEBUG ((((return_value == NL_STOP) ? LM_ERROR : LM_DEBUG),
              ACE_TEXT ("\"%s\": nl80211 error: \"%s\" (was: command: %d; errno: \"%s\" (%d)), %s\n"),
              ACE_TEXT (Net_Common_Tools::NetlinkAddressToString (netlink_address).c_str ()),
              ACE_TEXT (string_p),
              genlmsghdr_p->cmd,
              ACE_TEXT (ACE_OS::strerror (-message_in->error)),
              -message_in->error,
              ((return_value == NL_STOP) ? ACE_TEXT ("stopping") : ACE_TEXT ("skipping"))));

  *(cb_data_p->error) = message_in->error;

  return return_value;
}

int
network_wlan_nl80211_interfaces_cb (struct nl_msg* message_in,
                                    void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_interfaces_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_INTERFACE);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->interfaces);
  char buffer_a[IF_NAMESIZE];

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
  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX]))// ||
//                !nlattr_a[NL80211_ATTR_WIPHY]   ||
//                !nlattr_a[NL80211_ATTR_IFTYPE]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  ACE_OS::memset (buffer_a, 0, sizeof (char[IF_NAMESIZE]));
  if (unlikely (!::if_indextoname (nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]),
                                   buffer_a)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::if_indextoname(%u): \"%m\", aborting\n"),
                nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX])));
    return NL_STOP;
  } // end IF
  cb_data_p->interfaces->push_back (buffer_a);

  return NL_OK;
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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->index);
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));

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
//                !nlattr_a[NL80211_ATTR_WIPHY]   ||
                !nlattr_a[NL80211_ATTR_IFTYPE]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
    return NL_SKIP;

  ACE_ASSERT (cb_data_p->type == NL80211_IFTYPE_UNSPECIFIED);
  cb_data_p->type =
      static_cast<enum nl80211_iftype> (nla_get_u32 (nlattr_a[NL80211_ATTR_IFTYPE]));

  *cb_data_p->done = 1;

  return NL_OK;
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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->index);
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));

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
  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
    return NL_SKIP;

  if (unlikely (!nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss' attribute, aborting\n")));
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
//    case NL80211_BSS_STATUS_AUTHENTICATED:
    case NL80211_BSS_STATUS_ASSOCIATED:
    case NL80211_BSS_STATUS_IBSS_JOINED:
      break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("invalid/unknown bss status (was: %d), aborting\n"),
                  static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]))));
      return NL_STOP;
    }
  } // end SWITCH
  result = nla_memcpy (&(cb_data_p->address.ether_addr_octet),
                       nlattr_2[NL80211_BSS_BSSID],
                       ETH_ALEN);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nla_memcpy(%d): \"%m\", aborting\n"),
                NL80211_BSS_BSSID));
    return NL_STOP;
  } // end IF

  *cb_data_p->done = 1;

  return NL_OK;
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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));
  ACE_ASSERT (cb_data_p->index);
  ACE_ASSERT (cb_data_p->SSIDs);
  ACE_ASSERT (cb_data_p->SSIDs->empty ());

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
  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
    return NL_SKIP;

  if (unlikely (//!nlattr_a[NL80211_ATTR_GENERATION] ||
                !nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'generation/bss' attribute(s), aborting\n")));
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
  if (unlikely (!nlattr_2[NL80211_BSS_BSSID]               ||
                !nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing bss 'bssid/IEs' attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (!nlattr_2[NL80211_BSS_STATUS])
    return NL_SKIP;
  status_e =
    static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]));
  switch (status_e)
  {
//    case NL80211_BSS_STATUS_AUTHENTICATED:
    case NL80211_BSS_STATUS_ASSOCIATED:
    case NL80211_BSS_STATUS_IBSS_JOINED:
      break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("invalid/unknown bss status (was: %d), aborting\n"),
                  static_cast<enum nl80211_bss_status> (nla_get_u32 (nlattr_2[NL80211_BSS_STATUS]))));
      return NL_STOP;
    }
  } // end SWITCH

  ACE_OS::memcpy (&cb_data_p->address.ether_addr_octet,
                  nla_data (nlattr_2[NL80211_BSS_BSSID]),
                  ETH_ALEN);

  // *NOTE*: information elements are encoded like so: id[1]len[1]/data[len]
  //         (see also: http://standards.ieee.org/findstds/standard/802.11-2016.html)
  void* information_elements_p =
      nla_data (nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]);
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
    information_element_p = NULL;
  } while (true);
  if (unlikely (!information_element_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'SSID' information element, aborting\n")));
    return NL_STOP;
  } // end IF
  ACE_ASSERT (information_element_p->length <= NET_WLAN_ESSID_MAX_SIZE);
  std::string SSID_string (reinterpret_cast<char*> (&information_element_p->data),
                           information_element_p->length);
  cb_data_p->SSIDs->push_back (SSID_string);

  *cb_data_p->done = 1;

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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->index);
  ACE_ASSERT (cb_data_p->SSIDs);
//  ACE_ASSERT (cb_data_p->done);
//  ACE_ASSERT (!(*cb_data_p->done));
  std::string SSID_string;

  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
  struct nlattr* nlattr_2[NL80211_BSS_MAX + 1];
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
  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
    return NL_SKIP;

  if (unlikely (!nlattr_a[NL80211_ATTR_BSS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss' attribute, aborting\n")));
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
  if (unlikely (!nlattr_2[NL80211_BSS_INFORMATION_ELEMENTS]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'bss information elements' attribute, continuing\n")));
    return NL_SKIP;
  } // end IF
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
    information_element_p = NULL;
  } while (true);
  if (unlikely (!information_element_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing 'SSID' information element, aborting\n")));
    return NL_STOP;
  } // end IF
  ACE_ASSERT (information_element_p->length <= NET_WLAN_ESSID_MAX_SIZE);
  SSID_string.assign (reinterpret_cast<char*> (&information_element_p->data),
                      information_element_p->length);
  cb_data_p->SSIDs->push_back (SSID_string);

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
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->features);
  ACE_ASSERT (cb_data_p->features->extendedFeatures.empty ());
  ACE_ASSERT (!cb_data_p->features->features);
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));
  ACE_ASSERT (!cb_data_p->name.empty ());

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
  if (unlikely (!nlattr_a[NL80211_ATTR_WIPHY_NAME]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (ACE_OS::strcmp (cb_data_p->name.c_str (),
                      nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME])))
    return NL_SKIP;

  ACE_ASSERT (nlattr_a[NL80211_ATTR_WIPHY]);
  if (unlikely (!nlattr_a[NL80211_ATTR_FEATURE_FLAGS] ||
                !nlattr_a[NL80211_ATTR_EXT_FEATURES]))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("\"%s\" [%u]: missing 'features'/'ext. features' attribute(s), continuing\n"),
                ACE_TEXT (nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME])),
                nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY])));
  if (likely (nlattr_a[NL80211_ATTR_FEATURE_FLAGS]))
    cb_data_p->features->features =
        nla_get_u32 (nlattr_a[NL80211_ATTR_FEATURE_FLAGS]);
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
          cb_data_p->features->extendedFeatures.insert (static_cast<enum nl80211_ext_feature_index> ((i * 8) + j));
  } // end IF

  *cb_data_p->done = 1;

  return NL_OK;
}

int
network_wlan_nl80211_feature_2_cb (struct nl_msg* message_in,
                                   void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_feature_2_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_GET_PROTOCOL_FEATURES);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->protocolFeatures);
  ACE_ASSERT (!(*cb_data_p->protocolFeatures));
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));

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
//  if (unlikely (!nlattr_a[NL80211_ATTR_IFINDEX]))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("missing attribute(s), aborting\n")));
//    return NL_STOP;
//  } // end IF
//  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_IFINDEX]))
//    return NL_SKIP;

  if (unlikely (!nlattr_a[NL80211_ATTR_PROTOCOL_FEATURES]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  *cb_data_p->protocolFeatures =
      nla_get_u32 (nlattr_a[NL80211_ATTR_PROTOCOL_FEATURES]);

  *cb_data_p->done = 1;

  return NL_OK;
}

int
network_wlan_nl80211_wiphy_cb (struct nl_msg* message_in,
                               void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_wiphy_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_WIPHY);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
  ACE_ASSERT (cb_data_p->done);
  ACE_ASSERT (!(*cb_data_p->done));
  ACE_ASSERT (cb_data_p->name.empty ());

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
  if (unlikely (!nlattr_a[NL80211_ATTR_WIPHY]     ||
                !nlattr_a[NL80211_ATTR_WIPHY_NAME]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  if (cb_data_p->index != nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY]))
    return NL_SKIP;

  cb_data_p->name = nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME]);

  *cb_data_p->done = 1;

  return NL_OK;
}

int
network_wlan_nl80211_wiphys_cb (struct nl_msg* message_in,
                                void* argument_in)
{
  //  NETWORK_TRACE (ACE_TEXT ("network_wlan_nl80211_wiphys_cb"));

  // sanity check(s)
  ACE_ASSERT (message_in);
  ACE_ASSERT (argument_in);

  struct genlmsghdr* genlmsghdr_p =
      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
  ACE_ASSERT (genlmsghdr_p);
  ACE_ASSERT (genlmsghdr_p->cmd == NL80211_CMD_NEW_WIPHY);
  struct Net_WLAN_nl80211_CBData* cb_data_p =
      static_cast<struct Net_WLAN_nl80211_CBData*> (argument_in);
//  ACE_ASSERT (cb_data_p->monitor);
  ACE_ASSERT (cb_data_p->wiPhys);
  Net_WLAN_WiPhyIdentifiersIterator_t iterator;

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
  if (unlikely (!nlattr_a[NL80211_ATTR_WIPHY]     ||
                !nlattr_a[NL80211_ATTR_WIPHY_NAME]))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("missing attribute(s), aborting\n")));
    return NL_STOP;
  } // end IF
  // *TODO*: for reasons yet unknown, the callback is invoked several times for
  //         the same wiphy
  iterator =
      cb_data_p->wiPhys->find (nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME]));
  if (iterator != cb_data_p->wiPhys->end ())
    return NL_SKIP;

  cb_data_p->wiPhys->insert (std::make_pair (nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME]),
                                             nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY])));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("detected wiphy \"%s\" (index: %u)...\n"),
//              ACE_TEXT ("\"%s\": detected wiphy \"%s\" (index: %u)...\n"),
//              ACE_TEXT (cb_data_p->monitor->interfaceIdentifier ().c_str ()),
              ACE_TEXT (nla_get_string (nlattr_a[NL80211_ATTR_WIPHY_NAME])),
              nla_get_u32 (nlattr_a[NL80211_ATTR_WIPHY])));
#endif // _DEBUG

  return NL_OK;
}

//////////////////////////////////////////

Net_InterfaceIdentifiers_t
Net_WLAN_Tools::getInterfaces (struct nl_sock* socketHandle_in,
                               int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  // initialize return value(s)
  Net_InterfaceIdentifiers_t return_value;

  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.interfaces = &return_value;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (!driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return return_value;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,               // port #
                              NL_AUTO_SEQ,                // sequence #
                              driverFamilyId_in,          // family id
                              0,                          // (user-) hdrlen
                              NLM_F_REQUEST | NLM_F_DUMP, // flags
                              NL80211_CMD_GET_INTERFACE,  // command id
                              0)))                        // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF

  cb_data_s.dumping = true;
  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_interfaces_cb,
                                                 &cb_data_s,
                                                 ACE_TEXT_ALWAYS_CHAR (""),
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_INTERFACE));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

clean:
  if (message_p)
    nlmsg_free (message_p);

  return return_value;
}

bool
Net_WLAN_Tools::isInterface (const std::string& interfaceIdentifier_in,
                             struct nl_sock* socketHandle_in,
                             int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::isInterface"));

  struct Net_WLAN_nl80211_CBData cb_data_s;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false; // *TODO*: avoid false negatives
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return false; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,              // port #
                              NL_AUTO_SEQ,               // sequence #
                              driverFamilyId_in,         // family id
                              0,                         // (user-) hdrlen
                              NLM_F_REQUEST,             // flags
                              NL80211_CMD_GET_INTERFACE, // command id
                              0)))                       // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_interface_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_INTERFACE));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

clean:
  if (message_p)
  {
    nlmsg_free (message_p); message_p = NULL;
  } // end IF

  return ((cb_data_s.type == NL80211_IFTYPE_ADHOC)   ||
          (cb_data_s.type == NL80211_IFTYPE_STATION) ||
          (cb_data_s.type == NL80211_IFTYPE_MONITOR));
}

Net_WLAN_WiPhyIdentifiers_t
Net_WLAN_Tools::getWiPhys (const std::string& interfaceIdentifier_in,
                           struct nl_sock* socketHandle_in,
                           int driverFamilyId_in,
                           bool splitWiPhyDump_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getWiPhys"));

  // initialize return values
  Net_WLAN_WiPhyIdentifiers_t return_value;

  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.wiPhys = &return_value;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return return_value;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,               // port #
                              NL_AUTO_SEQ,                // sequence #
                              driverFamilyId_in,          // family id
                              0,                          // (user-) hdrlen
//                              (splitWiPhyDump_in ? (NLM_F_REQUEST | NLM_F_DUMP) : NLM_F_REQUEST), // flags
                              NLM_F_REQUEST | NLM_F_DUMP, // flags
                              NL80211_CMD_GET_WIPHY,      // command id
                              0)))                        // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  // *TODO*: find out why/what this is
  if (splitWiPhyDump_in)
    NLA_PUT_FLAG (message_p, NL80211_ATTR_SPLIT_WIPHY_DUMP);

  cb_data_s.dumping = true;
  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_wiphys_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_WIPHY));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return return_value;
}

std::string
Net_WLAN_Tools::wiPhyIndexToWiPhyNameString (const std::string& interfaceIdentifier_in,
                                             struct nl_sock* socketHandle_in,
                                             int driverFamilyId_in,
                                             int wiPhyIndex_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::wiPhyIndexToWiPhyNameString"));

  // initialize return values
  std::string return_value;

  struct Net_WLAN_nl80211_CBData cb_data_s;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return return_value;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,          // port #
                              NL_AUTO_SEQ,           // sequence #
                              driverFamilyId_in,     // family id
                              0,                     // (user-) hdrlen
                              NLM_F_REQUEST,         // flags
                              NL80211_CMD_GET_WIPHY, // command id
                              0)))                   // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_WIPHY,
               static_cast<ACE_UINT32> (wiPhyIndex_in));
  cb_data_s.index = static_cast<unsigned int> (wiPhyIndex_in);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_wiphy_cb,
                                                 &cb_data_s,
                                                 ACE_TEXT_ALWAYS_CHAR (""),
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_WIPHY));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

  return_value = cb_data_s.name;

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return return_value;
}

bool
Net_WLAN_Tools::getWiPhyFeatures (const std::string& interfaceIdentifier_in,
                                  const std::string& wiPhyIdentifier_in,
                                  struct nl_sock* socketHandle_in,
                                  int driverFamilyId_in,
                                  struct Net_WLAN_nl80211_Features& features_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getWiPhyFeatures"));

  // initialize return values
  bool result = false;
  features_out.features = 0;
  features_out.extendedFeatures.clear ();

  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.features = &features_out;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                wiPhyIdentifier_in.empty ()                 ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,          // port #
                              NL_AUTO_SEQ,           // sequence #
                              driverFamilyId_in,     // family id
                              0,                     // (user-) hdrlen
                              NLM_F_REQUEST,         // flags
                              NL80211_CMD_GET_WIPHY, // command id
                              0)))                   // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  NLA_PUT_STRING (message_p,
                  NL80211_ATTR_WIPHY_NAME,
                  wiPhyIdentifier_in.c_str ());
  cb_data_s.name = wiPhyIdentifier_in;

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_feature_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_WIPHY));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

clean:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::hasWiPhyFeature (const struct Net_WLAN_nl80211_Features& features_in,
                                 enum nl80211_feature_flags feature_in,
                                 enum nl80211_ext_feature_index extendedFeature_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::hasWiPhyFeature"));

  // sanity check(s)
  ACE_ASSERT (feature_in || (extendedFeature_in < NUM_NL80211_EXT_FEATURES));

  if (likely (feature_in))
    return (features_in.features & static_cast<ACE_UINT32> (feature_in));

  return (features_in.extendedFeatures.find (extendedFeature_in) != features_in.extendedFeatures.end ());
}

bool
Net_WLAN_Tools::getProtocolFeatures (const std::string& interfaceIdentifier_in,
                                     struct nl_sock* socketHandle_in,
                                     int driverFamilyId_in,
                                     ACE_UINT32& features_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getProtocolFeatures"));

  // initialize return values
  bool result = false;
  features_out = 0;

  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.protocolFeatures = &features_out;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,                      // port #
                              NL_AUTO_SEQ,                       // sequence #
                              driverFamilyId_in,                 // family id
                              0,                                 // (user-) hdrlen
                              NLM_F_REQUEST,                     // flags
                              NL80211_CMD_GET_PROTOCOL_FEATURES, // command id
                              0)))                               // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_feature_2_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_PROTOCOL_FEATURES));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

clean:
//nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::authenticate (const std::string& interfaceIdentifier_in,
                              const struct ether_addr& accessPointMACAddress_in,
                              const std::string& ESSID_in,
                              enum nl80211_auth_type authenticationType_in,
                              ACE_UINT32 frequency_in,
                              struct nl_sock* socketHandle_in,
                              int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::authenticate"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
                ESSID_in.empty ()                                  ||
                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)       ||
//                (authenticationType_in != )                ||
                !frequency_in                                      ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,             // port #
                              NL_AUTO_SEQ,              // sequence #
                              driverFamilyId_in,        // family id
                              0,                        // (user-) hdrlen
                              NLM_F_REQUEST,            // flags
                              NL80211_CMD_AUTHENTICATE, // command id
                              0)))                      // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
  NLA_PUT (message_p,
           NL80211_ATTR_MAC,
           ETH_ALEN,
           &accessPointMACAddress_in.ether_addr_octet);
  // NL80211_ATTR_INACTIVITY_TIMEOUT
  // NL80211_ATTR_STATUS_CODE
  NLA_PUT (message_p,
           NL80211_ATTR_SSID,
           ESSID_in.size (),
           ESSID_in.c_str ());
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_WIPHY_FREQ,
               frequency_in);
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_AUTH_TYPE,
               authenticationType_in);
  // *TODO* support MDE and FTE
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
           //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_AUTHENTICATE));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::associate (const std::string& interfaceIdentifier_in,
                           const struct ether_addr& accessPointMACAddress_in,
                           const std::string& SSID_in,
                           enum nl80211_auth_type authenticationType_in,
                           ACE_UINT32 frequency_in,
                           struct nl_sock* socketHandle_in,
                           int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associate"));

  bool result = false;
//  ACE_UINT16 station_capabilities_i = 0;
//  ACE_UINT16 listen_interval_i = 0;
//  uint8_t rates_a[NL80211_MAX_SUPP_RATES];
//  ACE_UINT16 power_capabilities_i = 0;
//  int result_2 = -1;
//  uint8_t ie_buffer_a[BUFSIZ];

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
                SSID_in.empty ()                                   ||
                (SSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)        ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,          // port #
                              NL_AUTO_SEQ,           // sequence #
                              driverFamilyId_in,     // family id
                              0,                     // (user-) hdrlen
                              NLM_F_REQUEST,         // flags
                              NL80211_CMD_ASSOCIATE, // command id
                              0)))                   // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
  // NL80211_ATTR_CONTROL_PORT
  // NL80211_ATTR_PREV_BSSID
  // NL80211_ATTR_SOCKET_OWNER
  // NL80211_ATTR_USE_MFP
  // NL80211_ATTR_USE_RRM
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           &accessPointMACAddress_in.ether_addr_octet);
  // see also: IEEE Std 802.11-2007 7.3.1.4
//  station_capabilities_i |= 1 << 10; // support short preamble
//  station_capabilities_i |= 1 << 8;  // support channel agility
//  station_capabilities_i |= 1 << 7;  // support DSSS spectrum management
//  station_capabilities_i |= 1 << 5;  // support short slot time
//  NLA_PUT_U16 (message_p,
//               NL80211_ATTR_STA_CAPABILITY,
//               station_capabilities_i);
//  NLA_PUT (message_p,
//           NL80211_ATTR_STA_EXT_CAPABILITY,
//           0,
//           NULL);
  // see also: IEEE Std 802.11-2007 7.3.1.6
//  NLA_PUT_U16 (message_p,
//               NL80211_ATTR_STA_LISTEN_INTERVAL,
//               listen_interval_i);
  NLA_PUT (message_p,
           NL80211_ATTR_SSID,
           SSID_in.size (),
           SSID_in.c_str ());
  if (unlikely (!Net_Common_Tools::isAny (accessPointMACAddress_in)))
    NLA_PUT (message_p,
             NL80211_ATTR_MAC,
             ETH_ALEN,
             &(accessPointMACAddress_in.ether_addr_octet));
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_WIPHY_FREQ,
               frequency_in);
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_AUTH_TYPE,
               authenticationType_in);
  //  ACE_OS::memset (rates_a, 0, sizeof (uint8_t[NL80211_MAX_SUPP_RATES]));
  // MSB 1: BSSBasicRateSet (i.e. required), 0: optional; | x of 500kb/s
  // (see also: IEEE Std 802.11-2007 10.4.4.2)
//  rates_a[0] = ((1 << 7) | 2);  // 1 Mb/s
//  rates_a[1] = ((1 << 7) | 4);  // 2 Mb/s
//  rates_a[2] = ((1 << 7) | 11); // 5,5 Mb/s
//  rates_a[3] = ((1 << 7) | 22); // 11 Mb/s
//  rates_a[4] = 12;   // 6 Mb/s
//  rates_a[5] = 18;   // 9 Mb/s
//  rates_a[6] = 24;   // 12 Mb/s
//  rates_a[7] = 36;   // 18 Mb/s
//  rates_a[8] = 48;   // 24 Mb/s
//  rates_a[9] = 72;   // 36 Mb/s
//  rates_a[10] = 96;  // 48 Mb/s
//  rates_a[11] = 108; // 54 Mb/s
//  NLA_PUT (message_p,
//           NL80211_ATTR_STA_SUPPORTED_RATES,
//           sizeof (uint8_t[NL80211_MAX_SUPP_RATES]),
//           &rates_a);
//  ACE_OS::memset (ie_buffer_a, 0, sizeof (char[BUFSIZ]));
//  ie_buffer_a[0] = 33; // see also: IEEE Std 802.11-2007 7.3.2
//  ie_buffer_a[1] = 2;
//  // *NOTE*: "...The Minimum{/Maximum} Transmit Power Capability field shall be
//  //         set to the nominal minimum transmit power with which the STA is
//  //         capable of transmitting in the current channel, with a tolerance
//  //         ± 5 dB.
//  //         The field is coded as a signed integer in units of decibels
//  //         relative to 1 mW..."
//  // see also : IEEE Std 802.11-2007 7.3.2.16
//  power_capabilities_i = (15 << 8) | 15; // 15 dBm
//  ACE_OS::memcpy (&ie_buffer_a[2], &power_capabilities_i, sizeof (ACE_UINT16));
//  NLA_PUT (message_p,
//           NL80211_ATTR_IE,
//           4,
//           &ie_buffer_a);
//  // *NOTE*: regulatory domain policy is an inconsistent mess.
//  //         see also: output of 'iw reg get' (Germany):
//  // global
//  // country DE: DFS-ETSI
//  //        (2400 - 2483 @ 40), (N/A, 20), (N/A)
//  //        (5150 - 5250 @ 80), (N/A, 20), (N/A), NO-OUTDOOR, AUTO-BW
//  //        (5250 - 5350 @ 80), (N/A, 20), (0 ms), NO-OUTDOOR, DFS, AUTO-BW
//  //        (5470 - 5725 @ 160), (N/A, 26), (0 ms), DFS
//  //        (5725 - 5875 @ 80), (N/A, 13), (N/A)
//  //        (57000 - 66000 @ 2160), (N/A, 40), (N/A)
//  ie_buffer_a[0] = 36; // see also: IEEE Std 802.11-2007 7.3.2
//  ie_buffer_a[1] = 40;
//  // 2.4GHz band 2.412-2.472, in steps of 5MHz
//  ie_buffer_a[2] = 1;
//  ie_buffer_a[3] = 13;
//  // 5GHz band 5.18-5.7, spread, 20 MHz separation
//  // reg. class 1 (US/EU)
//  ie_buffer_a[4] = 36; // 5.18 GHz
//  ie_buffer_a[5] = 1;
//  ie_buffer_a[6] = 40; // 5.2 GHz
//  ie_buffer_a[7] = 1;
//  ie_buffer_a[8] = 44; // 5.22 GHz
//  ie_buffer_a[9] = 1;
//  ie_buffer_a[10] = 48; // 5.24 GHz
//  ie_buffer_a[11] = 1;
//  // reg. class 2 (US/EU)
//  ie_buffer_a[12] = 52; // 5.26 GHz
//  ie_buffer_a[13] = 1;
//  ie_buffer_a[14] = 56; // 5.28 GHz
//  ie_buffer_a[15] = 1;
//  ie_buffer_a[16] = 60; // 5.3 GHz
//  ie_buffer_a[17] = 1;
//  ie_buffer_a[18] = 64; // 5.32 GHz
//  ie_buffer_a[19] = 1;
//  // reg. class 3 (EU)
//  ie_buffer_a[20] = 100; // 5.5 GHz
//  ie_buffer_a[21] = 1;
//  ie_buffer_a[22] = 104; // 5.52 GHz
//  ie_buffer_a[23] = 1;
//  ie_buffer_a[24] = 108; // 5.54 GHz
//  ie_buffer_a[25] = 1;
//  ie_buffer_a[26] = 112; // 5.56 GHz
//  ie_buffer_a[27] = 1;
//  ie_buffer_a[28] = 116; // 5.58 GHz
//  ie_buffer_a[29] = 1;
//  ie_buffer_a[30] = 120; // 5.6 GHz
//  ie_buffer_a[31] = 1;
//  ie_buffer_a[32] = 124; // 5.62 GHz
//  ie_buffer_a[33] = 1;
//  ie_buffer_a[34] = 128; // 5.64 GHz
//  ie_buffer_a[35] = 1;
//  ie_buffer_a[36] = 132; // 5.66 GHz
//  ie_buffer_a[37] = 1;
//  ie_buffer_a[38] = 136; // 5.68 GHz
//  ie_buffer_a[39] = 1;
//  ie_buffer_a[40] = 140; // 5.7 GHz
//  ie_buffer_a[41] = 1;
//  NLA_PUT (message_p,
//           NL80211_ATTR_STA_SUPPORTED_CHANNELS,
//           42,
//           &ie_buffer_a);
//  ie_buffer_a[0] = 48; // see also: IEEE Std 802.11-2007 7.3.2
//  ie_buffer_a[1] = 2;
//  ie_buffer_a[2] = 1; // version 1
//  ie_buffer_a[3] = 0;
//  NLA_PUT (message_p,
//           NL80211_ATTR_IE,
//           4,
//           &ie_buffer_a);
//  ie_buffer_a[0] = 46; // see also: IEEE Std 802.11-2007 7.3.2
//  ie_buffer_a[1] = 1;
//  ie_buffer_a[2] = 0;
////  ie_buffer_a[2] |= (1 << 3); // Q-ACK
////  ie_buffer_a[2] |= (1 << 0); // more data ACK
//  nla_put_flag (message_p,
//                NL80211_ATTR_CONTROL_PORT);
//  NLA_PUT (message_p,
//           NL80211_ATTR_IE,
//           3,
//           &ie_buffer_a);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
                            //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_ASSOCIATE));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::connect (const std::string& interfaceIdentifier_in,
                         const struct ether_addr& accessPointMACAddress_in,
                         const std::string& ESSID_in,
                         struct nl_sock* socketHandle_in,
                         int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::connect"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
                ESSID_in.empty ()                                  ||
                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)       ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,        // port #
                              NL_AUTO_SEQ,         // sequence #
                              driverFamilyId_in,   // family id
                              0,                   // (user-) hdrlen
                              NLM_F_REQUEST,       // flags
                              NL80211_CMD_CONNECT, // command id
                              0)))                 // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
  // NL80211_ATTR_SOCKET_OWNER
  // NL80211_ATTR_USE_MFP
  // NL80211_ATTR_USE_RRM
  NLA_PUT (message_p,
           NL80211_ATTR_MAC,
           ETH_ALEN,
           &accessPointMACAddress_in.ether_addr_octet);
  NLA_PUT (message_p,
           NL80211_ATTR_SSID,
           ESSID_in.size (),
           ESSID_in.c_str ());
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
           //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_CONNECT));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::deauthenticate (const std::string& interfaceIdentifier_in,
                                struct nl_sock* socketHandle_in,
                                int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deauthenticate"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
//                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
//                ESSID_in.empty ()                                  ||
//                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)       ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,               // port #
                              NL_AUTO_SEQ,                // sequence #
                              driverFamilyId_in,          // family id
                              0,                          // (user-) hdrlen
                              NLM_F_REQUEST,              // flags
                              NL80211_CMD_DEAUTHENTICATE, // command id
                              0)))                        // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           &accessPointMACAddress_in.ether_addr_octet);
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           ESSID_in.size (),
//           ESSID_in.c_str ());
  // *TODO*: callers specify the most appropriate reason code
  NLA_PUT_U16 (message_p,
               NL80211_ATTR_REASON_CODE,
               NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING);
//      nla_put_flag (message_p,
//                    NL80211_ATTR_LOCAL_STATE_CHANGE);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
           //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_DEAUTHENTICATE));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::disassociate (const std::string& interfaceIdentifier_in,
                              struct nl_sock* socketHandle_in,
                              int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disassociate"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
//                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
//                ESSID_in.empty ()                                  ||
//                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)       ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,             // port #
                              NL_AUTO_SEQ,              // sequence #
                              driverFamilyId_in,        // family id
                              0,                        // (user-) hdrlen
                              NLM_F_REQUEST,            // flags
                              NL80211_CMD_DISASSOCIATE, // command id
                              0)))                      // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           &accessPointMACAddress_in.ether_addr_octet);
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           ESSID_in.size (),
//           ESSID_in.c_str ());
  // *TODO*: callers specify the most appropriate reason code
  NLA_PUT_U16 (message_p,
               NL80211_ATTR_REASON_CODE,
               NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING);
//      nla_put_flag (message_p,
//                    NL80211_ATTR_LOCAL_STATE_CHANGE);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
           //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_DISASSOCIATE));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::disconnect (const std::string& interfaceIdentifier_in,
                            struct nl_sock* socketHandle_in,
                            int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disconnect"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()                    ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)        ||
//                Net_Common_Tools::isAny (accessPointMACAddress_in) ||
//                ESSID_in.empty ()                                  ||
//                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE)       ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,           // port #
                              NL_AUTO_SEQ,            // sequence #
                              driverFamilyId_in,      // family id
                              0,                      // (user-) hdrlen
                              NLM_F_REQUEST,          // flags
                              NL80211_CMD_DISCONNECT, // command id
                              0)))                    // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", returning\n")));
    goto error;
  } // end IF
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           &accessPointMACAddress_in.ether_addr_octet);
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           ESSID_in.size (),
//           ESSID_in.c_str ());
  // *TODO*: callers specify the most appropriate reason code
  NLA_PUT_U16 (message_p,
               NL80211_ATTR_REASON_CODE,
               NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING);
//      nla_put_flag (message_p,
//                    NL80211_ATTR_LOCAL_STATE_CHANGE);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
           //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_DISCONNECT));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return result;
}

bool
Net_WLAN_Tools::scan (const std::string& interfaceIdentifier_in,
                      struct nl_sock* socketHandle_in,
                      int driverFamilyId_in,
                      const struct Net_WLAN_nl80211_Features& features_in,
                      const struct ether_addr& accessPointMACAddress_in,
                      const std::string& ESSID_in,
                      bool lowPriority_in,
                      bool flushCacheFirst_in,
                      bool randomizeMACAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::scan"));

  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()              ||
                (interfaceIdentifier_in.size () > IFNAMSIZ)  ||
                (ESSID_in.size () > NET_WLAN_ESSID_MAX_SIZE) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF

#if defined (ACE_LINUX)
  bool handle_capabilities_b = false;
#endif // ACE_LINUX
  struct nl_msg* message_p = NULL;
  struct nl_msg* nested_attribute_p = NULL;
  ACE_UINT32 scan_flags_i = 0;

#if defined (ACE_LINUX)
  // (temporarily try to) elevate privileges to start scan
  if (!Common_OS_Tools::hasCapability (CAP_NET_ADMIN,
                                       CAP_EFFECTIVE))
  {
    if (likely (Common_OS_Tools::setCapability (CAP_NET_ADMIN,
                                                CAP_EFFECTIVE)))
      handle_capabilities_b = true;
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setCapability(%s), continuing\n"),
                  ACE_TEXT (Common_OS_Tools::capabilityToString (CAP_NET_ADMIN).c_str ())));
  } // end IF
#endif // ACE_LINUX

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,             // port #
                              NL_AUTO_SEQ,              // sequence #
                              driverFamilyId_in,        // family id
                              0,                        // (user-) hdrlen
                              NLM_F_REQUEST,            // flags
                              NL80211_CMD_TRIGGER_SCAN, // command id
                              0)))                      // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  // *NOTE*: "...NL80211_ATTR_BSSID can be used to specify a BSSID to scan for;
  //         if not included, the wildcard BSSID will be used. ..."
  if (unlikely (!Net_Common_Tools::isAny (accessPointMACAddress_in)))
    NLA_PUT (message_p,
             NL80211_ATTR_BSSID,
             ETH_ALEN,
             &(accessPointMACAddress_in.ether_addr_octet));
  // *NOTE: "...If no SSID is passed, no probe requests are sent and a passive
  //         scan is performed. ..."
  nested_attribute_p = nlmsg_alloc ();
  if (unlikely (!nested_attribute_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  NLA_PUT (nested_attribute_p,
           1,
           (ESSID_in.empty () ? 0 : ESSID_in.size ()),
           (ESSID_in.empty () ? ACE_TEXT_ALWAYS_CHAR ("") : ESSID_in.c_str ()));
  nla_put_nested (message_p,
                  NL80211_ATTR_SCAN_SSIDS,
                  nested_attribute_p);
  nlmsg_free (nested_attribute_p);
  nested_attribute_p = NULL;
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           SSID_in.size (),
//           SSID_in.c_str ());
  if (lowPriority_in &&
      Net_WLAN_Tools::hasWiPhyFeature (features_in,
                                       NL80211_FEATURE_LOW_PRIORITY_SCAN,
                                       MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_LOW_PRIORITY;
  if (flushCacheFirst_in &&
      Net_WLAN_Tools::hasWiPhyFeature (features_in,
                                       NL80211_FEATURE_SCAN_FLUSH,
                                       MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_FLUSH;
//  NL80211_SCAN_FLAG_AP
//  // *NOTE*: check features first; set NL80211_ATTR_MAC[, NL80211_ATTR_MAC_MASK]
  if (randomizeMACAddress_in &&
      (Net_WLAN_Tools::hasWiPhyFeature (features_in,
                                        NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR,
                                        MAX_NL80211_EXT_FEATURES) ||
       Net_WLAN_Tools::hasWiPhyFeature (features_in,
                                        NL80211_FEATURE_ND_RANDOM_MAC_ADDR,
                                        MAX_NL80211_EXT_FEATURES)))
    scan_flags_i |= NL80211_SCAN_FLAG_RANDOM_ADDR;
  // *TODO*: find out what these are
// NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME
// NL80211_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION;
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_SCAN_FLAGS,
               scan_flags_i);
  if (scan_flags_i & NL80211_SCAN_FLAG_RANDOM_ADDR)
  {
    // *NOTE*: "...the NL80211_ATTR_MAC and NL80211_ATTR_MAC_MASK attributes may
    //         also be given in which case only the masked bits will be
    //         preserved from the MAC address and the remainder randomised. If
    //         the attributes are not given full randomisation (46 bits, locally
    //         administered 1, multicast 0) is assumed. ..."
//    struct ether_addr interface_mac_address_s;
//    ACE_OS::memset (&interface_mac_address_s, 0, sizeof (struct ether_addr));
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC_MASK,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
  } // end IF
  // *NOTE*: "...if passed, define which channels should be scanned; if not
  //          passed, all channels allowed for the current regulatory domain are
  //          used. ..."
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_SCAN_FREQUENCIES,
//               authentication_type_i);
//  // *NOTE*: do not send probe requests at 'Complementary Code Keying' rate
//  nla_put_flag (message_p,
//                NL80211_ATTR_TX_NO_CCK_RATE);
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_MEASUREMENT_DURATION,
//               0);
// NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY
//  NLA_PUT_FLAG (message_p,
//                NL80211_ATTR_SCHED_SCAN_MULTI);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
                            //  and FT IEs..."
           0,
           NULL);

  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 NULL,
                                                 NULL,
                                                 interfaceIdentifier_in,
                                                 message_p,
                                                 false)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_TRIGGER_SCAN));
    goto error;
  } // end IF
  ACE_ASSERT (!message_p);

  result = true;

error:
nla_put_failure:
  if (nested_attribute_p)
    nlmsg_free (nested_attribute_p);
  if (message_p)
    nlmsg_free (message_p);
#if defined (ACE_LINUX)
  if (handle_capabilities_b)
    if (!Common_OS_Tools::dropCapability (CAP_NET_ADMIN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::dropCapability(CAP_NET_ADMIN): \"%m\", continuing\n")));
#endif // ACE_LINUX

  return result;
}

struct ether_addr
Net_WLAN_Tools::associatedBSSID (const std::string& interfaceIdentifier_in,
                                 struct nl_sock* socketHandle_in,
                                 int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedBSSID"));

  struct Net_WLAN_nl80211_CBData cb_data_s;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return cb_data_s.address;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return cb_data_s.address;
  } // end IF
  // *NOTE*: "...When getting scan results without triggering scan first, you'll
  //         always get the information about currently associated BSS..."
  // *NOTE*: NL80211_CMD_GET_SCAN only supports dumping (i.e. NLM_F_DUMP)
  //         (returns -EOPNOTSUPP (95) otherwise)
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,         // port #
                              NL_AUTO_SEQ,          // sequence #
                              driverFamilyId_in,    // family id
                              0,                    // (user-) hdrlen
                              NLM_F_REQUEST,        // flags
                              NL80211_CMD_GET_SCAN, // command id
                              0)))                  // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           &(ap_mac_address_s.ether_addr_octet));

  cb_data_s.dumping = true;
  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_bssid_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_SCAN));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);
  ACE_ASSERT (!Net_Common_Tools::isAny (cb_data_s.address));

clean:
//nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return cb_data_s.address;
}

std::string
Net_WLAN_Tools::associatedSSID (const std::string& interfaceIdentifier_in,
                                struct nl_sock* socketHandle_in,
                                int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  Net_WLAN_SSIDs_t SSIDs_a;
  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.SSIDs = &SSIDs_a;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF
  // *NOTE*: "...When getting scan results without triggering scan first, you'll
  //         always get the information about currently associated BSS..."
  // *NOTE*: NL80211_CMD_GET_SCAN only supports dumping (i.e. NLM_F_DUMP)
  //         (returns -EOPNOTSUPP (95) otherwise)
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,               // port #
                              NL_AUTO_SEQ,                // sequence #
                              driverFamilyId_in,          // family id
                              0,                          // (user-) hdrlen
//                              NLM_F_REQUEST | NLM_F_DUMP, // flags
                              NLM_F_DUMP,                 // flags
                              NL80211_CMD_GET_SCAN,       // command id
                              0)))                        // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
//  NLA_PUT (message_p,
//           NL80211_ATTR_MAC,
//           ETH_ALEN,
//           ap_mac_address_s.ether_addr_octet);

  cb_data_s.dumping = true;
  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_ssid_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_SCAN));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

clean:
//nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return (SSIDs_a.empty () ? ACE_TEXT_ALWAYS_CHAR ("")
                           : SSIDs_a.front ());
}

Net_WLAN_SSIDs_t
Net_WLAN_Tools::getSSIDs (const std::string& interfaceIdentifier_in,
                          struct nl_sock* socketHandle_in,
                          int driverFamilyId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getSSIDs"));

  // initialize return value(s)
  Net_WLAN_SSIDs_t return_value;

  struct Net_WLAN_nl80211_CBData cb_data_s;
  cb_data_s.SSIDs = &return_value;
  struct nl_msg* message_p = NULL;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()             ||
                (interfaceIdentifier_in.size () > IFNAMSIZ) ||
                !driverFamilyId_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return return_value;
  } // end IF

  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  // *NOTE*: NL80211_CMD_GET_SCAN only supports dumping (i.e. NLM_F_DUMP)
  //         (returns -EOPNOTSUPP (95) otherwise)
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,               // port #
                              NL_AUTO_SEQ,                // sequence #
                              driverFamilyId_in,          // family id
                              0,                          // (user-) hdrlen
                              NLM_F_REQUEST | NLM_F_DUMP, // flags
                              NL80211_CMD_GET_SCAN,       // command id
                              0)))                        // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put(): \"%m\", aborting\n")));
    goto clean;
  } // end IF

  cb_data_s.dumping = true;
  if (unlikely (!Net_WLAN_Tools::nL80211Command (socketHandle_in,
                                                 network_wlan_nl80211_ssids_cb,
                                                 &cb_data_s,
                                                 interfaceIdentifier_in,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::nL80211Command(%d), aborting\n"),
                NL80211_CMD_GET_SCAN));
    goto clean;
  } // end IF
  ACE_ASSERT (!message_p);

clean:
  if (message_p)
    nlmsg_free (message_p);

  return return_value;
}

bool
Net_WLAN_Tools::nL80211Command (struct nl_sock* socketHandle_in,
                                nl_recvmsg_msg_cb_t messageCB_in,
                                struct Net_WLAN_nl80211_CBData* CBData_inout,
                                const std::string& interfaceIdentifier_in,
                                struct nl_msg*& message_inout,
                                bool dispatchReplies_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::nL80211Command"));

  bool result = false;

  struct nl_sock* socket_handle_p = NULL;
  bool release_handle_b = false;
  int result_2 = -1;
  struct nl_cb* callback_p = NULL;
  struct nl_cb* callback_2 = NULL;
  int error_i = 0, done_i = 0;
  int* error_p = &error_i, *done_p = &done_i;
  bool release_error_b = false, release_done_b = false;

  // sanity check(s)
  socket_handle_p = socketHandle_in;
  if (!socket_handle_p)
  {
    socket_handle_p = nl_socket_alloc ();
    if (unlikely (!socket_handle_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_socket_alloc(): \"%m\", aborting\n")));
      return false;
    } // end IF
    release_handle_b = true;
    //  nl_socket_set_nonblocking (socketHandle_);
    //  nl_socket_set_buffer_size (socketHandle_, int rx, int tx);

    result_2 = nl_connect (socket_handle_p, NETLINK_GENERIC);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_connect(%@): \"%s\", aborting\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF
    nl_socket_disable_auto_ack (socket_handle_p);
    nl_socket_disable_msg_peek (socket_handle_p);

    callback_p = nl_cb_alloc (NL_CB_DEFAULT);
    if (unlikely (!callback_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nl_cb_alloc(): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    result_2 = nl_cb_err (callback_p,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_error_cb,
                          CBData_inout);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_cb_err(%@,%d): \"%s\", aborting\n"),
                  socket_handle_p,
                  NL_CB_CUSTOM,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF
    result_2 = nl_cb_set (callback_p,
                          NL_CB_FINISH,
                          NL_CB_CUSTOM,
                          network_wlan_nl80211_finish_cb,
                          CBData_inout);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_cb_set(%@): \"%s\", aborting\n"),
                  callback_p,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF
    nl_socket_set_cb (socket_handle_p, callback_p);
    nl_cb_put (callback_p); callback_p = NULL;
  } // end IF
  ACE_ASSERT (socket_handle_p);

  // retain callbacks
  callback_p = nl_socket_get_cb (socket_handle_p);
  if (unlikely (!callback_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_socket_get_cb(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  callback_2 = nl_cb_clone (callback_p);
  if (unlikely (!callback_2))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nl_cb_clone(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  if (likely (messageCB_in))
  {
    result_2 = nl_cb_set (callback_2,
                          NL_CB_VALID,
                          NL_CB_CUSTOM,
                          messageCB_in,
                          CBData_inout);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_cb_set(%@): \"%s\", aborting\n"),
                  callback_2,
                  ACE_TEXT (nl_geterror (result_2))));
      goto clean;
    } // end IF
  } // end IF
//  nl_socket_set_cb (handle_in, callback_2);

  ACE_ASSERT (message_inout);
  if (likely (!interfaceIdentifier_in.empty ()))
  {
    // sanity check(s)
    ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

    unsigned int index_i =
        ::if_nametoindex (interfaceIdentifier_in.c_str ());
    if (unlikely (!index_i))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
      goto clean;
    } // end IF
    NLA_PUT_U32 (message_inout,
                 NL80211_ATTR_IFINDEX,
                 static_cast<ACE_UINT32> (index_i));

    if (CBData_inout)
      CBData_inout->index = index_i;
  } // end IF

  result_2 = nl_send_auto (socket_handle_p, message_inout);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(%@): \"%s\", aborting\n"),
                socket_handle_p,
                ACE_TEXT (nl_geterror (result_2))));
    goto clean;
  } // end IF
  nlmsg_free (message_inout); message_inout = NULL;

  if (!dispatchReplies_in)
    goto continue_;

  if (CBData_inout)
  {
    if (!CBData_inout->done)
    {
      CBData_inout->done = &done_i;
      release_done_b = true;
    } // end IF
    if (!CBData_inout->error)
    {
      CBData_inout->error = &error_i;
      release_error_b = true;
    } // end IF
  } // end IF
  ACE_ASSERT (error_p && done_p);

  // *NOTE*: "...Stops reading if one of the callbacks returns NL_STOP or
  //         nl_recv returns either 0 or a negative error code.
  //         A non-blocking sockets causes the function to return immediately if
  //         no data is available. ..."
  do
  { // *IMPORTANT NOTE*: there is no known way to cleanly unblock from a socket
    //                   read. Possible solutions:
    //                   - close()/shutdown() do not work
    //                   - SIGINT might work, ugly
    //                   - flag-setting and sending a 'dummy request', ugly
    //                   --> use ACE event dispatch and circumvent nl_recvmsgs()
    result_2 = nl_recvmsgs (socket_handle_p, callback_2);
    if (unlikely (result_2 < 0))
    {
      if (-result_2 == NLE_BAD_SOCK)           // 3 : shutdown
        break;
      else if ((-result_2 != NLE_BUSY)      && // 25
               (-result_2 != -NLE_DUMP_INTR))  // 33: state has changed, dump (see: NLM_F_DUMP) is inconsistent
                                               //     most probable reason: driver received new scan results
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\" (%d), returning\n"),
                    socket_handle_p,
                    ACE_TEXT (nl_geterror (result_2)), result_2));
        break;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", continuing\n"),
                  socket_handle_p,
                  ACE_TEXT (nl_geterror (result_2))));
    } // end IF
    if (unlikely (*error_p &&
                  (-(*error_p) != EAGAIN)))  // 11: socket is non-blocking, no data
//                  (-(*error_p) != ENOTSUP)))) // 95: operation not supported
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                  socket_handle_p,
                  ACE_TEXT (ACE_OS::strerror (-(*error_p)))));
      break;
    } // end IF
    if (*done_p || !result_2)
      break;
  } while (true);

continue_:
  result = true;

clean:
nla_put_failure:
  if (message_inout)
  {
    nlmsg_free (message_inout); message_inout = NULL;
  } // end IF
  if (callback_2)
    nl_cb_put (callback_2);
//  if (callback_p)
//    nl_socket_set_cb (handle_in, callback_p);
  if (release_handle_b)
  {
    nl_close (socket_handle_p);
    nl_socket_free (socket_handle_p);
  } // end IF
  if (release_done_b)
  { ACE_ASSERT (CBData_inout);
    CBData_inout->done = NULL;
  } // end IF
  if (release_error_b)
  { ACE_ASSERT (CBData_inout);
    CBData_inout->error = NULL;
  } // end IF

  return result;
}
