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

#include "pcp_tools.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_process_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "pcp_codes.h"
#include "pcp_defines.h"

std::string
PCP_Tools::dump (const struct PCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::dump"));

  std::ostringstream converter;
  std::string string_buffer;

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("version: \t");
  string_buffer += PCP_Tools::VersionToString (record_in.version);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("opcode: \t");
  string_buffer +=
    PCP_Tools::OpcodeToString (static_cast<enum PCP_Codes::OpcodeType> (record_in.opcode & 0x7F));
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("reserved: \t");
  converter << static_cast<unsigned int> (record_in.reserved);
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("result code: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  string_buffer += PCP_Tools::ResultCodeToString (record_in.result_code);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("lifetime: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.lifetime;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("epoch time: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.epoch_time;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("reserved_2: \t");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.reserved_2;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  switch (record_in.opcode)
  {
    case PCP_Codes::PCP_OPCODE_ANNOUNCE:
      break;
    case PCP_Codes::PCP_OPCODE_MAP:
    {
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("nonce: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.map.nonce;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("protocol: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << static_cast<ACE_UINT16> (record_in.map.protocol);
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("reserved: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.map.reserved;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("internal port: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.map.internal_port;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("external port: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.map.external_port;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("external address: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      string_buffer +=
        Net_Common_Tools::IPAddressToString (*record_in.map.external_address,
                                             true,   // address only
                                             false); // do not resolve
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      break;
    }
    case PCP_Codes::PCP_OPCODE_PEER:
    {
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("nonce: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.peer.nonce;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("protocol: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << static_cast<ACE_UINT16> (record_in.peer.protocol);
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("reserved: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.peer.reserved;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("internal port: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.peer.internal_port;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("external port: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.peer.external_port;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("external address: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      string_buffer +=
        Net_Common_Tools::IPAddressToString (*record_in.peer.external_address,
                                             true,   // address only
                                             false); // do not resolve
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("remote peer port: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.peer.remote_peer_port;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("remote peer address: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      string_buffer +=
        Net_Common_Tools::IPAddressToString (*record_in.peer.remote_peer_address,
                                             true,   // address only
                                             false); // do not resolve
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      break;
    }
    case PCP_Codes::PCP_OPCODE_AUTHENTICATION:
    {
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("session id: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.authentication.session_id;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("sequence number: \t");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << record_in.authentication.sequence_number;
      string_buffer += converter.str ();
      string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown opcode (was: %d), aborting\n"),
                  record_in.opcode));
      return ACE_TEXT_ALWAYS_CHAR ("");
    }
  } // end SWITCH
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("options (");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.options.size ();
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  for (PCP_OptionsConstIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    string_buffer +=
      PCP_Tools::OptionToString (static_cast<PCP_Codes::OptionType> ((*iterator).code));
    string_buffer += ACE_TEXT_ALWAYS_CHAR (": \t");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << static_cast<ACE_UINT16> ((*iterator).reserved);
    string_buffer += converter.str ();
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (*iterator).length;
    string_buffer += converter.str ();
    switch ((*iterator).code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        string_buffer +=
          Net_Common_Tools::IPAddressToString (*(*iterator).third_party.address,
                                               true,   // address only
                                               false); // do not resolve
        break;
      }
      case PCP_Codes::PCP_OPTION_PREFER_FAILURE:
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).filter.reserved;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).filter.prefix_length;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).filter.remote_peer_port;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
        string_buffer +=
          Net_Common_Tools::IPAddressToString (*(*iterator).filter.remote_peer_address,
                                               true,   // address only
                                               false); // do not resolve
        break;
      }
      case PCP_Codes::PCP_OPTION_NONCE: // rfc7652
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).nonce.nonce;
        string_buffer += converter.str ();
        break;
      }
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).authentication_tag.session_id;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).authentication_tag.sequence_number;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).authentication_tag.key_id;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t\"");
        for (unsigned int i = 0;
             i < static_cast<unsigned int> ((*iterator).length - 12);
             ++i)
          string_buffer += (*iterator).authentication_tag.data[i];
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        break;
      }
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).pa_authentication_tag.key_id;
        string_buffer += converter.str ();
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\t\"");
        for (unsigned int i = 0;
             i < static_cast<unsigned int> ((*iterator).length - 4);
             ++i)
          string_buffer += (*iterator).pa_authentication_tag.data[i];
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        break;
      }
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t\"");
        for (unsigned int i = 0;
             i < static_cast<unsigned int> ((*iterator).length);
             ++i)
          string_buffer += (*iterator).eap_payload.data[i];
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        break;
      }
      case PCP_Codes::PCP_OPTION_PSEUDO_RANDOM_FUNCTION:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).pseudo_random_function.id;
        string_buffer += converter.str ();
        break;
      }
      case PCP_Codes::PCP_OPTION_MAC_ALGORITHM:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).mac_algorithm.id;
        string_buffer += converter.str ();
        break;
      }
      case PCP_Codes::PCP_OPTION_SESSION_LIFETIME:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).session_lifetime.lifetime;
        string_buffer += converter.str ();
        break;
      }
      case PCP_Codes::PCP_OPTION_RECEIVED_PAK:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).received_pak.sequence_number;
        string_buffer += converter.str ();
        break;
      }
      case PCP_Codes::PCP_OPTION_ID_INDICATOR:
      {
        string_buffer += ACE_TEXT_ALWAYS_CHAR (" \t\"");
        for (unsigned int i = 0;
             i < static_cast<unsigned int> ((*iterator).length);
             ++i)
          string_buffer += (*iterator).id_indicator.data[i];
        string_buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown option code (was: %d), aborting\n"),
                    (*iterator).code));
        break;
      }
    } // end SWITCH
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end FOR
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("options /END\n");

  return string_buffer;
}

std::string
PCP_Tools::VersionToString (PCP_Version_t version_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::VersionToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (version_in)
  {
    //case PCP_Codes::PCP_VERSION_1:
    //  result = ACE_TEXT_ALWAYS_CHAR ("VERSION_1"); break;
    case PCP_Codes::PCP_VERSION_2:
      result = ACE_TEXT_ALWAYS_CHAR ("VERSION_2"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown version (was: %d), aborting\n"),
                  version_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
PCP_Tools::OpcodeToString (PCP_Opcode_t opcode_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::OpcodeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (opcode_in)
  {
    case PCP_Codes::PCP_OPCODE_ANNOUNCE:
      result = ACE_TEXT_ALWAYS_CHAR ("ANNOUNCE"); break;
    case PCP_Codes::PCP_OPCODE_MAP:
      result = ACE_TEXT_ALWAYS_CHAR ("MAP"); break;
    case PCP_Codes::PCP_OPCODE_PEER:
      result = ACE_TEXT_ALWAYS_CHAR ("PEER"); break;
    case PCP_Codes::PCP_OPCODE_AUTHENTICATION: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown opcode (was: %d), aborting\n"),
                  opcode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
PCP_Tools::ResultCodeToString (PCP_ResultCode_t resultCode_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::ResultCodeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (resultCode_in)
  {
    case PCP_Codes::PCP_RESULTCODE_SUCCESS:
      result = ACE_TEXT_ALWAYS_CHAR ("SUCCESS"); break;
    case PCP_Codes::PCP_RESULTCODE_UNSUPP_VERSION:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPP_VERSION"); break;
    case PCP_Codes::PCP_RESULTCODE_NOT_AUTHORIZED:
      result = ACE_TEXT_ALWAYS_CHAR ("NOT_AUTHORIZED"); break;
    case PCP_Codes::PCP_RESULTCODE_MALFORMED_REQUEST:
      result = ACE_TEXT_ALWAYS_CHAR ("MALFORMED_REQUEST"); break;
    case PCP_Codes::PCP_RESULTCODE_UNSUPP_OPCODE:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPP_OPCODE"); break;
    case PCP_Codes::PCP_RESULTCODE_UNSUPP_OPTION:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPP_OPTION"); break;
    case PCP_Codes::PCP_RESULTCODE_MALFORMED_OPTION:
      result = ACE_TEXT_ALWAYS_CHAR ("MALFORMED_OPTION"); break;
    case PCP_Codes::PCP_RESULTCODE_NETWORK_FAILURE:
      result = ACE_TEXT_ALWAYS_CHAR ("NETWORK_FAILURE"); break;
    case PCP_Codes::PCP_RESULTCODE_NO_RESOURCES:
      result = ACE_TEXT_ALWAYS_CHAR ("NO_RESOURCES"); break;
    case PCP_Codes::PCP_RESULTCODE_UNSUPP_PROTOCOL:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPP_PROTOCOL"); break;
    case PCP_Codes::PCP_RESULTCODE_USER_EX_QUOTA:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_EX_QUOTA"); break;
    case PCP_Codes::PCP_RESULTCODE_CANNOT_PROVIDE_EXTERNAL:
      result = ACE_TEXT_ALWAYS_CHAR ("CANNOT_PROVIDE_EXTERNAL"); break;
    case PCP_Codes::PCP_RESULTCODE_ADDRESS_MISMATCH:
      result = ACE_TEXT_ALWAYS_CHAR ("ADDRESS_MISMATCH"); break;
    case PCP_Codes::PCP_RESULTCODE_EXCESSIVE_REMOTE_PEERS:
      result = ACE_TEXT_ALWAYS_CHAR ("EXCESSIVE_REMOTE_PEERS"); break;
    case PCP_Codes::PCP_RESULTCODE_INITIATION: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("INITIATION"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHENTICATION_REQUIRED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_REQUIRED"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHENTICATION_FAILED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_FAILED"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHENTICATION_SUCCEEDED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_SUCCEEDED"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHORIZATION_FAILED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHORIZATION_FAILED"); break;
    case PCP_Codes::PCP_RESULTCODE_SESSION_TERMINATED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("SESSION_TERMINATED"); break;
    case PCP_Codes::PCP_RESULTCODE_UNKNOWN_SESSION_ID: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("UNKNOWN_SESSION_ID"); break;
    case PCP_Codes::PCP_RESULTCODE_DOWNGRADE_ATTACK_DETECTED: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("DOWNGRADE_ATTACK_DETECTED"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHENTICATION_REQUEST: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_REQUEST"); break;
    case PCP_Codes::PCP_RESULTCODE_AUTHENTICATION_REPLY: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_REPLY"); break;
    case PCP_Codes::PCP_RESULTCODE_STANDARDS_ACTION_BEGIN:
    // *TODO*
    case PCP_Codes::PCP_RESULTCODE_STANDARDS_ACTION_END:
      result = ACE_TEXT_ALWAYS_CHAR ("STANDARDS_ACTION"); break;
    case PCP_Codes::PCP_RESULTCODE_SPECIFICATION_REQUIRED_BEGIN:
    // *TODO*
    case PCP_Codes::PCP_RESULTCODE_SPECIFICATION_REQUIRED_END:
      result = ACE_TEXT_ALWAYS_CHAR ("SPECIFICATION_REQUIRED"); break;
    case PCP_Codes::PCP_RESULTCODE_PRIVATE_USE_BEGIN:
    // *TODO*
    case PCP_Codes::PCP_RESULTCODE_PRIVATE_USE_END:
      result = ACE_TEXT_ALWAYS_CHAR ("PRIVATE_USE"); break;
    case PCP_Codes::PCP_RESULTCODE_STANDARDS_ACTION_RESERVED:
      result = ACE_TEXT_ALWAYS_CHAR ("STANDARDS_ACTION_RESERVED"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown result code (was: %d), aborting\n"),
                  resultCode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
PCP_Tools::OptionToString (PCP_Option_t option_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::OptionToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (option_in)
  {
    case PCP_Codes::PCP_OPTION_THIRD_PARTY:
      result = ACE_TEXT_ALWAYS_CHAR ("THIRD_PARTY"); break;
    case PCP_Codes::PCP_OPTION_PREFER_FAILURE:
      result = ACE_TEXT_ALWAYS_CHAR ("PREFER_FAILURE"); break;
    case PCP_Codes::PCP_OPTION_FILTER:
      result = ACE_TEXT_ALWAYS_CHAR ("FILTER"); break;
    case PCP_Codes::PCP_OPTION_NONCE: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("NONCE"); break;
    case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_TAG"); break;
    case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("PA_AUTHENTICATION_TAG"); break;
    case PCP_Codes::PCP_OPTION_EAP_PAYLOAD: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("EAP_PAYLOAD"); break;
    case PCP_Codes::PCP_OPTION_PSEUDO_RANDOM_FUNCTION: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("PSEUDO_RANDOM_FUNCTION"); break;
    case PCP_Codes::PCP_OPTION_MAC_ALGORITHM: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("MAC_ALGORITHM"); break;
    case PCP_Codes::PCP_OPTION_SESSION_LIFETIME: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("SESSION_LIFETIME"); break;
    case PCP_Codes::PCP_OPTION_RECEIVED_PAK: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("RECEIVED_PAK"); break;
    case PCP_Codes::PCP_OPTION_ID_INDICATOR: // rfc7652
      result = ACE_TEXT_ALWAYS_CHAR ("ID_INDICATOR"); break;
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

void
PCP_Tools::mapAddress (const ACE_INET_Addr& address_in,
                       ACE_UINT8 mappedAddress_inout[])
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::mapAddress"));

  ACE_OS::memset (mappedAddress_inout,
                  0,
                  sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
  mappedAddress_inout[10] = 0xFF; mappedAddress_inout[11] = 0xFF;
  ACE_UINT32 ip_address = address_in.get_ip_address ();
  ip_address =
    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (ip_address)
                                           : ip_address);
  *reinterpret_cast<ACE_UINT32*> (&mappedAddress_inout[12]) = ip_address;
}

void
PCP_Tools::clear (struct PCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::clear"));

  switch (record_in.opcode)
  {
    case PCP_Codes::PCP_OPCODE_MAP:
      record_in.map.external_address = NULL;
      break;
    case PCP_Codes::PCP_OPCODE_PEER:
      record_in.peer.external_address = NULL;
      record_in.peer.remote_peer_address = NULL;
      break;
    default:
      break;
  } // end SWITCH

  // *NOTE*: there is dynamic data in options
  //         - THIRD_PARTY, FILTER
  //         - AUTHENTICATION_TAG, PA_AUTHENTICATION_TAG
  //         - EAP_PAYLOAD, ID_INDICATOR only
  for (PCP_OptionsIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    switch ((*iterator).code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
        (*iterator).third_party.address = NULL;
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
        (*iterator).filter.remote_peer_address = NULL;
        break;
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG:
        (*iterator).authentication_tag.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG:
        (*iterator).pa_authentication_tag.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD:
        (*iterator).eap_payload.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_ID_INDICATOR:
        (*iterator).id_indicator.data = NULL;
        break;
      default:
        break;
    } // end SWITCH
  } // end FOR
}

void
PCP_Tools::free (struct PCP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Tools::free"));

  switch (record_in.opcode)
  {
    case PCP_Codes::PCP_OPCODE_MAP:
      delete record_in.map.external_address;
      record_in.map.external_address = NULL;
      break;
    case PCP_Codes::PCP_OPCODE_PEER:
      delete record_in.peer.external_address;
      record_in.peer.external_address = NULL;
      delete record_in.peer.remote_peer_address;
      record_in.peer.remote_peer_address = NULL;
      break;
    default:
      break;
  } // end SWITCH

  // *NOTE*: there is dynamic data in options
  //         - THIRD_PARTY, FILTER
  //         - AUTHENTICATION_TAG, PA_AUTHENTICATION_TAG
  //         - EAP_PAYLOAD, ID_INDICATOR only
  for (PCP_OptionsIterator_t iterator = record_in.options.begin ();
       iterator != record_in.options.end ();
       ++iterator)
  {
    switch ((*iterator).code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
        delete (*iterator).third_party.address;
        (*iterator).third_party.address = NULL;
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
        delete (*iterator).filter.remote_peer_address;
        (*iterator).filter.remote_peer_address = NULL;
        break;
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG:
        delete [] (*iterator).authentication_tag.data;
        (*iterator).authentication_tag.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG:
        delete [] (*iterator).pa_authentication_tag.data;
        (*iterator).pa_authentication_tag.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD:
        delete [] (*iterator).eap_payload.data;
        (*iterator).eap_payload.data = NULL;
        break;
      case PCP_Codes::PCP_OPTION_ID_INDICATOR:
        delete [] (*iterator).id_indicator.data;
        (*iterator).id_indicator.data = NULL;
        break;
      default:
        break;
    } // end SWITCH
  } // end FOR
}
