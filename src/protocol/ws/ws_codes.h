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

#ifndef WEBSOCKET_CODES_H
#define WEBSOCKET_CODES_H

#include "ace/Global_Macros.h"

class WebSocket_Codes
{
 public:
  enum OpCodeType : uint8_t
  {
    OPCODE_CONTINUATION = 0,
    /////////////////////////////////////
    OPCODE_TEXT         = 1,
    OPCODE_BINARY       = 2,
    /////////////////////////////////////
    OPCODE_CLOSE        = 8,
    OPCODE_PING         = 9,
    OPCODE_PONG         = 10,
    /////////////////////////////////////
    OPCODE_MAX          = 15,
    OPCODE_INVALID
  };

  enum StatusType
  {
    STATUS_OK                    = 1000,
    STATUS_GOING_AWAY            = 1001,
    STATUS_PROTOCOL_ERROR        = 1002,
    STATUS_UNSUPPORTED_DATA      = 1003,
    /////////////////////////////////////
    STATUS_NO_CODE_RECEIVED      = 1005,
    STATUS_CONNECTION_CLOSED     = 1006,
    STATUS_INVALID_PAYLOAD_DATA  = 1007,
    STATUS_POLICY_VIOLATED       = 1008,
    STATUS_MESSAGE_TOO_BIG       = 1009,
    STATUS_UNSUPPORTED_EXTENSION = 1010,
    STATUS_INTERNAL_SERVER_ERROR = 1011,
    /////////////////////////////////////
    STATUS_TLS_HANDSHAKE_FAILURE = 1015,
    /////////////////////////////////////
    STATUS_MAX                   = 4999,
    STATUS_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Codes ())
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Codes (const WebSocket_Codes&))
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Codes& operator= (const WebSocket_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~WebSocket_Codes ())
};

// convenience typedefs
typedef WebSocket_Codes::OpCodeType WebSocket_OpCode_t;
typedef WebSocket_Codes::StatusType WebSocket_Status_t;

#endif
