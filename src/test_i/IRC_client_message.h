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

#ifndef IRC_CLIENT_MESSAGE_H
#define IRC_CLIENT_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"

#include "IRC_client_IRCmessage.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class IRC_Client_SessionMessage;
template <typename MessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
template <typename MessageType,
          typename SessionMessageType> class Stream_CachedMessageAllocatorHeapBase_T;

class IRC_Client_Message
 : public Stream_DataMessageBase_T<IRC_Client_IRCMessage,
                                   IRC_Client_CommandType_t>
{
  // enable access to specific private ctors...
  friend class Stream_MessageAllocatorHeapBase_T<IRC_Client_Message,
                                                 IRC_Client_SessionMessage>;
  friend class Stream_CachedMessageAllocatorHeapBase_T<IRC_Client_Message,
                                                       IRC_Client_SessionMessage>;

 public:
  IRC_Client_Message (unsigned int); // size
  virtual ~IRC_Client_Message ();

  virtual IRC_Client_CommandType_t getCommand () const; // return value: message type

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

  static std::string CommandType2String (const IRC_Client_CommandType_t&);

 protected:
   // *NOTE*: to be used by allocators...
   IRC_Client_Message (ACE_Data_Block*, // data block to use
                       ACE_Allocator*); // message allocator
   //   IRC_Client_Message(ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  IRC_Client_Message (const IRC_Client_Message&);

 private:
  typedef Stream_DataMessageBase_T<IRC_Client_IRCMessage,
                                   IRC_Client_CommandType_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Message ());
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Message& operator= (const IRC_Client_Message&));
};

#endif
