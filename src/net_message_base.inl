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

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"

#include "net_macros.h"

template <typename HeaderType,
          typename ProtocolCommandType>
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::Net_MessageBase_T (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , isInitialized_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::Net_MessageBase_T"));

}

// *NOTE*: this is implicitly invoked by duplicate() as well...
template <typename HeaderType,
          typename ProtocolCommandType>
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::Net_MessageBase_T (const Net_MessageBase_T& message_in)
 : inherited (message_in)
 , isInitialized_ (message_in.isInitialized_)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::Net_MessageBase_T"));

}

template <typename HeaderType,
          typename ProtocolCommandType>
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::Net_MessageBase_T (ACE_Data_Block* dataBlock_in,
                                                           ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in, // use this when destruction is imminent...
              true)                // increment the message ID ?
 , isInitialized_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::Net_MessageBase_T"));

}

template <typename HeaderType,
          typename ProtocolCommandType>
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::~Net_MessageBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::~Net_MessageBase_T"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator

  // clean up
  isInitialized_ = false;
}

template <typename HeaderType,
          typename ProtocolCommandType>
void
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::initialize (ACE_Data_Block* dataBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::initialize"));

  // sanity check: shouldn't be initialized...
  ACE_ASSERT (!isInitialized_);

  // init base class...
  inherited::initialize (dataBlock_in);

  isInitialized_ = true;
}

template <typename HeaderType,
          typename ProtocolCommandType>
void
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("***** Message (ID: %u) *****\n"),
              getID ()));
}

template <typename HeaderType,
          typename ProtocolCommandType>
HeaderType
Net_MessageBase_T<HeaderType,
                  ProtocolCommandType>::getHeader () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::getHeader"));

//  int result = -1;

  // initialize return value(s)
  HeaderType message_header;
  ACE_OS::memset (&message_header, 0, sizeof (HeaderType));

  // sanity check(s)
  ACE_ASSERT (inherited::size () >= sizeof (HeaderType)); // enough space ?
  if (inherited::total_length () < sizeof (HeaderType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not enough data (needed: %u, had: %u), aborting\n"),
                sizeof (HeaderType),
                inherited::total_length ()));
    return message_header;
  } // end IF

  if (inherited::length () >= sizeof (HeaderType))
  {
    ACE_OS::memcpy (&message_header,
                    inherited::rd_ptr (),
                    sizeof (HeaderType));
    return message_header;
  } // end IF

  // --> part of the header data lies in the continuation

  const ACE_Message_Block* source_message_block_p = this;
  size_t missing_data = sizeof (HeaderType) - inherited::length ();

  // copy first bit
  ACE_OS::memcpy (&message_header,
                  inherited::rd_ptr (),
                  inherited::length ());

  size_t amount = 0;
  unsigned char* destination_p =
    reinterpret_cast<unsigned char*> (&message_header) + inherited::length ();
  while (missing_data)
  {
    source_message_block_p = inherited::cont ();
    ACE_ASSERT (source_message_block_p);

    // skip over any "empty" continuations...
    while (source_message_block_p->length () == 0)
      source_message_block_p = source_message_block_p->cont ();

    // copy some data over...
    amount =
      ((source_message_block_p->length () < missing_data) ? source_message_block_p->length ()
                                                          : missing_data);
    ACE_OS::memcpy (destination_p,
                    source_message_block_p->rd_ptr (),
                    amount);

    destination_p += amount;
    missing_data -= amount;
  } // end WHILE

  return message_header;
}

// void
// Net_MessageBase_T::adjustDataOffset (const Net_MessageHeader_t& headerType_in)
// {
//NETWORK_TRACE (ACE_TEXT ("Net_MessageBase_T::adjustDataOffset"));
//
//   unsigned long dataOffset = 0;
//
//   // create header
//   switch (headerType_in)
//   {
//     case RPG_Net_Protocol_Layer::ETHERNET:
//     {
//       // *NOTE*: Ethernet headers are 14 bytes long...
//       dataOffset = ETH_HLEN;
//
//       break;
//     }
//     case RPG_Net_Protocol_Layer::FDDI_LLC_SNAP:
//     {
//       // *NOTE*:
//       // - FDDI LLC headers are 13 bytes long...
//       // - FDDI SNAP headers are 8 bytes long...
//       dataOffset = FDDI_K_SNAP_HLEN;
//
//       break;
//     }
//     case RPG_Net_Protocol_Layer::IPv4:
//     {
//       // *NOTE*: IPv4 header field "Header Length" gives the size of the
//       // IP header in 32 bit words...
//       // *NOTE*: use our current offset...
//       dataOffset = (reinterpret_cast<iphdr*> (//                                          rd_ptr())->ihl * 4);
//
//       break;
//     }
//     case RPG_Net_Protocol_Layer::TCP:
//     {
//       // *NOTE*: TCP header field "Data Offset" gives the size of the
//       // TCP header in 32 bit words...
//       // *NOTE*: use our current offset...
//       dataOffset = (reinterpret_cast<tcphdr*> (//                                          rd_ptr())->doff * 4);
//
//       break;
//     }
//     case RPG_Net_Protocol_Layer::UDP:
//     {
//       // *NOTE*: UDP headers are 8 bytes long...
//       dataOffset = 8;
//
//       break;
//     }
// //     case RPG_Net_Protocol_Layer::ASTERIX_offset:
// //     {
// //       // *NOTE*: ASTERIX "resilience" headers are 4 bytes long...
// //       dataOffset = FLB_RPS_ASTERIX_RESILIENCE_BYTES;
// //
// //       break;
// //     }
// //     case RPG_Net_Protocol_Layer::ASTERIX:
// //     {
// //       // *NOTE*: ASTERIX headers are 3 bytes long...
// //       dataOffset = FLB_RPS_ASTERIX_HEADER_SIZE;
// //
// //       break;
// //     }
//     default:
//     {
//       std::string type_string;
//       RPG_Net_Protocol_Layer::ProtocolLayer2String(headerType_in,
//                                                           type_string);
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("message (ID: %u) header (type: \"%s\") is currently unsupported, continuing\n"),
//                  getID(),
//                  type_string.c_str()));
//
//       break;
//     }
//   } // end SWITCH
//
//   // advance rd_ptr() to the start of the data (or to the next header in the stack)...
//   rd_ptr(dataOffset);
// }
