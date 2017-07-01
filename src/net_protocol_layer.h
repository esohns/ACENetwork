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

#ifndef Net_PROTOCOL_LAYER_H
#define Net_PROTOCOL_LAYER_H

#include "ace/Global_Macros.h"

#include <string>

class Net_Protocol_Layer
{
 public:
  // define different protocol layers
  // *IMPORTANT NOTE*: code relies on the fact that higher-level protocols are
  //                   assigned larger values
  enum ProtocolLayer
  {
    INVALID = -1,
    // *** Link Layer Protocols
    ETHERNET,
    FDDI_LLC_SNAP, // this actually wraps several layers...
    // *** Network Layer Protocols
    IPv4,
    IPv6,
    // *** IPv4 Protocols ***
    ICMP,
    IGMP,
    PIM, // Protocol Independent Multicast
    RAW,
    // *** Transport Layer Protocols
    UDP,
    TCP,
    // *** Higher-Level Protocols
    IRC
//     ASTERIX_offset,
//     ASTERIX
  };

  // debug tools
  static void ProtocolLayer2String(const ProtocolLayer&, // header type
                                   std::string&);        // return value: corresp. string

 private:
  // safety measures
  ACE_UNIMPLEMENTED_FUNC(Net_Protocol_Layer());
  ACE_UNIMPLEMENTED_FUNC(virtual ~Net_Protocol_Layer());
  ACE_UNIMPLEMENTED_FUNC(Net_Protocol_Layer(const Net_Protocol_Layer&));
  ACE_UNIMPLEMENTED_FUNC(Net_Protocol_Layer& operator=(const Net_Protocol_Layer&));
};

// convenience typedefs
typedef Net_Protocol_Layer::ProtocolLayer Net_Protocol_t;
typedef Net_Protocol_t Net_MessageHeader_t;

#endif
