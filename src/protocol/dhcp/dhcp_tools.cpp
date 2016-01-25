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

#include "net_common_tools.h"
#include "net_macros.h"

std::string
DHCP_Tools::dump (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::dump"));

  std::ostringstream converter;
  std::string buffer;
  buffer = ACE_TEXT_ALWAYS_CHAR ("op: ");
  buffer += DHCP_Tools::Op2String (record_in.op);
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("htype: ");
  buffer += record_in.htype;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("hlen: ");
  buffer += record_in.hlen;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("hops: ");
  buffer += record_in.hops;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("xid: ");
  buffer += record_in.xid;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("secs: ");
  converter << record_in.secs;
  buffer += converter.str ();
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("flags: 0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::hex << record_in.flags;
  buffer += converter.str ();
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("ciaddr: ");
  buffer += record_in.ciaddr;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("yiaddr: ");
  buffer += record_in.yiaddr;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("siaddr: ");
  buffer += record_in.siaddr;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("giaddr: ");
  buffer += record_in.giaddr;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("chaddr: ");
  buffer += Net_Common_Tools::MACAddress2String (record_in.chaddr);
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("sname: ");
  buffer += record_in.sname;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("file: ");
  buffer += record_in.file;
  buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  buffer += ACE_TEXT_ALWAYS_CHAR ("options (");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.options.size ();
  buffer += converter.str ();
  buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  for (DHCP_OptionsIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": \"");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  } // end FOR
  buffer += ACE_TEXT_ALWAYS_CHAR ("options /END\n");

  return buffer;
}

std::string
DHCP_Tools::Op2String (const DHCP_Op_t& op_in)
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
DHCP_Tools::Option2String (const DHCP_Option_t& option_in)
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
DHCP_Tools::MessageType2String (const DHCP_MessageType_t& type_in)
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

  if (type_in == ACE_TEXT_ALWAYS_CHAR ("DHCPDISCOVER"))
    return DHCP_Codes::DHCP_MESSAGE_DISCOVER;
//  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("POST"))
//    return HTTP_Codes::HTTP_METHOD_POST;
//  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("HEAD"))
//    return HTTP_Codes::HTTP_METHOD_HEAD;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown message type (was: \"%s\"), aborting\n"),
                ACE_TEXT (type_in.c_str ())));
  } // end IF

  return DHCP_Codes::DHCP_MESSAGE_INVALID;
}

bool
DHCP_Tools::isRequest (const DHCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Tools::isRequest"));

  return (record_in.op == DHCP_Codes::DHCP_OP_REQUEST);
}
