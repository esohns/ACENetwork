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

class Net_Remote_Comm
{
 public:
  // define different types of messages
  enum MessageType_t
  {
    NET_MESSAGE_INVALID = -1,
    NET_MESSAGE_PING,
    NET_MESSAGE_PONG,
    /////////////////////////////////////
    NET_MESSAGE_MAX
  };

  // define a common message header
#ifdef _MSC_VER
#pragma pack (push, 1)
#endif
  struct MessageHeader
  {
    // *NOTE*: messageLength is (currently) defined as:
    // *PORTABILITY*: total message length - sizeof(unsigned int) !
    unsigned int  messageLength;
    MessageType_t messageType;
#ifdef __GNUC__
  } __attribute__ ((__packed__));
#else
  };
#endif

  // -----------**** protocol messages ****-----------
  struct PingMessage
  {
    MessageHeader messageHeader;
    unsigned int  counter;
#ifdef __GNUC__
  } __attribute__ ((__packed__));
#else
  };
#endif
  struct PongMessage
  {
    MessageHeader messageHeader;
#ifdef __GNUC__
  } __attribute__ ((__packed__));
#else
  };
#endif
  // -----------**** protocol messages END ****-----------
#ifdef _MSC_VER
#pragma pack (pop)
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Remote_Comm ())
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm (const Net_Remote_Comm&))
  ACE_UNIMPLEMENTED_FUNC (Net_Remote_Comm& operator= (const Net_Remote_Comm&))
};

// convenient typedefs
typedef Net_Remote_Comm::MessageType_t Net_MessageType_t;
typedef Net_Remote_Comm::MessageHeader Net_MessageHeader_t;

#endif
