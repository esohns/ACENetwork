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

#ifndef NET_REMOTE_COMM_H
#define NET_REMOTE_COMM_H

#include "ace/Global_Macros.h"

// *TODO*: make this portable by considering host endianness !!!

class Net_Remote_Comm
{
 public:
  // define different types of messages
  enum MessageType : int8_t
  {
    NET_MESSAGE_INVALID = -1,
    NET_MESSAGE_PING,
    NET_MESSAGE_PONG,
    /////////////////////////////////////
    NET_MESSAGE_MAX
  };

  // define a common message header
#if defined (_MSC_VER)
#pragma pack (push, 1)
#endif // _MSC_VER
  struct MessageHeader
  {
    uint16_t    length;
    MessageType type;
#if defined (__GNUC__)
  } __attribute__ ((__packed__));
#else
  };
#endif // __GNUC__

  // -----------**** protocol messages ****-----------
  struct PingMessage
  {
    MessageHeader header;
    uint32_t      counter;
#if defined (__GNUC__)
  } __attribute__ ((__packed__));
#else
  };
#endif // __GNUC__
  struct PongMessage
  {
    MessageHeader header;
#if defined (__GNUC__)
  } __attribute__ ((__packed__));
#else
  };
#endif // __GNUC__
  // -----------**** protocol messages END ****-----------
#if defined (_MSC_VER)
#pragma pack (pop)
#endif // _MSC_VER

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Remote_Comm ())
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm (const Net_Remote_Comm&))
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm& operator= (const Net_Remote_Comm&))
};

// convenient typedefs
typedef Net_Remote_Comm::MessageType Net_MessageType_t;
typedef Net_Remote_Comm::MessageHeader Net_MessageHeader_t;

#endif
