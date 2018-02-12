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
#include "net_common_tools.h"

#include <regex>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <cguid.h>
#include <guiddef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#else
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/ether.h>
#include "ifaddrs.h"
#include "iwlib.h"
#endif

#include "ace/Dirent_Selector.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

//////////////////////////////////////////

enum Net_LinkLayerType&
operator++ (enum Net_LinkLayerType& lhs_inout) // prefix-
{ ACE_ASSERT (lhs_inout < NET_LINKLAYER_MAX);
  int result = lhs_inout + 1;
  if (unlikely (result == NET_LINKLAYER_MAX))
    lhs_inout = NET_LINKLAYER_ATM;
  else
    lhs_inout = static_cast<enum Net_LinkLayerType> (result);

  return lhs_inout;
}
enum Net_LinkLayerType
operator++ (enum Net_LinkLayerType& lhs_in, int) // postfix-
{
  enum Net_LinkLayerType result = lhs_in;
  ++lhs_in;

  return result;
}

//////////////////////////////////////////

std::string
Net_Common_Tools::IPAddressToString (unsigned short port_in,
                                     ACE_UINT32 IPAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::IPAddressToString"));

  // initialize return value(s)
  std::string return_value;

  int result = -1;
  ACE_INET_Addr inet_addr;
  ACE_TCHAR buffer[32]; // "xxx.xxx.xxx.xxx:yyyyy\0"
  ACE_OS::memset (&buffer, 0, sizeof (buffer));
  result = inet_addr.set (port_in,
                          IPAddress_in,
                          0,  // no need to encode, data IS in network byte order !
                          0); // only needed for IPv6...
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  result = inet_addr.addr_to_string (buffer,
                                     sizeof (buffer),
                                     1); // want IP address, not hostname !
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Inet_Addr::addr_to_string(): \"%m\", aborting\n")));
    return return_value;
  } // end IF

  // copy string from buffer
  return_value = buffer;

  // clean up: if port number was 0, cut off the trailing ":0" !
  if (!port_in)
  {
    std::string::size_type last_colon_pos =
      return_value.find_last_of (':',
                                 std::string::npos); // begin searching at the end !
    if (likely (last_colon_pos != std::string::npos))
      return_value = return_value.substr (0, last_colon_pos);
  } // end IF

  return return_value;
}

bool
Net_Common_Tools::matchIPAddress (std::string& address_in)
{
  NETWORK_TRACE ("Net_Common_Tools::matchIPAddress");

  std::string regex_string =
    //ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{0,3})(\\.[[:digit:]]{0,3}){0,3}$");
    ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{0,3})(\\.[[:digit:]]{0,3})?(\\.[[:digit:]]{0,3})?(\\.[[:digit:]]{0,3})?$");
  std::regex::flag_type flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  //try {
    regex.assign (regex_string, flags);
  //} catch (std::regex_error exception_in) {
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
  //              ACE_TEXT (exception_in.what ())));
  //  goto refuse;
  //}
  if (unlikely (!std::regex_match (address_in,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
    return false;
  ACE_ASSERT (match_results.ready () && !match_results.empty ());

  // validate all groups
  std::stringstream converter;
  std::string group_string;
  unsigned int group = 0;
  std::string::size_type start_position;
  for (unsigned int i = 1; i < match_results.size (); ++i)
  {
    if (unlikely (!match_results[i].matched))
      return false;

    group_string = match_results[i].str ();
    if (likely (i > 1))
      group_string.erase (0, 1);
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << group_string;
    converter >> group;
    if (unlikely (group > 255))
      return false; // refuse groups > 255

    start_position = group_string.find ('0', 0);
    if (unlikely ((start_position == 0) &&
                  (group_string.size () > 1)))
      return false; // refuse leading 0s
  } // end FOR

  return true;
}

bool
Net_Common_Tools::isLocal (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE ("Net_Common_Tools::isLocal");

  // sanity check(s)
  if (unlikely (address_in.is_any () ||
                address_in.is_loopback ()))
    return true;

  // retrieve all assigned local addresses
  ACE_INET_Addr local_ip_address;
  //ACE_INET_Addr network_address;
  std::vector<ACE_INET_Addr> local_ip_addresses_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //ULONG flags = (GAA_FLAG_INCLUDE_PREFIX             |
  //               GAA_FLAG_INCLUDE_WINS_INFO          |
  //               GAA_FLAG_INCLUDE_GATEWAYS           |
  //               GAA_FLAG_INCLUDE_ALL_INTERFACES     |
  //               GAA_FLAG_INCLUDE_ALL_COMPARTMENTS   |
  //               GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER);
  //struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
  //ULONG buffer_length = 0;
  //ULONG result =
  //  GetAdaptersAddresses (AF_UNSPEC,              // Family
  //                        flags,                  // Flags
  //                        NULL,                   // Reserved
  //                        ip_adapter_addresses_p, // AdapterAddresses
  //                        &buffer_length);        // SizePointer
  //if (result != ERROR_BUFFER_OVERFLOW)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (buffer_length);
  //ip_adapter_addresses_p =
  //  static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
  //if (!ip_adapter_addresses_p)
  //{
  //  ACE_DEBUG ((LM_CRITICAL,
  //              ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  //  return false;
  //} // end IF
  //result =
  //  GetAdaptersAddresses (AF_UNSPEC,              // Family
  //                        flags,                  // Flags
  //                        NULL,                   // Reserved
  //                        ip_adapter_addresses_p, // AdapterAddresses
  //                        &buffer_length);        // SizePointer
  //if (result != NO_ERROR)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

  //  // clean up
  //  ACE_FREE_FUNC (ip_adapter_addresses_p);

  //  return false;
  //} // end IF

  //struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 =
  //  ip_adapter_addresses_p;
  //struct _IP_ADAPTER_UNICAST_ADDRESS_LH* unicast_address_p = NULL;
  //struct _IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway_address_p = NULL;
  //struct _SOCKET_ADDRESS* socket_address_p = NULL;
  //struct sockaddr_in* sockaddr_in_p = NULL;
  //do
  //{
  //  unicast_address_p = ip_adapter_addresses_2->FirstUnicastAddress;
  //  ACE_ASSERT (unicast_address_p);
  //  do
  //  {
  //    socket_address_p = &unicast_address_p->Address;
  //    ACE_ASSERT (socket_address_p->lpSockaddr);
  //    if (socket_address_p->lpSockaddr->sa_family != AF_INET)
  //      continue;

  //    sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
  //    netmask_mask = std::numeric_limits<ACE_UINT32>::max ();
  //    netmask_mask = netmask_mask << ((4 * 8) - unicast_address_p->OnLinkPrefixLength);
  //    netmask = sockaddr_in_p->sin_addr.S_un.S_addr & netmask_mask;
  //    adapter_ip_netmasks_a.push_back (netmask);

  //    result = network_address.set (static_cast<u_short> (0),
  //                                  netmask,
  //                                  1,
  //                                  0);
  //    if (result == -1)
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_INET_Addr::set(0,%u): \"%m\", aborting\n"),
  //                  netmask));

  //      // clean up
  //      ACE_FREE_FUNC (ip_adapter_addresses_p);

  //      return false;
  //    } // end IF
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("found ip address on network %s\n"),
  //                ACE_TEXT (Net_Common_Tools::IPAddressToString (network_address).c_str ())));

  //    unicast_address_p = unicast_address_p->Next;
  //  } while (unicast_address_p);
  //  ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
  //} while (ip_adapter_addresses_2);

  //// clean up
  //ACE_FREE_FUNC (ip_adapter_addresses_p);

  struct _MIB_IPADDRTABLE* table_p = NULL;
  DWORD table_size = 0;
  DWORD result = 0;
  //IN_ADDR ip_address;

  table_p =
    static_cast<struct _MIB_IPADDRTABLE*> (HeapAlloc (GetProcessHeap (),
                                                      0,
                                                      sizeof (struct _MIB_IPADDRTABLE)));
  if (unlikely (!table_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::HeapAlloc(%u): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
    return false; // *TODO*: this could lead to false negatives
  } // end IF
  // step1: determine size of the table
  result = GetIpAddrTable (table_p, &table_size, FALSE);
  if (unlikely (!table_p ||
                (result != ERROR_INSUFFICIENT_BUFFER)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetIpAddrTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));

    HeapFree (GetProcessHeap (), 0, table_p);

    return false; // *TODO*: this could lead to false negatives
  } // end IF
  HeapFree (GetProcessHeap (), 0, table_p);
  table_p = NULL;
  table_p =
    static_cast<struct _MIB_IPADDRTABLE*> (HeapAlloc (GetProcessHeap (),
                                                      0,
                                                      table_size));
  if (unlikely (!table_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::HeapAlloc(%u): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
    return false; // *TODO*: this could lead to false negatives
  } // end IF
  // step2: get the actual table data
  result = GetIpAddrTable (table_p, &table_size, FALSE);
  if (unlikely (result != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetIpAddrTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    HeapFree (GetProcessHeap (), 0, table_p);

    return false; // *TODO*: this could lead to false negatives
  } // end IF

  for (DWORD i = 0;
       i < table_p->dwNumEntries;
       ++i)
  {
    //printf("\tType and State[%d]:", i);
    //if (pIPAddrTable->table[i].wType & MIB_IPADDR_PRIMARY)
    //    printf("\tPrimary IP Address");
    //if (pIPAddrTable->table[i].wType & MIB_IPADDR_DYNAMIC)
    //    printf("\tDynamic IP Address");
    //if (pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)
    //    printf("\tAddress is on disconnected interface");
    //if (pIPAddrTable->table[i].wType & MIB_IPADDR_DELETED)
    //    printf("\tAddress is being deleted");
    //if (pIPAddrTable->table[i].wType & MIB_IPADDR_TRANSIENT)
    //    printf("\tTransient address");

    result =
      local_ip_address.set (static_cast<u_short> (0),
                            table_p->table[i].dwAddr,
                            0, // already in network byte order
                            0);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(0,%u): \"%m\", aborting\n"),
                  table_p->table[i].dwAddr));

      // clean up
      HeapFree (GetProcessHeap (), 0, table_p);

      return false;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found ip address %s on network %s\n"),
    //            ACE_TEXT (ACE_OS::inet_ntoa (ip_address)),
    //            ACE_TEXT (Net_Common_Tools::IPAddressToString (network_address, true).c_str ())));

    local_ip_addresses_a.push_back (local_ip_address);
  } // end FOR

  HeapFree (GetProcessHeap (), 0, table_p);
#else
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  struct in_addr in_addr_s;
  int result = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  struct sockaddr_in* sockaddr_in_p, *sockaddr_in_2 = NULL;
  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
    if (unlikely (!ifaddrs_2->ifa_addr))
      continue;
    if (unlikely (ifaddrs_2->ifa_addr->sa_family != AF_INET))
      continue;

    sockaddr_in_p = reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_addr);
    sockaddr_in_2 =
        reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_netmask);
    in_addr_s = sockaddr_in_p->sin_addr;
    in_addr_s.s_addr &= sockaddr_in_2->sin_addr.s_addr;
    sockaddr_in_2 =
        reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_netmask);
    result =
      local_ip_address.set (static_cast<u_short> (0),
                            sockaddr_in_p->sin_addr.s_addr,
                            0, // already in network byte order
                            0);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(0,%u): \"%m\", aborting\n"),
                  sockaddr_in_p->sin_addr.s_addr));
      return false;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found ip address %s on network %s\n"),
    //            ACE_TEXT (ACE_OS::inet_ntoa (in_addr_s)),
    //            ACE_TEXT (Net_Common_Tools::IPAddressToString (local_ip_address, true).c_str ())));

    local_ip_addresses_a.push_back (local_ip_address);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

  for (std::vector<ACE_INET_Addr>::const_iterator iterator = local_ip_addresses_a.begin ();
       iterator != local_ip_addresses_a.end ();
       ++iterator)
    if (address_in.get_ip_address () == (*iterator).get_ip_address ())
      return true;

  return false;
}

ACE_INET_Addr
Net_Common_Tools::stringToIPAddress (std::string& address_in)
{
  NETWORK_TRACE ("Net_Common_Tools::stringToIPAddress");

  // sanity check(s)
  // *NOTE*: ACE_INET_Addr::string_to_address() needs a trailing port number to
  //         function properly (see: ace/INET_Addr.h:237)
  //         --> append one if necessary
  std::string ip_address_string = address_in;
  std::string::size_type position = ip_address_string.find (':', 0);
  if (likely (position == std::string::npos))
    ip_address_string += ACE_TEXT_ALWAYS_CHAR (":0");

  int result = -1;
  ACE_INET_Addr inet_address;
  result = inet_address.string_to_addr (ip_address_string.c_str (),
                                        AF_INET);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(): \"%m\", aborting\n")));
    return ACE_INET_Addr ();
  } // end IF

  return inet_address;
}

std::string
Net_Common_Tools::IPProtocolToString (unsigned char protocol_in)
{
  NETWORK_TRACE ("Net_Common_Tools::IPProtocolToString");

  switch (protocol_in)
  {
    case IPPROTO_IP: // OR case IPPROTO_HOPOPTS:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IP/IPPROTO_HOPOPTS");
    case IPPROTO_ICMP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ICMP");
    case IPPROTO_IGMP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IGMP");
    case IPPROTO_IPIP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IPIP");
    case IPPROTO_TCP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_TCP");
    case IPPROTO_EGP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_EGP");
    case IPPROTO_PUP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_PUP");
    case IPPROTO_UDP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_UDP");
    case IPPROTO_IDP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IDP");
    case IPPROTO_TP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_TP");
    case IPPROTO_IPV6:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IPV6");
    case IPPROTO_ROUTING:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ROUTING");
    case IPPROTO_FRAGMENT:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_FRAGMENT");
    case IPPROTO_RSVP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_RSVP");
    case IPPROTO_GRE:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_GRE");
    case IPPROTO_ESP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ESP");
    case IPPROTO_AH:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_AH");
    case IPPROTO_ICMPV6:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ICMPV6");
    case IPPROTO_NONE:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_NONE");
    case IPPROTO_DSTOPTS:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_DSTOPTS");
    case IPPROTO_MTP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_MTP");
    case IPPROTO_ENCAP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ENCAP");
    case IPPROTO_PIM:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_PIM");
    case IPPROTO_COMP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_COMP");
    case IPPROTO_SCTP:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_SCTP");
    case IPPROTO_RAW:
      return ACE_TEXT_ALWAYS_CHAR ("IPPROTO_RAW");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown IP protocol (was: %1u), aborting\n"),
                  protocol_in));
      break;
    }
  } // end SWITCH

  return ACE_TEXT_ALWAYS_CHAR ("");
}

std::string
Net_Common_Tools::TransportLayerTypeToString (enum Net_TransportLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::TransportLayerTypeToString"));

  switch (type_in)
  {
    case NET_TRANSPORTLAYER_IP_CAST:
      return ACE_TEXT_ALWAYS_CHAR ("IP");
#if defined (ACE_LINUX)
    case NET_TRANSPORTLAYER_NETLINK:
      return ACE_TEXT_ALWAYS_CHAR ("Netlink");
#endif
    case NET_TRANSPORTLAYER_TCP:
      return ACE_TEXT_ALWAYS_CHAR ("TCP");
    case NET_TRANSPORTLAYER_UDP:
      return ACE_TEXT_ALWAYS_CHAR ("UDP");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

  return ACE_TEXT_ALWAYS_CHAR ("");
}

std::string
Net_Common_Tools::LinkLayerAddressToString (const unsigned char* const addressDataPtr_in,
                                            enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::LinkLayerAddressToString"));

  // initialize return value(s)
  std::string return_value;

  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
    case NET_LINKLAYER_802_11:
    {
      ACE_TCHAR buffer[NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE];
      ACE_OS::memset (&buffer, 0, sizeof (buffer));

      // *PORTABILITY*: ether_ntoa_r is not portable
      // *TODO*: implement an ACE wrapper function for unsupported platforms
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (ACE_USES_WCHAR)
      PWSTR result_2 =
        RtlEthernetAddressToStringW (reinterpret_cast<const _DL_EUI48* const> (addressDataPtr_in),
                                     buffer);
#else
      PSTR result_2 =
        RtlEthernetAddressToStringA (reinterpret_cast<const _DL_EUI48* const> (addressDataPtr_in),
                                     buffer);
#endif
      if (unlikely (result_2 != (buffer + NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE - 1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::RtlEthernetAddressToString(), aborting\n")));
        break;
      } // end IF
#else
      const struct ether_addr* const ether_addr_p =
          reinterpret_cast<const struct ether_addr* const> (addressDataPtr_in);
      if (unlikely (::ether_ntoa_r (ether_addr_p,
                                    buffer) != buffer))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::ether_ntoa_r(): \"%m\", aborting\n")));
        break;
      } // end IF
#endif
      return_value = ACE_TEXT_ALWAYS_CHAR (buffer);

      break;
    }
    case NET_LINKLAYER_PPP:
    {
      // *NOTE*: being point-to-point in nature, PPP does not support (link
      //         layer-) addressing
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("link layer type \"%s\" does not support addressing, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerTypeToString (type_in).c_str ())));
      break;
    }
    case NET_LINKLAYER_ATM:
    case NET_LINKLAYER_FDDI:
    { // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR (""));

      ACE_NOTREACHED (break;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown link layer type (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerTypeToString (type_in).c_str ())));
      break;
    }
  } // end SWITCH

  return return_value;
}
std::string
Net_Common_Tools::LinkLayerTypeToString (enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::LinkLayerTypeToString"));

  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
      return ACE_TEXT_ALWAYS_CHAR ("IEEE 802.3");
    case NET_LINKLAYER_802_11:
      return ACE_TEXT_ALWAYS_CHAR ("IEEE 802.11");
    case NET_LINKLAYER_ATM:
      return ACE_TEXT_ALWAYS_CHAR ("ATM");
    case NET_LINKLAYER_FDDI:
      return ACE_TEXT_ALWAYS_CHAR ("FDDI");
    case NET_LINKLAYER_PPP:
      return ACE_TEXT_ALWAYS_CHAR ("PPP");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown link layer type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

  return ACE_TEXT_ALWAYS_CHAR ("");
}

std::string
Net_Common_Tools::EthernetProtocolTypeIdToString (unsigned short frameType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::EthernetProtocolTypeIdToString"));

  switch (ACE_NTOHS (frameType_in))
  {
    case ETH_P_LOOP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_LOOP");
    case ETHERTYPE_GRE_ISO:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_GRE_ISO");
    case ETH_P_PUP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PUP");
    case ETH_P_PUPAT:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PUPAT");
    case ETHERTYPE_SPRITE:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_SPRITE");
    case ETH_P_IP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IP");
    case ETH_P_X25:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_X25");
    case ETH_P_ARP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ARP");
    case ETH_P_BPQ:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_BPQ");
    case ETH_P_IEEEPUP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IEEEPUP");
    case ETH_P_IEEEPUPAT:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IEEEPUPAT");
    case ETHERTYPE_NS:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_NS");
    case ETHERTYPE_TRAIL:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_TRAIL");
    case ETH_P_DEC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DEC");
    case ETH_P_DNA_DL:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_DL");
    case ETH_P_DNA_RC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_RC");
    case ETH_P_DNA_RT:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_RT");
    case ETH_P_LAT:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_LAT");
    case ETH_P_DIAG:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DIAG");
    case ETH_P_CUST:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_CUST");
    case ETH_P_SCA:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_SCA");
    case ETH_P_RARP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_RARP");
    case ETHERTYPE_LANBRIDGE:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_LANBRIDGE");
    case ETHERTYPE_DECDNS:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_DECDNS");
    case ETHERTYPE_DECDTS:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_DECDTS");
    case ETHERTYPE_VEXP:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VEXP");
    case ETHERTYPE_VPROD:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VPROD");
    case ETH_P_ATALK:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATALK");
    case ETH_P_AARP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_AARP");
    case ETH_P_8021Q:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_8021Q");
    case ETH_P_IPX:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IPX");
    case ETH_P_IPV6:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IPV6");
    case ETHERTYPE_MPCP:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_MPCP");
    case ETHERTYPE_SLOW:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_SLOW");
    case ETHERTYPE_PPP:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_PPP");
    case ETH_P_WCCP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_WCCP");
    case ETH_P_MPLS_UC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_MPLS_UC");
    case ETH_P_MPLS_MC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_MPLS_MC");
    case ETH_P_ATMMPOA:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATMMPOA");
    case ETH_P_PPP_DISC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_DISC");
    case ETH_P_PPP_SES:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_SES");
    case ETHERTYPE_JUMBO:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_JUMBO");
    case ETH_P_ATMFATE:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATMFATE");
    case ETHERTYPE_EAPOL:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_EAPOL");
    case ETH_P_AOE:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_AOE");
    case ETH_P_TIPC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_TIPC");
    case ETHERTYPE_LOOPBACK:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_LOOPBACK");
    case ETHERTYPE_VMAN:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VMAN");
    case ETHERTYPE_ISO:
      return ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_ISO");
// ********************* non-DIX types ***********************
    case ETH_P_802_3:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_802_3");
    case ETH_P_AX25:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_AX25");
    case ETH_P_ALL:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ALL");
    case ETH_P_802_2:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_802_2");
    case ETH_P_SNAP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_SNAP");
    case ETH_P_DDCMP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_DDCMP");
    case ETH_P_WAN_PPP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_WAN_PPP");
    case ETH_P_PPP_MP:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_MP");
    case ETH_P_LOCALTALK:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_LOCALTALK");
    case ETH_P_PPPTALK:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPPTALK");
    case ETH_P_TR_802_2:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_TR_802_2");
    case ETH_P_MOBITEX:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_MOBITEX");
    case ETH_P_CONTROL:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_CONTROL");
    case ETH_P_IRDA:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_IRDA");
    case ETH_P_ECONET:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ECONET");
    case ETH_P_HDLC:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_HDLC");
    case ETH_P_ARCNET:
      return ACE_TEXT_ALWAYS_CHAR ("ETH_P_ARCNET");
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown ethernet frame type (was: 0x%x), continuing\n"),
                  ACE_NTOHS (frameType_in)));

      // IEEE 802.3 ? --> adjust result string
      if (likely (ACE_NTOHS (frameType_in) <= ETH_DATA_LEN))
        return ACE_TEXT_ALWAYS_CHAR ("UNKNOWN_IEEE_802_3_FRAME_TYPE");

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ethernet frame type (was: 0x%x), aborting\n"),
                  ACE_NTOHS (frameType_in)));

      break;
    }
  } // end SWITCH

  return ACE_TEXT_ALWAYS_CHAR ("");
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_Common_Tools::interfaceToExternalIPAddress (REFGUID interfaceIdentifier_in,
#else
Net_Common_Tools::interfaceToExternalIPAddress (const std::string& interfaceIdentifier_in,
#endif
                                                ACE_INET_Addr& IPAddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToExternalIPAddress"));

  // initialize return value(s)
  IPAddress_out.reset ();

  // *TODO*: this implementation is broken; it does not consider the specific
  //         interface, but returns the external IP of the interface that
  //         happens to route the DNS resolution query (see below)

  // debug info
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (interface_identifier.empty ())
#endif
    interface_identifier = Net_Common_Tools::getDefaultInterface ();

  // step1: determine the 'internal' IP address
  ACE_INET_Addr internal_ip_address, gateway_ip_address;
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                         internal_ip_address,
                                                         gateway_ip_address)))

  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif
    return false;
  } // end IF

  int result = -1;
  // *TODO*: this should work on most Linux/Windows systems, but is really a bad
  //         idea, as it relies on:
  //         - local 'nslookup[.exe]' tool
  //         - the 'opendns.com' domain resolution service
  //         - local temporary files
  //         - system(3) call
  //         and is totally inefficient
  //         --> remove ASAP
  std::string filename_string =
      Common_File_Tools::getTempFilename (ACE_TEXT_ALWAYS_CHAR (""));
  std::string command_line_string =
      ACE_TEXT_ALWAYS_CHAR ("nslookup myip.opendns.com. resolver1.opendns.com >> ");
  command_line_string += filename_string;

  result = ACE_OS::system (ACE_TEXT (command_line_string.c_str ()));
//  result = execl ("/bin/sh", "sh", "-c", command, (char *) 0);
  if (unlikely ((result == -1)      ||
                !WIFEXITED (result) ||
                WEXITSTATUS (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::system(\"%s\"): \"%m\" (result was: %d), aborting\n"),
                ACE_TEXT (command_line_string.c_str ()),
                WEXITSTATUS (result)));
    return false;
  } // end IF
  unsigned char* data_p = NULL;
  if (unlikely (!Common_File_Tools::load (filename_string,
                                          data_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (filename_string.c_str ())));
    return false;
  } // end IF
  if (unlikely (!Common_File_Tools::deleteFile (filename_string)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::deleteFile(\"%s\"), continuing\n"),
                ACE_TEXT (filename_string.c_str ())));

  std::string resolution_record_string = reinterpret_cast<char*> (data_p);
  delete [] data_p;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("nslookup data: \"%s\"\n"),
//              ACE_TEXT (resolution_record_string.c_str ())));

  std::string external_ip_address;
  std::istringstream converter;
  char buffer [BUFSIZ];
  std::string regex_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_TEXT_ALWAYS_CHAR ("^([^:]+)(?::[[:blank:]]*)(.+)(?:\r)$");
#else
    ACE_TEXT_ALWAYS_CHAR ("^([^:]+)(?::[[:blank:]]*)(.+)$");
#endif
  std::regex regex (regex_string);
  std::smatch match_results;
  converter.str (resolution_record_string);
  bool is_first = true;
  std::string buffer_string;
  do
  {
    converter.getline (buffer, sizeof (buffer));
    buffer_string = buffer;
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
      continue;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());

    if (match_results[1].matched &&
        !ACE_OS::strcmp (match_results[1].str ().c_str (),
                         ACE_TEXT_ALWAYS_CHAR (NET_ADDRESS_NSLOOKUP_RESULT_ADDRESS_KEY_STRING)))
    {
      if (is_first)
      {
        is_first = false;
        continue;
      } // end IF

      ACE_ASSERT (match_results[2].matched);
      external_ip_address = match_results[2];
      break;
    } // end IF
  } while (!converter.fail ());
  if (unlikely (external_ip_address.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to resolve IP address (was: %s), aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (internal_ip_address).c_str ())));
    return false;
  } // end IF
  IPAddress_out = Net_Common_Tools::stringToIPAddress (external_ip_address);

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("interface \"%s\" --> %s (--> %s)\n"),
  //            ACE_TEXT (interfaceIdentifier_in.c_str ()),
  //            ACE_TEXT (Net_Common_Tools::IPAddressToString (internal_ip_address).c_str ()),
  //            ACE_TEXT (Net_Common_Tools::IPAddressToString (IPAddress_out).c_str ())));

  return true;
}

struct ether_addr
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_Common_Tools::interfaceToLinkLayerAddress (REFGUID interfaceIdentifier_in)
#else
Net_Common_Tools::interfaceToLinkLayerAddress (const std::string& interfaceIdentifier_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToLinkLayerAddress"));

  // initialize return value(s)
  struct ether_addr result;
  ACE_OS::memset (&result, 0, sizeof (struct ether_addr));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
  //ACE_ASSERT (sizeof (MACAddress_out) >= MAX_ADAPTER_ADDRESS_LENGTH);

  NET_IFINDEX interface_index = 0;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 = 0;

  interface_index =
    Net_Common_Tools::interfaceToIndex (interfaceIdentifier_in);
  if (unlikely (!interface_index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
    return result;
  } // end IF

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
                              &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_interfaceIdentifier_info_p =
    static_cast<struct _IP_ADAPTER_INFO*> (ACE_MALLOC_FUNC (buffer_length));
  if (unlikely (!ip_interfaceIdentifier_info_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return result;
  } // end IF
  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
                              &buffer_length);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

    return result;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    ACE_OS::memcpy (&result, ip_interfaceIdentifier_info_2->Address,
                    ETH_ALEN);
    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

  // clean up
  ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);
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

  struct sockaddr_ll* sockaddr_ll_p = NULL;
  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
    if (ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                        ifaddrs_2->ifa_name))
      continue;

#if defined (ACE_LINUX)
    if (ifaddrs_2->ifa_addr->sa_family != AF_PACKET)
#else
    // *TODO*: this may work on BSD and APPLE systems
    if (ifaddrs_2->ifa_addr->sa_family != AF_LINK)
#endif
      continue;

    sockaddr_ll_p =
        reinterpret_cast<struct sockaddr_ll*> (ifaddrs_2->ifa_addr);
    ACE_OS::memcpy (&result, sockaddr_ll_p->sll_addr,
                    ETH_ALEN);
    break;
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::string
Net_Common_Tools::interfaceToString (REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToString"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  if  (unlikely (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF

  NET_IFINDEX interface_index = 0;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 = 0;

  interface_index = Net_Common_Tools::interfaceToIndex (interfaceIdentifier_in);
  if (unlikely (!interface_index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_interfaceIdentifier_info_p =
    static_cast<struct _IP_ADAPTER_INFO*> (ACE_MALLOC_FUNC (buffer_length));
  if (unlikely (!ip_interfaceIdentifier_info_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    goto error;
  } // end IF
  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
                              &buffer_length);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    result = ACE_TEXT_ALWAYS_CHAR (ip_interfaceIdentifier_info_2->Description);

    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

error:
  // clean up
  ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

  return result;
}

ULONG
Net_Common_Tools::interfaceToIndex (REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToIndex"));

  // initialize return value(s)
  NET_IFINDEX result = 0;

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

  DWORD result_2 = 0;
  union _NET_LUID_LH interface_luid_u;
  interface_luid_u.Value = 0;
  NET_IFINDEX interface_index = 0;
  result_2 = ConvertInterfaceGuidToLuid (&interfaceIdentifier_in,
                                         &interface_luid_u);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::ConvertInterfaceGuidToLuid(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  result_2 = ConvertInterfaceLuidToIndex (&interface_luid_u,
                                          &result);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::ConvertInterfaceLuidToIndex(%q): \"%s\", aborting\n"),
                interface_luid_u.Value,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF

  return result;
}
struct _GUID
Net_Common_Tools::indexToInterface (ULONG interfaceIndex_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::indexToInterface"));

  // initialize return value(s)
  struct _GUID result = GUID_NULL;

  // sanity check(s)
  ACE_ASSERT (interfaceIndex_in);

  union _NET_LUID_LH interface_luid_u;
  interface_luid_u.Value = 0;
  DWORD result_2 = ConvertInterfaceIndexToLuid (interfaceIndex_in,
                                                &interface_luid_u);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ConvertInterfaceIndexToLuid(%u), aborting\n"),
                interfaceIndex_in,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return GUID_NULL;
  } // end IF
  result_2 = ConvertInterfaceLuidToGuid (&interface_luid_u,
                                         &result);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ConvertInterfaceLuidToGuid(%q), aborting\n"),
                interface_luid_u.Value,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return GUID_NULL;
  } // end IF

  return result;
}
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
ACE_INET_Addr
Net_Common_Tools::getGateway (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getGateway"));

  ACE_INET_Addr result;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  // *IMPORTANT NOTE*: the Linux kernel does not maintain specific network
  //                   topology information other than its routing tables.
  //                   The routing tables are initialized and manipulated from
  //                   user space libraries and programs (i.e. glibc, dhclient,
  //                   etc.) using the Netlink protocol.
  //                   --> retrieve/parse the routing table entries to extract
  //                       the gateway address(es)
  // *TODO*: encapsulate the sockets/networking code with ACE

  int socket_handle = ACE_OS::socket (PF_NETLINK,     // protocol family
                                      SOCK_DGRAM,     // type
                                      NETLINK_ROUTE); // protocol
  if (unlikely (socket_handle < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(PF_NETLINK,NETLINK_ROUTE): \"%m\", aborting\n")));
    return result;
  } // end IF

  char buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct nlmsghdr* nl_message_header_p =
      reinterpret_cast<struct nlmsghdr*> (buffer);
  struct rtmsg* rt_message_p =
      static_cast<struct rtmsg*> (NLMSG_DATA (nl_message_header_p));
  struct rtattr* rt_attribute_p = NULL;
  nl_message_header_p->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
  nl_message_header_p->nlmsg_type = RTM_GETROUTE;
  nl_message_header_p->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
  int message_sequence_i = 0;
  pid_t pid_i = ACE_OS::getpid ();
  nl_message_header_p->nlmsg_seq = message_sequence_i++;
  nl_message_header_p->nlmsg_pid = pid_i;

  unsigned int received_bytes = 0;
  char* buffer_p = buffer;
  int length_i = 0;
  bool done, skip = false;
  ACE_INET_Addr current_interface_address;
  std::string current_interface_string;

  int flags = 0;
  // send request
  int result_2 = ACE_OS::send (socket_handle,
                               reinterpret_cast<char*> (nl_message_header_p),
                               nl_message_header_p->nlmsg_len,
                               flags);
  if (unlikely ((result_2 < 0) ||
                (static_cast<__u32> (result_2) != nl_message_header_p->nlmsg_len)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::send(%d,%u): \"%m\", aborting\n"),
                socket_handle,
                nl_message_header_p->nlmsg_len));
    goto clean;
  } // end IF

  ACE_OS::memset (buffer, 0, sizeof (buffer));
  // receive reply/ies
  do
  { ACE_ASSERT (received_bytes < BUFSIZ);
    result_2 = ACE_OS::recv (socket_handle,
                             buffer_p,
                             BUFSIZ - received_bytes);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::recv(%d): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
    nl_message_header_p = reinterpret_cast<struct nlmsghdr*> (buffer_p);

    if (unlikely (!NLMSG_OK (nl_message_header_p, result_2) ||
                  (nl_message_header_p->nlmsg_type == NLMSG_ERROR)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("error in Netlink message, aborting\n")));
      goto clean;
    } // end IF
    if (nl_message_header_p->nlmsg_type == NLMSG_DONE)
      break;

    buffer_p += result_2;
    received_bytes += result_2;

    if ((nl_message_header_p->nlmsg_flags & NLM_F_MULTI) == 0)
      break;
  } while ((nl_message_header_p->nlmsg_seq != static_cast<__u32> (message_sequence_i)) ||
           (nl_message_header_p->nlmsg_pid != static_cast<__u32> (pid_i)));

  // parse routing tables entries
  done = false;
  for (nl_message_header_p = reinterpret_cast<struct nlmsghdr*> (buffer);
       (NLMSG_OK (nl_message_header_p, received_bytes) && !done);
       nl_message_header_p = NLMSG_NEXT (nl_message_header_p, received_bytes))
  {
    current_interface_address.reset ();
    current_interface_string.resize (0);

    rt_message_p =
        static_cast<struct rtmsg*> (NLMSG_DATA (nl_message_header_p));
    ACE_ASSERT (rt_message_p);

    // inspect only AF_INET-specific rules from the main table
    if ((rt_message_p->rtm_family != AF_INET) ||
        (rt_message_p->rtm_table != RT_TABLE_MAIN))
      continue;

    // *NOTE*: the attributes don't seem to appear in any specfic order; retain
    //         address information until the device has been identified, and
    //         vice versa
    rt_attribute_p = static_cast<struct rtattr*> (RTM_RTA (rt_message_p));
    ACE_ASSERT (rt_attribute_p);
    length_i = RTM_PAYLOAD (nl_message_header_p);
    skip = false;
    for (;
         RTA_OK (rt_attribute_p, length_i) && !skip;
         rt_attribute_p = RTA_NEXT (rt_attribute_p, length_i))
    {
      switch (rt_attribute_p->rta_type)
      {
        case RTA_IIF:
        case RTA_OIF:
        {
          char buffer_2[IF_NAMESIZE];
          ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
          if (unlikely (!::if_indextoname (*static_cast<int*> (RTA_DATA (rt_attribute_p)),
                                           buffer_2)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to if_indextoname(): \"%m\", continuing\n")));
            break;
          } // end IF
          current_interface_string = buffer_2;
          if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                               buffer_2))
          {
            if (!current_interface_address.is_any ())
            {
              result = current_interface_address;
              skip = true;
              done = true; // done
            } // end IF
          } // end IF
          else
            skip = true; // rule applies to a different interface --> skip ahead
          break;
        }
        case RTA_DST:
        {
          in_addr_t inet_address = 0;
          ACE_OS::memcpy (&inet_address,
                          RTA_DATA (rt_attribute_p),
                          sizeof (in_addr_t));
          ACE_INET_Addr inet_address_2;
          result_2 = inet_address_2.set (static_cast<u_short> (0),
                                         inet_address,
                                         0, // already in network byte order
                                         0);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
            break;
          } // end IF
          if (!inet_address_2.is_any ())
            skip = true; // apparently the gateway address is only set on rules
                         // with destination 0.0.0.0
          break;
        }
        case RTA_GATEWAY:
        {
          in_addr_t inet_address = 0;
          ACE_OS::memcpy (&inet_address,
                          RTA_DATA (rt_attribute_p),
                          sizeof (in_addr_t));
          result_2 =
              current_interface_address.set (static_cast<u_short> (0),
                                             inet_address,
                                             0, // already in network byte order
                                             0);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
            break;
          } // end IF

          if (!ACE_OS::strcmp (current_interface_string.c_str (),
                               interfaceIdentifier_in.c_str ()))
          {
            result = current_interface_address;
            skip = true; // skip
            done = true; // done
          } // end IF
          break;
        }
        default:
        {
//          ACE_DEBUG ((LM_DEBUG,
//                      ACE_TEXT ("found attribute (type was: %d)\n"),
//                      rt_attribute_p->rta_type));
          break;
        }
      } // end SWITCH
    } // end FOR
  } // end FOR

clean:
  if (socket_handle != ACE_INVALID_HANDLE)
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  socket_handle));
  } // end IF

  return result;
}
#endif

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Net_Common_Tools::interfaceToIPAddress (REFGUID interfaceIdentifier_in,
#else
Net_Common_Tools::interfaceToIPAddress (const std::string& interfaceIdentifier_in,
#endif
                                        ACE_INET_Addr& IPAddress_out,
                                        ACE_INET_Addr& gatewayIPAddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToIPAddress"));

  // initialize return value(s)
  IPAddress_out.reset ();
  gatewayIPAddress_out.reset ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
    interface_identifier = Net_Common_Tools::getDefaultInterface ();
#else
  std::string interface_identifier_string = interfaceIdentifier_in;
  if (unlikely (interface_identifier_string.empty ()))
    interface_identifier_string = Net_Common_Tools::getDefaultInterface ();
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interface_identifier, GUID_NULL));

  NET_IFINDEX interface_index =
    Net_Common_Tools::interfaceToIndex (interface_identifier);
  if (unlikely (!interface_index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interface_identifier).c_str ())));
    return false;
  } // end IF

  ULONG flags = (GAA_FLAG_INCLUDE_PREFIX             |
                 GAA_FLAG_INCLUDE_WINS_INFO          |
                 GAA_FLAG_INCLUDE_GATEWAYS           |
                 GAA_FLAG_INCLUDE_ALL_INTERFACES     |
                 GAA_FLAG_INCLUDE_ALL_COMPARTMENTS   |
                 GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER);
  struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
  ULONG buffer_length = 0;
  ULONG result =
    GetAdaptersAddresses (AF_UNSPEC,              // Family
                          flags,                  // Flags
                          NULL,                   // Reserved
                          ip_adapter_addresses_p, // AdapterAddresses
                          &buffer_length);        // SizePointer
  if (unlikely (result != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_addresses_p =
    static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
  if (unlikely (!ip_adapter_addresses_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF
  result =
    GetAdaptersAddresses (AF_UNSPEC,              // Family
                          flags,                  // Flags
                          NULL,                   // Reserved
                          ip_adapter_addresses_p, // AdapterAddresses
                          &buffer_length);        // SizePointer
  if (unlikely (result != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_adapter_addresses_p);

    return false;
  } // end IF

  struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 =
    ip_adapter_addresses_p;
  struct _IP_ADAPTER_UNICAST_ADDRESS_LH* unicast_address_p = NULL;
  struct _IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway_address_p = NULL;
  struct _SOCKET_ADDRESS* socket_address_p = NULL;
  struct sockaddr_in* sockaddr_in_p = NULL;
  do
  {
//    if ((ip_adapter_addresses_2->OperStatus != IfOperStatusUp) ||
//        (!ip_adapter_addresses_2->FirstUnicastAddress))
//      continue;
    if (ip_adapter_addresses_2->IfIndex != interface_index)
      goto continue_;

    unicast_address_p = ip_adapter_addresses_2->FirstUnicastAddress;
    ACE_ASSERT (unicast_address_p);
    do
    {
      socket_address_p = &unicast_address_p->Address;
      ACE_ASSERT (socket_address_p->lpSockaddr);
      if (socket_address_p->lpSockaddr->sa_family == AF_INET)
        break;

      unicast_address_p = unicast_address_p->Next;
    } while (unicast_address_p);
    if (!unicast_address_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("adapter \"%s\":\"%s\" does not currently have any unicast IPv4 address, aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description)));

      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      return false;
    } // end IF
    sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
    result = IPAddress_out.set (sockaddr_in_p,
                                socket_address_p->iSockaddrLength);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));

      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      return false;
    } // end IF

    gateway_address_p = ip_adapter_addresses_2->FirstGatewayAddress;
    ACE_ASSERT (gateway_address_p);
    do
    {
      socket_address_p = &gateway_address_p->Address;
      ACE_ASSERT (socket_address_p->lpSockaddr);
      if (socket_address_p->lpSockaddr->sa_family == AF_INET)
        break;

      gateway_address_p = gateway_address_p->Next;
    } while (gateway_address_p);
    if (!gateway_address_p)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("adapter \"%s\":\"%s\" does not currently have any gateway IPv4 address, continuing\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description)));
      break;
    } // end IF
    socket_address_p = &gateway_address_p->Address;
    sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
    result = gatewayIPAddress_out.set (sockaddr_in_p,
                                       socket_address_p->iSockaddrLength);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));

      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      return false;
    } // end IF
    break;

continue_:
    ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
  } while (ip_adapter_addresses_2);

  // clean up
  ACE_FREE_FUNC (ip_adapter_addresses_p);
#else
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  struct sockaddr_in* sockaddr_in_p = NULL;
//  struct sockaddr_ll* sockaddr_ll_p = NULL;
  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
//    if (((ifaddrs_2->ifa_flags & IFF_UP) == 0) ||
//        (!ifaddrs_2->ifa_addr))
//      continue;
    if (ACE_OS::strcmp (interface_identifier_string.c_str (),
                        ifaddrs_2->ifa_name))
      continue;

    if (!ifaddrs_2->ifa_addr)
      continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;

    sockaddr_in_p = reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_addr);
    result = IPAddress_out.set (sockaddr_in_p,
                                sizeof (struct sockaddr_in));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));

      // clean up
      ::freeifaddrs (ifaddrs_p);

      return false;
    } // end IF
    break;
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */

  gatewayIPAddress_out = Net_Common_Tools::getGateway (interfaceIdentifier_in);
#endif

//  result = IPAddress_out.addr_to_string (buffer,
//                                         sizeof (buffer),
//                                         1);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("interface \"%s\" --> %s\n"),
//              ACE_TEXT (interfaceIdentifier_in.c_str ()),
//              buffer));

  return true;
}

bool
Net_Common_Tools::IPAddressToInterface (const ACE_INET_Addr& IPAddress_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                        struct _GUID& interfaceIdentifier_out)
#else
                                        std::string& interfaceIdentifier_out)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::IPAddressToInterface"));

  // initialize return value(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  interfaceIdentifier_out = GUID_NULL;
#else
  interfaceIdentifier_out.clear ();
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (Net_Common_Tools::isLocal (IPAddress_in))
  {
    struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
    ULONG buffer_length = 0;
    ULONG result =
      GetAdaptersAddresses (AF_UNSPEC,              // Family
                            0,                      // Flags
                            NULL,                   // Reserved
                            ip_adapter_addresses_p, // AdapterAddresses
                            &buffer_length);        // SizePointer
    if (unlikely (result != ERROR_BUFFER_OVERFLOW))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (buffer_length);
    ip_adapter_addresses_p =
      static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
    if (unlikely (!ip_adapter_addresses_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      return false;
    } // end IF
    result =
      GetAdaptersAddresses (AF_UNSPEC,              // Family
                            0,                      // Flags
                            NULL,                   // Reserved
                            ip_adapter_addresses_p, // AdapterAddresses
                            &buffer_length);        // SizePointer
    if (unlikely (result != NO_ERROR))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      return false;
    } // end IF

    struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 =
      ip_adapter_addresses_p;
    struct _IP_ADAPTER_UNICAST_ADDRESS_LH* unicast_address_p = NULL;
    struct _SOCKET_ADDRESS* socket_address_p = NULL;
    struct sockaddr_in* sockaddr_in_p, *sockaddr_in_2 = NULL;
    ULONG network_mask = 0;
    union _NET_LUID_LH interface_luid_u;
    interface_luid_u.Value = 0;
    sockaddr_in_2 = (struct sockaddr_in*)IPAddress_in.get_addr ();
    do
    {
      unicast_address_p = ip_adapter_addresses_2->FirstUnicastAddress;
      ACE_ASSERT (unicast_address_p);
      do
      {
        socket_address_p = &unicast_address_p->Address;
        ACE_ASSERT (socket_address_p->lpSockaddr);
        if (socket_address_p->lpSockaddr->sa_family == AF_INET)
          break;

        unicast_address_p = unicast_address_p->Next;
      } while (unicast_address_p);
      if (unlikely (!unicast_address_p))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("adapter \"%s:\"%s does not currently have any unicast IPv4 address, continuing\n"),
        //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
        //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description)));
        goto continue_;
      } // end IF

      // *NOTE*: this works for IPv4 addresses only
      ACE_ASSERT (unicast_address_p->OnLinkPrefixLength <= 32);
      network_mask = ~((1 << (32 - unicast_address_p->OnLinkPrefixLength)) - 1);
      sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("found adapter \"%s\": \"%s\" on network %s\n"),
      //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
      //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (ACE_INET_Addr (static_cast<u_short> (0),
      //                                                                         static_cast<ACE_UINT32> ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_p->sin_addr.S_un.S_addr) & network_mask
      //                                                                                                                                        : sockaddr_in_p->sin_addr.S_un.S_addr & network_mask)),
      //                                                          true).c_str ())));

      if (((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_p->sin_addr.S_un.S_addr) & network_mask
                                                 : sockaddr_in_p->sin_addr.S_un.S_addr & network_mask) !=
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_2->sin_addr.S_un.S_addr) & network_mask
                                                 : sockaddr_in_2->sin_addr.S_un.S_addr & network_mask))
        goto continue_;

      interfaceIdentifier_out =
        Net_Common_Tools::indexToInterface (ip_adapter_addresses_2->IfIndex);
      if (unlikely (InlineIsEqualGUID (interfaceIdentifier_out, GUID_NULL)))
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                    ip_adapter_addresses_2->IfIndex));

      break;

continue_:
      ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
    } while (ip_adapter_addresses_2);

    // clean up
    ACE_FREE_FUNC (ip_adapter_addresses_p);
  } // end IF
  else
  {
    DWORD result = NO_ERROR;
    struct _MIB_IPFORWARDROW route_s;

    ACE_OS::memset (&route_s, 0, sizeof (struct _MIB_IPFORWARDROW));
    result = GetBestRoute (IPAddress_in.get_ip_address (),
                           0,
                           &route_s);
    if (unlikely (result != NO_ERROR))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetBestRoute(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (IPAddress_in).c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF

    interfaceIdentifier_out =
      Net_Common_Tools::indexToInterface (route_s.dwForwardIfIndex);
    if (unlikely (InlineIsEqualGUID (interfaceIdentifier_out, GUID_NULL)))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                  route_s.dwForwardIfIndex));
  } // end ELSE
#else
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  struct sockaddr_in* sockaddr_in_p =
      static_cast<struct sockaddr_in*> (IPAddress_in.get_addr ());
//  struct sockaddr_ll* sockaddr_ll_p = NULL;
  struct sockaddr_in* sockaddr_in_2 = NULL;
  struct sockaddr_in* sockaddr_in_3 = NULL;
  in_addr_t inet_address_network = 0;
  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
    if (!ifaddrs_2->ifa_addr)
      continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;

    sockaddr_in_2 = reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_addr);
    sockaddr_in_3 =
        reinterpret_cast<struct sockaddr_in*> (ifaddrs_2->ifa_netmask);
    inet_address_network =
        sockaddr_in_2->sin_addr.s_addr & sockaddr_in_3->sin_addr.s_addr;
    if (inet_address_network !=
        (sockaddr_in_p->sin_addr.s_addr & sockaddr_in_3->sin_addr.s_addr))
      continue;

    interfaceIdentifier_out = ifaddrs_2->ifa_name;
    break;
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return (!InlineIsEqualGUID (interfaceIdentifier_out, GUID_NULL));
#else
  return (!interfaceIdentifier_out.empty ());
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct _GUID
#else
std::string
#endif
Net_Common_Tools::getDefaultInterface (enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultInterface"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID result = GUID_NULL;
#else
  std::string result;
#endif

  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
    case NET_LINKLAYER_PPP:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
      struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 = NULL;
      ULONG buffer_length = 0;
      ULONG result_2 = 0;
      struct _GUID interface_identifier = GUID_NULL;
      std::map<ULONG, struct _GUID> connected_interfaces;
      ACE_INET_Addr inet_address, gateway_address;

      result_2 =
        GetAdaptersAddresses (AF_UNSPEC,              // Family
                              0,                      // Flags
                              NULL,                   // Reserved
                              ip_adapter_addresses_p, // AdapterAddresses
                              &buffer_length);        // SizePointer
      if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return GUID_NULL;
      } // end IF
      ACE_ASSERT (buffer_length);
      ip_adapter_addresses_p =
        static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
      if (unlikely (!ip_adapter_addresses_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
        return GUID_NULL;
      } // end IF
      result_2 =
        GetAdaptersAddresses (AF_UNSPEC,              // Family
                              0,                      // Flags
                              NULL,                   // Reserved
                              ip_adapter_addresses_p, // AdapterAddresses
                              &buffer_length);        // SizePointer
      if (unlikely (result_2 != NO_ERROR))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF

      // step1: retrieve 'connected' interfaces
      ip_adapter_addresses_2 = ip_adapter_addresses_p;
      do
      { // *TODO*: encapsulate this translation
        if ((type_in == NET_LINKLAYER_802_3) &&
            ((ip_adapter_addresses_2->IfType != IF_TYPE_ETHERNET_CSMACD) ||
             (ip_adapter_addresses_2->IfType != IF_TYPE_IS088023_CSMACD)))
          goto continue_;
        else if ((type_in == NET_LINKLAYER_PPP) &&
                 (ip_adapter_addresses_2->IfType != IF_TYPE_PPP))
          goto continue_;
        if ((ip_adapter_addresses_2->OperStatus != IfOperStatusUp) ||
            !ip_adapter_addresses_2->FirstUnicastAddress)
          goto continue_;

        interface_identifier =
          Net_Common_Tools::indexToInterface (ip_adapter_addresses_2->IfIndex);
        if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                      ip_adapter_addresses_2->IfIndex));
          goto error;
        } // end IF

        connected_interfaces.insert (std::make_pair (ip_adapter_addresses_2->Ipv4Metric,
                                                     interface_identifier));

        //// debug info
        //if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
        //                                                       inet_address,
        //                                                       gateway_address)))
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
        //              ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
        //  goto error;
        //} // end IF
        //if (likely (type_in == NET_LINKLAYER_802_3))
        //{
        //  ACE_ASSERT (ip_adapter_addresses_2->PhysicalAddressLength >= ETH_ALEN);
        //} // end IF
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found network interface \"%s\"[%s]: IP#: %s; MAC#: %s\n"),
        //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
        //            ACE_TEXT (ip_adapter_addresses_2->AdapterName),
        //            ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address).c_str ()),
        //            ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (ip_adapter_addresses_2->PhysicalAddress,
        //                                                                  type_in).c_str ())));

continue_:
        ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
      } while (ip_adapter_addresses_2);

error:
      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      if (likely (!connected_interfaces.empty ()))
      {
        result = connected_interfaces.begin ()->second;
#if defined (_DEBUG)
        if (unlikely (!Net_Common_Tools::interfaceToIPAddress (result,
                                                               inet_address,
                                                               gateway_address)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                      ACE_TEXT (Net_Common_Tools::interfaceToString (result).c_str ())));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("default interface: \"%s\" (gateway: %s)\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (result).c_str ()),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (gateway_address).c_str ())));
#endif
      } // end IF
#else
      // *TODO*: this should work on most Unixy systems, but is a really bad
      //         idea:
      //         - relies on local 'ip'
      //         - temporary files
      //         - system(3) call
      //         --> extremely inefficient; remove ASAP
      std::string command_line_string = ACE_TEXT_ALWAYS_CHAR ("ip route");
      std::string ip_route_records_string;
      if (unlikely (!Common_Tools::command (command_line_string,
                                            ip_route_records_string)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Tools::command(\"%s\"), aborting\n"),
                    ACE_TEXT (ip_route_records_string.c_str ())));
        return ACE_TEXT_ALWAYS_CHAR ("");
      } // end IF
    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("ip data: \"%s\"\n"),
    //              ACE_TEXT (ip_route_records_string.c_str ())));

      std::istringstream converter;
      char buffer [BUFSIZ];
      std::string regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^default via ([[:digit:].]+) dev ([[:alnum:]]+)(?:.*)$");
      std::regex regex (regex_string);
      std::cmatch match_results;
      converter.str (ip_route_records_string);
      do
      {
        converter.getline (buffer, sizeof (buffer));
        if (unlikely (!std::regex_match (buffer,
                                         match_results,
                                         regex,
                                         std::regex_constants::match_default)))
          continue;
        ACE_ASSERT (match_results.ready () && !match_results.empty ());

        ACE_ASSERT (match_results[1].matched);
        ACE_ASSERT (match_results[2].matched);
        result = match_results[2];

        break;
      } while (!converter.fail ());
      if (unlikely (result.empty ()))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve default interface from route data (was: \"%s\"), aborting\n"),
                    ACE_TEXT (ip_route_records_string.c_str ())));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("default interface: \"%s\" (gateway: %s)\n"),
                    ACE_TEXT (result.c_str ()),
                    ACE_TEXT (match_results[1].str ().c_str ())));
#endif
#endif
      break;
    }
    case NET_LINKLAYER_802_11:
    {
      Net_InterfaceIdentifiers_t interface_identifiers_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
      HANDLE client_handle = ACE_INVALID_HANDLE;
      DWORD result_2 = WlanOpenHandle (maximum_client_version,
                                       NULL,
                                       &negotiated_version,
                                       &client_handle);
      if (unlikely (result_2 != ERROR_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::WlanOpenHandle(%u): \"%s\", aborting\n"),
                    maximum_client_version,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return result;
      } // end IF
      ACE_ASSERT (client_handle != ACE_INVALID_HANDLE);

      struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
      result_2 = WlanEnumInterfaces (client_handle,
                                     NULL,
                                     &interface_list_p);
      if (unlikely (result_2 != ERROR_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::WlanEnumInterfaces(0x%@): \"%s\", aborting\n"),
                    client_handle,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
      ACE_ASSERT (interface_list_p);
      for (DWORD i = 0;
           i < interface_list_p->dwNumberOfItems;
           ++i)
        interface_identifiers_a.push_back (interface_list_p->InterfaceInfo[i].InterfaceGuid);
      WlanFreeMemory (interface_list_p);

error_2:
      result_2 = WlanCloseHandle (client_handle,
                                  NULL);
      if (unlikely (result_2 != ERROR_SUCCESS))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
#else
#if defined (ACE_HAS_GETIFADDRS)
      int socket_handle = -1;
      struct iwreq iwreq_s;
      ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
      int result_2 = -1;

      socket_handle = ACE_OS::socket (AF_INET,
                                      SOCK_STREAM,
                                      0);
      if (unlikely (socket_handle == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
        return result;
      } // end IF

      struct ifaddrs* ifaddrs_p = NULL;
      result_2 = ::getifaddrs (&ifaddrs_p);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
        goto clean;
      } // end IF
      ACE_ASSERT (ifaddrs_p);

      // verify the presence of Wireless Extensions
      for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
           ifaddrs_2;
           ifaddrs_2 = ifaddrs_2->ifa_next)
      {
        // *TODO*: only tests for SIOCGIWNAME ioctl
        ACE_OS::strncpy (iwreq_s.ifr_name,
                         ifaddrs_2->ifa_name,
                         IFNAMSIZ);
        result_2 = ACE_OS::ioctl (socket_handle,
                                  SIOCGIWNAME,
                                  &iwreq_s);
        if (!result_2)
          interface_identifiers_a.push_back (ifaddrs_2->ifa_name);
      } // end FOR

clean:
      result_2 = ACE_OS::close (socket_handle);
      if (unlikely (result_2 == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                    socket_handle));
      if (likely (ifaddrs_p))
        ::freeifaddrs (ifaddrs_p);
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);

    ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif
      if (likely (!interface_identifiers_a.empty ()))
        return interface_identifiers_a.front ();
      break;
    }
    case NET_LINKLAYER_ATM:
    case NET_LINKLAYER_FDDI:
    {
      ACE_ASSERT (false);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_NOTSUP_RETURN (GUID_NULL);
#else
      ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR (""));
#endif
      ACE_NOTREACHED (break;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown link layer type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return GUID_NULL;
#else
  return ACE_TEXT_ALWAYS_CHAR ("");
#endif
}
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct _GUID
#else
std::string
#endif
Net_Common_Tools::getDefaultInterface (int linkLayerType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultInterface"));

  // step1: retrieve 'default' device for each link layer type specified
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::vector<struct _GUID> interfaces;
  struct _GUID interface_identifier;
#else
  std::vector<std::string> interfaces;
  std::string interface_identifier;
#endif
  for (enum Net_LinkLayerType i = NET_LINKLAYER_ATM;
       i < NET_LINKLAYER_MAX;
       ++i)
    if (linkLayerType_in & i)
    {
      interface_identifier = Net_Common_Tools::getDefaultInterface (i);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
      if (unlikely (interface_identifier.empty ()))
#endif
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::getDefaultInterface() (type was: \"%s\"), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::LinkLayerTypeToString (i).c_str ())));
        continue;
      } // end IF
      interfaces.push_back (interface_identifier);
    } // end IF

  if (likely (!interfaces.empty ()))
    return interfaces.front ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return GUID_NULL;
#else
  return ACE_TEXT_ALWAYS_CHAR ("");
#endif
}

bool
Net_Common_Tools::getAddress (std::string& hostName_inout,
                              std::string& dottedDecimal_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getAddress"));

//  struct hostent host, *result_p;
//  ACE_OS::memset (&host, 0, sizeof (struct hostent));
//  ACE_HOSTENT_DATA buffer;
//  ACE_OS::memset (buffer, 0, sizeof (ACE_HOSTENT_DATA));
//  int error = 0;

  int result = -1;
  ACE_INET_Addr inet_address;
  ACE_TCHAR buffer[HOST_NAME_MAX];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  if (hostName_inout.empty ())
  {
    // sanity check
    if (unlikely (dottedDecimal_inout.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid arguments (hostname/address empty), aborting\n")));
      return false;
    } // end IF

    result = inet_address.set (dottedDecimal_inout.c_str (), AF_INET);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (dottedDecimal_inout.c_str ())));
      return false;
    } // end IF

    result = inet_address.get_host_name (buffer, sizeof (buffer));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::get_host_name(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (dottedDecimal_inout.c_str ())));
      return false;
    } // end IF

    dottedDecimal_inout = buffer;
  } // end IF
  else
  {
    result = inet_address.set (0,
                               hostName_inout.c_str (),
                               1,
                               AF_INET);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostName_inout.c_str ())));
      return false;
    } // end IF

    const char* result_p = inet_address.get_host_addr (buffer, sizeof (buffer));
    if (unlikely (!result_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostName_inout.c_str ())));
      return false;
    } // end IF

    dottedDecimal_inout = buffer;
  } // end ELSE
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("\"%s\" <--> %s\n"),
  //            ACE_TEXT (hostName_inout.c_str ()),
  //            ACE_TEXT (dottedDecimal_inout.c_str ())));

  return true;
}

bool
Net_Common_Tools::getHostname (std::string& hostname_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getHostname"));

  hostname_out = Common_Tools::getHostName ();

  return !hostname_out.empty ();
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
bool
Net_Common_Tools::setPathMTUDiscovery (ACE_HANDLE handle_in,
                                       int option_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setPathMTUDiscovery"));

  // sanity check(s)
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  int result = -1;

  int optval = option_in;
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               IPPROTO_IP,
                               IP_MTU_DISCOVER,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,%s,%d): \"%m\", aborting\n"),
                handle_in,
                ACE_TEXT ("IP_MTU_DISCOVER"),
                option_in));
    return false;
  } // end IF

  return true;
}

bool
Net_Common_Tools::getPathMTU (const ACE_INET_Addr& destinationAddress_in,
                              unsigned int& MTU_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getPathMTU"));

  // initialize return value(s)
  bool result = false;
  MTU_out = 0;

  // sanity check(s)
  ACE_ASSERT (destinationAddress_in.get_addr_size () ==
              sizeof (struct sockaddr));

  ACE_HANDLE socket_handle = ACE_OS::socket (ACE_ADDRESS_FAMILY_INET,
                                             SOCK_DGRAM,
                                             0);
  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,%d): \"%m\", aborting\n"),
                ACE_ADDRESS_FAMILY_INET, SOCK_DGRAM, 0));
    return false;
  } // end IF

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result_2 = destinationAddress_in.addr_to_string (buffer,
                                                       sizeof (buffer),
                                                       1);
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  bool do_disassociate = false;
  struct sockaddr* sockaddr_p =
      reinterpret_cast<struct sockaddr*> (destinationAddress_in.get_addr ());
  result_2 = ACE_OS::connect (socket_handle,
                              sockaddr_p,
                              destinationAddress_in.get_addr_size ());
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::connect(%d,%s): \"%m\", aborting\n"),
                socket_handle,
                buffer));
    return false;
  } // end IF
  do_disassociate = true;

  // *NOTE*: IP_MTU works only on connect()ed sockets (see man 7 ip)
  int optval = 0;
  int optlen = sizeof (optval);
  result = ACE_OS::getsockopt (socket_handle,
                               IPPROTO_IP,
                               IP_MTU,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, %s): \"%m\", aborting\n"),
                socket_handle,
                ACE_TEXT ("IP_MTU")));
    goto close;
  } // end IF
  result = true;

  MTU_out = optval;

close:
  if (likely (do_disassociate))
  {
    // *NOTE*: see man connect(2)
    struct sockaddr socket_address = *sockaddr_p;
    socket_address.sa_family = AF_UNSPEC;
    result_2 = ACE_OS::connect (socket_handle,
                                &socket_address,
                                sizeof (socket_address));
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::connect(%d,%s): \"%m\", continuing\n"),
                  socket_handle,
                  buffer));
  } // end IF

  return result;
}
#endif

unsigned int
Net_Common_Tools::getMTU (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getMTU"));

  int result = -1;

  int optval = 0;
  int optlen = sizeof (optval);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_MAX_MSG_SIZE,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_MAX_MSG_SIZE): \"%m\", aborting\n"),
                handle_in));
    return 0;
  } // end IF
#else
  // *NOTE*: IP_MTU works only on connect()ed sockets (see man 7 ip)
  result = ACE_OS::getsockopt (handle_in,
                               IPPROTO_IP,
                               IP_MTU,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_UNUSED_ARG (error);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_MAX_MSG_SIZE): \"%m\", aborting\n"),
                handle_in));
#else
    if (error != ENOTCONN) // 107
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::getsockopt(%d,IP_MTU): \"%m\", aborting\n"),
                  handle_in));
#endif
    return 0;
  } // end IF
#endif

  return static_cast<unsigned int> (optval);
}

bool
Net_Common_Tools::setSocketBuffer (ACE_HANDLE handle_in,
                                   int optname_in,
                                   int size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setSocketBuffer"));

  // sanity check(s)
  if ((optname_in != SO_RCVBUF) &&
      (optname_in != SO_SNDBUF))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("0x%@: invalid socket option (was: %d), aborting\n"),
                handle_in,
                optname_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%d: invalid socket option (was: %d), aborting\n"),
                handle_in,
                optname_in));
#endif
    return false;
  } // end IF

  int result = -1;

  int optval = size_in;
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               SOL_SOCKET,
                               optname_in,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,%s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,%s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#endif
    return false;
  } // end IF

  // validate result
  optval = 0;
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               optname_in,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,%s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,%s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#endif
    return false;
  } // end IF

  if (unlikely (optval != size_in))
  {
    // *NOTE*: (for some reason,) Linux will actually set twice the given size
#if defined (ACE_LINUX)
    if (likely (optval == (size_in * 2)))
      return true;
#endif

    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("ACE_OS::getsockopt(0x%@,%s) returned %d (expected: %d), aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF")),
                optval,
                size_in));
#else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("ACE_OS::getsockopt(%d,%s) returned %d (expected: %d), aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF")),
                optval,
                size_in));
#endif
    return false;
  } // end IF

  //ACE_DEBUG((LM_DEBUG,
  //           ACE_TEXT("set \"%s\" option of socket %d to: %d\n"),
  //           ((optname_in == SO_RCVBUF) ? ACE_TEXT("SO_RCVBUF")
  //                                      : ACE_TEXT("SO_SNDBUF")),
  //           handle_in,
  //           size_in));

  return true;
}

bool
Net_Common_Tools::getNoDelay (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getNoDelay"));

  int result = -1;
  int optval = 0;
  int optlen = sizeof (optval);
  result = ACE_OS::getsockopt (handle_in,
                               IPPROTO_TCP,
                               TCP_NODELAY,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

  return (optval == 1);
}

bool
Net_Common_Tools::setNoDelay (ACE_HANDLE handle_in,
                              bool noDelay_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setNoDelay"));

  int result = -1;
  int optval = (noDelay_in ? 1 : 0);
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               IPPROTO_TCP,
                               TCP_NODELAY,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

  // validate result
  optval = 0;
  result = ACE_OS::getsockopt (handle_in,
                               IPPROTO_TCP,
                               TCP_NODELAY,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

  //ACE_DEBUG((LM_DEBUG,
  //           ACE_TEXT("setsockopt(%d,TCP_NODELAY): %s\n"),
  //           handle_in,
  //           (noDelay_in ? ((optval == 1) ? "on" : "off")
  //                       : ((optval == 0) ? "off" : "on"))));

  return (noDelay_in ? (optval == 1) : (optval == 0));
}

bool
Net_Common_Tools::setKeepAlive (ACE_HANDLE handle_in,
                                bool on_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setKeepAlive"));

  int result = -1;
  int optval = (on_in ? 1 : 0);
  int optlen = sizeof (optval);
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_KEEPALIVE,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

  optval = (on_in ? 1 : 0);
  result = ACE_OS::setsockopt (handle_in,
                               SOL_SOCKET,
                               SO_KEEPALIVE,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("setsockopt(%d,SO_KEEPALIVE): %s\n"),
//              handle_in,
//              (on_in ? ACE_TEXT ("on") : ACE_TEXT ("off"))));

  return (result == 0);
}

bool
Net_Common_Tools::setLinger (ACE_HANDLE handle_in,
                             bool on_in,
                             unsigned short seconds_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setLinger"));

  int result = -1;
  struct linger optval;
  ACE_OS::memset (&optval, 0, sizeof (optval));
  int optlen = sizeof (optval);
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_LINGER,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

  optval.l_onoff = static_cast<unsigned short> (on_in ? 1 : 0);
  if (seconds_in)
    optval.l_linger = seconds_in;
  result = ACE_OS::setsockopt (handle_in,
                               SOL_SOCKET,
                               SO_LINGER,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("setsockopt(%d,SO_LINGER): %s (%d second(s))\n"),
//              handle_in,
//              (optval.l_onoff ? ACE_TEXT ("on") : ACE_TEXT ("off")),
//              optval.l_linger));

  return (result == 0);
}

bool
Net_Common_Tools::setReuseAddress (ACE_HANDLE handle_in
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   )
#else
                                   , bool reusePort_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setReuseAddress"));

  int result = -1;
  int optval = 1;
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               SOL_SOCKET,
                               SO_REUSEADDR,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEADDR): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (reusePort_in)
  {
    result = ACE_OS::setsockopt (handle_in,
                                 SOL_SOCKET,
                                 SO_REUSEPORT,
                                 reinterpret_cast<const char*> (&optval),
                                 optlen);
    if (unlikely (result))
    {
      // *PORTABILITY*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEPORT): \"%m\", aborting\n"),
                  handle_in));
      return false;
    } // end IF
  } // end IF
#endif

  return true;
}

#if defined (ACE_LINUX)
bool
Net_Common_Tools::enableErrorQueue (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::enableErrorQueue"));

  int result = -1;

  int optval = 1;
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               IPPROTO_IP,
                               IP_RECVERR,
                               reinterpret_cast<char*> (&optval),
                               optlen);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,IP_RECVERR): \"%m\", aborting\n"),
                handle_in));
    return false;
  } // end IF

  return true;
}
#endif

ACE_INET_Addr
Net_Common_Tools::getBoundAddress (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getBoundAddress"));

  ACE_INET_Addr result;
  int result_2 = -1;
  struct sockaddr_in socket_address_s;
  ACE_OS::memset (&socket_address_s, 0, sizeof (struct sockaddr));
  int socket_address_length = sizeof (struct sockaddr_in);

  result_2 =
    ACE_OS::getsockname (handle_in,
                         reinterpret_cast<struct sockaddr*> (&socket_address_s),
                         &socket_address_length);
  if (unlikely (result_2 == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockname(0x%@): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockname(%d): \"%m\", continuing\n"),
                handle_in));
#endif
    return result;
  } // end IF

  result_2 = result.set (&socket_address_s,
                         socket_address_length);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
    return result;
  } // end IF

  return result;
}

int
Net_Common_Tools::getProtocol (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getProtocol"));

  int result = -1;
  int optval = 0;
  int optlen = sizeof (optval);
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_TYPE, // SO_STYLE
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (unlikely (result))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_TYPE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,SO_TYPE): \"%m\", aborting\n"),
                handle_in));
#endif
    return -1;
  } // end IF

  return optval;
}

bool
Net_Common_Tools::sendDatagram (const ACE_INET_Addr& localSAP_in,
                                const ACE_INET_Addr& remoteSAP_in,
                                ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::sendDatagram"));

  bool result = false;
  int result_2 = -1;
  ssize_t result_3 = -1;
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  size_t bytes_to_send = messageBlock_in->length ();

  handle_h = ACE_OS::socket (AF_INET,    // family
                             SOCK_DGRAM, // type
                             0);         // protocol
  if (unlikely (handle_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                AF_INET, SOCK_DGRAM));
    return false;
  } // end IF

  // set source address ?
  if (unlikely (!localSAP_in.is_any ()))
  {
    int one = 1;
    result_2 = ACE_OS::setsockopt (handle_h,
                                   SOL_SOCKET,
                                   SO_REUSEADDR,
                                   reinterpret_cast<char*> (&one),
                                   sizeof (int));
    if (unlikely (result_2 == -1))
    {
      int error = ACE_OS::last_error ();
      ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  handle_h));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEADDR): \"%m\", aborting\n"),
                  handle_h));
#endif
      goto error;
    } // end IF

    result_2 =
      ACE_OS::bind (handle_h,
                    reinterpret_cast<struct sockaddr*> (localSAP_in.get_addr ()),
                    localSAP_in.get_addr_size ());
    if (unlikely (result_2 == -1))
    {
      int error = ACE_OS::last_error ();
      ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::bind(0x%@,%s): \"%m\", aborting\n"),
                  handle_h,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_in).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::bind(%d,%s): \"%m\", aborting\n"),
                  handle_h,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_in).c_str ())));
#endif
      goto error;
    } // end IF
  } // end IF

  result_3 =
    ACE_OS::sendto (handle_h,
                    messageBlock_in->rd_ptr (),
                    bytes_to_send,
                    0,
                    reinterpret_cast<struct sockaddr*> (remoteSAP_in.get_addr ()),
                    remoteSAP_in.get_addr_size ());
  if (unlikely (static_cast<size_t> (result_3) != bytes_to_send))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sendto(0x%@,%s,%u) (result was: %d): \"%m\", aborting\n"),
                handle_h,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteSAP_in).c_str ()),
                bytes_to_send, result_3));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sendto(%d,%s,%u) (result was: %d): \"%m\", aborting\n"),
                handle_h,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteSAP_in).c_str ()),
                bytes_to_send, result_3));
#endif
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("dispatched %u byte(s): %s --> %s\n"),
              bytes_to_send,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_in).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteSAP_in).c_str ())));

  // *TODO*: shutdown() first ?

  result = true;

error:
  result_2 = ACE_OS::closesocket (handle_h);
  if (unlikely (result_2 == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                handle_h));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                handle_h));
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Net_Common_Tools::setLoopBackFastPath (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setLoopBackFastPath"));

  // *NOTE*: works for TCP connections only (see:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/jj841212(v=vs.85).aspx)
  // sanity check(s)
  ACE_ASSERT (Net_Common_Tools::getProtocol (handle_in) == SOCK_STREAM);

  int optval = 1; // --> enable
  unsigned long number_of_bytes_returned = 0;
  int result =
    ACE_OS::ioctl (handle_in,                 // socket handle
                   SIO_LOOPBACK_FAST_PATH,    // control code
                   &optval,                   // input buffer
                   sizeof (optval),           // size of input buffer
                   NULL,                      // output buffer
                   0,                         // size of output buffer
                   &number_of_bytes_returned, // #bytes returned
                   NULL, NULL);               // overlapped / function
  if (unlikely (result == SOCKET_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(0x%@,SIO_LOOPBACK_FAST_PATH): \"%s\", aborting\n"),
                handle_in,
                ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("enabled SIO_LOOPBACK_FAST_PATH on 0x%@\n"),
              handle_in));

  return true;
}
#endif

//Net_IInetConnectionManager_t*
//Net_Common_Tools::getConnectionManager ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getConnectionManager"));

//  return NET_CONNECTIONMANAGER_SINGLETON::instance ();
//}

std::string
Net_Common_Tools::URLToHostName (const std::string& URL_in,
                                 bool returnProtocol_in,
                                 bool returnPort_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::URLToHostName"));

  std::string result;

  std::string regex_string =
      ACE_TEXT_ALWAYS_CHAR ("^([[:alpha:]]+://)?([^\\/\\:]+)(\\:[[:digit:]]{1,5})?(.+)?$");
  std::regex regex (regex_string);
  std::smatch match_results;
  if (unlikely (!std::regex_match (URL_in,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URL_in.c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  if (unlikely (!match_results[2].matched))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URL_in.c_str ())));
    return result;
  } // end IF

  if (returnProtocol_in &&
      match_results[1].matched)
    result = match_results[1];
  result += match_results[2];
  if (returnPort_in &&
      match_results[3].matched)
    result += match_results[3];

  return result;
}

//std::string
//Net_Common_Tools::generateUniqueName (const std::string& prefix_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::generateUniqueName"));

//  std::string result;

//  // sanity check(s)
//  ACE_ASSERT (prefix_in.size () <= (BUFSIZ - 6 + 1));

//  // *NOTE*: see also: man 3 mkstemp
//  ACE_TCHAR buffer[BUFSIZ];
//  if (unlikely (!prefix_in.empty ()))
//    ACE_OS::strcpy (buffer, prefix_in.c_str ());
//  ACE_OS::strcpy (buffer + prefix_in.size (), ACE_TEXT ("XXXXXX"));
//  ACE_OS::mktemp (buffer);
//  if (unlikely (!ACE_OS::strlen (buffer)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_OS::mktemp(): \"%m\", aborting\n")));
//    return std::string ();
//  } // end IF
//  result = buffer;

//  return result;
//}

#if defined (ACE_LINUX)
bool
Net_Common_Tools::isNetworkManagerRunning ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::isNetworkManagerRunning"));

  std::string commandline_string =
      ACE_TEXT_ALWAYS_CHAR ("systemctl status NetworkManager");
  std::string output_string;
  if (unlikely (!Common_Tools::command (commandline_string,
                                        output_string)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::command(\"%s\"), aborting\n"),
                ACE_TEXT (commandline_string.c_str ())));
    return false; // *TODO*: avoid false negative
  } // end IF

  std::istringstream converter;
  char buffer [BUFSIZ];
  std::string regex_string =
      ACE_TEXT_ALWAYS_CHAR ("^   Active: (.+) \\((.+)\\) since (.+); (.+) ago$");
  std::regex regex (regex_string);
  std::cmatch match_results;
  converter.str (output_string);
  std::string status_string;
  do
  {
    converter.getline (buffer, sizeof (buffer));
    if (likely (!std::regex_match (buffer,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
      continue;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    ACE_ASSERT (match_results[1].matched);
    ACE_ASSERT (match_results[2].matched);
    ACE_ASSERT (match_results[3].matched);
    ACE_ASSERT (match_results[4].matched);

    status_string = match_results[1];
    if (ACE_OS::strcmp (status_string.c_str (),
                        ACE_TEXT_ALWAYS_CHAR ("active")))
      break;
    status_string = match_results[2];
    if (ACE_OS::strcmp (status_string.c_str (),
                        ACE_TEXT_ALWAYS_CHAR ("running")))
      break;
    return true;
  } while (!converter.fail ());

  return false;
}

void
Net_Common_Tools::toggleNetworkManager (bool toggle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::isNetworkManagerRunning"));

  std::string commandline_string =
      ACE_TEXT_ALWAYS_CHAR ("systemctl ");
  commandline_string += (toggle_in ? ACE_TEXT_ALWAYS_CHAR ("start")
                                   : ACE_TEXT_ALWAYS_CHAR ("stop"));
  commandline_string += ACE_TEXT_ALWAYS_CHAR (" NetworkManager");
  std::string output_string;
  if (unlikely (!Common_Tools::command (commandline_string,
                                        output_string)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::command(\"%s\"), returning\n"),
                ACE_TEXT (commandline_string.c_str ())));
}
#endif
