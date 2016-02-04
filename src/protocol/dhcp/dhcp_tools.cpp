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

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("op: ");
  string_buffer += DHCP_Tools::Op2String (record_in.op);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("htype: ");
  converter << static_cast<unsigned int> (record_in.htype);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("hlen: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << static_cast<unsigned int> (record_in.hlen);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("hops: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << static_cast<unsigned int> (record_in.hops);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("xid: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.xid;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("secs: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.secs;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("flags: 0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::hex << record_in.flags;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("ciaddr: ");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.ciaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("yiaddr: ");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.yiaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("siaddr: ");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.siaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("giaddr: ");
  string_buffer += Net_Common_Tools::IPAddress2String (0, record_in.giaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("chaddr: ");
  string_buffer += Net_Common_Tools::MACAddress2String (record_in.chaddr);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("sname: \"");
  string_buffer += record_in.sname;
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("file: \"");
  string_buffer += record_in.file;
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("options (");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.options.size ();
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  DHCP_Codes::OptionFieldType field_type;
  for (DHCP_OptionsIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    field_type =
      DHCP_Tools::Option2FieldType (static_cast<DHCP_Codes::OptionType> ((*iterator).first));
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << static_cast<unsigned int> ((*iterator).first);
    string_buffer += converter.str ();
    string_buffer += ACE_TEXT_ALWAYS_CHAR (": ");
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
            converter << (i + 1);
            string_buffer += converter.str ();
            string_buffer += ACE_TEXT_ALWAYS_CHAR (": ");
            string_buffer += Net_Common_Tools::IPAddress2String (0, ip_address);
          } // end FOR
        break;
      }
      case DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER:
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        // *TODO*: support value multiplicity here
        switch ((*iterator).second.size ())
        {
          case 1:
            converter <<
              static_cast<unsigned int> (*reinterpret_cast<const unsigned char*> ((*iterator).second.c_str ()));
            break;
          case 2:
            converter <<
              *reinterpret_cast<const unsigned short*> ((*iterator).second.c_str ());
            break;
          case 4:
            converter <<
              *reinterpret_cast<const unsigned int*> ((*iterator).second.c_str ());
            break;
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
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("DHCP_MESSAGETYPE"); break;
//    case DHCP_Codes::DHCP_METHOD_POST:
//      result = ACE_TEXT_ALWAYS_CHAR ("POST"); break;
//    case DHCP_Codes::DHCP_METHOD_HEAD:
//      result = ACE_TEXT_ALWAYS_CHAR ("HEAD"); break;
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
//    case DHCP_Codes::DHCP_METHOD_POST:
//      result = ACE_TEXT_ALWAYS_CHAR ("POST"); break;
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
    case DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE:
      return DHCP_Codes::DHCP_OPTION_FIELDTYPE_INTEGER;
    case DHCP_Codes::DHCP_OPTION_SUBNETMASK:
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

unsigned int
DHCP_Tools::generateXID ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::generateXID"));

  return static_cast<unsigned int> (ACE_OS::rand_r (&Common_Tools::randomSeed_));
}
