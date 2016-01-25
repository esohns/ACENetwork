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

#ifndef DHCP_MESSAGE_H
#define DHCP_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"

#include "dhcp_codes.h"
#include "dhcp_exports.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class DHCP_SessionMessage;
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

template <typename AllocatorConfigurationType,
          typename DataType>
class DHCP_Message_T
 : public Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   DataType,
                                   DHCP_MessageType_t>
{
  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<AllocatorConfigurationType,
                                                 DHCP_Message_T<AllocatorConfigurationType,
                                                                DataType>,
                                                 DHCP_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<AllocatorConfigurationType,
                                               DHCP_Message_T<AllocatorConfigurationType,
                                                              DataType>,
                                               DHCP_SessionMessage>;

 public:
  DHCP_Message_T (unsigned int); // size
  virtual ~DHCP_Message_T ();

  virtual DHCP_MessageType_t command () const; // return value: message type
  static std::string Command2String (DHCP_MessageType_t);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // "normalize" the data in this message (fragment) by:
  // 1. aligning the rd_ptr with base() --> ACE_Message_Block::crunch()/::memmove()
  // *WARNING*: if we share buffers, this may well clobber data referenced by
  // preceding messages THAT MAY STILL BE IN USE DOWNSTREAM
  // --> safe only IFF stream processing is single-threaded !
  // --> still, we make a "best-effort", simply to reduce fragmentation...
  // 2. COPYING all bits from any continuation(s) into our buffer (until
  //    capacity() has been reached)
  // 3. adjusting the write pointer accordingly
  // 4. releasing obsoleted continuations
  // --> *NOTE*: IF this is done CONSISTENTLY, AND:
  // - our buffer has capacity for a FULL message (i.e. maximum allowed size)
  // - our peer keeps to the standard and doesn't send oversized messages (!)
  // --> THEN this measure ensures that EVERY single buffer contains a CONTIGUOUS
  //     and COMPLETE message...
  void crunch ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
   // *NOTE*: to be used by allocators...
   DHCP_Message_T (ACE_Data_Block*, // data block to use
                   ACE_Allocator*,  // message allocator
                   bool = true);    // increment running message counter ?
   //   DHCP_Message_T (ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  DHCP_Message_T (const DHCP_Message_T&);

 private:
  typedef Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   DataType,
                                   DHCP_MessageType_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (DHCP_Message_T ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Message_T& operator= (const DHCP_Message_T&))
};

// include template implementation
#include "dhcp_message.inl"

#endif
