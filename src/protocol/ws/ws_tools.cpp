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

#include "ws_tools.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <locale>
#include <regex>
#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_tools.h"

#include "common_math_tools.h"

//#include "stream_dec_common.h"

#include "net_macros.h"

#include "ws_defines.h"

std::string
WebSocket_Tools::dump (const struct WebSocket_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Tools::dump"));

  std::ostringstream converter;
  std::string buffer;

  return buffer;
}

std::string
WebSocket_Tools::OpCodeToString (const WebSocket_OpCode_t& opcode_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Tools::OpCodeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (opcode_in)
  {
    case WebSocket_Codes::OPCODE_CONTINUATION:
      result = ACE_TEXT_ALWAYS_CHAR ("CONTINUATION"); break;
    case WebSocket_Codes::OPCODE_TEXT:
      result = ACE_TEXT_ALWAYS_CHAR ("TEXT"); break;
    case WebSocket_Codes::OPCODE_BINARY:
      result = ACE_TEXT_ALWAYS_CHAR ("BINARY"); break;
    case WebSocket_Codes::OPCODE_CLOSE:
      result = ACE_TEXT_ALWAYS_CHAR ("CLOSE"); break;
    case WebSocket_Codes::OPCODE_PING:
      result = ACE_TEXT_ALWAYS_CHAR ("PING"); break;
    case WebSocket_Codes::OPCODE_PONG:
      result = ACE_TEXT_ALWAYS_CHAR ("PONG"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown method (was: %d), aborting\n"),
                  opcode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
WebSocket_Tools::StatusToString (const WebSocket_Status_t& status_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Tools::StatusToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (status_in)
  {
    case WebSocket_Codes::STATUS_OK:
      result = ACE_TEXT_ALWAYS_CHAR ("OK"); break;
    case WebSocket_Codes::STATUS_GOING_AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("GOING_AWAY"); break;
    case WebSocket_Codes::STATUS_PROTOCOL_ERROR:
      result = ACE_TEXT_ALWAYS_CHAR ("PROTOCOL_ERROR"); break;
    case WebSocket_Codes::STATUS_UNSUPPORTED_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPPORTED_DATA"); break;
    case WebSocket_Codes::STATUS_NO_CODE_RECEIVED:
      result = ACE_TEXT_ALWAYS_CHAR ("NO_CODE_RECEIVED"); break;
    case WebSocket_Codes::STATUS_CONNECTION_CLOSED:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECTION_CLOSED"); break;
    case WebSocket_Codes::STATUS_INVALID_PAYLOAD_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("INVALID_PAYLOAD_DATA"); break;
    case WebSocket_Codes::STATUS_POLICY_VIOLATED:
      result = ACE_TEXT_ALWAYS_CHAR ("POLICY_VIOLATED"); break;
    case WebSocket_Codes::STATUS_MESSAGE_TOO_BIG:
      result = ACE_TEXT_ALWAYS_CHAR ("MESSAGE_TOO_BIG"); break;
    case WebSocket_Codes::STATUS_UNSUPPORTED_EXTENSION:
      result = ACE_TEXT_ALWAYS_CHAR ("UNSUPPORTED_EXTENSION"); break;
    case WebSocket_Codes::STATUS_INTERNAL_SERVER_ERROR:
      result = ACE_TEXT_ALWAYS_CHAR ("INTERNAL_SERVER_ERROR"); break;
    case WebSocket_Codes::STATUS_TLS_HANDSHAKE_FAILURE:
      result = ACE_TEXT_ALWAYS_CHAR ("TLS_HANDSHAKE_FAILURE");
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown status (was: %d), aborting\n"),
                  status_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
WebSocket_Tools::generateSecHeaderKey ()
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Tools::generateSecHeaderKey"));

  // initialize result
  std::string result;

  static std::uniform_int_distribution<ACE_UINT64> distribution;
  ACE_UINT64 value_i = Common_Tools::getRandomNumber (distribution);
  ACE_UINT64 value_2 = Common_Tools::getRandomNumber (distribution);
  std::vector<uint8_t> values_a;
  values_a.push_back ((value_i & 0xFF00000000000000) >> 56);
  values_a.push_back ((value_i & 0x00FF000000000000) >> 48);
  values_a.push_back ((value_i & 0x0000FF0000000000) >> 40);
  values_a.push_back ((value_i & 0x000000FF00000000) >> 32);
  values_a.push_back ((value_i & 0x00000000FF000000) >> 24);
  values_a.push_back ((value_i & 0x0000000000FF0000) >> 16);
  values_a.push_back ((value_i & 0x000000000000FF00) >> 8);
  values_a.push_back (value_i & 0x00000000000000FF);

  values_a.push_back ((value_2 & 0xFF00000000000000) >> 56);
  values_a.push_back ((value_2 & 0x00FF000000000000) >> 48);
  values_a.push_back ((value_2 & 0x0000FF0000000000) >> 40);
  values_a.push_back ((value_2 & 0x000000FF00000000) >> 32);
  values_a.push_back ((value_2 & 0x00000000FF000000) >> 24);
  values_a.push_back ((value_2 & 0x0000000000FF0000) >> 16);
  values_a.push_back ((value_2 & 0x000000000000FF00) >> 8);
  values_a.push_back (value_2 & 0x00000000000000FF);

  result = Common_Math_Tools::encodeBase64 (values_a.data (),
                                            values_a.size ());

  return result;
}
