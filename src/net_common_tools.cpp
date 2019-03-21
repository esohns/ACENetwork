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
#include <guiddef.h>
#include <iphlpapi.h>
#include <Ks.h>
#include <mstcpip.h>
#include <wlanapi.h>
#else
#include "ifaddrs.h"
#include "linux/if_ether.h"
#include "linux/if_packet.h"
#include "net/if.h"
#include "netinet/ether.h"
#if defined (NETLINK_SUPPORT)
#include "linux/genetlink.h"
#include "linux/netlink.h"
#include "linux/nl80211.h"

#include "netlink/genl/genl.h"
#include "netlink/msg.h"
#endif // NETLINK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Dirent_Selector.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#if defined (ACE_LINUX)
#if defined (DBUS_SUPPORT)
#include "common_dbus_defines.h"
#include "common_dbus_tools.h"
#endif // DBUS_SUPPORT
#include "common_string_tools.h"

#include "common_timer_tools.h"
#elif (defined (ACE_WIN32) || defined (ACE_WIN64))
#include "common_error_tools.h"
#endif // ACE_LINUX

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#if defined (ACE_LINUX)
#include "net_wlan_configuration.h"
#if defined (DBUS_SUPPORT)
#include "net_wlan_defines.h"
#endif // DBUS_SUPPORT
#include "net_wlan_tools.h"
#endif // ACE_LINUX

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

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Net_Common_Tools::isInterfaceEnabled (REFGUID interfaceIdentifier_in)
#else
Net_Common_Tools::isInterfaceEnabled (const std::string& interfaceIdentifier_in)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
Net_Common_Tools::isInterfaceEnabled (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::isInterfaceEnabled"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

    bool result = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#else
  struct ifreq ifreq_s;
  int result_2 = -1;
  int socket_handle_h = ACE_INVALID_HANDLE;

  socket_handle_h = ACE_OS::socket (AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (unlikely (socket_handle_h == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET,SOCK_DGRAM,IPPROTO_IP): \"%m\", aborting\n")));
    return false; // *TODO*: avoid false negatives
  } // end IF

  ACE_OS::memset (&ifreq_s, 0, sizeof (struct ifreq));
  ACE_OS::strcpy (ifreq_s.ifr_name, interfaceIdentifier_in.c_str ());
  result_2 = ACE_OS::ioctl (socket_handle_h,
                            SIOCGIFFLAGS,
                            &ifreq_s);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIFFLAGS): \"%m\", aborting\n")));
    goto error;
  } // end IF

  result = (ifreq_s.ifr_ifru.ifru_flags & IFF_UP);

error:
  if (socket_handle_h != ACE_INVALID_HANDLE)
  {
    result_2 = ACE_OS::close (socket_handle_h);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  socket_handle_h));
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}
bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Net_Common_Tools::toggleInterface (REFGUID interfaceIdentifier_in)
#else
Net_Common_Tools::toggleInterface (const std::string& interfaceIdentifier_in)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
Net_Common_Tools::toggleInterface (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::toggleInterface"));

  bool result = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#else
  struct ifreq ifreq_s;
//  struct ifconf ifconf_s;
//  char buffer_a[BUFSIZ];
  int result_2 = -1;

  int socket_handle_h = ACE_OS::socket (AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (unlikely (socket_handle_h == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET,SOCK_DGRAM,IPPROTO_IP): \"%m\", aborting\n")));
    return false;
  } // end IF

  ACE_OS::memset (&ifreq_s, 0, sizeof (struct ifreq));
  ACE_OS::strcpy (ifreq_s.ifr_name, interfaceIdentifier_in.c_str ());
  result_2 = ACE_OS::ioctl (socket_handle_h,
                            SIOCGIFFLAGS,
                            &ifreq_s);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIFFLAGS): \"%m\", aborting\n"),
                socket_handle_h));
    goto error;
  } // end IF

  if (Net_Common_Tools::isInterfaceEnabled (interfaceIdentifier_in))
    ifreq_s.ifr_ifru.ifru_flags &= ~IFF_UP;
  else
    ifreq_s.ifr_ifru.ifru_flags |= IFF_UP;

  result_2 = ACE_OS::ioctl (socket_handle_h,
                            SIOCSIFFLAGS,
                            &ifreq_s);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIFFLAGS): \"%m\", aborting\n"),
                socket_handle_h));
    goto error;
  } // end IF

  result = true;

error:
  if (socket_handle_h != ACE_INVALID_HANDLE)
  {
    result_2 = ACE_OS::close (socket_handle_h);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  socket_handle_h));
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
std::string
Net_Common_Tools::NetlinkAddressToString (const Net_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::NetlinkAddressToString"));

  // initialize return value(s)
  std::string return_value;

  int result = -1;
  ACE_TCHAR buffer_a[BUFSIZ]; // "%u:%u\0"
  ACE_OS::memset (&buffer_a, 0, sizeof (buffer_a));
  result = address_in.addr_to_string (buffer_a,
                                      sizeof (buffer_a),
                                      1); // N/A
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", aborting\n")));
    return return_value;
  } // end IF

  // copy string from buffer
  return_value = ACE_TEXT_ALWAYS_CHAR (buffer_a);

  return return_value;
}

//std::string
//Net_Common_Tools::dump (struct nl_msg* message_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::dump"));

//  std::string result;

//  struct genlmsghdr* genlmsghdr_p =
//      static_cast<struct genlmsghdr*> (nlmsg_data (nlmsg_hdr (message_in)));
//  ACE_ASSERT (genlmsghdr_p);
//  struct nlattr* nlattr_a[NL80211_ATTR_MAX + 1];
//  nla_parse (nlattr_a,
//             NL80211_ATTR_MAX,
//             genlmsg_attrdata (genlmsghdr_p, 0),
//             genlmsg_attrlen (genlmsghdr_p, 0),
//             NULL);
//  const struct nlattr* nlattr_p = NULL;
//  int rem, i = 0;
//  std::ostringstream converter;
//  nla_for_each_attr (nlattr_p, genlmsg_attrdata (genlmsghdr_p, 0), genlmsg_attrlen (genlmsghdr_p, 0), rem)
//  { ACE_ASSERT (nlattr_p);
//    result = ACE_TEXT_ALWAYS_CHAR ("#");
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.clear ();
//    converter << ++i;
//    result += converter.str ();
//    result += ACE_TEXT_ALWAYS_CHAR (": ");
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.clear ();
//    converter << nla_type (nlattr_p);
//    result += converter.str ();
//    result += ACE_TEXT_ALWAYS_CHAR ("; ");
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.clear ();
//    converter << nla_len (nlattr_p);
//    result += converter.str ();
//    result += ACE_TEXT_ALWAYS_CHAR (" byte(s)\n");
//  } // end FOR

//  return result;
//}
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

std::string
Net_Common_Tools::IPAddressToString (unsigned short port_in,
                                     ACE_UINT32 IPAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::IPAddressToString"));

  // initialize return value(s)
  std::string return_value;

  int result = -1;
  ACE_INET_Addr inet_addr;
  char buffer_a[32]; // "xxx.xxx.xxx.xxx:yyyyy\0"
  ACE_OS::memset (&buffer_a, 0, sizeof (char[32]));
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
  result = inet_addr.addr_to_string (buffer_a,
                                     sizeof (char[32]),
                                     1); // want IP address, not hostname !
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Inet_Addr::addr_to_string(): \"%m\", aborting\n")));
    return return_value;
  } // end IF

  // copy string from buffer
  return_value = buffer_a;

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
//  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (!match_results.empty ());

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
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

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
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return false; // *TODO*: this could lead to false negatives
  } // end IF
  // step1: determine size of the table
  result = GetIpAddrTable (table_p, &table_size, FALSE);
  if (unlikely (!table_p ||
                (result != ERROR_INSUFFICIENT_BUFFER)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetIpAddrTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));

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
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return false; // *TODO*: this could lead to false negatives
  } // end IF
  // step2: get the actual table data
  result = GetIpAddrTable (table_p, &table_size, FALSE);
  if (unlikely (result != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetIpAddrTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

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
Net_Common_Tools::LinkLayerAddressToString (const uint8_t* const addressDataPtr_in,
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
      ACE_TCHAR buffer_a[NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE];
      ACE_OS::memset (&buffer_a, 0, sizeof (ACE_TCHAR[NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE]));
      int result_2 = -1;

      // *PORTABILITY*: ether_ntoa_r is not portable
      // *TODO*: implement an ACE wrapper function for unsupported platforms
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0601) // _WIN32_WINNT_WIN7 *TODO*: Windows XP support
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN7
#if defined (ACE_USES_WCHAR)
      PWSTR result_3 =
        RtlEthernetAddressToStringW (reinterpret_cast<const _DL_EUI48* const> (addressDataPtr_in),
                                     buffer_a);
#else
      PSTR result_3 =
        RtlEthernetAddressToStringA (reinterpret_cast<const _DL_EUI48* const> (addressDataPtr_in),
                                     buffer_a);
#endif // ACE_USES_WCHAR
      if (unlikely (result_3 != (buffer_a + NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE - 1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::RtlEthernetAddressToString(), aborting\n")));
        break;
      } // end IF
#else
      const struct ether_addr* const ether_addr_p =
          reinterpret_cast<const struct ether_addr* const> (addressDataPtr_in);
      result_2 =
          ACE_OS::snprintf (buffer_a,
                            NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE,
                            ACE_TEXT ("%02x:%02x:%02x:%02x:%02x:%02x"),
                            ether_addr_p->ether_addr_octet[0],
                            ether_addr_p->ether_addr_octet[1],
                            ether_addr_p->ether_addr_octet[2],
                            ether_addr_p->ether_addr_octet[3],
                            ether_addr_p->ether_addr_octet[4],
                            ether_addr_p->ether_addr_octet[5]);
      if (unlikely (result_2 < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::snprintf(): \"%m\", aborting\n")));
        break;
      } // end IF
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0601)
#else
      const struct ether_addr* const ether_addr_p =
          reinterpret_cast<const struct ether_addr* const> (addressDataPtr_in);
      // *PORTABILITY*: apparently some versions of ether_ntoa(3) omit leading
      //                zeroes
//      if (unlikely (!::ether_ntoa_r (ether_addr_p,
//                                     buffer_a)))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::ether_ntoa_r(): \"%m\", aborting\n")));
//        break;
//      } // end IF
      result_2 =
          ACE_OS::snprintf (buffer_a,
                            NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE,
                            ACE_TEXT ("%02x:%02x:%02x:%02x:%02x:%02x"),
                            ether_addr_p->ether_addr_octet[0],
                            ether_addr_p->ether_addr_octet[1],
                            ether_addr_p->ether_addr_octet[2],
                            ether_addr_p->ether_addr_octet[3],
                            ether_addr_p->ether_addr_octet[4],
                            ether_addr_p->ether_addr_octet[5]);
      if (unlikely (result_2 < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::snprintf(): \"%m\", aborting\n")));
        break;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64
      return_value = ACE_TEXT_ALWAYS_CHAR (buffer_a);

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
struct ether_addr
Net_Common_Tools::stringToLinkLayerAddress (const std::string& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::LinkLayerTypeToString"));

  // initialize return value(s)
  struct ether_addr return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!address_in.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (return_value);

  ACE_NOTREACHED (return return_value;)
#else
  if (unlikely (!::ether_aton_r (address_in.c_str (),
                                 &return_value)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ether_aton_r(\"%s\"): \"%m\"), aborting\n"),
                ACE_TEXT (address_in.c_str ())));
    return return_value;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Net_Common_Tools::interfaceToExternalIPAddress (REFGUID interfaceIdentifier_in,
#else
Net_Common_Tools::interfaceToExternalIPAddress (const std::string& interfaceIdentifier_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
Net_Common_Tools::interfaceToExternalIPAddress (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (interface_identifier.empty ())
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
    interface_identifier = Net_Common_Tools::getDefaultInterface ();

  // step1: determine the 'internal' IP address
  ACE_INET_Addr internal_ip_address, gateway_ip_address;
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                         internal_ip_address,
                                                         gateway_ip_address)))

  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
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
  unsigned int file_size_i = 0;
  if (unlikely (!Common_File_Tools::load (filename_string,
                                          data_p,
                                          file_size_i)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
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
  char buffer_a[BUFSIZ];
  std::string regex_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_TEXT_ALWAYS_CHAR ("^([^:]+)(?::[[:blank:]]*)(.+)(?:\r)$");
#else
    ACE_TEXT_ALWAYS_CHAR ("^([^:]+)(?::[[:blank:]]*)(.+)$");
#endif // ACE_WIN32 || ACE_WIN64
  std::regex regex (regex_string);
  std::smatch match_results;
  converter.str (resolution_record_string);
  bool is_first = true;
  std::string buffer_string;
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_string = buffer_a;
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
      continue;
//    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    ACE_ASSERT (!match_results.empty ());

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
Net_Common_Tools::interfaceToLinkLayerAddress (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToLinkLayerAddress"));

  // initialize return value(s)
  struct ether_addr result;
  ACE_OS::memset (&result, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    return result;
  } // end IF

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);
    return result;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    ACE_OS::memcpy (&result,
                    ip_interfaceIdentifier_info_2->Address,
                    ETH_ALEN);
    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

  // clean up
  if (ip_interfaceIdentifier_info_p)
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
#endif // ACE_LINUX
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
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}
std::string
Net_Common_Tools::linkLayerAddressToInterface (const struct ether_addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::linkLayerAddressToInterface"));

  // initialize return value(s)
  std::string return_value;

//  // sanity check(s)
//  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
//  //ACE_ASSERT (sizeof (MACAddress_out) >= MAX_ADAPTER_ADDRESS_LENGTH);

//  NET_IFINDEX interface_index = 0;
//  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
//  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
//  ULONG buffer_length = 0;
//  ULONG result_2 = 0;

//  interface_index =
//    Net_Common_Tools::interfaceToIndex (interfaceIdentifier_in);
//  if (unlikely (!interface_index))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
//                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
//    return result;
//  } // end IF

//  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
//                              &buffer_length);
//  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (buffer_length);
//  ip_interfaceIdentifier_info_p =
//    static_cast<struct _IP_ADAPTER_INFO*> (ACE_MALLOC_FUNC (buffer_length));
//  if (unlikely (!ip_interfaceIdentifier_info_p))
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return result;
//  } // end IF
//  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
//                              &buffer_length);
//  if (unlikely (result_2 != NO_ERROR))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));

//    // clean up
//    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

//    return result;
//  } // end IF

//  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
//  do
//  {
//    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
//      goto continue_;

//    ACE_OS::memcpy (&result, ip_interfaceIdentifier_info_2->Address,
//                    ETH_ALEN);
//    break;

//continue_:
//    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
//  } while (ip_interfaceIdentifier_info_2);

//  // clean up
//  ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (return_value);

  ACE_NOTREACHED (return return_value;)

  return return_value;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ether_addr
Net_Common_Tools::interfaceToLinkLayerAddress_2 (REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToLinkLayerAddress_2"));

  // initialize return value(s)
  struct ether_addr result;
  ACE_OS::memset (&result, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

  NET_IFINDEX interface_index = 0;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 = 0;

  interface_index =
    Net_Common_Tools::interfaceToIndex_2 (interfaceIdentifier_in);
  if (unlikely (!interface_index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex_2(%s), aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ())));
    return result;
  } // end IF

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);
    return result;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    ACE_OS::memcpy (&result,
                    ip_interfaceIdentifier_info_2->Address,
                    ETH_ALEN);
    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

  // clean up
  if (ip_interfaceIdentifier_info_p)
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

  return result;
}
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
struct _GUID
Net_Common_Tools::linkLayerAddressToInterface_2 (const struct ether_addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::linkLayerAddressToInterface_2"));

  // initialize return value(s)
  struct _GUID return_value = GUID_NULL;

//  // sanity check(s)
//  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
//  //ACE_ASSERT (sizeof (MACAddress_out) >= MAX_ADAPTER_ADDRESS_LENGTH);

//  NET_IFINDEX interface_index = 0;
//  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
//  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
//  ULONG buffer_length = 0;
//  ULONG result_2 = 0;

//  interface_index =
//    Net_Common_Tools::interfaceToIndex (interfaceIdentifier_in);
//  if (unlikely (!interface_index))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
//                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
//    return result;
//  } // end IF

//  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
//                              &buffer_length);
//  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (buffer_length);
//  ip_interfaceIdentifier_info_p =
//    static_cast<struct _IP_ADAPTER_INFO*> (ACE_MALLOC_FUNC (buffer_length));
//  if (unlikely (!ip_interfaceIdentifier_info_p))
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return result;
//  } // end IF
//  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p,
//                              &buffer_length);
//  if (unlikely (result_2 != NO_ERROR))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));

//    // clean up
//    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

//    return result;
//  } // end IF

//  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
//  do
//  {
//    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
//      goto continue_;

//    ACE_OS::memcpy (&result, ip_interfaceIdentifier_info_2->Address,
//                    ETH_ALEN);
//    break;

//continue_:
//    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
//  } while (ip_interfaceIdentifier_info_2);

//  // clean up
//  ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (return_value);

  ACE_NOTREACHED (return return_value;)

  return return_value;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::string
Net_Common_Tools::interfaceToConnection (REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToConnection"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

  std::string section_string =
    ACE_TEXT_ALWAYS_CHAR ("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\");
  section_string += Common_Tools::GUIDToString (interfaceIdentifier_in);
  HKEY key_p = NULL;
  // *NOTE*: this fails intermittently (*TODO*: why ?) --> retry a few times
retry:
  unsigned int retry_resolve_key_i = 0;
  key_p =
    ACE_Configuration_Win32Registry::resolve_key (HKEY_LOCAL_MACHINE,
                                                  ACE_TEXT_CHAR_TO_TCHAR (section_string.c_str ()),
                                                  0); // do not create
  if (unlikely (!key_p))
  {
    ++retry_resolve_key_i;
    if (retry_resolve_key_i >= COMMON_WIN32_REGISTRY_RESOLVE_RETRIES)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Win32Registry::resolve_key(HKEY_LOCAL_MACHINE,\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (section_string.c_str ())));
      return result;
    } // end IF
#if defined (_DEBUG)
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to ACE_Configuration_Win32Registry::resolve_key(HKEY_LOCAL_MACHINE,\"%s\"): \"%m\", retrying\n"),
                  ACE_TEXT (section_string.c_str ())));
#endif // _DEBUG
    goto retry;
  } // end IF
  result = Common_Tools::getKeyValue (key_p,
                                      ACE_TEXT_ALWAYS_CHAR ("Connection"),
                                      ACE_TEXT_ALWAYS_CHAR ("Name"));
  ACE_ASSERT (!result.empty ());
  //RegCloseKey (key_p); key_p = NULL;

  return result;
}

std::string
Net_Common_Tools::interfaceToString (NET_IFINDEX interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToString"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (interfaceIdentifier_in);

  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_2 = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 = 0;

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interfaceIdentifier_in) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    result = ACE_TEXT_ALWAYS_CHAR (ip_interfaceIdentifier_info_2->Description);

    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

error:
  // clean up
  if (ip_interfaceIdentifier_info_p)
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);

  // strip trailing note
  std::string::size_type last_dash_pos =
    result.find_last_of ('-',
                         std::string::npos); // begin searching at the end !
  if ((last_dash_pos != std::string::npos) &&
      (result[last_dash_pos - 1] == ' '))
    result.erase (last_dash_pos - 1, std::string::npos);

  return result;
}
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
NET_IFINDEX
Net_Common_Tools::interfaceToIndex_2 (REFGUID interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToIndex_2"));

  // initialize return value(s)
  NET_IFINDEX result = 0;

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  union _NET_LUID_LH interface_luid_u;
  interface_luid_u.Value = 0;
  DWORD result_2 = ConvertInterfaceGuidToLuid (&interfaceIdentifier_in,
                                               &interface_luid_u);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::ConvertInterfaceGuidToLuid(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  result_2 = ConvertInterfaceLuidToIndex (&interface_luid_u,
                                          &result);
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::ConvertInterfaceLuidToIndex(%q): \"%s\", aborting\n"),
                interface_luid_u.Value,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
#else
  IP_INTERFACE_NAME_INFO* ip_interface_name_info_p = NULL;
  DWORD count = 0;
  DWORD dwFlags = 0;
  DWORD result_2 =
    NhpAllocateAndGetInterfaceInfoFromStack (&ip_interface_name_info_p, // ppTable
                                             &count,                    // pdwCount
                                             FALSE,                     // bOrder
                                             GetProcessHeap (),         // hHeap
                                             dwFlags);                  // Flags
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::NhpAllocateAndGetInterfaceInfoFromStack(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  for (DWORD i = 0;
       i < count;
       ++i)
  { //ACE_ASSERT (ip_interface_name_info_p[i]);
    if (InlineIsEqualGUID (ip_interface_name_info_p[i].DeviceGuid, interfaceIdentifier_in))
    {
      result = ip_interface_name_info_p[i].Index;
      break;
    } // end IF
  } // end FOR

  // clean up
  if (!HeapFree (GetProcessHeap (), dwFlags, ip_interface_name_info_p))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::HeapFree(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
  ip_interface_name_info_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  return result;
}
struct _GUID
Net_Common_Tools::indexToInterface_2 (NET_IFINDEX interfaceIndex_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::indexToInterface_2"));

  // initialize return value(s)
  struct _GUID result = GUID_NULL;

  // sanity check(s)
  ACE_ASSERT (interfaceIndex_in);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  union _NET_LUID_LH interface_luid_u;
  interface_luid_u.Value = 0;
  DWORD result_2 = ConvertInterfaceIndexToLuid (interfaceIndex_in,
                                                &interface_luid_u);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ConvertInterfaceIndexToLuid(%u), aborting\n"),
                interfaceIndex_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return GUID_NULL;
  } // end IF
  result_2 = ConvertInterfaceLuidToGuid (&interface_luid_u,
                                         &result);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ConvertInterfaceLuidToGuid(%q), aborting\n"),
                interface_luid_u.Value,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return GUID_NULL;
  } // end IF
#else
  IP_INTERFACE_NAME_INFO* ip_interface_name_info_p = NULL;
  DWORD count = 0;
  DWORD dwFlags = 0;
  DWORD result_2 =
    NhpAllocateAndGetInterfaceInfoFromStack (&ip_interface_name_info_p, // ppTable
                                             &count,                    // pdwCount
                                             FALSE,                     // bOrder
                                             GetProcessHeap (),         // hHeap
                                             dwFlags);                  // Flags
  if (unlikely (result_2 != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::NhpAllocateAndGetInterfaceInfoFromStack(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return GUID_NULL;
  } // end IF
  for (DWORD i = 0;
       i < count;
       ++i)
  { //ACE_ASSERT (ip_interface_name_info_p[i]);
    if (ip_interface_name_info_p[i].Index == interfaceIndex_in)
    {
      result = ip_interface_name_info_p[i].DeviceGuid;
      break;
    } // end IF
  } // end FOR

  // clean up
  if (!HeapFree (GetProcessHeap (), dwFlags, ip_interface_name_info_p))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::HeapFree(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
  ip_interface_name_info_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  return result;
}

std::string
Net_Common_Tools::adapterName (const std::string& friendlyName_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::adapterName"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (!friendlyName_in.empty ());

  ULONG flags = (GAA_FLAG_INCLUDE_PREFIX             |
                 GAA_FLAG_INCLUDE_WINS_INFO          |
                 GAA_FLAG_INCLUDE_GATEWAYS           |
                 GAA_FLAG_INCLUDE_ALL_INTERFACES     |
                 GAA_FLAG_INCLUDE_ALL_COMPARTMENTS   |
                 GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER);
  IP_ADAPTER_ADDRESSES* ip_adapter_addresses_p = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 =
    GetAdaptersAddresses (AF_UNSPEC,              // Family
                          flags,                  // Flags
                          NULL,                   // Reserved
                          ip_adapter_addresses_p, // AdapterAddresses
                          &buffer_length);        // SizePointer
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_addresses_p =
    static_cast<IP_ADAPTER_ADDRESSES*> (ACE_MALLOC_FUNC (buffer_length));
  if (unlikely (!ip_adapter_addresses_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return result;
  } // end IF
  result_2 =
    GetAdaptersAddresses (AF_UNSPEC,              // Family
                          flags,                  // Flags
                          NULL,                   // Reserved
                          ip_adapter_addresses_p, // AdapterAddresses
                          &buffer_length);        // SizePointer
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
    return result;
  } // end IF

  IP_ADAPTER_ADDRESSES* ip_adapter_addresses_2 = ip_adapter_addresses_p;
  do
  {
    if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName)),
                         friendlyName_in.c_str ()))
      break;

    ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
  } while (ip_adapter_addresses_2);
  if (ip_adapter_addresses_2)
    result = ip_adapter_addresses_2->AdapterName;

  // clean up
  ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;

  return result;
}
NET_IFINDEX
Net_Common_Tools::interfaceToIndex (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToIndex"));

  // initialize return value(s)
  NET_IFINDEX result = 0;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  struct _GUID interface_identifier_s = GUID_NULL, interface_identifier_2 = GUID_NULL;
  if (Common_Tools::isGUID (interfaceIdentifier_in))
    interface_identifier_s =
      Common_Tools::StringToGUID (interfaceIdentifier_in);

  std::string interface_identifier;
  struct _IP_ADAPTER_INFO* adapter_info_p = NULL, *adapter_info_2 = NULL;
  ULONG result_2 = 0;
  ULONG buffer_length_u = 0;
  // retrieve the necessary size into the ulOutBufLen variable
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  adapter_info_p = (struct _IP_ADAPTER_INFO*)MALLOC (buffer_length_u);
  if (!adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    goto error;
  } // end IF
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(%u): \"%s\", aborting\n"),
                buffer_length_u,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  adapter_info_2 = adapter_info_p;
  while (adapter_info_2)
  {
    if (Common_Tools::isGUID (adapter_info_2->AdapterName))
    {
      interface_identifier_2 =
        Common_Tools::StringToGUID (adapter_info_2->AdapterName);
      interface_identifier =
        Net_Common_Tools::interfaceToConnection (interface_identifier_2);
      ACE_ASSERT (!interface_identifier.empty ());
    } // end IF
    else
      interface_identifier = adapter_info_2->AdapterName;
    if (InlineIsEqualGUID (interface_identifier_s, interface_identifier_2) ||
        !ACE_OS::strcmp (interface_identifier.c_str (),
                         interfaceIdentifier_in.c_str ()))
    {
      result = adapter_info_2->Index;
      break;
    } // end ELSE IF

    adapter_info_2 = adapter_info_2->Next;
  } // end WHILE

error:
  if (adapter_info_p)
    FREE (adapter_info_p);

  return result;
}
std::string
Net_Common_Tools::indexToInterface (NET_IFINDEX interfaceIndex_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::indexToInterface"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (interfaceIndex_in);

  struct _IP_ADAPTER_INFO* adapter_info_p = NULL, *adapter_info_2 = NULL;
  ULONG result_2 = 0;
  ULONG buffer_length_u = 0;

  // retrieve the necessary size into the ulOutBufLen variable
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  adapter_info_p = (struct _IP_ADAPTER_INFO*)MALLOC (buffer_length_u);
  if (!adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    goto error;
  } // end IF
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(%u): \"%s\", aborting\n"),
                buffer_length_u,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  adapter_info_2 = adapter_info_p;
  while (adapter_info_2)
  {
    if (adapter_info_2->Index == interfaceIndex_in)
    {
      // *NOTE*: apparently AdapterName contains the interface identifier GUID
      //         --> look up its 'friendly name' in the registry
      // *TODO*: use GetAdaptersAddresses() instead
      if (Common_Tools::isGUID (adapter_info_2->AdapterName))
        result =
          Net_Common_Tools::interfaceToConnection (Common_Tools::StringToGUID (adapter_info_2->AdapterName));
      else
        result = adapter_info_2->AdapterName;
      break;
    } // end IF

    adapter_info_2 = adapter_info_2->Next;
  } // end WHILE

error:
  if (adapter_info_p)
    FREE (adapter_info_p);

  return result;
}

#if defined (_DEBUG)
void
Net_Common_Tools::dump (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  struct _IP_ADAPTER_INFO* adapter_info_p = NULL, *adapter_info_2 = NULL;
  ULONG result_2 = 0;
  ULONG buffer_length_u = sizeof (struct _IP_ADAPTER_INFO);
  struct ether_addr ether_addr_s;
  std::string adapter_type_string;
  errno_t error_i;
  struct tm tm_s;
  char buffer_a[32];
  char buffer_2[32];

  adapter_info_p =
    (struct _IP_ADAPTER_INFO*)MALLOC (sizeof (struct _IP_ADAPTER_INFO));
  if (!adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  // retrieve the necessary size into the ulOutBufLen variable
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  FREE (adapter_info_p); adapter_info_p = NULL;
  adapter_info_p = (struct _IP_ADAPTER_INFO*)MALLOC (buffer_length_u);
  if (!adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  result_2 = GetAdaptersInfo (adapter_info_p, &buffer_length_u);
  if (result_2 != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(%u): \"%s\", returning\n"),
                buffer_length_u,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  adapter_info_2 = adapter_info_p;
  while (adapter_info_2)
  {
    ACE_OS::memset (&ether_addr_s, 0, sizeof (struct ether_addr));
    ACE_ASSERT (adapter_info_2->AddressLength == ETH_ALEN);
    ACE_OS::memcpy (&ether_addr_s.ether_addr_octet, &adapter_info_2->Address[0], ETH_ALEN);
    switch (adapter_info_2->Type)
    {
      case MIB_IF_TYPE_OTHER:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("Other");
        break;
      case MIB_IF_TYPE_ETHERNET:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("Ethernet");
        break;
      case MIB_IF_TYPE_TOKENRING:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("Token Ring");
        break;
      case MIB_IF_TYPE_FDDI:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("FDDI");
        break;
      case MIB_IF_TYPE_PPP:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("PPP");
        break;
      case MIB_IF_TYPE_LOOPBACK:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("Lookback");
        break;
      case MIB_IF_TYPE_SLIP:
        adapter_type_string = ACE_TEXT_ALWAYS_CHAR ("Slip");
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown adapter type (was: %d), returning\n"),
                    adapter_info_2->Type));
        goto error;
      }
    } // end SWITCH
    error_i = _localtime32_s (&tm_s, (__time32_t*)&adapter_info_2->LeaseObtained);
    if (error_i)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to _localtime32_s(%d): \"%s\", returning\n"),
                  adapter_info_2->LeaseObtained,
                  ACE_TEXT (Common_Error_Tools::errorToString (error_i).c_str ())));
      goto error;
    } // end IF
    error_i = asctime_s (buffer_a, sizeof (char[32]), &tm_s);
    if (error_i)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to asctime_s(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (error_i).c_str ())));
      goto error;
    } // end IF
    error_i = _localtime32_s (&tm_s, (__time32_t*)&adapter_info_2->LeaseExpires);
    if (error_i)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to _localtime32_s(%d): \"%s\", returning\n"),
                  adapter_info_2->LeaseExpires,
                  ACE_TEXT (Common_Error_Tools::errorToString (error_i).c_str ())));
      goto error;
    } // end IF
    error_i = asctime_s (buffer_2, sizeof (char[32]), &tm_s);
    if (error_i)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to asctime_s(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (error_i).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\tComboIndex: \t%d\n\tAdapter Name: \t%s\n\tAdapter Description: \t%s\n\tAdapter Address: \t%s\n\tIndex: \t%d\n\tType: \t%s\n\tIP Address: \t%s\n\tIP Mask: \t%s\n\tGateway: \t%s\n\tDHCP Enabled: \t%s\n\tDHCP Server: \t%s\n\tLease Obtained: \t%s\n\tLease Expires: \t%s\n\tPrimary Wins Server: \t%s\n\tSecondary Wins Server: \t%s\n"),
                adapter_info_2->ComboIndex,
                ACE_TEXT (adapter_info_2->AdapterName),
                //ACE_TEXT_WCHAR_TO_TCHAR (adapter_info_2->Description),
                ACE_TEXT (adapter_info_2->Description),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<uint8_t*> (&(ether_addr_s.ether_addr_octet)), NET_LINKLAYER_802_3).c_str ()),
                adapter_info_2->Index,
                ACE_TEXT (adapter_type_string.c_str ()),
                ACE_TEXT (adapter_info_2->IpAddressList.IpAddress.String),
                ACE_TEXT (adapter_info_2->IpAddressList.IpMask.String),
                ACE_TEXT (adapter_info_2->GatewayList.IpAddress.String),
                (adapter_info_2->DhcpEnabled ? ACE_TEXT ("Yes") : ACE_TEXT ("No")),
                ACE_TEXT (adapter_info_2->DhcpServer.IpAddress.String),
                ACE_TEXT (buffer_a),
                ACE_TEXT (buffer_2),
                (adapter_info_2->HaveWins ? ACE_TEXT ("Yes") : ACE_TEXT ("No")),
                ACE_TEXT (adapter_info_2->PrimaryWinsServer.IpAddress.String),
                ACE_TEXT (adapter_info_2->SecondaryWinsServer.IpAddress.String)));

    adapter_info_2 = adapter_info_2->Next;
  } // end WHILE

error:
  if (adapter_info_p)
    FREE (adapter_info_p);
}
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

ACE_INET_Addr
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Net_Common_Tools::getGateway (REFGUID interfaceIdentifier_in)
#else
Net_Common_Tools::getGateway (const std::string& interfaceIdentifier_in)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
Net_Common_Tools::getGateway (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getGateway"));

  ACE_INET_Addr result;

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in , GUID_NULL));
#else
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ULONG result_2 = 0;
  struct _IP_ADAPTER_INFO* ip_interfaceIdentifier_info_p = NULL, *ip_interfaceIdentifier_info_2 = NULL;
  ULONG buffer_length = 0;
  int result_3 = -1;
  std::string ip_address_string;

  NET_IFINDEX interface_index =
    Net_Common_Tools::interfaceToIndex (Net_Common_Tools::adapterName (interfaceIdentifier_in));
  if (unlikely (!interface_index))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interfaceIdentifier_in).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    return result;
  } // end IF

  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
  result_2 = GetAdaptersInfo (ip_interfaceIdentifier_info_p, &buffer_length);
  if (unlikely (result_2 != NO_ERROR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p); ip_interfaceIdentifier_info_p = NULL;
    return result;
  } // end IF

  ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_p;
  do
  {
    if (static_cast<DWORD> (interface_index) != ip_interfaceIdentifier_info_2->Index)
      goto continue_;

    ip_address_string =
      ip_interfaceIdentifier_info_2->GatewayList.IpAddress.String;
    ip_address_string += ACE_TEXT_ALWAYS_CHAR (":0");
    result_3 = result.set (ip_address_string.c_str (), AF_INET);
    if (unlikely (result_3 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(%s): \"%m\", aborting\n"),
                  ACE_TEXT (ip_address_string.c_str ())));
      ACE_FREE_FUNC (ip_interfaceIdentifier_info_p); ip_interfaceIdentifier_info_p = NULL;
      return result;
    } // end IF
    break;

continue_:
    ip_interfaceIdentifier_info_2 = ip_interfaceIdentifier_info_2->Next;
  } while (ip_interfaceIdentifier_info_2);

  // clean up
  if (ip_interfaceIdentifier_info_p)
    ACE_FREE_FUNC (ip_interfaceIdentifier_info_p);
#else
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

  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char [BUFSIZ]));
  struct nlmsghdr* nl_message_header_p =
      reinterpret_cast<struct nlmsghdr*> (buffer_a);
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
  char* buffer_p = buffer_a;
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

  ACE_OS::memset (buffer_a, 0, sizeof (char [BUFSIZ]));
  // receive reply/ies
  do
  { ACE_ASSERT (received_bytes < BUFSIZ);
    result_2 = ACE_OS::recv (socket_handle,
                             buffer_p,
                             sizeof (char [BUFSIZ]) - received_bytes);
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
  for (nl_message_header_p = reinterpret_cast<struct nlmsghdr*> (buffer_p);
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
          ACE_OS::memset (buffer_2, 0, sizeof (char[IF_NAMESIZE]));
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
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

bool
Net_Common_Tools::interfaceToIPAddress (const std::string& interfaceIdentifier_in,
                                        ACE_INET_Addr& IPAddress_out,
                                        ACE_INET_Addr& gatewayIPAddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interfaceToIPAddress"));

  // initialize return value(s)
  IPAddress_out.reset ();
  gatewayIPAddress_out.reset ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = interfaceIdentifier_in;
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (unlikely (interface_identifier.empty ()))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier = interfaceIdentifier_in;
  if (unlikely (interface_identifier.empty ()))
#endif // ACE_WIN32 || ACE_WIN64
    interface_identifier = Net_Common_Tools::getDefaultInterface ();

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (!InlineIsEqualGUID (interface_identifier, GUID_NULL));
#else
  ACE_ASSERT (!interface_identifier.empty ());
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  ACE_ASSERT (!interface_identifier.empty ());
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  NET_IFINDEX interface_index =
    Net_Common_Tools::interfaceToIndex (interface_identifier);
  if (unlikely (!interface_index))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIndex(%s), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    return false;
  } // end IF

  ULONG flags = (GAA_FLAG_INCLUDE_PREFIX             |
                 GAA_FLAG_INCLUDE_WINS_INFO          |
                 GAA_FLAG_INCLUDE_GATEWAYS           |
                 GAA_FLAG_INCLUDE_ALL_INTERFACES     |
                 GAA_FLAG_INCLUDE_ALL_COMPARTMENTS   |
                 GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER);
  IP_ADAPTER_ADDRESSES* ip_adapter_addresses_p = NULL;
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_addresses_p =
    static_cast<IP_ADAPTER_ADDRESSES*> (ACE_MALLOC_FUNC (buffer_length));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
    return false;
  } // end IF

  IP_ADAPTER_ADDRESSES* ip_adapter_addresses_2 = ip_adapter_addresses_p;
  IP_ADAPTER_UNICAST_ADDRESS* unicast_address_p = NULL;
  IP_ADAPTER_UNICAST_ADDRESS* gateway_address_p = NULL;
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
    while (unicast_address_p)
    {
      socket_address_p = &unicast_address_p->Address;
      ACE_ASSERT (socket_address_p->lpSockaddr);
      if (socket_address_p->lpSockaddr->sa_family == AF_INET)
        break;
      unicast_address_p = unicast_address_p->Next;
    } // end WHILE
    if (unlikely (!unicast_address_p))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("adapter \"%s\":\"%s\" does not currently have any unicast IPv4 address, aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
                  ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description)));
      ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
      return false;
    } // end IF
    sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
    result = IPAddress_out.set (sockaddr_in_p,
                                socket_address_p->iSockaddrLength);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
      ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
      return false;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
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
      ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
      return false;
    } // end IF
#else
    gatewayIPAddress_out =
      Net_Common_Tools::getGateway (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName)));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    break;

continue_:
    ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
  } while (ip_adapter_addresses_2);

  // clean up
  ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
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
    if (ACE_OS::strcmp (interface_identifier.c_str (),
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
      ::freeifaddrs (ifaddrs_p); ifaddrs_p = NULL;
      return false;
    } // end IF
    break;
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p); ifaddrs_p = NULL;
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */

  gatewayIPAddress_out = Net_Common_Tools::getGateway (interfaceIdentifier_in);
#endif // ACE_WIN32 || ACE_WIN64

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

std::string
Net_Common_Tools::IPAddressToInterface (const ACE_INET_Addr& IPAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::IPAddressToInterface"));

  // initialize return value(s)
  std::string result;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (Net_Common_Tools::isLocal (IPAddress_in))
  {
    IP_ADAPTER_ADDRESSES* ip_adapter_addresses_p = NULL;
    ULONG buffer_length = 0;
    ULONG result_2 =
      GetAdaptersAddresses (AF_UNSPEC,              // Family
                            0,                      // Flags
                            NULL,                   // Reserved
                            ip_adapter_addresses_p, // AdapterAddresses
                            &buffer_length);        // SizePointer
    if (unlikely (result_2 != ERROR_BUFFER_OVERFLOW))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result;
    } // end IF
    ACE_ASSERT (buffer_length);
    ip_adapter_addresses_p =
      static_cast<IP_ADAPTER_ADDRESSES*> (ACE_MALLOC_FUNC (buffer_length));
    if (unlikely (!ip_adapter_addresses_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      return result;
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
      return result;
    } // end IF

    IP_ADAPTER_ADDRESSES* ip_adapter_addresses_2 = ip_adapter_addresses_p;
    IP_ADAPTER_UNICAST_ADDRESS* unicast_address_p = NULL;
    struct _SOCKET_ADDRESS* socket_address_p = NULL;
    struct sockaddr_in* sockaddr_in_p, *sockaddr_in_2 = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ULONG network_mask = 0;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    //union _NET_LUID_LH interface_luid_u;
    //interface_luid_u.Value = 0;
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT (unicast_address_p->OnLinkPrefixLength <= 32);
      network_mask = ~((1 << (32 - unicast_address_p->OnLinkPrefixLength)) - 1);
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("found adapter \"%s\": \"%s\" on network %s\n"),
      //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
      //            ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->Description),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (ACE_INET_Addr (static_cast<u_short> (0),
      //                                                                          static_cast<ACE_UINT32> ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_p->sin_addr.S_un.S_addr) & network_mask
      //                                                                                                                                         : sockaddr_in_p->sin_addr.S_un.S_addr & network_mask)),
      //                                                           true).c_str ())));
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (result);
      ACE_NOTREACHED (return result;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      sockaddr_in_p = (struct sockaddr_in*)socket_address_p->lpSockaddr;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_p->sin_addr.S_un.S_addr) & network_mask
                                                 : sockaddr_in_p->sin_addr.S_un.S_addr & network_mask) !=
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (sockaddr_in_2->sin_addr.S_un.S_addr) & network_mask
                                                 : sockaddr_in_2->sin_addr.S_un.S_addr & network_mask))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        goto continue_;

      result =
        Net_Common_Tools::indexToInterface (ip_adapter_addresses_2->IfIndex);
      if (unlikely (result.empty ()))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                    ip_adapter_addresses_2->IfIndex));

      break;

continue_:
      ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
    } while (ip_adapter_addresses_2);

    // clean up
    ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;
  } // end IF
  else
  {
    DWORD result_2 = NO_ERROR;
    struct _MIB_IPFORWARDROW route_s;

    ACE_OS::memset (&route_s, 0, sizeof (struct _MIB_IPFORWARDROW));
    result_2 = GetBestRoute (IPAddress_in.get_ip_address (),
                             0,
                             &route_s);
    if (unlikely (result_2 != NO_ERROR))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetBestRoute(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (IPAddress_in).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result;
    } // end IF

    result =
      Net_Common_Tools::indexToInterface (route_s.dwForwardIfIndex);
    if (unlikely (result.empty ()))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                  route_s.dwForwardIfIndex));
  } // end ELSE
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

    result = ifaddrs_2->ifa_name;
    break;
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return result;)
#endif /* ACE_HAS_GETIFADDRS */
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

std::string
Net_Common_Tools::getDefaultInterface (enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultInterface"));

  std::string result;

  std::string interface_identifier;
  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
    case NET_LINKLAYER_PPP:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      IP_ADAPTER_ADDRESSES* ip_adapter_addresses_p = NULL, *ip_adapter_addresses_2 = NULL;
      ULONG buffer_length = 0;
      ULONG result_2 = 0;
      std::map<ULONG, std::string> connected_interfaces;
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return result;
      } // end IF
      ACE_ASSERT (buffer_length);
      ip_adapter_addresses_p =
        static_cast<IP_ADAPTER_ADDRESSES*> (ACE_MALLOC_FUNC (buffer_length));
      if (unlikely (!ip_adapter_addresses_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
        return result;
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
          Net_Common_Tools::indexToInterface_2 (ip_adapter_addresses_2->IfIndex);
#else
          Net_Common_Tools::indexToInterface (ip_adapter_addresses_2->IfIndex);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        if (unlikely (interface_identifier.empty ()))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::indexToInterface(%u), aborting\n"),
                      ip_adapter_addresses_2->IfIndex));
          goto error;
        } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
        connected_interfaces.insert (std::make_pair (ip_adapter_addresses_2->Ipv4Metric,
                                                     interface_identifier));
#else
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (result);
        ACE_NOTREACHED (return result;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

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
      ACE_FREE_FUNC (ip_adapter_addresses_p); ip_adapter_addresses_p = NULL;

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
                      ACE_TEXT (result.c_str ())));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("default interface: \"%s\" (gateway: %s)\n"),
                    ACE_TEXT (result.c_str ()),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (gateway_address).c_str ())));
#endif // _DEBUG
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
      int exit_status_i = 0;
      if (unlikely (!Common_Tools::command (command_line_string,
                                            exit_status_i,
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
      char buffer_a [BUFSIZ];
      std::string regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^default via ([[:digit:].]+) dev ([[:alnum:]]+)(?:.*)$");
      std::regex regex (regex_string);
      std::cmatch match_results;
      converter.str (ip_route_records_string);
      do
      {
        converter.getline (buffer_a, sizeof (char[BUFSIZ]));
        if (unlikely (!std::regex_match (buffer_a,
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
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    case NET_LINKLAYER_802_11:
    {
      Net_InterfaceIdentifiers_t interface_identifiers_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
      DWORD maximum_client_version =
#if defined (WINXP_SUPPORT)
        WLAN_API_VERSION_1_0;
#pragma message ("compiling for WLAN API version " COMMON_STRINGIZE(WLAN_API_VERSION_1_0))
#else
        WLAN_API_VERSION; // use SDK-native
#pragma message ("compiling for WLAN API version " COMMON_STRINGIZE(WLAN_API_VERSION))
#endif // WINXP_SUPPORT
      DWORD negotiated_version = 0;
      HANDLE client_handle = ACE_INVALID_HANDLE;
      DWORD result_2 = WlanOpenHandle (maximum_client_version,
                                       NULL,
                                       &negotiated_version,
                                       &client_handle);
      if (unlikely (result_2 != ERROR_SUCCESS))
      { // *NOTE*: most likely reason: ERROR_SERVICE_NOT_ACTIVE (1062): Wireless
        //         Zero Configuration (WZC) service has not been started
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::WlanOpenHandle(%u): \"%s\", aborting\n"),
                    maximum_client_version,
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
      ACE_ASSERT (interface_list_p);
      for (DWORD i = 0;
           i < interface_list_p->dwNumberOfItems;
           ++i)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
        interface_identifiers_a.push_back (interface_list_p->InterfaceInfo[i].InterfaceGuid);
#else
        interface_identifiers_a.push_back (Net_Common_Tools::indexToInterface (Net_Common_Tools::interfaceToIndex_2 (interface_list_p->InterfaceInfo[i].InterfaceGuid)));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      WlanFreeMemory (interface_list_p); interface_list_p = NULL;

error_2:
      result_2 = WlanCloseHandle (client_handle,
                                  NULL);
      if (unlikely (result_2 != ERROR_SUCCESS))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
#endif // WLANAPI_SUPPORT
#else
#if defined (NL80211_SUPPORT)
      Net_WLAN_IInetMonitor_t* wlan_inet_monitor_p =
          NET_WLAN_INETNL80211MONITOR_SINGLETON::instance ();
      ACE_ASSERT (wlan_inet_monitor_p);
#endif // NL80211_SUPPORT

#if defined (ACE_HAS_GETIFADDRS)
      struct ifaddrs* ifaddrs_p = NULL;
      int result_2 = ::getifaddrs (&ifaddrs_p);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
        goto clean;
      } // end IF
      ACE_ASSERT (ifaddrs_p);

      for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
           ifaddrs_2;
           ifaddrs_2 = ifaddrs_2->ifa_next)
      {
        if (Net_WLAN_Tools::isInterface (ifaddrs_2->ifa_name))
          interface_identifiers_a.push_back (ifaddrs_2->ifa_name);
      } // end FOR

clean:
      if (likely (ifaddrs_p))
      {
        ::freeifaddrs (ifaddrs_p); ifaddrs_p = NULL;
      } // end IF
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_NOTREACHED (return ACE_TEXT_ALWAYS_CHAR ("");)
#endif /* ACE_HAS_GETIFADDRS */
#endif // ACE_WIN32 || ACE_WIN64
      if (likely (!interface_identifiers_a.empty ()))
        return interface_identifiers_a.front ();
      break;
    }
    case NET_LINKLAYER_ATM:
    case NET_LINKLAYER_FDDI:
    {
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR (""));
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

  return ACE_TEXT_ALWAYS_CHAR ("");
}

std::string
Net_Common_Tools::getDefaultInterface (int linkLayerType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultInterface"));

  // step1: retrieve 'default' device for each link layer type specified
  Net_InterfaceIdentifiers_t interfaces_a;
  std::string interface_identifier;
  for (enum Net_LinkLayerType i = NET_LINKLAYER_ATM;
       i < NET_LINKLAYER_MAX;
       ++i)
    if (linkLayerType_in & i)
    {
      interface_identifier = Net_Common_Tools::getDefaultInterface (i);
      if (unlikely (interface_identifier.empty ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::getDefaultInterface() (type was: \"%s\"), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::LinkLayerTypeToString (i).c_str ())));
        continue;
      } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      interfaces_a.push_back (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (interface_identifier)));
#else
      interfaces_a.push_back (interface_identifier);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      interfaces_a.push_back (interface_identifier);
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF

  if (likely (!interfaces_a.empty ()))
    return interfaces_a.front ();

  return ACE_TEXT_ALWAYS_CHAR ("");
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
  ACE_TCHAR buffer_a[HOST_NAME_MAX];
  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[HOST_NAME_MAX]));

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

    result = inet_address.get_host_name (buffer_a, sizeof (ACE_TCHAR[HOST_NAME_MAX]));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::get_host_name(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (dottedDecimal_inout.c_str ())));
      return false;
    } // end IF

    dottedDecimal_inout = buffer_a;
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

    const char* result_p = inet_address.get_host_addr (buffer_a, sizeof (ACE_TCHAR[HOST_NAME_MAX]));
    if (unlikely (!result_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostName_inout.c_str ())));
      return false;
    } // end IF

    dottedDecimal_inout = buffer_a;
  } // end ELSE

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
#endif // ACE_WIN32 || ACE_WIN64

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
    if (error != ENOTCONN) // 107
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::getsockopt(%d,IP_MTU): \"%m\", aborting\n"),
                  handle_in));
    return 0;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
    return false;
  } // end IF

  if (unlikely (optval != size_in))
  {
    // *NOTE*: (for some reason,) Linux will actually set twice the given size
#if defined (ACE_LINUX)
    if (likely (optval == (size_in * 2)))
      return true;
#endif // ACE_LINUX

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
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEADDR): \"%m\", aborting\n"),
                handle_in));
#endif // ACE_WIN32 || ACE_WIN64
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
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEPORT): \"%m\", aborting\n"),
                  handle_in));
      return false;
    } // end IF
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // ACE_LINUX

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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("dispatched %u byte(s): %s --> %s\n"),
              bytes_to_send,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_in).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteSAP_in).c_str ())));
#endif // _DEBUG
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
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
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
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("enabled SIO_LOOPBACK_FAST_PATH on 0x%@\n"),
              handle_in));

  return true;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#endif // ACE_WIN32 || ACE_WIN64

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
//  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (!match_results.empty ());
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
Net_Common_Tools::isNetworkManagerManagingInterface (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::isNetworkManagerManagingInterface"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  if (!Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR (NET_EXE_NETWORKMANAGER_STRING)))
    return false; // *TODO*: avoid false negatives

#if defined (DBUS_SUPPORT)
//  // sanity check(s)
//  if (!Common_DBus_Tools::isUnitRunning (NULL,
//                                         COMMON_SYSTEMD_UNIT_NETWORKMANAGER))
//    return false; // *TODO*: avoid false negatives

  struct DBusError error_s;
  dbus_error_init (&error_s);
  struct DBusConnection* connection_p = NULL;
  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  const char* argument_string_p = NULL;
  dbus_bool_t value_b = 0;

  connection_p = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                       &error_s);
  if (unlikely (!connection_p ||
                dbus_error_is_set (&error_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", aborting\n"),
                ACE_TEXT (error_s.message)));

    dbus_error_free (&error_s);

    return false; // *TODO* avoid false negatives
  } // end IF
  dbus_connection_set_exit_on_disconnect (connection_p,
                                          false);

  std::string device_object_path_string =
      Net_WLAN_Tools::deviceToDBusObjectPath (connection_p,
                                              interfaceIdentifier_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusObjectPath(\"%s\"), aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean; // *TODO* avoid false negatives
  } // end IF

  message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Managed");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_p,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_VARIANT)))
    goto clean; // *TODO* avoid false negatives
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_BOOLEAN);
  dbus_message_iter_get_basic (&iterator_2, &value_b);
  dbus_message_unref (reply_p); reply_p = NULL;

  result = value_b;

clean:
  if (connection_p)
  {
    dbus_connection_close (connection_p);
    dbus_connection_unref (connection_p);
  } // end IF
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif // DBUS_SUPPORT

  return result;
}

bool
Net_Common_Tools::networkManagerManageInterface (const std::string& interfaceIdentifier_in,
                                                 bool toggle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::networkManagerManageInterface"));

  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/NetworkManager/NetworkManager.conf");
  ACE_Configuration_Heap configuration;
  ACE_Ini_ImpExp ini_importer (configuration);
  int result_2 = -1;
  ACE_TString sub_section_string, value_name_string, value_string;
  int index = 0, index_2 = 0;
  const ACE_Configuration_Section_Key& root_section =
      configuration.root_section ();
  ACE_Configuration_Section_Key sub_section_key;
  enum ACE_Configuration::VALUETYPE value_type_e;
  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:(.+),)*(.*)$");
  std::string regex_string_2 =
      ACE_TEXT_ALWAYS_CHAR ("^(mac:(?:([01234567890abcdef]{2}):){5}[01234567890abcdef]{2}|interface-name:[^;]+;)*(mac:(?:([01234567890abcdef]{2}):){5}[01234567890abcdef]{2}|interface-name:.+)?$");
  std::regex regex;
  std::smatch smatch;
  std::string value_string_2, value_string_3;
  std::string::size_type position = std::string::npos;
  std::smatch::iterator iterator;
  bool section_found_b = false, value_found_b = false, modified_b = false;
  bool is_managing_interface_b =
      Net_Common_Tools::isNetworkManagerManagingInterface (interfaceIdentifier_in);

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  if ((toggle_in && is_managing_interface_b))// ||
//      (!toggle_in && !is_managing_interface_b))
    return true; // nothing to do
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  result_2 = configuration.open (ACE_DEFAULT_CONFIG_SECTION_SIZE);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open(%d): \"%m\", aborting\n"),
                ACE_DEFAULT_CONFIG_SECTION_SIZE));
    return false;
  } // end IF
  result_2 =
      ini_importer.import_config (ACE_TEXT (configuration_file_path.c_str ()));
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Ini_ImpExp::import_config(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  // step1: make sure the 'keyfile' plugin is loaded
  try {
    regex.assign (regex_string,
                  std::regex::ECMAScript);
  } catch (std::regex_error& error) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (regex_string.c_str ()),
                ACE_TEXT (error.what ())));
    return false;
  }
  do
  {
    result_2 = configuration.enumerate_sections (root_section,
                                                 index,
                                                 sub_section_string);
    if (result_2)
      break;
    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
                        ACE_TEXT_ALWAYS_CHAR ("main")))
      goto next;

    section_found_b = true;
    result_2 = configuration.open_section (root_section,
                                           sub_section_string.c_str (),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    do
    {
      result_2 = configuration.enumerate_values (sub_section_key,
                                                 index_2,
                                                 value_name_string,
                                                 value_type_e);
      if (result_2)
        break;
      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
                          ACE_TEXT_ALWAYS_CHAR ("plugins")))
        goto next_2;

      value_found_b = true;
      result_2 = configuration.get_string_value (sub_section_key,
                                                 value_name_string.c_str (),
                                                 value_string);
      ACE_ASSERT (result_2 == 0);
      value_string_2 = value_string.c_str ();
      if (!std::regex_match (value_string_2,
                             smatch,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
                    ACE_TEXT (configuration_file_path.c_str ())));
        return false;
      } // end IF
      for (iterator = ++smatch.begin ();
           iterator != smatch.end ();
           ++iterator)
      { ACE_ASSERT ((*iterator).matched);
        value_string_2 = (*iterator).str ();
        if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                             value_string_2.c_str ()))
          break;
      } // end FOR
      if ((iterator == smatch.end ()) &&
          toggle_in)
      {
        if (!value_string.empty ())
          value_string += ACE_TEXT_ALWAYS_CHAR (",");
        value_string += ACE_TEXT_ALWAYS_CHAR ("keyfile");
        result_2 = configuration.set_string_value (sub_section_key,
                                                   value_name_string.c_str (),
                                                   value_string);
        ACE_ASSERT (result_2 == 0);
        modified_b = true;
      } // end IF

next_2:
      ++index_2;
    } while (true);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
                  sub_section_string.c_str ()));
      return false;
    } // end IF

    break;

next:
    ++index;
  } while (true);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!section_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("main")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("main"),
                                           1,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end IF
  else if (unlikely (!value_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("main"),
                ACE_TEXT ("plugins")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("main"),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end ELSE IF

  // step2: add/remove 'unmanaged-devices' entry to/from the 'keyfile' section
  index = 0, index_2 = 0;
  section_found_b = false, value_found_b = false;
  try {
    regex.assign (regex_string_2,
                  std::regex::ECMAScript);
  } catch (std::regex_error& error) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (regex_string_2.c_str ()),
                ACE_TEXT (error.what ())));
    return false;
  }
  do
  {
    result_2 = configuration.enumerate_sections (root_section,
                                                 index,
                                                 sub_section_string);
    if (result_2)
      break;
    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
                        ACE_TEXT_ALWAYS_CHAR ("keyfile")))
      goto next_3;

    section_found_b = true;
    result_2 = configuration.open_section (root_section,
                                           sub_section_string.c_str (),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    do
    {
      result_2 = configuration.enumerate_values (sub_section_key,
                                                 index_2,
                                                 value_name_string,
                                                 value_type_e);
      if (result_2)
        break;
      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
                          ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices")))
        goto next_4;

      value_found_b = true;
      result_2 = configuration.get_string_value (sub_section_key,
                                                 value_name_string.c_str (),
                                                 value_string);
      ACE_ASSERT (result_2 == 0);
      value_string_2 = value_string.c_str ();
      if (!std::regex_match (value_string_2,
                             smatch,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
                    ACE_TEXT (configuration_file_path.c_str ())));
        return false;
      } // end IF
      for (iterator = ++smatch.begin ();
           iterator != smatch.end ();
           ++iterator)
      {
        if (!(*iterator).matched)
          continue;
        value_string_2 = (*iterator).str ();
        position = value_string_2.find (':', 0);
        ACE_ASSERT (position != std::string::npos);
        if (position == 3) // mac:
        {
          struct ether_addr ether_addr_s =
              Net_Common_Tools::stringToLinkLayerAddress (value_string_2);
          value_string_2 =
              Net_Common_Tools::linkLayerAddressToInterface (ether_addr_s);
          ACE_ASSERT (!value_string_2.empty ());
        } // end IF
        else
        { ACE_ASSERT (position == 14); // interface-name:
          value_string_2.erase (0, position + 1);
        } // end ELSE
        if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                             value_string_2.c_str ()))
          break;
      } // end FOR
      if (toggle_in)
      {
        if (iterator != smatch.end ())
        {
          value_string.clear ();
          for (std::smatch::iterator iterator_2  = ++smatch.begin ();
               iterator_2 != smatch.end ();
               ++iterator_2)
          {
            if (!(*iterator_2).matched ||
                (iterator_2 == iterator))
              continue;
            if (iterator != ++smatch.begin ())
              value_string += ACE_TEXT_ALWAYS_CHAR (";");
            value_string_3 = (*iterator_2).str ();
            value_string += value_string_3.c_str ();
          } // end FOR

          result_2 =
              configuration.set_string_value (sub_section_key,
                                              ACE_TEXT ("unmanaged-devices"),
                                              value_string);
          ACE_ASSERT (result_2 == 0);
          modified_b = true;
        } // end IF
      } // end IF
      else
      {
        if (iterator == smatch.end ())
        {
          if (!value_string.empty ())
            value_string += ACE_TEXT_ALWAYS_CHAR (";");
          value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
          value_string += interfaceIdentifier_in.c_str ();
          result_2 =
              configuration.set_string_value (sub_section_key,
                                              ACE_TEXT ("unmanaged-devices"),
                                              value_string);
          ACE_ASSERT (result_2 == 0);
          modified_b = true;
        } // end IF
      } // end ELSE

next_4:
      ++index_2;
    } while (true);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
                  sub_section_string.c_str ()));
      return false;
    } // end IF

next_3:
    ++index;
  } while (true);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!section_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("keyfile")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                                           1,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    if (toggle_in)
      value_string.clear ();
    else
    {
      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
      value_string += interfaceIdentifier_in.c_str ();
    } // end ELSE
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
                                        value_string);
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end IF
  else if (unlikely (!value_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("keyfile"),
                ACE_TEXT ("unmanaged-devices")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    if (toggle_in)
      value_string.clear ();
    else
    {
      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
      value_string += interfaceIdentifier_in.c_str ();
    } // end ELSE
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
                                        value_string);
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end ELSE IF

  if (modified_b)
  {
    bool drop_capabilities = false;
    bool drop_privileges = false;
    if (!Common_File_Tools::canWrite (configuration_file_path, static_cast<uid_t> (-1)))
    {
      if (Common_Tools::canCapability (CAP_DAC_OVERRIDE))
        drop_capabilities = Common_Tools::setCapability (CAP_DAC_OVERRIDE,
                                                         CAP_EFFECTIVE);
      else if (Common_Tools::switchUser (0)) // try seteuid()
        drop_privileges = true;
    } // end IF

    if (unlikely (!Common_File_Tools::backup (configuration_file_path)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::backup(\"\"), continuing\n"),
                  ACE_TEXT (configuration_file_path.c_str ())));

    result_2 =
        ini_importer.export_config (ACE_TEXT (configuration_file_path.c_str ()));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Ini_ImpExp::export_config(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (configuration_file_path.c_str ())));
      if (drop_capabilities)
        Common_Tools::dropCapability (CAP_DAC_OVERRIDE);
      else if (drop_privileges)
        Common_Tools::switchUser (static_cast<uid_t> (-1));
      return false;
    } // end IF
    if (drop_capabilities)
      Common_Tools::dropCapability (CAP_DAC_OVERRIDE);
    else if (drop_privileges)
      Common_Tools::switchUser (static_cast<uid_t> (-1));
  } // end IF

  return true;
}

bool
Net_Common_Tools::isIfUpDownManagingInterface (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::isIfUpDownManagingInterface"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/network/interfaces");
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  std::map<std::string, std::string> logical_to_physical_interfaces_m;
  std::map<std::string, std::string>::const_iterator map_iterator;
  Net_InterfaceIdentifiers_t managed_interface_identifiers_a,
      unmanaged_interface_identifiers_a;
  unsigned char* data_p = NULL;
  unsigned int file_size_i = 0;
  if (unlikely (!Common_File_Tools::load (configuration_file_path,
                                          data_p,
                                          file_size_i)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p);

  std::istringstream converter (reinterpret_cast<char*> (data_p),
                                std::ios_base::in);
  char buffer_a [BUFSIZ];
  std::string regex_string_stanza_1st =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(iface)|(mapping)|(auto)|(no-auto-down)|(no-scripts)|(allow-)|(source)|(source-directory) )(.+)$");
  std::string regex_string_iface =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(address)|(gateway)|(up) )(.+)$");
  std::string regex_string_mapping =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(map)|(script) )(.+)$");
  std::string regex_string_mapping_2 =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(.+)(?: (.+))*$");
  std::regex regex (regex_string_stanza_1st);
  std::smatch match_results;
  std::string buffer_string;
  int index_i = 0;
  bool is_comment_b = false;
  bool is_iface_b = false;
  std::string mapped_interface_identifier_string;
  bool is_mapping_b = false, is_mapping_2 = false;
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_string = buffer_a;
    if (Common_String_Tools::isspace (buffer_string))
    {
      if (is_mapping_b)
        mapped_interface_identifier_string.clear ();

      is_comment_b = false;
      is_iface_b = false;
      is_mapping_b = false;
      is_mapping_2 = false;
      regex.assign (regex_string_stanza_1st);
      continue;
    } // end IF
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
      continue;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    index_i = 1;
    for (std::smatch::iterator iterator = ++match_results.begin ();
         iterator != match_results.end ();
         ++iterator, ++index_i)
    {
      if (index_i == 1)
      {
        is_comment_b =
            !ACE_OS::strcmp (match_results[1].str ().c_str (),
                             ACE_TEXT_ALWAYS_CHAR ("#"));
        continue;
      } // end IF
      if (is_iface_b)
      {
        if (is_comment_b)
          continue;
        goto iface;
      } // end IF
      else if (is_mapping_b || is_mapping_2)
      {
        if (is_comment_b)
          continue;
        goto map;
      } // end IF

      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("auto")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
      } // end IF
      else if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                                ACE_TEXT_ALWAYS_CHAR ("iface")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        is_iface_b = true;
        regex.assign (regex_string_iface);
      } // end IF
      else if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                                ACE_TEXT_ALWAYS_CHAR ("mapping")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
        {
          mapped_interface_identifier_string =
              match_results[index_i + 1].str ();
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        } // end ELSE
        is_mapping_b = true;
        regex.assign (regex_string_mapping);
      } // end IF
      ++iterator;
      continue;

iface:
      continue;

map:
      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("map")))
      { ACE_ASSERT (is_mapping_b);
        is_mapping_2 = true;
        regex.assign (regex_string_mapping_2);
        continue;
      } // end IF

      if (is_mapping_2)
      { ACE_ASSERT (!mapped_interface_identifier_string.empty ());
        for (std::smatch::iterator iterator_2 = iterator;
             iterator_2 != match_results.end ();
             ++iterator_2)
          logical_to_physical_interfaces_m.insert (std::make_pair ((*iterator_2).str (),
                                                                   mapped_interface_identifier_string));
        is_mapping_2 = false;
      } // end IF
    } // end FOR
  } while (!converter.fail ());

//clean:
  if (data_p)
    delete [] data_p;

  // replace all logical interfaces with their physical interfaces
  for (Net_InterfacesIdentifiersIterator_t iterator = managed_interface_identifiers_a.begin ();
       iterator != managed_interface_identifiers_a.end ();
       ++iterator)
  {
    map_iterator = logical_to_physical_interfaces_m.find (*iterator);
    if (map_iterator == logical_to_physical_interfaces_m.end ())
      continue;

    managed_interface_identifiers_a.push_back ((*map_iterator).second);
    managed_interface_identifiers_a.erase (iterator);
    iterator = managed_interface_identifiers_a.begin ();
  } // end FOR
  for (Net_InterfacesIdentifiersIterator_t iterator = managed_interface_identifiers_a.begin ();
       iterator != managed_interface_identifiers_a.end ();
       ++iterator)
    if (!ACE_OS::strcmp ((*iterator).c_str (),
                         interfaceIdentifier_in.c_str ()))
      return true;

  return false;
}

//bool
//Net_Common_Tools::ifUpDownManageInterface (const std::string& interfaceIdentifier_in,
//                                           bool toggle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::ifUpDownManageInterface"));

//  // sanity check(s)
//  ACE_ASSERT (!interfaceIdentifier_in.empty ());
//  std::string configuration_file_path =
//      ACE_TEXT_ALWAYS_CHAR ("/etc/NetworkManager/NetworkManager.conf");
//  if (unlikely (!Common_File_Tools::isReadable (configuration_file_path)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid NetworkManager configuration file (was: \"%s\"), aborting\n"),
//                ACE_TEXT (configuration_file_path.c_str ())));
//    return false;
//  } // end IF

//  ACE_Configuration_Heap configuration;
//  int result_2 = configuration.open (ACE_DEFAULT_CONFIG_SECTION_SIZE);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration_Heap::open(%d): \"%m\", aborting\n"),
//                ACE_DEFAULT_CONFIG_SECTION_SIZE));
//    return false;
//  } // end IF
//  ACE_Ini_ImpExp ini_importer (configuration);
//  result_2 =
//      ini_importer.import_config (ACE_TEXT (configuration_file_path.c_str ()));
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Ini_ImpExp::import_config(\"%s\"): \"%m\", aborting\n"),
//                ACE_TEXT (configuration_file_path.c_str ())));
//    return false;
//  } // end IF
//  ACE_TString sub_section_string, value_name_string, value_string;
//  int index = 0, index_2 = 0;
//  const ACE_Configuration_Section_Key& root_section =
//      configuration.root_section ();
//  ACE_Configuration_Section_Key sub_section_key;
//  enum ACE_Configuration::VALUETYPE value_type_e;
//  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:(.+),)*(.*)$");
//  std::string regex_string_2 =
//      ACE_TEXT_ALWAYS_CHAR ("^(?:(?:mac:((?:[01234567890abcdef]{2}:){5}[01234567890abcdef]{2})|interface-name:(.+));)*(?:mac:((?:[01234567890abcdef]{2}:){5}[01234567890abcdef]{2})|interface-name:(.+))?$");
//  std::regex regex;
//  std::smatch smatch;
//  std::string value_string_2, value_string_3;
//  std::string::size_type position = std::string::npos;
//  std::smatch::iterator iterator;
//  bool section_found_b = false, value_found_b = false, modified_b = false;

//  // step1: make sure the 'keyfile' plugin is loaded
//  try {
//    regex.assign (regex_string,
//                  std::regex::ECMAScript);
//  } catch (std::regex_error& error) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
//                ACE_TEXT (regex_string.c_str ()),
//                ACE_TEXT (error.what ())));
//    return false;
//  }
//  do
//  {
//    result_2 = configuration.enumerate_sections (root_section,
//                                                 index,
//                                                 sub_section_string);
//    if (result_2)
//      break;
//    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
//                        ACE_TEXT_ALWAYS_CHAR ("main")))
//      goto next;

//    section_found_b = true;
//    result_2 = configuration.open_section (root_section,
//                                           sub_section_string.c_str (),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    do
//    {
//      result_2 = configuration.enumerate_values (sub_section_key,
//                                                 index_2,
//                                                 value_name_string,
//                                                 value_type_e);
//      if (result_2)
//        break;
//      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
//                          ACE_TEXT_ALWAYS_CHAR ("plugins")))
//        goto next_2;

//      value_found_b = true;
//      result_2 = configuration.get_string_value (sub_section_key,
//                                                 value_name_string.c_str (),
//                                                 value_string);
//      ACE_ASSERT (result_2 == 0);
//      value_string_2 = value_string.c_str ();
//      if (!std::regex_match (value_string_2,
//                             smatch,
//                             regex,
//                             std::regex_constants::match_default))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
//                    ACE_TEXT (configuration_file_path.c_str ())));
//        return false;
//      } // end IF
//      for (iterator = ++smatch.begin ();
//           iterator != smatch.end ();
//           ++iterator)
//      { ACE_ASSERT ((*iterator).matched);
//        value_string_2 = (*iterator).str ();
//        if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                             value_string_2.c_str ()))
//          break;
//      } // end FOR
//      if ((iterator == smatch.end ()) &&
//          toggle_in)
//      {
//        if (!value_string.empty ())
//          value_string += ACE_TEXT_ALWAYS_CHAR (",");
//        value_string += ACE_TEXT_ALWAYS_CHAR ("keyfile");
//        result_2 = configuration.set_string_value (sub_section_key,
//                                                   value_name_string.c_str (),
//                                                   value_string);
//        ACE_ASSERT (result_2 == 0);
//        modified_b = true;
//      } // end IF

//next_2:
//      ++index_2;
//    } while (true);
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
//                  sub_section_string.c_str ()));
//      return false;
//    } // end IF

//    break;

//next:
//    ++index;
//  } while (true);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  if (unlikely (!section_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("main")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("main"),
//                                           1,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
//                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end IF
//  else if (unlikely (!value_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("main"),
//                ACE_TEXT ("plugins")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("main"),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
//                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end ELSE IF

//  // step2: add/remove 'unmanaged-devices' entry to/from the 'keyfile' section
//  index = 0, index_2 = 0;
//  section_found_b = false, value_found_b = false;
//  try {
//    regex.assign (regex_string_2,
//                  std::regex::ECMAScript);
//  } catch (std::regex_error& error) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
//                ACE_TEXT (regex_string_2.c_str ()),
//                ACE_TEXT (error.what ())));
//    return false;
//  }
//  do
//  {
//    result_2 = configuration.enumerate_sections (root_section,
//                                                 index,
//                                                 sub_section_string);
//    if (result_2)
//      break;
//    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
//                        ACE_TEXT_ALWAYS_CHAR ("keyfile")))
//      goto next_3;

//    section_found_b = true;
//    result_2 = configuration.open_section (root_section,
//                                           sub_section_string.c_str (),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    do
//    {
//      result_2 = configuration.enumerate_values (sub_section_key,
//                                                 index_2,
//                                                 value_name_string,
//                                                 value_type_e);
//      if (result_2)
//        break;
//      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
//                          ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices")))
//        goto next_4;

//      value_found_b = true;
//      result_2 = configuration.get_string_value (sub_section_key,
//                                                 value_name_string.c_str (),
//                                                 value_string);
//      ACE_ASSERT (result_2 == 0);
//      value_string_2 = value_string.c_str ();
//      if (!std::regex_match (value_string_2,
//                             smatch,
//                             regex,
//                             std::regex_constants::match_default))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
//                    ACE_TEXT (configuration_file_path.c_str ())));
//        return false;
//      } // end IF
//      for (iterator = ++smatch.begin ();
//           iterator != smatch.end ();
//           ++iterator)
//      { ACE_ASSERT ((*iterator).matched);
//        value_string_2 = (*iterator).str ();
//        position = value_string_2.find (':', 0);
//        if (position == 2)
//        {
//          struct ether_addr ether_addr_s =
//              Net_Common_Tools::stringToLinkLayerAddress (value_string_2);
//          value_string_2 =
//              Net_Common_Tools::linkLayerAddressToInterfaceIdentifier (ether_addr_s);
//          ACE_ASSERT (!value_string_2.empty ());
//        } // end IF
//        if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
//                             value_string_2.c_str ()))
//          break;
//      } // end FOR
//      if (toggle_in)
//      {
//        if (iterator != smatch.end ())
//        {
//          value_string.clear ();
//          for (std::smatch::iterator iterator_2  = ++smatch.begin ();
//               iterator_2 != smatch.end ();
//               ++iterator_2)
//          {
//            if (iterator_2 == iterator)
//              continue;
//            if (iterator != ++smatch.begin ())
//              value_string += ACE_TEXT_ALWAYS_CHAR (";");
//            value_string_3 = (*iterator_2).str ();
//            position = value_string_3.find (':', 0);
//            if (position == 2)
//              value_string += ACE_TEXT_ALWAYS_CHAR ("mac:");
//            else
//              value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//            value_string += value_string_3.c_str ();
//          } // end FOR

//          result_2 =
//              configuration.set_string_value (sub_section_key,
//                                              ACE_TEXT ("unmanaged-devices"),
//                                              value_string);
//          ACE_ASSERT (result_2 == 0);
//          modified_b = true;
//        } // end IF
//      } // end IF
//      else
//      {
//        if (iterator == smatch.end ())
//        {
//          if (!value_string.empty ())
//            value_string += ACE_TEXT_ALWAYS_CHAR (";");
//          value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//          value_string += interfaceIdentifier_in.c_str ();
//          result_2 =
//              configuration.set_string_value (sub_section_key,
//                                              ACE_TEXT ("unmanaged-devices"),
//                                              value_string);
//          ACE_ASSERT (result_2 == 0);
//          modified_b = true;
//        } // end IF
//      } // end ELSE

//next_4:
//      ++index_2;
//    } while (true);
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
//                  sub_section_string.c_str ()));
//      return false;
//    } // end IF

//next_3:
//    ++index;
//  } while (true);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  if (unlikely (!section_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("keyfile")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                                           1,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    if (toggle_in)
//      value_string.clear ();
//    else
//    {
//      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//      value_string += interfaceIdentifier_in.c_str ();
//    } // end ELSE
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
//                                        value_string);
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end IF
//  else if (unlikely (!value_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("keyfile"),
//                ACE_TEXT ("unmanaged-devices")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    if (toggle_in)
//      value_string.clear ();
//    else
//    {
//      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//      value_string += interfaceIdentifier_in.c_str ();
//    } // end ELSE
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
//                                        value_string);
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end ELSE IF

//  if (modified_b)
//  {
//    bool drop_privileges = false;
//    if (!Common_File_Tools::canWrite (configuration_file_path, static_cast<uid_t> (-1)))
//      drop_privileges = Common_Tools::switchUser (0);

//    if (unlikely (!Common_File_Tools::backup (configuration_file_path)))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::backup(\"\"), continuing\n"),
//                  ACE_TEXT (configuration_file_path.c_str ())));

//    result_2 =
//        ini_importer.export_config (ACE_TEXT (configuration_file_path.c_str ()));
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Ini_ImpExp::export_config(): \"%m\", aborting\n")));

//      if (drop_privileges)
//        Common_Tools::switchUser (static_cast<uid_t> (-1));

//      return false;
//    } // end IF
//    if (drop_privileges)
//      Common_Tools::switchUser (static_cast<uid_t> (-1));
//  } // end IF

//  return true;
//}
#endif // ACE_LINUX
