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

#include <sstream>

#include "dhcp_tools.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

std::string
DHCP_Tools::dump (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::dump"));

  std::ostringstream converter;
  std::string string_buffer;

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("op: \t");
  string_buffer += DHCP_Tools::Op2String (record_in.op);
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
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.ciaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("yiaddr: \t");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.yiaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("siaddr: \t");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.siaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("giaddr: \t");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.giaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("chaddr: \t");
  string_buffer += Net_Common_Tools::MACAddress2String (record_in.chaddr);
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
      DHCP_Tools::Option2FieldType (option_type);
    string_buffer += DHCP_Tools::Option2String (option_type);
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
          string_buffer += Net_Common_Tools::IPAddress2String (0, ip_address);
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
            string_buffer += Net_Common_Tools::IPAddress2String (0, ip_address);
          } // end FOR
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
DHCP_Tools::Op2String (DHCP_Op_t op_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::Op2String"));

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
DHCP_Tools::Option2String (DHCP_Option_t option_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::Option2String"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (option_in)
  {
    case DHCP_Codes::DHCP_OPTION_SUBNETMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("SUBNETMASK"); break;
    case DHCP_Codes::DHCP_OPTION_TIMEOFFSET:
      result = ACE_TEXT_ALWAYS_CHAR ("TIMEOFFSET"); break;
    case DHCP_Codes::DHCP_OPTION_GATEWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("GATEWAY"); break;
    case DHCP_Codes::DHCP_OPTION_DOMAINNAMESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("DOMAINNAMESERVER"); break;
    case DHCP_Codes::DHCP_OPTION_HOSTNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("HOSTNAME"); break;
    case DHCP_Codes::DHCP_OPTION_IP_MTU:
      result = ACE_TEXT_ALWAYS_CHAR ("IP_MTU"); break;
    case DHCP_Codes::DHCP_OPTION_IP_BROADCASTADDRESS:
      result = ACE_TEXT_ALWAYS_CHAR ("IP_BROADCASTADDRESS"); break;
    case DHCP_Codes::DHCP_OPTION_VENDORSPECIFICINFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("VENDORSPECIFICINFORMATION"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_IPADDRESSLEASETIME"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_MESSAGETYPE"); break;
    case DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_SERVERIDENTIFIER"); break;
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
DHCP_Tools::MessageType2String (DHCP_MessageType_t type_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::MessageType2String"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (type_in)
  {
    case DHCP_Codes::DHCP_MESSAGE_DISCOVER:
      result = ACE_TEXT_ALWAYS_CHAR ("DISCOVER"); break;
    case DHCP_Codes::DHCP_MESSAGE_OFFER:
      result = ACE_TEXT_ALWAYS_CHAR ("OFFER"); break;
//    case DHCP_Codes::DHCP_METHOD_HEAD:
//      result = ACE_TEXT_ALWAYS_CHAR ("HEAD"); break;
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
DHCP_Tools::MessageType2Type (const std::string& type_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::MessageType2Type"));

  // sanity check(s)
  ACE_ASSERT (type_in.size () == 1);

  const char* char_p = type_in.c_str ();

  return static_cast<DHCP_MessageType_t> (*char_p);
}

DHCP_OptionFieldType_t
DHCP_Tools::Option2FieldType (DHCP_Option_t option_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::Option2FieldType"));

  switch (option_in)
  {
    case DHCP_Codes::DHCP_OPTION_SUBNETMASK:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    case DHCP_Codes::DHCP_OPTION_TIMEOFFSET:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_GATEWAY:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    case DHCP_Codes::DHCP_OPTION_DOMAINNAMESERVER:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    case DHCP_Codes::DHCP_OPTION_HOSTNAME:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_STRING;
    case DHCP_Codes::DHCP_OPTION_IP_MTU:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_IP_BROADCASTADDRESS:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    case DHCP_Codes::DHCP_OPTION_VENDORSPECIFICINFORMATION:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_STRING;
    case DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_ADDRESS;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown option (was: \"%s\"), aborting\n"),
                  ACE_TEXT (DHCP_Tools::Option2String (option_in).c_str ())));
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

  return static_cast<unsigned int> (ACE_OS::rand_r (&Common_Tools::randomSeed_));
}
