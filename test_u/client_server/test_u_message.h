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

#ifndef TEST_U_MESSAGE_H
#define TEST_U_MESSAGE_H

#include <string>

#include "stream_message_base.h"
#include "stream_messageallocatorheap_base.h"

#include "net_remote_comm.h"

#include "test_u_common.h"

#include "net_client_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Net_SessionMessage;

class Net_Message
 : public Stream_MessageBase_2<Stream_AllocatorConfiguration,
                               Net_ControlMessage_t,
                               Net_SessionMessage,
                               Net_Remote_Comm::MessageHeader,
                               Net_MessageType_t>
{
  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Net_ControlMessage_t,
                                                 Net_Message,
                                                 Net_SessionMessage>;

 public:
  Net_Message (unsigned int); // size
  virtual ~Net_Message ();

  virtual Net_MessageType_t command () const; // return value: message type
  static std::string CommandType2String (Net_MessageType_t);

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Net_Message (const Net_Message&);

 private:
  typedef Stream_MessageBase_2<Stream_AllocatorConfiguration,
                               Net_ControlMessage_t,
                               Net_SessionMessage,
                               Net_Remote_Comm::MessageHeader,
                               Net_MessageType_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Message ())
  // *NOTE*: to be used by allocators
  Net_Message (ACE_Data_Block*, // data block to use
               ACE_Allocator*); // message allocator
//   Net_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Net_Message& operator= (const Net_Message&))
};

#endif
