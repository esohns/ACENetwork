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

#ifndef IRC_MESSAGE_H
#define IRC_MESSAGE_H

#include <ace/Global_Macros.h>

#include "stream_data_message_base.h"

#include "irc_common.h"
#include "irc_exports.h"
#include "irc_record.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class IRC_SessionMessage;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CachedMessageAllocator_T;

class IRC_Export IRC_Message
 : public Stream_DataMessageBase_T<Stream_AllocatorConfiguration,
                                   IRC_ControlMessage_t,
                                   IRC_SessionMessage,
                                   IRC_Record,
                                   IRC_CommandType_t>
{
  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 IRC_ControlMessage_t,
                                                 IRC_Message,
                                                 IRC_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<Stream_AllocatorConfiguration,
                                               IRC_ControlMessage_t,
                                               IRC_Message,
                                               IRC_SessionMessage>;

 public:
  IRC_Message (unsigned int); // size
  virtual ~IRC_Message ();

  virtual IRC_CommandType_t command () const; // return value: message type

  // implement Common_IDumpState
  virtual void dump_state () const;

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  static std::string CommandType2String (const IRC_CommandType_t&);

 protected:
   // *NOTE*: to be used by allocators
   IRC_Message (ACE_Data_Block*, // data block to use
                ACE_Allocator*); // message allocator
   //   IRC_Client_Message(ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  IRC_Message (const IRC_Message&);

 private:
  typedef Stream_DataMessageBase_T<Stream_AllocatorConfiguration,
                                   IRC_ControlMessage_t,
                                   IRC_SessionMessage,
                                   IRC_Record,
                                   IRC_CommandType_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Message ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Message& operator= (const IRC_Message&))
};

#endif
