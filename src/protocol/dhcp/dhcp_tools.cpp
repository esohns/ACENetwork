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
#include "dhcp_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_SUPPORT)
#include <regex>
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_process_tools.h"
#include "common_tools.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_SUPPORT)
#include "common_file_tools.h"
#include "common_string_tools.h"

#include "common_timer_tools.h"
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "net_common_tools.h"
#include "net_macros.h"

#include "dhcp_codes.h"
#include "dhcp_defines.h"

std::string
DHCP_Tools::dump (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::dump"));

  std::ostringstream converter;
  std::string string_buffer;

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("op: \t");
  string_buffer += DHCP_Tools::OpToString (record_in.op);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("htype: \t");
  converter << static_cast<unsigned int> (record_in.htype);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("hlen: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << static_cast<unsigned int> (record_in.hlen);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("hops: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << static_cast<unsigned int> (record_in.hops);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("xid: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.xid;
  converter << ACE_TEXT_ALWAYS_CHAR (" (0x");
  converter << std::hex << record_in.xid << std::dec;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR (")\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("secs: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.secs;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("flags: \t0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::hex << record_in.flags << std::dec;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("ciaddr: \t");
  string_buffer += Net_Common_Tools::IPAddressToString (0, record_in.ciaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("yiaddr: \t");
  string_buffer += Net_Common_Tools::IPAddressToString (0, record_in.yiaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("siaddr: \t");
  string_buffer += Net_Common_Tools::IPAddressToString (0, record_in.siaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("giaddr: \t");
  string_buffer += Net_Common_Tools::IPAddressToString (0, record_in.giaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("chaddr: \t");
  string_buffer +=
    Net_Common_Tools::LinkLayerAddressToString (record_in.chaddr,
                                                NET_LINKLAYER_802_3); // *TODO*: support other link layers
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("sname: \t\"");
  string_buffer += record_in.sname;
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("file: \t\"");
  string_buffer += record_in.file;
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("options (");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.options.size ();
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  DHCP_Codes::OptionType option_type;
  DHCP_Codes::OptionFieldType field_type;
  for (DHCP_OptionsIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    option_type = static_cast<DHCP_Codes::OptionType> ((*iterator).first);
    field_type =
      DHCP_Tools::OptionToFieldType (option_type);
    string_buffer += DHCP_Tools::OptionToString (option_type);
    string_buffer += ACE_TEXT_ALWAYS_CHAR (" (");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << option_type;
    string_buffer += converter.str ();
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("): \t");
    switch (field_type)
    {
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS:
      {
        // sanity check(s)
        ACE_ASSERT ((*iterator).second.size () % 4 == 0);

        ACE_UINT32 ip_address =
          *reinterpret_cast<const unsigned int*> ((*iterator).second.c_str ());
        if ((*iterator).second.size () == 4)
          string_buffer += Net_Common_Tools::IPAddressToString (0, ip_address);
        else
          for (unsigned int i = 0;
               i < ((*iterator).second.size () / 4);
               ++i)
          {
            ip_address =
              *reinterpret_cast<const unsigned int*> ((*iterator).second.c_str () + (i * 4));
            string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter.clear ();
            converter << ACE_TEXT_ALWAYS_CHAR ("\t#");
            converter << (i + 1);
            string_buffer += converter.str ();
            string_buffer += ACE_TEXT_ALWAYS_CHAR (": ");
            string_buffer += Net_Common_Tools::IPAddressToString (0, ip_address);
          } // end FOR
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_COMMAND:
      {
        DHCP_Codes::MessageType message_type =
            static_cast<DHCP_Codes::MessageType> (*reinterpret_cast<const unsigned char*> ((*iterator).second.c_str ()));
        string_buffer += DHCP_Tools::MessageTypeToString (message_type);
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_STRING:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        string_buffer += (*iterator).second.c_str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER:
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        // *TODO*: support value cardinality here
        switch ((*iterator).second.size ())
        {
          case 1:
            converter <<
              static_cast<unsigned int> (*reinterpret_cast<const unsigned char*> ((*iterator).second.c_str ()));
            break;
          case 2:
          {
            unsigned short value =
                *reinterpret_cast<const unsigned short*> ((*iterator).second.c_str ());
            if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
              value = ACE_SWAP_WORD (value);
            converter << value;
            break;
          }
          case 4:
          {
            unsigned int value =
                *reinterpret_cast<const unsigned int*> ((*iterator).second.c_str ());
            if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
              value = ACE_SWAP_LONG (value);
            converter << value;
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown field size (was: %d), aborting\n"),
                        (*iterator).second.size ()));
            break;
          }
        } // end SWITCH
        string_buffer += converter.str ();
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESSES:
      {
        // sanity check(s)
        ACE_ASSERT ((*iterator).second.size () >= 5);
        unsigned int num_ip_addresses =
          *reinterpret_cast<const unsigned char*> ((*iterator).second.c_str ()) / 4;
        ACE_ASSERT (num_ip_addresses >= 1);

        ACE_UINT32 ip_address =
          *reinterpret_cast<const unsigned int*> ((*iterator).second.c_str () + 1);
        if (num_ip_addresses == 1)
          string_buffer += Net_Common_Tools::IPAddressToString (0, ip_address);
        else
        {
          char* pointer_p =
            const_cast<char*> ((*iterator).second.c_str () + 1);
          unsigned int used_bytes = 1;
next_list:
          num_ip_addresses =
            *reinterpret_cast<const unsigned char*> (pointer_p) / 4;
          ++pointer_p;
          for (unsigned int i = 0;
            i < num_ip_addresses;
            ++i)
          {
            ip_address =
              *reinterpret_cast<const unsigned int*> (pointer_p + (i * 4));
            string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter.clear ();
            converter << ACE_TEXT_ALWAYS_CHAR ("\t#");
            converter << (i + 1);
            string_buffer += converter.str ();
            string_buffer += ACE_TEXT_ALWAYS_CHAR (": ");
            string_buffer += Net_Common_Tools::IPAddressToString (0, ip_address);
          } // end FOR
          pointer_p += num_ip_addresses * 4;
          used_bytes += num_ip_addresses * 4;
          if (used_bytes < (*iterator).second.size ())
            goto next_list;
        } // end ELSE
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_LENGTH:
      {
        unsigned int num_bytes = (*iterator).second.size ();
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown field type (was: %d), aborting\n"),
                    field_type));
        break;
      }
    } // end SWITCH
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end FOR
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("options /END\n");

  return string_buffer;
}

std::string
DHCP_Tools::OpToString (DHCP_Op_t op_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::OpToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (op_in)
  {
    case DHCP_Codes::DHCP_OP_REQUEST:
      result = ACE_TEXT_ALWAYS_CHAR ("BOOTREQUEST"); break;
    case DHCP_Codes::DHCP_OP_REPLY:
      result = ACE_TEXT_ALWAYS_CHAR ("BOOTREPLY"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown op (was: %d), aborting\n"),
                  op_in));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
DHCP_Tools::OptionToString (DHCP_Option_t option_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::OptionToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (option_in)
  {
    //case DHCP_Codes::DHCP_OPTION_PAD:
    //  result = ACE_TEXT_ALWAYS_CHAR ("PAD"); break;
    case DHCP_Codes::DHCP_OPTION_SUBNETMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("SUBNETMASK"); break;
    case DHCP_Codes::DHCP_OPTION_TIMEOFFSET:
      result = ACE_TEXT_ALWAYS_CHAR ("TIMEOFFSET"); break;
    case DHCP_Codes::DHCP_OPTION_GATEWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("GATEWAY"); break;
    case DHCP_Codes::DHCP_OPTION_TIMESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("TIMESERVER"); break;
    case DHCP_Codes::DHCP_OPTION_IEN116NAMESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("IEN116NAMESERVER"); break;
    case DHCP_Codes::DHCP_OPTION_DOMAINNAMESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("DOMAINNAMESERVER"); break;
    case DHCP_Codes::DHCP_OPTION_LOGSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("LOGSERVER"); break;
    case DHCP_Codes::DHCP_OPTION_COOKIESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("COOKIESERVER"); break;
    case DHCP_Codes::DHCP_OPTION_LPRSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("LPRSERVER"); break;
    case DHCP_Codes::DHCP_OPTION_IMPRESSSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("IMPRESSSERVER"); break;
    case DHCP_Codes::DHCP_OPTION_RLPSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("RLPSERVER"); break;
    case DHCP_Codes::DHCP_OPTION_HOSTNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("HOSTNAME"); break;
    //
    case DHCP_Codes::DHCP_OPTION_BOOTFILESIZE:
      result = ACE_TEXT_ALWAYS_CHAR ("BOOTFILESIZE"); break;
    case DHCP_Codes::DHCP_OPTION_MERITDUMPFILE:
      result = ACE_TEXT_ALWAYS_CHAR ("MERITDUMPFILE"); break;
    case DHCP_Codes::DHCP_OPTION_DOMAINNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("DOMAINNAME"); break;
    case DHCP_Codes::DHCP_OPTION_SWAPSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("SWAPSERVER"); break;
    case DHCP_Codes::DHCP_OPTION_ROOTPATH:
      result = ACE_TEXT_ALWAYS_CHAR ("ROOTPATH"); break;
    //
    case DHCP_Codes::DHCP_OPTION_IP_MTU:
      result = ACE_TEXT_ALWAYS_CHAR ("IP_MTU"); break;
    case DHCP_Codes::DHCP_OPTION_IP_BROADCASTADDRESS:
      result = ACE_TEXT_ALWAYS_CHAR ("IP_BROADCASTADDRESS"); break;
    //
    case DHCP_Codes::DHCP_OPTION_NTP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_NTP_SERVER"); break;
    case DHCP_Codes::DHCP_OPTION_VENDORSPECIFICINFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_VENDORSPECIFICINFORMATION"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_IPADDRESSLEASETIME"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_MESSAGETYPE"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_SERVERIDENTIFIER"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_RENEWALT1TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_RENEWALT1TIME"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_REBINDINGT2TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_REBINDINGT2TIME"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_VENDORCLASSIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_VENDORCLASSIDENTIFIER"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_CLIENTIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR("DHCP_CLIENTIDENTIFIER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_NETWAREIP_DOMAINNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("NETWAREIP_DOMAINNAME"); break;
    case DHCP_Codes::DHCP_OPTION_NETWAREIP_INFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("NETWAREIP_INFORMATION"); break;
    //
    case DHCP_Codes::DHCP_OPTION_NISP_DOMAIN:
      result = ACE_TEXT_ALWAYS_CHAR ("NISP_DOMAIN"); break;
    case DHCP_Codes::DHCP_OPTION_NISP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("NISP_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_TFTP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("TFTP_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_BOOTFILE:
      result = ACE_TEXT_ALWAYS_CHAR ("BOOTFILE"); break;
    //
    case DHCP_Codes::DHCP_OPTION_MIP_HOMEAGENT:
      result = ACE_TEXT_ALWAYS_CHAR ("MIP_HOMEAGENT"); break;
    //
    case DHCP_Codes::DHCP_OPTION_SMTP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("SMTP_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_POP3_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("POP3_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_NNTP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("NNTP_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_WWW_DEFAULTSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("WWW_DEFAULTSERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_FINGER_DEFAULTSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("FINGER_DEFAULTSERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_IRC_DEFAULTSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("IRC_DEFAULTSERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_STREETTALK_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("STREETTALK_SERVER"); break;
    case DHCP_Codes::DHCP_OPTION_STREETTALK_DIRECTORYASSISTANCESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("STREETTALK_DIRECTORYASSISTANCESERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_USERCLASS:
      result = ACE_TEXT_ALWAYS_CHAR ("USERCLASS"); break;
    //
    case DHCP_Codes::DHCP_OPTION_SLP_DIRECTORYAGENT:
      result = ACE_TEXT_ALWAYS_CHAR ("SLP_DIRECTORYAGENT"); break;
    case DHCP_Codes::DHCP_OPTION_SLP_SERVICESCOPE:
      result = ACE_TEXT_ALWAYS_CHAR ("SLP_SERVICESCOPE"); break;
    //
    case DHCP_Codes::DHCP_OPTION_RAPIDCOMMIT:
      result = ACE_TEXT_ALWAYS_CHAR ("RAPIDCOMMIT"); break;
    //
    case DHCP_Codes::DHCP_OPTION_CLIENTFQDN:
      result = ACE_TEXT_ALWAYS_CHAR ("CLIENTFQDN"); break;
    //
    case DHCP_Codes::DHCP_OPTION_RELAYAGENTINFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("RELAYAGENTINFORMATION"); break;
    //
    case DHCP_Codes::DHCP_OPTION_ISNS_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("ISNS_SERVER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_NDS_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("NDS_SERVER"); break;
    case DHCP_Codes::DHCP_OPTION_NDS_TREE:
      result = ACE_TEXT_ALWAYS_CHAR ("NDS_TREE"); break;
    case DHCP_Codes::DHCP_OPTION_NDS_CONTEXT:
      result = ACE_TEXT_ALWAYS_CHAR ("NDS_CONTEXT"); break;
    //
    case DHCP_Codes::DHCP_OPTION_BCMCCS_DOMAINNAMELIST:
      result = ACE_TEXT_ALWAYS_CHAR ("BCMCCS_DOMAINNAMELIST"); break;
    case DHCP_Codes::DHCP_OPTION_BCMCCS_IPADDRESS:
      result = ACE_TEXT_ALWAYS_CHAR ("BCMCCS_IPADDRESS"); break;
    //
    case DHCP_Codes::DHCP_OPTION_AUTHENTICATION:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION"); break;
    //
    case DHCP_Codes::DHCP_OPTION_CLIENTLASTTRANSACTIONTIME:
      result = ACE_TEXT_ALWAYS_CHAR ("CLIENTLASTTRANSACTIONTIME"); break;
    case DHCP_Codes::DHCP_OPTION_CLIENTASSOCIATEDIP:
      result = ACE_TEXT_ALWAYS_CHAR ("CLIENTASSOCIATEDIP"); break;
    //
    case DHCP_Codes::DHCP_OPTION_IPXE_CLIENTSYSTEMARCHITECTURETYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("IPXE_CLIENTSYSTEMARCHITECTURETYPE"); break;
    case DHCP_Codes::DHCP_OPTION_IPXE_CLIENTNETWORKINTERFACEIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR ("IPXE_CLIENTNETWORKINTERFACEIDENTIFIER"); break;
    case DHCP_Codes::DHCP_OPTION_IPXE_CLIENTMACHINEIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR ("IPXE_CLIENTMACHINEIDENTIFIER"); break;
    //
    case DHCP_Codes::DHCP_OPTION_CLASSLESSROUTE:
      result = ACE_TEXT_ALWAYS_CHAR ("CLASSLESSROUTE"); break;
    //
    case DHCP_Codes::DHCP_OPTION_GEOCONF:
      result = ACE_TEXT_ALWAYS_CHAR ("GEOCONF"); break;
    //
    case DHCP_Codes::DHCP_OPTION_VENDOR_CLASS:
      result = ACE_TEXT_ALWAYS_CHAR ("VENDOR_CLASS"); break;
    case DHCP_Codes::DHCP_OPTION_VENDOR_SPECIFIC_INFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("VENDOR_SPECIFIC_INFORMATION"); break;
    //
    case DHCP_Codes::DHCP_OPTION_PCP_SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("PCP_SERVER"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown option (was: %d), aborting\n"),
                  option_in));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
DHCP_Tools::MessageTypeToString (DHCP_MessageType_t type_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::MessageTypeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (type_in)
  {
    case DHCP_Codes::DHCP_MESSAGE_DISCOVER:
      result = ACE_TEXT_ALWAYS_CHAR ("DISCOVER"); break;
    case DHCP_Codes::DHCP_MESSAGE_OFFER:
      result = ACE_TEXT_ALWAYS_CHAR ("OFFER"); break;
    case DHCP_Codes::DHCP_MESSAGE_REQUEST:
      result = ACE_TEXT_ALWAYS_CHAR ("REQUEST"); break;
    case DHCP_Codes::DHCP_MESSAGE_DECLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("DECLINE"); break;
    case DHCP_Codes::DHCP_MESSAGE_ACK:
      result = ACE_TEXT_ALWAYS_CHAR ("ACK"); break;
    case DHCP_Codes::DHCP_MESSAGE_NAK:
      result = ACE_TEXT_ALWAYS_CHAR ("NAK"); break;
    case DHCP_Codes::DHCP_MESSAGE_RELEASE:
      result = ACE_TEXT_ALWAYS_CHAR ("RELEASE"); break;
    case DHCP_Codes::DHCP_MESSAGE_INFORM:
      result = ACE_TEXT_ALWAYS_CHAR ("INFORM"); break;
    //
    case DHCP_Codes::DHCP_MESSAGE_FORCERENEW:
      result = ACE_TEXT_ALWAYS_CHAR ("FORCERENEW"); break;
    //
    case DHCP_Codes::DHCP_MESSAGE_LEASEQUERY:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEQUERY"); break;
    case DHCP_Codes::DHCP_MESSAGE_LEASEUNASSIGNED:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEUNASSIGNED"); break;
    case DHCP_Codes::DHCP_MESSAGE_LEASEUNKNOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEUNKNOWN"); break;
    case DHCP_Codes::DHCP_MESSAGE_LEASEACTIVE:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEACTIVE"); break;
    //
    case DHCP_Codes::DHCP_MESSAGE_BULKLEASEQUERY:
      result = ACE_TEXT_ALWAYS_CHAR ("BULKLEASEQUERY"); break;
    case DHCP_Codes::DHCP_MESSAGE_LEASEQUERYDONE:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEQUERYDONE"); break;
    //
    case DHCP_Codes::DHCP_MESSAGE_ACTIVELEASEQUERY:
      result = ACE_TEXT_ALWAYS_CHAR ("ACTIVELEASEQUERY"); break;
    case DHCP_Codes::DHCP_MESSAGE_LEASEQUERYSTATUS:
      result = ACE_TEXT_ALWAYS_CHAR ("LEASEQUERYSTATUS"); break;
    case DHCP_Codes::DHCP_MESSAGE_TLS:
      result = ACE_TEXT_ALWAYS_CHAR ("TLS"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

  return result;
}

DHCP_MessageType_t
DHCP_Tools::MessageTypeToType (const std::string& type_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::MessageTypeToType"));

  // sanity check(s)
  ACE_ASSERT (type_in.size () == 1);

  const char* char_p = type_in.c_str ();

  return static_cast<DHCP_MessageType_t> (*char_p);
}

DHCP_OptionFieldType_t
DHCP_Tools::OptionToFieldType (DHCP_Option_t option_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::OptionToFieldType"));

  switch (option_in)
  {
    case DHCP_Codes::DHCP_OPTION_SUBNETMASK:
    case DHCP_Codes::DHCP_OPTION_GATEWAY:
    case DHCP_Codes::DHCP_OPTION_DOMAINNAMESERVER:
    case DHCP_Codes::DHCP_OPTION_IP_BROADCASTADDRESS:
    case DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER:
    case DHCP_Codes::DHCP_OPTION_NTP_SERVER:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_COMMAND;
    case DHCP_Codes::DHCP_OPTION_TIMEOFFSET:
    case DHCP_Codes::DHCP_OPTION_IP_MTU:
    case DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME:
    case DHCP_Codes::DHCP_OPTION_DHCP_RENEWALT1TIME:
    case DHCP_Codes::DHCP_OPTION_DHCP_REBINDINGT2TIME:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_HOSTNAME:
    case DHCP_Codes::DHCP_OPTION_DOMAINNAME:
    case DHCP_Codes::DHCP_OPTION_VENDORSPECIFICINFORMATION:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_STRING;
    case DHCP_Codes::DHCP_OPTION_VENDOR_SPECIFIC_INFORMATION:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_LENGTH;
    case DHCP_Codes::DHCP_OPTION_PCP_SERVER:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESSES;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown option (was: \"%s\"), aborting\n"),
                  ACE_TEXT (DHCP_Tools::OptionToString (option_in).c_str ())));
      break;
    }
  } // end SWITCH

  return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INVALID;
}

bool
DHCP_Tools::isRequest (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::isRequest"));

  return (record_in.op == DHCP_Codes::DHCP_OP_REQUEST);
}
DHCP_MessageType_t
DHCP_Tools::type (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::type"));

  // initialize result
  DHCP_MessageType_t result = DHCP_Codes::DHCP_MESSAGE_INVALID;

  DHCP_OptionsIterator_t iterator =
      record_in.options.find (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE);
  ACE_ASSERT (iterator != record_in.options.end ());
  result =
      static_cast<DHCP_MessageType_t> (*reinterpret_cast<const unsigned char*> ((*iterator).second.c_str ()));

  return result;
}

unsigned int
DHCP_Tools::generateXID ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::generateXID"));

  return static_cast<unsigned int> (ACE_OS::rand_r (&Common_Tools::randomSeed));
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_USE)
bool
DHCP_Tools::hasDHClientOmapiSupport ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::hasDHClientOmapiSupport"));

  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/dhcp/dhclient.conf");
  unsigned char* data_p = NULL;
  unsigned int file_size_i = 0;

  // sanity check(s)
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false; // *TODO*: avoid false negatives
  } // end IF

  if (unlikely (!Common_File_Tools::load (configuration_file_path,
                                          data_p,
                                          file_size_i)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (data_p);

  std::istringstream converter (reinterpret_cast<char*> (data_p),
                                std::ios_base::in);
  char buffer_a [BUFSIZ];
  std::string buffer_line_string;
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)?(?:[[:space:]]*)(?:(omapi) )(?:[[:space:]]*)(?:(port) )(?:[[:space:]]*)(.+)$");
  std::regex regex (regex_string);
  std::smatch match_results;
  int index_i = 0;
  bool is_comment_b = false;
  bool is_omapi_stanza_b = false, is_omapi_port_stanza_b = false;
  int omapi_port_i = 0;
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_line_string = buffer_a;
    if (Common_String_Tools::isspace (buffer_line_string))
    {
      is_comment_b = false;
      is_omapi_stanza_b = false;
      is_omapi_port_stanza_b = false;
      ACE_UNUSED_ARG (is_omapi_port_stanza_b);
      continue;
    } // end IF
    if (!std::regex_match (buffer_line_string,
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
      if (!(*iterator).matched)
        continue;
      if (index_i == 1) // **TODO*: there may be leading whitespace; this should read (index_i == 'index of first non-whitespace match')
      {
        is_comment_b =
            !ACE_OS::strcmp (match_results[1].str ().c_str (),
                             ACE_TEXT_ALWAYS_CHAR ("#"));
        continue;
      } // end IF

      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("omapi")))
        is_omapi_stanza_b = true;
      else if (is_omapi_stanza_b &&
               !ACE_OS::strcmp ((*iterator).str ().c_str (),
                                ACE_TEXT_ALWAYS_CHAR ("port")))
      {
        is_omapi_port_stanza_b = true;
        ACE_UNUSED_ARG (is_omapi_port_stanza_b);
        if (!is_comment_b &&
            match_results[index_i + 1].matched)
        {
          omapi_port_i =
              ACE_OS::atoi (match_results[index_i + 1].str ().c_str ());
          break; // done
        } // end IF
      } // end ELSE IF
    } // end FOR
    if (omapi_port_i)
      break; // done
  } while (!converter.fail ());

//clean:
  if (data_p)
    delete [] data_p;

  return (omapi_port_i != 0);
}

bool
DHCP_Tools::connectDHClient (const ACE_INET_Addr& address_in,
                             dhcpctl_handle authenticator_in,
                             Net_DHClientCallback_t callback_in,
                             void* userData_in,
                             dhcpctl_handle& connection_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::connectDHClient"));

  // sanity check(s)
  ACE_ASSERT (connection_out == dhcpctl_null_handle);
  if (unlikely (!Common_Process_Tools::id (ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_STRING))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Process_Tools::id(\"%s\"), aborting\n"),
                ACE_TEXT (DHCP_DHCLIENT_STRING)));
    return false;
  } // end IF

  unsigned int retries_i = 0;
  int result = -1;

retry:
  isc_result_t status_i =
      dhcpctl_connect (&connection_out,
                       address_in.get_host_name (),
                       address_in.get_port_number (),
                       authenticator_in);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    if (status_i == ISC_R_NOMORE) // 29: happens intermittently, for reasons yet unknown
    {
      ++retries_i;
      if (retries_i < DHCP_DHCLIENT_CONNECTION_RETRIES)
      {
        ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to ::dhcpctl_connect(%s): \"%s\", retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false).c_str ()),
                  ACE_TEXT (isc_result_totext (status_i))));
        result = ACE_OS::sleep (ACE_Time_Value (1, 0));
        ACE_ASSERT (result != -1);
        goto retry;
      } // end IF
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false).c_str ()),
                ACE_TEXT (isc_result_totext (status_i))));
    return false;
  } // end IF
  ACE_ASSERT (connection_out != dhcpctl_null_handle);

  if (likely (callback_in))
  {
    status_i = dhcpctl_set_callback (connection_out,
                                     userData_in,
                                     callback_in);
    if (unlikely (status_i != ISC_R_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::dhcpctl_set_callback(%@,%@,%@): \"%s\", aborting\n"),
                  connection_out,
                  userData_in,
                  callback_in,
                  ACE_TEXT (isc_result_totext (status_i))));
      DHCP_Tools::disconnectDHClient (connection_out);
//      connection_out = dhcpctl_null_handle;
      return false;
    } // end IF
  } // end IF

  return true;
}

void
DHCP_Tools::disconnectDHClient (dhcpctl_handle& connection_inout)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::disconnectDHClient"));

  // sanity check(s)
  ACE_ASSERT (connection_inout != dhcpctl_null_handle);

  isc_result_t status_i = omapi_disconnect (connection_inout, 1);
  if (unlikely (status_i != ISC_R_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::omapi_disconnect(%@,%d): \"%s\", continuing\n"),
                connection_inout, 1,
                ACE_TEXT (isc_result_totext (status_i))));
  connection_inout = dhcpctl_null_handle;
}

bool
DHCP_Tools::getInterfaceState (dhcpctl_handle connection_in,
                               const std::string& interfaceIdentifier_in,
                               Net_DHClientCallback_t callback_in,
                               void* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::getInterfaceState"));

  // sanity check(s)
  ACE_ASSERT (connection_in != dhcpctl_null_handle);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (callback_in);

  isc_result_t status_i = ISC_R_SUCCESS;
  dhcpctl_handle interface_h = dhcpctl_null_handle;
//  dhcpctl_status client_status = -1;
//  dhcpctl_data_string result_string = NULL;
//  std::string state_string;
  status_i =
      dhcpctl_new_object (&interface_h,
                          connection_in,
                          ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_new_object(%@,%s): \"%s\", aborting\n"),
                connection_in,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    return false;
  } // end IF
  ACE_ASSERT (interface_h != dhcpctl_null_handle);
  status_i =
      dhcpctl_set_string_value (interface_h,
                                ACE_TEXT_ALWAYS_CHAR (interfaceIdentifier_in.c_str ()),
                                ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_NAME_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_string_value(%@,%s,\"%s\"): \"%s\", aborting\n"),
                interface_h,
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_NAME_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&interface_h, MDL);
    return false;
  } // end IF
  status_i = dhcpctl_set_callback (interface_h,
                                   userData_in,
                                   callback_in);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_callback(%@,%@,%@): \"%s\", aborting\n"),
                interface_h,
                userData_in,
                callback_in,
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&interface_h, MDL);
    return false;
  } // end IF
  status_i = dhcpctl_open_object (interface_h,
                                  connection_in,
                                  DHCPCTL_CREATE);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%@,%d): \"%s\", aborting\n"),
                interface_h,
                connection_in,
                DHCPCTL_CREATE,
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&interface_h, MDL);
    return false;
  } // end IF
  // *TODO*: add a timeout here, or use asynchronous operations
//  status_i = dhcpctl_wait_for_completion (interface_h,
//                                          &client_status);
//  if (unlikely (status_i != ISC_R_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(%@): \"%s\", aborting\n"),
//                interface_h,
//                ACE_TEXT (isc_result_totext (status_i))));
//    omapi_object_dereference (&interface_h, MDL);
//    return false;
//  } // end IF
//  if (unlikely (client_status != ISC_R_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%d): \"%s\", aborting\n"),
//                interface_h,
//                DHCPCTL_CREATE,
//                ACE_TEXT (isc_result_totext (client_status))));
//    omapi_object_dereference (&interface_h, MDL);
//    return false;
//  } // end IF

//  status_i =
//      dhcpctl_get_value (&result_string,
//                         interface_h,
//                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING));
//  if (unlikely (status_i != ISC_R_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", aborting\n"),
//                interface_h,
//                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING),
//                ACE_TEXT (isc_result_totext (status_i))));
//    omapi_object_dereference (&interface_h, MDL);
//    return false;
//  } // end IF
//  ACE_ASSERT (result_string);
//  omapi_object_dereference (&interface_h, MDL);
//  interface_h = dhcpctl_null_handle;
//  if (omapi_ds_strcmp (result_string,
//                       state_in.c_str ()))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("\"%s\": invalid interface state (was: \"%s; expected: \"%s\"), returning\n"),
//                ACE_TEXT (interfaceIdentifier_in.c_str ()),
//                ACE_TEXT (std::string (reinterpret_cast<char*> (result_string->value), result_string->len).c_str ()),
//                ACE_TEXT (state_in.c_str ())));
//    dhcpctl_data_string_dereference (&result_string, MDL);
//    return false;
//  } // end IF
//  status_i =
//      dhcpctl_data_string_dereference (&result_string, MDL);
//  ACE_ASSERT (status_i == ISC_R_SUCCESS);

  return true;
}

bool
DHCP_Tools::hasState (dhcpctl_handle handle_in,
                      const std::string& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::hasState"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
  ACE_ASSERT (handle_in != dhcpctl_null_handle);
  ACE_ASSERT (!state_in.empty ());

  dhcpctl_data_string data_string_p = NULL;
  dhcpctl_status status_i =
      dhcpctl_get_value (&data_string_p,
                         handle_in,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", aborting\n"),
                handle_in,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    return false;
  } // end IF
  ACE_ASSERT (data_string_p);
  result = !omapi_ds_strcmp (data_string_p,
                             state_in.c_str ());
  status_i = dhcpctl_data_string_dereference (&data_string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);

  return result;
}

bool
DHCP_Tools::hasActiveLease (const std::string& leasesFilename_in,
                            const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::hasActiveLease"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (Common_File_Tools::isReadable (leasesFilename_in));
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  unsigned char* data_p = NULL;
  unsigned int file_size_i = 0;
  std::string file_content_string;
  std::string::size_type position_i = std::string::npos;
  if (unlikely (!Common_File_Tools::load (leasesFilename_in,
                                          data_p,
                                          file_size_i)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (leasesFilename_in.c_str ())));
    return false;
  } // end IF
  file_content_string = reinterpret_cast<char*> (data_p);
  delete [] data_p; data_p = NULL;

  std::istringstream converter;
  char buffer_a [BUFSIZ];
  std::string buffer_string;
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:  interface \x22)([^\x22]+)(?:\x22;)$");
  std::string regex_string_2 =
    ACE_TEXT_ALWAYS_CHAR ("^(?:  expire [[:digit:]]{1} )(.+)(?:;)$");
  std::regex regex (regex_string);
  std::regex regex_2 (regex_string_2);
  std::smatch match_results;
  ACE_Time_Value timestamp = ACE_Time_Value::zero;

  converter.str (file_content_string);
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_string = buffer_a;
    position_i = buffer_string.find (ACE_TEXT_ALWAYS_CHAR ("lease {"));
    if (position_i                       ||
        (position_i == std::string::npos))
      continue;
next_attribute:
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    if (converter.fail ())
      break;
    buffer_string = buffer_a;
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
      goto next_attribute;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    if (ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                        match_results[1].str ().c_str ()))
      continue;
next_attribute_2:
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    if (converter.fail ())
      break;
    buffer_string = buffer_a;
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex_2,
                           std::regex_constants::match_default))
      goto next_attribute_2;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    timestamp =
        Common_Timer_Tools::stringToTimestamp (match_results[1].str ());
    if (timestamp > COMMON_TIME_NOW_UTC)
    {
      result = true;
      break;
    } // end IF
  } while (!converter.fail ());

  return result;
}
bool
DHCP_Tools::hasActiveLease (dhcpctl_handle connection_in,
                            const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::hasActiveLease"));

  bool result = false;
  struct __omapi_object* lease_p = dhcpctl_null_handle;
  isc_result_t status_i, status_2;
  omapi_data_string_t* string_p = NULL;
  struct ether_addr ether_addr_s;
  ACE_OS::memset (&ether_addr_s, 0, sizeof (struct ether_addr));
  //  ACE_INET_Addr inet_address, inet_address_2;
//  std::string ip_address_string;
#if defined (_DEBUG)
  char buffer_a[INET_ADDRSTRLEN];
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (connection_in != dhcpctl_null_handle);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  ether_addr_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);

  status_i =
      dhcpctl_new_object (&lease_p,
                          connection_in,
                          ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_LEASE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_new_object(%@,%s): \"%s\", aborting\n"),
                connection_in,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_LEASE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    return false; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (lease_p != dhcpctl_null_handle);
//  status_i =
//      dhcpctl_set_string_value (lease_p,
//                                interfaceIdentifier_in.c_str (),
//                                ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING));
//  if (unlikely (status_i != ISC_R_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_set_string_value(%@,\"%s\",\"%s\"): \"%s\", returning\n"),
//                lease_p,
//                ACE_TEXT (interfaceIdentifier_in.c_str ()),
//                ACE_TEXT (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING),
//                ACE_TEXT (isc_result_totext (status_i))));
//    goto clean; // *TODO*: avoid false negatives
//  } // end IF
  status_i =
      dhcpctl_set_data_value (lease_p,
                              reinterpret_cast<char*> (&ether_addr_s.ether_addr_octet),
                              ETH_ALEN,
                              ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_data_value(%@,%s,\"%s\"): \"%s\", returning\n"),
                lease_p,
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  status_i =
      dhcpctl_set_int_value (lease_p,
                             1, // 1: ethernet
                             ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_int_value(%@,%d,\"%s\"): \"%s\", returning\n"),
                lease_p,
                1,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF

  status_i = dhcpctl_open_object (lease_p,
                                  connection_in,
                                  0);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%@,0): \"%s\", returning\n"),
                lease_p,
                connection_in,
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  // *TODO*: add a timeout here, or use asynchronous operations
  status_i = dhcpctl_wait_for_completion (lease_p,
                                          &status_2);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(%@): \"%s\", returning\n"),
                lease_p,
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (status_2 != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%@,0): \"%s\", returning\n"),
                lease_p,
                connection_in,
                ACE_TEXT (isc_result_totext (status_2))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF

  // check lease state
  status_i =
      dhcpctl_get_value (&string_p,
                         lease_p,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", returning\n"),
                lease_p,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (string_p);
//  if (!omapi_ds_strcmp (string_p,
//                        ACE_TEXT_ALWAYS_CHAR ("active")))
//    result = true;
  result = (*reinterpret_cast<unsigned int*> (string_p->value) == 2);
  status_i =
      omapi_data_string_dereference (&string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);

#if defined (_DEBUG)
  string_p = NULL;
  status_i =
      dhcpctl_get_value (&string_p,
                         lease_p,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", returning\n"),
                lease_p,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (string_p);
  ACE_OS::memset (buffer_a, 0, sizeof (char[INET_ADDRSTRLEN]));
  if (unlikely (!ACE_OS::inet_ntop (AF_INET,
                                    string_p->value,
                                    buffer_a,
                                    sizeof (char[INET_ADDRSTRLEN]))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::inet_ntop(%d,%@): \"%m\", returning\n"),
                AF_INET,
                string_p->value));
    goto clean; // *TODO*: avoid false negatives
  } // end IF
  status_i =
      omapi_data_string_dereference (&string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("DHCP client (IP address: %s) %s an active lease \n"),
              ACE_TEXT (buffer_a),
              (result ? ACE_TEXT ("has") : ACE_TEXT ("does not have"))));
#endif // _DEBUG

clean:
  if (likely (lease_p))
  {
    status_i =
        omapi_object_dereference (&lease_p, MDL);
    ACE_ASSERT (status_i == ISC_R_SUCCESS);
  } // end IF

  return result;
}

bool
DHCP_Tools::relinquishLease (dhcpctl_handle connection_in,
                             const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::relinquishLease"));

  bool result = false;
  dhcpctl_handle lease_h = dhcpctl_null_handle;
  isc_result_t status_i, status_2;
  omapi_data_string_t* string_p = NULL;
  struct ether_addr ether_addr_s;
  ACE_OS::memset (&ether_addr_s, 0, sizeof (struct ether_addr));
  //  ACE_INET_Addr inet_address, inet_address_2;
//  std::string ip_address_string;
#if defined (_DEBUG)
  char buffer_a[INET_ADDRSTRLEN];
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (connection_in != dhcpctl_null_handle);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  ether_addr_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);

  status_i =
      dhcpctl_new_object (&lease_h,
                          connection_in,
                          ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_LEASE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_new_object(%@,%s): \"%s\", aborting\n"),
                connection_in,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_LEASE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    return false; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (lease_h != dhcpctl_null_handle);
//  status_i =
//      dhcpctl_set_string_value (lease_h,
//                                interfaceIdentifier_in.c_str (),
//                                ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING));
//  if (unlikely (status_i != ISC_R_SUCCESS))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_set_string_value(%@,\"%s\",\"%s\"): \"%s\", returning\n"),
//                lease_h,
//                ACE_TEXT (interfaceIdentifier_in.c_str ()),
//                ACE_TEXT (DHCP_DHCLIENT_OBJECT_INTERFACE_STRING),
//                ACE_TEXT (isc_result_totext (status_i))));
//    goto clean; // *TODO*: avoid false negatives
//  } // end IF
  status_i =
      dhcpctl_set_data_value (lease_h,
                              reinterpret_cast<char*> (&ether_addr_s.ether_addr_octet),
                              ETH_ALEN,
                              ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_data_value(%@,%s,\"%s\"): \"%s\", returning\n"),
                lease_h,
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  status_i =
      dhcpctl_set_int_value (lease_h,
                             1, // 1: ethernet
                             ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_set_int_value(%@,%d,\"%s\"): \"%s\", returning\n"),
                lease_h,
                1,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF

  status_i = dhcpctl_open_object (lease_h,
                                  connection_in,
                                  0);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%@,0): \"%s\", returning\n"),
                lease_h,
                connection_in,
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  // *TODO*: add a timeout here, or use asynchronous operations
  status_i = dhcpctl_wait_for_completion (lease_h,
                                          &status_2);
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(%@): \"%s\", returning\n"),
                lease_h,
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  if (unlikely (status_2 != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_open_object(%@,%@,0): \"%s\", returning\n"),
                lease_h,
                connection_in,
                ACE_TEXT (isc_result_totext (status_2))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF

  // check lease state
  status_i =
      dhcpctl_get_value (&string_p,
                         lease_h,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", returning\n"),
                lease_h,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (string_p);
//  if (!omapi_ds_strcmp (string_p,
//                        ACE_TEXT_ALWAYS_CHAR ("active")))
//    result = true;
  result = (*reinterpret_cast<unsigned int*> (string_p->value) == 2);
  status_i =
      omapi_data_string_dereference (&string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);

#if defined (_DEBUG)
  string_p = NULL;
  status_i =
      dhcpctl_get_value (&string_p,
                         lease_h,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", returning\n"),
                lease_h,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  ACE_ASSERT (string_p);
  ACE_OS::memset (buffer_a, 0, sizeof (char[INET_ADDRSTRLEN]));
  if (unlikely (!ACE_OS::inet_ntop (AF_INET,
                                    string_p->value,
                                    buffer_a,
                                    sizeof (char[INET_ADDRSTRLEN]))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::inet_ntop(%d,%@): \"%m\", returning\n"),
                AF_INET,
                string_p->value));
    omapi_object_dereference (&lease_h, MDL);
    return false; // *TODO*: avoid false negatives
  } // end IF
  status_i =
      omapi_data_string_dereference (&string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("DHCP client (IP address: %s) %s an active lease \n"),
              ACE_TEXT (buffer_a),
              (result ? ACE_TEXT ("has") : ACE_TEXT ("does not have"))));
#endif // _DEBUG

  return result;
}

bool
DHCP_Tools::DHClientOmapiSupport (bool toggle_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::DHClientOmapiSupport"));

  // sanity check(s)
  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/dhcp/dhclient.conf");
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF
  bool has_omapi_support_b = DHCP_Tools::hasDHClientOmapiSupport ();
  if ((toggle_in && has_omapi_support_b) ||
      (!toggle_in && !has_omapi_support_b))
    return true; // nothing to do

  unsigned char* data_p = NULL;
  unsigned int file_size_i = 0;
  char buffer_a [BUFSIZ];
  std::string buffer_line_string;
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)?(?:[[:space:]]*)(?:(omapi) )(.+)$");
  std::regex regex (regex_string);
  std::smatch match_results;
  int index_i = 0;
  bool is_comment_b = false;
  std::stringstream converter;
  std::string buffer_string;

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

  if (toggle_in)
  {
    buffer_string.assign (reinterpret_cast<char*> (data_p),
                          file_size_i);
    buffer_string += ACE_TEXT_ALWAYS_CHAR ("\n# *EDIT*\nomapi port ");
    converter << DHCP_DHCLIENT_OMAPI_PORT;
    buffer_string += converter.str ();
    buffer_string += ACE_TEXT_ALWAYS_CHAR (";\n");
    goto clean;
  } // end IF

  // disable omapi support by commenting out the relevant stanza(s)
  converter.str (reinterpret_cast<char*> (data_p));
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_line_string = buffer_a;
    if (Common_String_Tools::isspace (buffer_line_string))
    {
      is_comment_b = false;
      ACE_UNUSED_ARG (is_comment_b);
      buffer_string += buffer_line_string;
      continue;
    } // end IF
    if (!std::regex_match (buffer_line_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
    {
      buffer_string += buffer_line_string;
      continue;
    } // end IF
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    index_i = 1;
    for (std::smatch::iterator iterator = ++match_results.begin ();
         iterator != match_results.end ();
         ++iterator, ++index_i)
    {
      if ((index_i == 1) && // **TODO*: there may be leading whitespace; this should read (index_i == 'index of first non-whitespace match')
          !ACE_OS::strcmp (match_results[index_i].str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("#")))
      {
        buffer_string += buffer_line_string;
        break;
      } // end IF

      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("omapi")))
      {
        buffer_string += ACE_TEXT_ALWAYS_CHAR ("#");
        buffer_string += buffer_line_string;
        break;
      } // end IF
    } // end FOR
  } while (!converter.fail ());

clean:
  if (data_p)
    delete [] data_p;

  if (unlikely (!Common_File_Tools::store (configuration_file_path,
                                           reinterpret_cast<const unsigned char*> (buffer_string.c_str ()),
                                           buffer_string.size ())))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  return true;
}
#endif // DHCLIENT_USE
#endif // ACE_WIN32 || ACE_WIN64
