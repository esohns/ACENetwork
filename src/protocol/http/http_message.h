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

#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_data_base.h"
#include "stream_data_message_base.h"

#include "http_codes.h"
#include "http_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
//class HTTP_SessionMessage;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

template <typename AllocatorConfigurationType,
          typename MessageType>
class HTTP_Message_T
 : public Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   MessageType,
                                   Stream_DataBase_T<struct HTTP_Record>,
                                   HTTP_Method_t>
{
  // enable access to specific private ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               AllocatorConfigurationType,
  //                                               ControlMessageType,
  //                                               HTTP_Message_T<AllocatorConfigurationType,
  //                                                              ControlMessageType,
  //                                                              SessionMessageType>,
  //                                               SessionMessageType>;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             AllocatorConfigurationType,
  //                                             ControlMessageType,
  //                                             HTTP_Message_T<AllocatorConfigurationType,
  //                                                            ControlMessageType,
  //                                                            SessionMessageType>,
  //                                             SessionMessageType>;

 public:
  HTTP_Message_T (unsigned int); // size
  virtual ~HTTP_Message_T ();

  virtual HTTP_Method_t command () const; // return value: message type
  static std::string CommandToString (HTTP_Method_t);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // "normalize" the data in this message (fragment) by:
  // 1. aligning the rd_ptr with base()
  //    --> ACE_Message_Block::crunch()/::memmove()
  // *WARNING*: if we share buffers, this may well clobber data referenced by
  // preceding messages THAT MAY STILL BE IN USE DOWNSTREAM
  // --> safe only IFF stream processing is single-threaded !
  // --> still, we make a "best-effort", simply to reduce fragmentation
  // 2. COPYING all bits from any continuation(s) into our buffer (until
  //    capacity() has been reached)
  // 3. adjusting the write pointer accordingly
  // 4. releasing obsoleted continuations
  // --> *NOTE*: IF this is done CONSISTENTLY, AND:
  // - our buffer has capacity for a FULL message (i.e. maximum allowed size)
  // - our peer keeps to the standard and doesn't send oversized messages (!)
  // --> THEN this measure ensures that EVERY single buffer contains a
  //     CONTIGUOUS and COMPLETE message
  virtual void crunch ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // *NOTE*: to be used by allocators
  HTTP_Message_T (Stream_SessionId_t,
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
  //   HTTP_Message_T (Stream_SessionId_t,
//                     ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  HTTP_Message_T (const HTTP_Message_T&);

 private:
  typedef Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   MessageType,
                                   Stream_DataBase_T<struct HTTP_Record>,
                                   HTTP_Method_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Message_T ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_Message_T& operator= (const HTTP_Message_T&))

  // convenient types
  typedef HTTP_Message_T<AllocatorConfigurationType,
                         MessageType> OWN_TYPE_T;
};

// include template definition
#include "http_message.inl"

#endif
