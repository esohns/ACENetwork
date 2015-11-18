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

#include "test_u_message.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_macros.h"

Net_Message::Net_Message (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::Net_Message"));

}

// *NOTE*: this is implicitly invoked by duplicate() as well...
Net_Message::Net_Message (const Net_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::Net_Message"));

}

Net_Message::Net_Message (ACE_Data_Block* dataBlock_in,
                          ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in) // use this when destruction is imminent...
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::Net_Message"));

}

Net_Message::~Net_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::~Net_Message"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator
}

Net_MessageType_t
Net_Message::command () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::command"));

  // sanity check(s)
  ACE_ASSERT (inherited::total_length () >= sizeof (Net_MessageHeader_t));

  if (inherited::length () < sizeof (Net_MessageHeader_t))
  { // assemble the header
    Net_MessageHeader_t message_header;
    ACE_OS::memset (&message_header, 0, sizeof (Net_MessageHeader_t));
    unsigned int remaining = sizeof (Net_MessageHeader_t);
    unsigned int bytes_to_write = 0;
    char* wr_ptr_p = reinterpret_cast<char*> (&message_header);
    for (const ACE_Message_Block* message_block_p = this;
         message_block_p;
         message_block_p = message_block_p->cont ())
    {
      bytes_to_write =
        (message_block_p->length () < remaining ? message_block_p->length ()
                                                : remaining);
      ACE_OS::memcpy (wr_ptr_p, message_block_p->rd_ptr (), bytes_to_write);
      remaining -= bytes_to_write;
      if (!remaining) break; // done

      wr_ptr_p += bytes_to_write;
    } // end FOR
    ACE_ASSERT (!remaining);

    return message_header.messageType;
  } // end IF

  Net_MessageHeader_t* message_header_p =
      reinterpret_cast<Net_MessageHeader_t*> (inherited::rd_ptr ());

  return message_header_p->messageType;
}

std::string
Net_Message::CommandType2String (Net_MessageType_t messageType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::CommandType2String"));

  std::string result = ACE_TEXT ("INVALID");

  switch (messageType_in)
  {
    case Net_Remote_Comm::NET_MESSAGE_PING:
      result = ACE_TEXT ("PING"); break;
    case Net_Remote_Comm::NET_MESSAGE_PONG:
      result = ACE_TEXT ("PONG"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  messageType_in));
      break;
    }
  } // end SWITCH

  return result;
}

ACE_Message_Block*
Net_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::duplicate"));

  Net_Message* message_p = NULL;

  // create a new Net_Message that contains unique copies of
  // the message block fields, but a (reference counted) "shallow" duplicate of
  // the datablock

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      Net_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
allocate:
    try
    {
      // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
      //         a "shallow" copy which just references our data block...
      // *IMPORTANT NOTE*: cached allocators require the object size as argument
      //                   to malloc() (instead of its internal "capacity()" !)
      // *TODO*: (depending on the allocator implementation) this senselessly
      // allocates a datablock anyway, only to immediately release it again...
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<Net_Message*> (inherited::message_block_allocator_->calloc (sizeof (Net_Message))),
                               Net_Message (*this));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), aborting\n"),
                  sizeof (Net_Message)));
      return NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_p->block ())
      goto allocate;
  } // end ELSE
  if (!message_p)
  {
    if (inherited::message_block_allocator_)
    {
      Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
      ACE_ASSERT (allocator_p);

      if (allocator_p->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate Net_Message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Net_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

void
Net_Message::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Message::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("***** Message (ID: %u) *****\n"),
              inherited::getID ()));

//   // step1: dump individual header types & offsets
//   std::string protocol_layer;
//   for (HEADERCONTAINER_CONSTITERATOR_TYPE iter = myHeaders.begin ();
//        iter != myHeaders.end ();
//        iter++)
//   {
//     RPG_Net_Protocol_Layer::ProtocolLayer2String(iter->first,
//                                                         protocol_layer);
//
//     ACE_DEBUG ((LM_INFO,
//                 ACE_TEXT ("--> header type: \"%s\" @ offset: %u\n"),
//                 protocol_layer.c_str (),
//                 iter->second));
//   } // end FOR
//
//   // step2: dump individual headers, top-down (collect sizes)
//   unsigned long sum_header_size = 0;
//   for (HEADERCONTAINER_CONSTITERATOR_TYPE iter = myHeaders.begin ();
//        iter != myHeaders.end ();
//        iter++)
//   {
//     switch (iter->first)
//     {
// //       case RPG_Net_Protocol_Layer::ASTERIX:
// //       {
// //         Stream_ASTERIXHeader header(*this,
// //                                     iter->second);
// //
// //         // remember size
// //         sum_header_size += header.length ();
// //
// //         header.dump_state();
// //
// //         break;
// //       }
// //       case RPG_Net_Protocol_Layer::ASTERIX_offset: // "resilience" bytes...
// //       {
// //         // remember size
// //         sum_header_size += FLB_RPS_ASTERIX_RESILIENCE_BYTES;
// //
// //         // don't have a header class for this...
// //         ACE_DEBUG ((LM_INFO,
// //                     ACE_TEXT (" *** ASTERIX_offset (%u bytes) ***\n"),
// //                     FLB_RPS_ASTERIX_RESILIENCE_BYTES));
// //
// //         break;
// //       }
//       case RPG_Net_Protocol_Layer::TCP:
//       {
//         struct tcphdr* header = reinterpret_cast<struct tcphdr*> (//                                                      (rd_ptr() + iter->second));
//
//         // remember size
//         // *NOTE*: TCP header field "Data Offset" gives the size of the
//         // TCP header in 32 bit words...
//         sum_header_size += (header->doff * 4);
//
//         // debug info
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("*** TCP (%u bytes) ***\nSource#: %u --> %u\nDestination#: %u --> %u\nSequence#: %u --> %u (swapped)\nAcknowledgement#: %u --> %u (swapped)\nReserved: %u\nData Offset: %u --> %u bytes\nFlags: %u\nWindow: %u --> %u (swapped)\nChecksum: %u --> %u (swapped)\nUrgent Pointer: %u --> %u (swapped)\n"),
//                     (header->doff * 4),
//                     header->source,
//                     ACE_NTOHS (header->source), // byte swapping
//                     header->dest,
//                     ACE_NTOHS (header->dest), // byte swapping
//                     header->seq,
//                     ACE_NTOHL (header->seq), // byte swapping
//                     header->ack_seq,
//                     ACE_NTOHL (header->ack_seq), // byte swapping
//                     header->res1,
//                     header->doff,
//                     (header->doff * 4), // convert to bytes (value is in 32 bit words)
//                     0, // *TODO*
//                     header->window,
//                     ACE_NTOHS (header->window), // byte swapping
//                     header->check,
//                     ACE_NTOHS (header->check), // byte swapping
//                     header->urg_ptr,
//                     ACE_NTOHS (header->urg_ptr))); // byte swapping
//
//         break;
//       }
//       case RPG_Net_Protocol_Layer::UDP:
//       {
//         struct udphdr* header = reinterpret_cast<struct udphdr*> (//                                                      (rd_ptr() + iter->second));
//
//         // remember size
//         // *NOTE*: UDP headers are 8 bytes long...
//         sum_header_size += sizeof (struct udphdr);
//
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT("*** UDP (%u bytes) ***\nSource#: %u --> %u\nDestination#: %u --> %u\nLength: %u --> %u bytes\nChecksum: %u --> %u (swapped)\n"),
//                     sizeof (struct udphdr),
//                     header->source,
//                     ACE_NTOHS (header->source), // byte swapping
//                     header->dest,
//                     ACE_NTOHS (header->dest), // byte swapping
//                     header->len,
//                     ACE_NTOHS (header->len), // byte swapping
//                     header->check,
//                     ACE_NTOHS (header->check))); // byte swapping
//
//         break;
//       }
//       case RPG_Net_Protocol_Layer::IPv4:
//       {
//         struct iphdr* header = reinterpret_cast<struct iphdr*> (//                                                     (rd_ptr() + iter->second));
//
//         // remember size
//         // *NOTE*: IPv4 header field "Header Length" gives the size of the
//         // IP header in 32 bit words...
//         sum_header_size += (header->ihl * 4);
//
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT("*** IP (%u bytes) ***\nVersion: %u\nHeader Length: %u --> %u bytes\nType-of-Service Flags: %u\nTotal Packet Length: %u --> %u bytes\nFragment Identifier: %u --> %u (swapped)\nFragmentation Flags: %u (3 LSBits)\nFragmentation Offset: %u\nTime-to-Live: %u\nProtocol Identifier: %u --> \"%s\"\nHeader Checksum: %u --> %u (swapped)\nSource#: %u --> \"%s\"\nDestination#: %u --> \"%s\"\nOptions: %u byte(s)\n"),
//                     (header->ihl * 4),
//                     header->version,
//                     header->ihl,
//                     (header->ihl * 4), // <-- Header Length is in in multiples of 32 bits
//                     header->tos,
//                     header->tot_len,
//                     ACE_NTOHS (header->tot_len), // byte swapping
//                     header->id,
//                     ACE_NTOHS (header->id), // byte swapping
//                     (ACE_NTOHS (header->frag_off) >> 13), // consider the head three bits...
//                     (ACE_NTOHS (header->frag_off) & IP_OFFMASK),
//                     header->ttl,
//                     header->protocol,
//                     Net_Common_Tools::IPProtocol2String (header->protocol).c_str (),
//                     header->check,
//                     ACE_NTOHS (header->check), // byte swapping
//                     header->saddr,
//                     Net_Common_Tools::IPAddress2String (header->saddr, 0).c_str (), // no port
//                     header->daddr,
//                     Net_Common_Tools::IPAddress2String (header->daddr, 0).c_str (), // no port
//                     (header->ihl - 5)));
//
//         break;
//       }
//       case RPG_Net_Protocol_Layer::FDDI_LLC_SNAP:
//       {
//         struct fddihdr* header = reinterpret_cast<struct fddihdr*> (//                                                       (rd_ptr() + iter->second));
//
//         // remember size
//         // *NOTE*: for the time being, we only support LLC SNAP...
//         sum_header_size += FDDI_K_SNAP_HLEN;
//
//         // *TODO*: add Organization Code
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("*** FDDI_LLC_SNAP (%u bytes) ***\nFrame Control: %u\nDestination MAC#: \"%s\"\nSource MAC#: \"%s\"\nDSAP: %u\nSSAP: %u\nControl Field: %u\nPacket Type ID: %u --> %u (swapped) --> \"%s\"\n"),
//                     FDDI_K_SNAP_HLEN,
//                     header->fc,
//                     RPG_Net_Common_Tools::MACAddress2String ((inherited::rd_ptr () + iter->second) + 1).c_str (),
//                     Net_Common_Tools::MACAddress2String ((inherited::rd_ptr () + iter->second) + 1 + FDDI_K_ALEN).c_str (),
//                     header->hdr.llc_snap.dsap,
//                     header->hdr.llc_snap.ssap,
//                     header->hdr.llc_snap.ctrl,
//                     header->hdr.llc_snap.ethertype,
//                     ACE_NTOHS (header->hdr.llc_snap.ethertype), // byte swapping
//                     Net_Common_Tools::EthernetProtocolTypeID2String (header->hdr.llc_snap.ethertype).c_str ()));
//
//         break;
//       }
//       case RPG_Net_Protocol_Layer::ETHERNET:
//       {
//         struct ether_header* header = reinterpret_cast<struct ether_header*> (//                                                            (rd_ptr() + iter->second));
//
//         // remember size
//         // *NOTE*: Ethernet headers are 14 bytes long...
//         sum_header_size += ETH_HLEN;
//
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("*** ETHERNET (%u bytes) ***\nDestination MAC#: \"%s\"\nSource MAC#: \"%s\"\nProtocol Type/Length: %u --> %u (swapped) --> \"%s\"\n"),
//                     ETH_HLEN,
//                     Net_Common_Tools::MACAddress2String ((inherited::rd_ptr () + iter->second)).c_str (),
//                     Net_Common_Tools::MACAddress2String ((inherited::rd_ptr () + iter->second) + ETH_ALEN).c_str (),
//                     header->ether_type,
//                     ACE_NTOHS(header->ether_type), // byte swapping
//                     Net_Common_Tools::EthernetProtocolTypeID2String (header->ether_type).c_str ()));
//
//         break;
//       }
//       default:
//       {
//         // debug info
//         Net_Protocol_Layer::ProtocolLayer2String (iter->first,
//                                                      protocol_layer);
//
//         ACE_DEBUG ((LM_ERROR,
//                     ACE_TEXT ("message (ID: %u) contains unknown protocol header type \"%s\" at offset: %u --> check implementation, continuing\n"),
//                     inherited::getID (),
//                     protocol_layer.c_str (),
//                     iter->second));
//
//         break;
//       }
//     } // end SWITCH
//   } // end FOR
//
//   // step3: dump total size
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("--> total message (ID: %u) size: %u byte(s) (%u header byte(s))\n"),
//               inherited::getID (),
//               (inherited::length () + sum_header_size),
//               sum_header_size));
}
