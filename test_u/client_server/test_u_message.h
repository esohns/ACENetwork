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

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_message_base.h"
#include "stream_messageallocatorheap_base.h"

#include "net_remote_comm.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_U_SessionMessage;

class Test_U_Message
 : public Stream_MessageBase_2<Net_MessageType_t,
                               enum Stream_MessageType,
                               Net_Remote_Comm::MessageHeader,
                               Net_MessageType_t>
{
  typedef Stream_MessageBase_2<Net_MessageType_t,
                               enum Stream_MessageType,
                               Net_Remote_Comm::MessageHeader,
                               Net_MessageType_t> inherited;

  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_Parser_FlexAllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;

 public:
  Test_U_Message (Stream_SessionId_t, // session id
                  size_t);            // size
  inline virtual ~Test_U_Message () {}

  virtual Net_MessageType_t command () const; // return value: message type
  static std::string CommandTypeToString (Net_MessageType_t);

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_U_Message (const Test_U_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message ())
  // *NOTE*: to be used by allocators
  Test_U_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
//   Test_U_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message& operator= (const Test_U_Message&))
};

#endif
