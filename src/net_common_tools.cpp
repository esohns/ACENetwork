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

#include <regex>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <iphlpapi.h>
#include <mstcpip.h>
#else
#include <netinet/ether.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <ifaddrs.h>
#endif

#include <ace/Dirent_Selector.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

//////////////////////////////////////////

enum Net_LinkLayerType&
operator++ (enum Net_LinkLayerType& lhs) // prefix-
{
  // roll over ?
  if (lhs == NET_LINKLAYER_MAX)
  {
    lhs = NET_LINKLAYER_ATM;
    return lhs;
  } // end IF

  int result = lhs << 1;
  lhs = static_cast<enum Net_LinkLayerType> (result);
  if (lhs >= NET_LINKLAYER_MAX) lhs = NET_LINKLAYER_MAX;

  return lhs;
}
enum Net_LinkLayerType
operator++ (enum Net_LinkLayerType& lhs, int) // postfix-
{
  enum Net_LinkLayerType result = lhs;
  ++lhs;
  return result;
}

//////////////////////////////////////////

std::string
Net_Common_Tools::IPAddress2String (unsigned short port_in,
                                    ACE_UINT32 IPAddress_in)
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
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
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

ACE_INET_Addr
Net_Common_Tools::string2IPAddress (std::string& address_in)
{
  NETWORK_TRACE ("Net_Common_Tools::string2IPAddress");

  // *NOTE*: ACE_INET_Addr::string_to_address() needs a trailing port number to
  //         function properly (see: ace/INET_Addr.h:237)
  //         --> append one if necessary
  std::string ip_address_string = address_in;
  std::string::size_type position = ip_address_string.find (':', 0);
  if (position == std::string::npos)
    ip_address_string += ACE_TEXT_ALWAYS_CHAR (":0");

  int result = -1;
  ACE_INET_Addr inet_addr;
  result = inet_addr.string_to_addr (ip_address_string.c_str (),
                                     AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(): \"%m\", aborting\n")));
    return ACE_INET_Addr ();
  } // end IF

  return inet_addr;
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
Net_Common_Tools::LinkLayerAddress2String (const unsigned char* const addressDataPtr_in,
                                           enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::LinkLayerAddress2String"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_INVALID");

  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
    case NET_LINKLAYER_802_11:
    {
      char buffer[NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE];
      ACE_OS::memset (&buffer, 0, sizeof (buffer));

      // *IMPORTANT NOTE*: ether_ntoa_r is not portable
      // *TODO*: ether_ntoa_r is not portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      PSTR result_2 =
        RtlEthernetAddressToStringA (reinterpret_cast<const _DL_EUI48* const> (addressDataPtr_in),
                                     buffer);
      if (result_2 != (buffer + NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE - 1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to RtlEthernetAddressToStringA(), aborting\n")));
        return result;
      } // end IF
      //char buffer[BUFSIZ];
      //ACE_OS::memset (buffer, 0, sizeof (buffer));
      //int result_2 =
      //  ACE_OS::sprintf (buffer,
      //                   ACE_TEXT_ALWAYS_CHAR ("%02X:%02X:%02X:%02X:%02X:%02X"),
      //                   addressDataPtr_in[0], addressDataPtr_in[1],
      //                   addressDataPtr_in[2], addressDataPtr_in[3],
      //                   addressDataPtr_in[4], addressDataPtr_in[5]);
      //if (result_2 == -1)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", aborting\n")));
      //  return std::string ();
      //} // end IF
      //result = buffer;
#else
      if (::ether_ntoa_r (reinterpret_cast<const ether_addr*> (addressDataPtr_in),
                          buffer) != buffer)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::ether_ntoa_r(): \"%m\", aborting\n")));
        return result;
      } // end IF
#endif
      result = buffer;

      break;
    }
    case NET_LINKLAYER_PPP:
    {
      // *NOTE*: being point-to-point in nature, PPP does not support (link
      //         layer-) addressing
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("link layer type \"%s\" does not support addressing, continuing\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerType2String (type_in).c_str ())));
      return ACE_TEXT_ALWAYS_CHAR ("");
    }
    case NET_LINKLAYER_ATM:
    case NET_LINKLAYER_FDDI:
    {
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (result);

      ACE_NOTREACHED (break;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown link layer type (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerType2String (type_in).c_str ())));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
Net_Common_Tools::LinkLayerType2String (enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::LinkLayerType2String"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_INVALID");

  switch (type_in)
  {
    case NET_LINKLAYER_ATM:
      return ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_ATM");
    case NET_LINKLAYER_802_3:
      return ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_802_3");
    case NET_LINKLAYER_FDDI:
      return ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_FDDI");
    case NET_LINKLAYER_PPP:
      return ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_PPP");
    case NET_LINKLAYER_802_11:
      return ACE_TEXT_ALWAYS_CHAR ("NET_LINKLAYER_802_11");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown link layer type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

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

bool
Net_Common_Tools::interface2ExternalIPAddress (const std::string& interfaceIdentifier_in,
                                               ACE_INET_Addr& IPAddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interface2ExternalIPAddress"));

  // initialize return value(s)
  IPAddress_out.reset ();

  std::string interface_identifier_string = interfaceIdentifier_in;
  if (interface_identifier_string.empty ())
    interface_identifier_string =
      Net_Common_Tools::getDefaultDeviceIdentifier ();

  // step1: determine the 'internal' IP address
  ACE_INET_Addr internal_ip_address;
  if (!Net_Common_Tools::interface2IPAddress (interface_identifier_string,
                                              internal_ip_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2IPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier_string.c_str ())));
    return false;
  } // end IF

  int result = -1;
  // *TODO*: this should work on most Linux/Windows systems, but is really a bad
  //         idea:
  //         - relies on local 'nslookup' tool
  //         - the 'opendns.com' domain resolution scheme
  //         - temporary files
  //         - system(3) call
  //         --> extremely inefficient; remove ASAP
  std::string filename_string =
      Common_File_Tools::getTempFilename (ACE_TEXT_ALWAYS_CHAR (""));
  std::string command_line_string =
      ACE_TEXT_ALWAYS_CHAR ("nslookup myip.opendns.com. resolver1.opendns.com >> ");
  command_line_string += filename_string;

  result = ACE_OS::system (ACE_TEXT (command_line_string.c_str ()));
//  result = execl ("/bin/sh", "sh", "-c", command, (char *) 0);
  if ((result == -1)      ||
      !WIFEXITED (result) ||
      WEXITSTATUS (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::system(\"%s\"): \"%m\" (result was: %d), aborting\n"),
                ACE_TEXT (command_line_string.c_str ()),
                WEXITSTATUS (result)));
    return false;
  } // end IF
  unsigned char* data_p = NULL;
  if (!Common_File_Tools::load (filename_string,
                                data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (filename_string.c_str ())));
    return false;
  } // end IF
  if (!Common_File_Tools::deleteFile (filename_string))
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
        (ACE_OS::strcmp (match_results[1].str ().c_str (),
                         ACE_TEXT_ALWAYS_CHAR (NET_ADDRESS_NSLOOKUP_RESULT_ADDRESS_KEY_STRING)) == 0))
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
  if (external_ip_address.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to resolve IP address (was: %s), aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddress2String (internal_ip_address).c_str ())));
    return false;
  } // end IF
  IPAddress_out = Net_Common_Tools::string2IPAddress (external_ip_address);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("interface \"%s\" --> %s (--> %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddress2String (internal_ip_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddress2String (IPAddress_out).c_str ())));
  
  return true;
}

bool
Net_Common_Tools::interface2MACAddress (const std::string& interfaceIdentifier_in,
                                        unsigned char MACAddress_out[])
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interface2MACAddress"));

//  // sanity check(s)
//  ACE_ASSERT (sizeof (MACAddress_out) >= 6);

  // initialize return value(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //  ACE_OS::memset (MACAddress_out, 0, sizeof (MACAddress_out));
    ACE_OS::memset (MACAddress_out, 0, 6);

  // sanity check(s)
  //ACE_ASSERT (sizeof (MACAddress_out) >= MAX_ADAPTER_ADDRESS_LENGTH);

  PIP_ADAPTER_INFO ip_adapter_info_p = NULL;
  ULONG buffer_length = 0;
  ULONG result = GetAdaptersInfo (ip_adapter_info_p, &buffer_length);
  if (result != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_info_p =
    static_cast<PIP_ADAPTER_INFO> (ACE_MALLOC_FUNC (buffer_length));
  if (!ip_adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF

  result = GetAdaptersInfo (ip_adapter_info_p,
                            &buffer_length);
  if (result != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_adapter_info_p);

    return false;
  } // end IF

  PIP_ADAPTER_INFO ip_adapter_info_2 = ip_adapter_info_p;
  do
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found network interface: \"%s\"...\n"),
    //            ACE_TEXT (Net_Common_Tools::MACAddress2String (ip_adapter_info_2->Address).c_str ())));

    if (ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                        ip_adapter_info_2->AdapterName))
      goto continue_;

    ACE_OS::memcpy (MACAddress_out, ip_adapter_info_2->Address,
                    6);
    break;

continue_:
    ip_adapter_info_2 = ip_adapter_info_2->Next;
  } while (ip_adapter_info_2);

  // clean up
  ACE_FREE_FUNC (ip_adapter_info_p);
#else
  //  ACE_OS::memset (MACAddress_out, 0, sizeof (MACAddress_out));
  ACE_OS::memset (MACAddress_out, 0, ETH_ALEN);

#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result = ::getifaddrs (&ifaddrs_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  struct sockaddr_ll* sockaddr_ll_p = NULL;
  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_p->ifa_next)
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
    ACE_OS::memcpy (MACAddress_out, sockaddr_ll_p->sll_addr,
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

  return true;
}
bool
Net_Common_Tools::interface2IPAddress (const std::string& interfaceIdentifier_in,
                                       ACE_INET_Addr& IPAddress_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::interface2IPAddress"));

  // initialize return value(s)
  IPAddress_out.reset ();

  std::string interface_identifier_string = interfaceIdentifier_in;
  if (interface_identifier_string.empty ())
    interface_identifier_string =
      Net_Common_Tools::getDefaultDeviceIdentifier ();

//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
  ULONG buffer_length = 0;
  ULONG result =
    GetAdaptersAddresses (AF_UNSPEC,              // Family
                          0,                      // Flags
                          NULL,                   // Reserved
                          ip_adapter_addresses_p, // AdapterAddresses
                          &buffer_length);        // SizePointer
  if (result != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_addresses_p =
    static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
  if (!ip_adapter_addresses_p)
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
  if (result != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_adapter_addresses_p);

    return false;
  } // end IF

  struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 =
    ip_adapter_addresses_p;
  struct _IP_ADAPTER_UNICAST_ADDRESS_LH* unicast_address_p = NULL;
  struct _SOCKET_ADDRESS* socket_address_p = NULL;
  struct sockaddr_in* sockaddr_in_p = NULL;
  do
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found network interface: \"%s\"...\n"),
    //            ACE_TEXT (Net_Common_Tools::MACAddress2String (ip_adapter_info_2->Address).c_str ())));

//    if ((ip_adapter_addresses_2->OperStatus != IfOperStatusUp) ||
//        (!ip_adapter_addresses_2->FirstUnicastAddress))
//      continue;
    if (ACE_OS::strcmp (interface_identifier_string.c_str (),
                        ip_adapter_addresses_2->AdapterName))
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
                  ACE_TEXT ("adapter \"%s:\"\"%s\" does not currently have any unicast IPv4 address, aborting\n"),
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
  if (result == -1)
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

    if (!ifaddrs_2->ifa_addr) continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET) continue;

    sockaddr_in_p = (struct sockaddr_in*)ifaddrs_2->ifa_addr;
    result = IPAddress_out.set (sockaddr_in_p,
                                sizeof (struct sockaddr_in));
    if (result == -1)
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

std::string
Net_Common_Tools::getDefaultDeviceIdentifier (enum Net_LinkLayerType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultDeviceIdentifier"));

  std::string result;

  switch (type_in)
  {
    case NET_LINKLAYER_802_3:
    case NET_LINKLAYER_PPP:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_p = NULL;
      ULONG buffer_length = 0;
      ULONG result_2 =
        GetAdaptersAddresses (AF_UNSPEC,              // Family
                              0,                      // Flags
                              NULL,                   // Reserved
                              ip_adapter_addresses_p, // AdapterAddresses
                              &buffer_length);        // SizePointer
      if (result_2 != ERROR_BUFFER_OVERFLOW)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        return result;
      } // end IF
      ACE_ASSERT (buffer_length);
      ip_adapter_addresses_p =
        static_cast<struct _IP_ADAPTER_ADDRESSES_LH*> (ACE_MALLOC_FUNC (buffer_length));
      if (!ip_adapter_addresses_p)
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
      if (result_2 != NO_ERROR)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::GetAdaptersAddresses(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ACE_FREE_FUNC (ip_adapter_addresses_p);

        return result;
      } // end IF

      // step1: retrieve 'connected' interfaces
      std::map<ULONG, std::string> connected_interfaces;
      struct _IP_ADAPTER_ADDRESSES_LH* ip_adapter_addresses_2 =
        ip_adapter_addresses_p;
      do
      {
        if (type_in == NET_LINKLAYER_802_3)
        {
          if ((ip_adapter_addresses_2->IfType != IF_TYPE_ETHERNET_CSMACD) ||
              (ip_adapter_addresses_2->IfType != IF_TYPE_IS088023_CSMACD))
            goto continue_;
        } // end IF
        else if (ip_adapter_addresses_2->IfType != IF_TYPE_PPP)
          goto continue_;
        if (ip_adapter_addresses_2->OperStatus == IfOperStatusUp)
          connected_interfaces.insert (std::make_pair (ip_adapter_addresses_2->Ipv4Metric,
                                                       ip_adapter_addresses_2->AdapterName));

        if (ip_adapter_addresses_2->FirstUnicastAddress)
        {
          // debug info
          ACE_INET_Addr inet_address;
          Net_Common_Tools::interface2IPAddress (ip_adapter_addresses_2->AdapterName,
                                                 inet_address);
          if (type_in == NET_LINKLAYER_802_3)
          {
            ACE_ASSERT (ip_adapter_addresses_2->PhysicalAddressLength >= ETH_ALEN);
          } // end IF
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("found network interface \"%s\"[%s]: IP#: %s; MAC#: %s...\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (ip_adapter_addresses_2->FriendlyName),
                      ACE_TEXT (ip_adapter_addresses_2->AdapterName),
                      ACE_TEXT (Net_Common_Tools::IPAddress2String (inet_address).c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddress2String (ip_adapter_addresses_2->PhysicalAddress,
                                                                           type_in).c_str ())));
        } // end IF

continue_:
        ip_adapter_addresses_2 = ip_adapter_addresses_2->Next;
      } while (ip_adapter_addresses_2);

      // clean up
      ACE_FREE_FUNC (ip_adapter_addresses_p);

      if (connected_interfaces.empty ())
        return result;

      result = connected_interfaces.begin ()->second;
#else
      // *TODO*: this should work on most Linux systems, but is really a bad idea:
      //         - relies on local 'ip'
      //         - temporary files
      //         - system(3) call
      //         --> extremely inefficient; remove ASAP
      std::string filename_string =
          Common_File_Tools::getTempFilename (ACE_TEXT_ALWAYS_CHAR (""));
      std::string command_line_string = ACE_TEXT_ALWAYS_CHAR ("ip route >> ");
      command_line_string += filename_string;

      int result_2 = ACE_OS::system (ACE_TEXT (command_line_string.c_str ()));
    //  result = execl ("/bin/sh", "sh", "-c", command, (char *) 0);
      if ((result_2 == -1)      ||
          !WIFEXITED (result_2) ||
          WEXITSTATUS (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::system(\"%s\"): \"%m\" (result was: %d), aborting\n"),
                    ACE_TEXT (command_line_string.c_str ()),
                    WEXITSTATUS (result_2)));
        return result;
      } // end IF
      unsigned char* data_p = NULL;
      if (!Common_File_Tools::load (filename_string,
                                    data_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (filename_string.c_str ())));
        return result;
      } // end IF
      if (!Common_File_Tools::deleteFile (filename_string))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::deleteFile(\"%s\"), continuing\n"),
                    ACE_TEXT (filename_string.c_str ())));

      std::string route_record_string = reinterpret_cast<char*> (data_p);
      delete [] data_p;
    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("ip data: \"%s\"\n"),
    //              ACE_TEXT (route_record_string.c_str ())));

      std::istringstream converter;
      char buffer [BUFSIZ];
      std::string regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^default via ([[:digit:].]+) dev ([[:alnum:]]+)(?:.*)$");
      std::regex regex (regex_string);
      std::smatch match_results;
      converter.str (route_record_string);
      do {
        converter.getline (buffer, sizeof (buffer));
        if (!std::regex_match (std::string (buffer),
                               match_results,
                               regex,
                               std::regex_constants::match_default))
          continue;
        ACE_ASSERT (match_results.ready () && !match_results.empty ());

        ACE_ASSERT (match_results[1].matched);
        ACE_ASSERT (match_results[2].matched);
        result = match_results[2];

        break;
      } while (!converter.fail ());
      if (result.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve default interface from route data (was: \"%s\"), aborting\n"),
                    ACE_TEXT (route_record_string.c_str ())));
        return result;
      } // end IF

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("default interface: \"%s\" (gateway: %s)\n"),
                  ACE_TEXT (result.c_str ()),
                  ACE_TEXT (match_results[1].str ().c_str ())));
#endif
      break;
    }
    case NET_LINKLAYER_ATM:
    case NET_LINKLAYER_FDDI:
    case NET_LINKLAYER_802_11:
    {
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (result);

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

  return result;
}
std::string
Net_Common_Tools::getDefaultInterface (int linkLayerType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getDefaultInterface"));

  std::string result;

  // step1: retrieve 'default' device for each link layer type specified
  std::vector<std::string> interfaces;
  std::string interface_identifier;
  for (enum Net_LinkLayerType i = NET_LINKLAYER_ATM;
       i < NET_LINKLAYER_MAX;
       ++i)
    if (linkLayerType_in & i)
    {
      interface_identifier = Net_Common_Tools::getDefaultDeviceIdentifier (i);
      if (interface_identifier.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::getDefaultDeviceIdentifier() (type was: \"%s\"), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::LinkLayerType2String (i).c_str ())));
        continue;
      } // end IF
      interfaces.push_back (interface_identifier);
    } // end IF

  if (!interfaces.empty ())
    result = interfaces.front ();

  return result;
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
  if (result)
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
  if (socket_handle == ACE_INVALID_HANDLE)
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
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  bool do_disassociate = false;
  struct sockaddr* sockaddr_p =
      reinterpret_cast<struct sockaddr*> (destinationAddress_in.get_addr ());
  result_2 = ACE_OS::connect (socket_handle,
                              sockaddr_p,
                              destinationAddress_in.get_addr_size ());
  if (result_2 == -1)
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
  if (result)
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
  if (do_disassociate)
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
  if (result)
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
  if (result)
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
  if (result)
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
  if (result)
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

  if (optval != size_in)
  {
    // *NOTE*: for some reason, Linux will actually set TWICE the size value
    if (Common_Tools::isLinux () && (optval == (size_in * 2)))
      return true;

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
  if (result)
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
  if (result)
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
  if (result)
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
                                bool keepAlive_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::setKeepAlive"));

  int result = -1;
  int optval = (keepAlive_in ? 1 : 0);
  int optlen = sizeof (optval);
  result = ACE_OS::setsockopt (handle_in,
                               SOL_SOCKET,
                               SO_KEEPALIVE,
                               reinterpret_cast<const char*> (&optval),
                               optlen);
  if (result)
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

  // validate result
  optval = 0;
  result = ACE_OS::getsockopt (handle_in,
                               SOL_SOCKET,
                               SO_KEEPALIVE,
                               reinterpret_cast<char*> (&optval),
                               &optlen);
  if (result)
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

  //ACE_DEBUG((LM_DEBUG,
  //           ACE_TEXT("setsockopt(%d,SO_KEEPALIVE): %s\n"),
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
  if (result)
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
  if (result)
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
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::setsockopt(%d,IP_RECVERR): \"%m\", aborting\n"),
                handle_in));
    return false;
  } // end IF

  return true;
}
#endif

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
  if (result)
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

//Net_IInetConnectionManager_t*
//Net_Common_Tools::getConnectionManager ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::getConnectionManager"));

//  return NET_CONNECTIONMANAGER_SINGLETON::instance ();
//}

std::string
Net_Common_Tools::URL2HostName (const std::string& URL_in,
                                bool returnProtocol_in,
                                bool returnPort_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Common_Tools::URL2HostName"));

  std::string result;

  std::string regex_string =
      ACE_TEXT_ALWAYS_CHAR ("^([[:alpha:]]+://)?([^\\/\\:]+)(\\:[[:digit:]]{1,5})?(.+)?$");
  std::regex regex (regex_string);
  std::smatch match_results;
  if (!std::regex_match (URL_in,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URL_in.c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  if (!match_results[2].matched)
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
