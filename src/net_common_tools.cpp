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

#include "net_common_tools.h"

#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mstcpip.h>
#else
#include <netinet/ether.h>
#endif

#include "ace/Dirent_Selector.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

std::string
Net_Common_Tools::IPAddress2String (unsigned short port_in,
                                    unsigned int IPAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::IPAddress2String"));

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
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Inet_Addr::set: \"%m\", aborting\n")));
    return return_value;
  } // end IF
  result = inet_addr.addr_to_string (buffer,
                                     sizeof (buffer),
                                     1); // want IP address, not hostname !
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Inet_Addr::addr_to_string: \"%m\", aborting\n")));
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
    if (last_colon_pos != std::string::npos)
      return_value = return_value.substr (0, last_colon_pos);
  } // end IF

  return return_value;
}

std::string
Net_Common_Tools::IPProtocol2String (unsigned char protocol_in)
{
  NETWORK_TRACE ("Net_Common_Tools::IPProtocol2String");

  // initialize return value(s)
  std::string result;

  switch (protocol_in)
  {
    case IPPROTO_IP: // OR case IPPROTO_HOPOPTS:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IP/IPPROTO_HOPOPTS");
      break;
    case IPPROTO_ICMP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ICMP");
      break;
    case IPPROTO_IGMP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IGMP");
      break;
    case IPPROTO_IPIP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IPIP");
      break;
    case IPPROTO_TCP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_TCP");
      break;
    case IPPROTO_EGP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_EGP");
      break;
    case IPPROTO_PUP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_PUP");
      break;
    case IPPROTO_UDP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_UDP");
      break;
    case IPPROTO_IDP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IDP");
      break;
    case IPPROTO_TP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_TP");
      break;
    case IPPROTO_IPV6:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_IPV6");
      break;
    case IPPROTO_ROUTING:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ROUTING");
      break;
    case IPPROTO_FRAGMENT:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_FRAGMENT");
      break;
    case IPPROTO_RSVP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_RSVP");
      break;
    case IPPROTO_GRE:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_GRE");
      break;
    case IPPROTO_ESP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ESP");
      break;
    case IPPROTO_AH:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_AH");
      break;
    case IPPROTO_ICMPV6:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ICMPV6");
      break;
    case IPPROTO_NONE:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_NONE");
      break;
    case IPPROTO_DSTOPTS:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_DSTOPTS");
      break;
    case IPPROTO_MTP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_MTP");
      break;
    case IPPROTO_ENCAP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_ENCAP");
      break;
    case IPPROTO_PIM:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_PIM");
      break;
    case IPPROTO_COMP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_COMP");
      break;
    case IPPROTO_SCTP:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_SCTP");
      break;
    case IPPROTO_RAW:
      result = ACE_TEXT_ALWAYS_CHAR ("IPPROTO_RAW");
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown IP protocol: %1u, continuing\n"),
                  protocol_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Net_Common_Tools::MACAddress2String (const char* const addressDataPtr_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::MACAddress2String"));

  // initialize return value(s)
  std::string result;

  // convert 6 bytes to ASCII
  // *IMPORTANT NOTE*: ether_ntoa_r is not portable...
  // *TODO*: ether_ntoa_r is not portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_UNUSED_ARG (addressDataPtr_in);
  // *TODO*: implement this
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);
  ACE_NOTREACHED (return result;)
#else
  // *TODO*: make this thread-safe !
  static char address[(ETH_ALEN * 2) + ETH_ALEN + 1]; // "ab:cd:ef:gh:ij:kl\0"
  ACE_OS::memset (&address, 0, sizeof (address));
  if (::ether_ntoa_r (reinterpret_cast<const ether_addr*> (addressDataPtr_in),
                      address) != address)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::ether_ntoa_r(): \"%m\", aborting\n")));
    return result;
  } // end IF

  result = address;
#endif

  return result;
}

std::string
Net_Common_Tools::EthernetProtocolTypeID2String (unsigned short frameType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::EthernetProtocolTypeID2String"));

  // initialize return value(s)
  std::string result;

  switch (ACE_NTOHS (frameType_in))
  {
    case ETH_P_LOOP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_LOOP");
      break;
    case ETHERTYPE_GRE_ISO:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_GRE_ISO");
      break;
    case ETH_P_PUP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PUP");
      break;
    case ETH_P_PUPAT:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PUPAT");
      break;
    case ETHERTYPE_SPRITE:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_SPRITE");
      break;
    case ETH_P_IP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IP");
      break;
    case ETH_P_X25:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_X25");
      break;
    case ETH_P_ARP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ARP");
      break;
    case ETH_P_BPQ:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_BPQ");
      break;
    case ETH_P_IEEEPUP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IEEEPUP");
      break;
    case ETH_P_IEEEPUPAT:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IEEEPUPAT");
      break;
    case ETHERTYPE_NS:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_NS");
      break;
    case ETHERTYPE_TRAIL:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_TRAIL");
      break;
    case ETH_P_DEC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DEC");
      break;
    case ETH_P_DNA_DL:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_DL");
      break;
    case ETH_P_DNA_RC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_RC");
      break;
    case ETH_P_DNA_RT:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DNA_RT");
      break;
    case ETH_P_LAT:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_LAT");
      break;
    case ETH_P_DIAG:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DIAG");
      break;
    case ETH_P_CUST:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_CUST");
      break;
    case ETH_P_SCA:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_SCA");
      break;
    case ETH_P_RARP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_RARP");
      break;
    case ETHERTYPE_LANBRIDGE:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_LANBRIDGE");
      break;
    case ETHERTYPE_DECDNS:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_DECDNS");
      break;
    case ETHERTYPE_DECDTS:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_DECDTS");
      break;
    case ETHERTYPE_VEXP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VEXP");
      break;
    case ETHERTYPE_VPROD:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VPROD");
      break;
    case ETH_P_ATALK:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATALK");
      break;
    case ETH_P_AARP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_AARP");
      break;
    case ETH_P_8021Q:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_8021Q");
      break;
    case ETH_P_IPX:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IPX");
      break;
    case ETH_P_IPV6:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IPV6");
      break;
    case ETHERTYPE_MPCP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_MPCP");
      break;
    case ETHERTYPE_SLOW:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_SLOW");
      break;
    case ETHERTYPE_PPP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_PPP");
      break;
    case ETH_P_WCCP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_WCCP");
      break;
    case ETH_P_MPLS_UC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_MPLS_UC");
      break;
    case ETH_P_MPLS_MC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_MPLS_MC");
      break;
    case ETH_P_ATMMPOA:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATMMPOA");
      break;
    case ETH_P_PPP_DISC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_DISC");
      break;
    case ETH_P_PPP_SES:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_SES");
      break;
    case ETHERTYPE_JUMBO:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_JUMBO");
      break;
    case ETH_P_ATMFATE:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ATMFATE");
      break;
    case ETHERTYPE_EAPOL:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_EAPOL");
      break;
    case ETH_P_AOE:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_AOE");
      break;
    case ETH_P_TIPC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_TIPC");
      break;
    case ETHERTYPE_LOOPBACK:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_LOOPBACK");
      break;
    case ETHERTYPE_VMAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_VMAN");
      break;
    case ETHERTYPE_ISO:
      result = ACE_TEXT_ALWAYS_CHAR ("ETHERTYPE_ISO");
      break;
// ********************* Non DIX types ***********************
    case ETH_P_802_3:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_802_3");
      break;
    case ETH_P_AX25:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_AX25");
      break;
    case ETH_P_ALL:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ALL");
      break;
    case ETH_P_802_2:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_802_2");
      break;
    case ETH_P_SNAP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_SNAP");
      break;
    case ETH_P_DDCMP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_DDCMP");
      break;
    case ETH_P_WAN_PPP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_WAN_PPP");
      break;
    case ETH_P_PPP_MP:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPP_MP");
      break;
    case ETH_P_LOCALTALK:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_LOCALTALK");
      break;
    case ETH_P_PPPTALK:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_PPPTALK");
      break;
    case ETH_P_TR_802_2:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_TR_802_2");
      break;
    case ETH_P_MOBITEX:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_MOBITEX");
      break;
    case ETH_P_CONTROL:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_CONTROL");
      break;
    case ETH_P_IRDA:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_IRDA");
      break;
    case ETH_P_ECONET:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ECONET");
      break;
    case ETH_P_HDLC:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_HDLC");
      break;
    case ETH_P_ARCNET:
      result = ACE_TEXT_ALWAYS_CHAR ("ETH_P_ARCNET");
      break;
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown ethernet frame type: \"0x%x\", continuing\n"),
                  ACE_NTOHS (frameType_in)));

      // IEEE 802.3 ? --> change result string...
      if (ACE_NTOHS (frameType_in) <= ETH_DATA_LEN)
        result = ACE_TEXT_ALWAYS_CHAR ("UNKNOWN_IEEE_802_3_FRAME_TYPE");

      break;
    }
  } // end SWITCH

  return result;
}

// bool
// Net_Common_Tools::selectNetworkInterface(const std::string& defaultInterfaceIdentifier_in,
//                                              std::string& interfaceIdentifier_out)
// {
//   NETWORK_TRACE(ACE_TEXT("Net_Common_Tools::selectNetworkInterface"));
//
//   // init return value(s)
//   interfaceIdentifier_out.resize(0);
//
//   pcap_if_t* all_devices = NULL;
//   char errbuf[PCAP_ERRBUF_SIZE]; // defined in pcap.h
//   if (pcap_findalldevs(&all_devices,
//                        errbuf) == -1)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to pcap_findalldevs(): \"%s\", aborting\n"),
//                errbuf));
//
//     return false;
//   } // end IF
//
//   // found default interface ?
//   unsigned long i = 1;
//   bool found_default = false;
//   pcap_if_t* device = NULL;
//   for (device = all_devices;
//        device;
//        device = device->next, i++)
//   {
//     // debug info
//     ACE_DEBUG((LM_INFO,
//                ACE_TEXT("#%u \"%s\": \"%s\"\n"),
//                i,
//                device->name,
//                (device->description ?
//                 device->description :
//                 ACE_TEXT("no description available"))));
//
//     if (defaultInterfaceIdentifier_in == std::string(device->name))
//     {
//       interfaceIdentifier_out = defaultInterfaceIdentifier_in;
//       found_default = true;
//     } // end IF
//   } // end FOR
//   i--;
//
//   // sanity check: found any suitable device at all ?
//   if (!all_devices)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("no interfaces found, aborting\n")));
//
//     // clean up
//     pcap_freealldevs(all_devices);
//
//     return false;
//   } // end IF
//
//   // couldn't find default interface ? ask user !
//   if (!found_default)
//   {
//     unsigned long device_number = 0;
//     std::cout << ACE_TEXT("default interface \"")
//               << defaultInterfaceIdentifier_in.c_str()
//               << ACE_TEXT("\" not found, please enter a valid interface number (1-")
//               << i
//               << ACE_TEXT("): ");
//     std::cin >> device_number;
//
//     // sanity check: out of range ?
//     if ((device_number < 1) ||
//          (device_number > i))
//     {
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("selection: %u was out of range, aborting\n"),
//                  device_number));
//
//       // clean up
//       pcap_freealldevs(all_devices);
//
//       return false;
//     } // end IF
//
//     // get selected device name
//     for (device = all_devices, i = 0;
//          i < (device_number - 1);
//          device = device->next, i++);
//
//     interfaceIdentifier_out = device->name;
//   } // end IF
//
//   // clean up
//   pcap_freealldevs(all_devices);
//
//   return true;
// }

bool
Net_Common_Tools::getInterfaceIPAddress (const std::string& interfaceIdentifier_in,
                                         std::string& IPaddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getInterfaceIPAddress"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);

  // initialize return value(s)
  IPaddress_out.clear ();

//   // validate/retrieve interface identifier index
//   unsigned long index = 0;
//   // *PORTABILITY*
//   index = ACE_OS::if_nametoindex(interfaceIdentifier_in.c_str());
//   if (!index)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to ACE_OS::if_nametoindex(\"%s\"): \"%s\", aborting\n"),
//                interfaceIdentifier_in.c_str(),
//                ACE_OS::strerror(ACE_OS::last_error())));
  //
//     return false;
//   } // end IF

  size_t count = 0;
  ACE_INET_Addr* addr_array = NULL;
  if (ACE::get_ip_interfaces (count,
                              addr_array))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::get_ip_interfaces(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (count == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("could not find any IP network interfaces, aborting\n")));
    return false;
  } // end IF

  // *NOTE*: beyond this point, clean up addr_array...

  char ip_address[MAXHOSTNAMELEN + 1];
  ACE_OS::memset (&ip_address, 0, sizeof (ip_address));
//   sockaddr_in* addr_handle = NULL;
  for (size_t i = 0;
       i < count;
       i++)
  {
    // reset buffer
    ACE_OS::memset (&ip_address, 0, sizeof (ip_address));

    // reset address handle
//     addr_handle = NULL;

    // sanity check: only support IPv4 (for now)
    if (addr_array[i].get_type () != AF_INET)
    {
      // try next one...
      continue;
    } // end IF

//     // sanity check: ignore loopback
//     if (addr_array[i].is_loopback())
//     {
//       // try next one...
//       continue;
//     } // end IF

    if (addr_array[i].get_host_addr (ip_address,
                                     sizeof (ip_address)) == NULL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(): \"%m\", aborting\n")));

      // clean up
      delete [] addr_array;

      return false;
    } // end IF

    // *TODO*: sanity check: correct interface ?
    // --> implement interface <--> address mapping
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("found host address: \"%s\"...\n"),
//                ip));

//     addr_handle = static_cast<sockaddr_in*> (// addr_array[i].get_addr());

    IPaddress_out = ip_address;
  } // end FOR

  // clean up
  delete [] addr_array;

  return true;
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
  ACE_TCHAR buffer[MAXHOSTNAMELEN];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  if (hostName_inout.empty ())
  {
    // sanity check
    if (dottedDecimal_inout.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid arguments (hostname/address empty), aborting\n")));
      return false;
    } // end IF

    result = inet_address.set (dottedDecimal_inout.c_str (), AF_INET);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (dottedDecimal_inout.c_str ())));
      return false;
    } // end IF

    result = inet_address.get_host_name (buffer, sizeof (buffer));
    if (result == -1)
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
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostName_inout.c_str ())));
      return false;
    } // end IF

    const char* result_p = inet_address.get_host_addr (buffer, sizeof (buffer));
    if (!result_p)
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

bool
Net_Common_Tools::setSocketBuffer (ACE_HANDLE handle_in,
                                   int optname_in,
                                   int size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setSocketBuffer"));

  // sanity check
  if ((optname_in != SO_RCVBUF) &&
      (optname_in != SO_SNDBUF))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid socket option (was: %d), aborting\n"),
                optname_in));
    return false;
  } // end IF

  int optval = size_in;
  int optlen = sizeof (optval);
  if (ACE_OS::setsockopt (handle_in,
                          SOL_SOCKET,
                          optname_in,
                          reinterpret_cast<const char*> (&optval),
                          optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@, %s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d, %s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#endif

    return false;
  } // end IF

  // validate result
  optval = 0;
  if (ACE_OS::getsockopt (handle_in,
                          SOL_SOCKET,
                          optname_in,
                          reinterpret_cast<char*> (&optval),
                          &optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@, %s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, %s): \"%m\", aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF"))));
#endif

    return false;
  } // end IF

  if (optval != size_in)
  {
    // *NOTE*: for some reason, Linux will actually set TWICE the size value...
    if (Common_Tools::isLinux () && (optval == (size_in * 2)))
      return true;

    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_OS::getsockopt(0x%@, %s) returned %d (expected: %d), aborting\n"),
                handle_in,
                ((optname_in == SO_SNDBUF) ? ACE_TEXT ("SO_SNDBUF")
                                           : ACE_TEXT ("SO_RCVBUF")),
                optval,
                size_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_OS::getsockopt(%d, %s) returned %d (expected: %d), aborting\n"),
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

  int optval = 0;
  int optlen = sizeof (optval);
  if (ACE_OS::getsockopt (handle_in,
                          IPPROTO_TCP,
                          TCP_NODELAY,
                          reinterpret_cast<char*> (&optval),
                          &optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%@, TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, TCP_NODELAY): \"%m\", aborting\n"),
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

  int optval = (noDelay_in ? 1 : 0);
  int optlen = sizeof (optval);
  if (ACE_OS::setsockopt (handle_in,
                          IPPROTO_TCP,
                          TCP_NODELAY,
                          reinterpret_cast<const char*> (&optval),
                          optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@, TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d, TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif

    return false;
  } // end IF

  // validate result
  optval = 0;
  if (ACE_OS::getsockopt (handle_in,
                          IPPROTO_TCP,
                          TCP_NODELAY,
                          reinterpret_cast<char*> (&optval),
                          &optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%@, TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, TCP_NODELAY): \"%m\", aborting\n"),
                handle_in));
#endif

    return false;
  } // end IF

  //ACE_DEBUG((LM_DEBUG,
  //           ACE_TEXT("setsockopt(%d, TCP_NODELAY): %s\n"),
  //           handle_in,
  //           (noDelay_in ? ((optval == 1) ? "on" : "off")
  //                       : ((optval == 0) ? "off" : "on"))));

  return (noDelay_in ? (optval == 1) : (optval == 0));
}

bool
Net_Common_Tools::setKeepAlive (ACE_HANDLE handle_in,
                                bool keepAlive_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setKeepAlive"));

  int optval = (keepAlive_in ? 1 : 0);
  int optlen = sizeof (optval);
  if (ACE_OS::setsockopt (handle_in,
                          SOL_SOCKET,
                          SO_KEEPALIVE,
                          reinterpret_cast<const char*> (&optval),
                          optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@, SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d, SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif

    return false;
  } // end IF

  // validate result
  optval = 0;
  if (ACE_OS::getsockopt (handle_in,
                          SOL_SOCKET,
                          SO_KEEPALIVE,
                          reinterpret_cast<char*> (&optval),
                          &optlen))
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%@, SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, SO_KEEPALIVE): \"%m\", aborting\n"),
                handle_in));
#endif

    return false;
  } // end IF

  //ACE_DEBUG((LM_DEBUG,
  //           ACE_TEXT("setsockopt(%d, SO_KEEPALIVE): %s\n"),
  //           handle_in,
  //           (keepAlive_in ? ((optval == 1) ? "on" : "off")
  //                         : ((optval == 0) ? "off" : "on"))));

  return (keepAlive_in ? (optval == 1) : (optval == 0));
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
  if (result == -1)
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%@, SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, SO_LINGER): \"%m\", aborting\n"),
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
  if (result == -1)
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@, SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d, SO_LINGER): \"%m\", aborting\n"),
                handle_in));
#endif
    return false;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("setsockopt(%d, SO_LINGER): %s (%d second(s))\n"),
//              handle_in,
//              (optval.l_onoff ? ACE_TEXT ("on") : ACE_TEXT ("off")),
//              optval.l_linger));

  return (result == 0);
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
  if (result == -1)
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%@, SO_TYPE): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d, SO_TYPE): \"%m\", aborting\n"),
                handle_in));
#endif
    return -1;
  } // end IF

  return optval;
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
  if (result == SOCKET_ERROR)
  {
    DWORD error = ::GetLastError ();
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(0x%@,SIO_LOOPBACK_FAST_PATH): \"%s\", aborting\n"),
                handle_in,
                ACE_TEXT (Common_Tools::error2String (error).c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("enabled SIO_LOOPBACK_FAST_PATH on 0x%@\n"),
              handle_in));

  return true;
}
#endif

unsigned int
Net_Common_Tools::getMaxMsgSize (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getMaxMsgSize"));

  int result = -1;
  int optval = 0;
  int optlen = sizeof (optval);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_MAX_MSG_SIZE,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (result == -1)
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
  if (result == -1)
  {
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(0x%@,IP_MTU): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getsockopt(%d,IP_MTU): \"%m\", aborting\n"),
                handle_in));
#endif
    return 0;
  } // end IF
#endif

  return static_cast<unsigned int> (optval);
}

//Net_IInetConnectionManager_t*
//Net_Common_Tools::getConnectionManager ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getConnectionManager"));

//  return NET_CONNECTIONMANAGER_SINGLETON::instance ();
//}
