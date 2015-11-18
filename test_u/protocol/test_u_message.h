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

#ifndef TEST_I_MESSAGE_H
#define TEST_I_MESSAGE_H

#include "ace/Global_Macros.h"

//#include <libxml/tree.h>

//#include "stream_data_message_base.h"
#include "stream_message_base.h"

#include "test_i_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_I_Stream_SessionMessage;
template <typename MessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Test_I_Stream_Message
 : public Stream_MessageBase
// : public Stream_DataMessageBase_T<xmlDoc,
//                                   Stream_CommandType_t>
{
  // grant access to specific private ctors...
  friend class Stream_MessageAllocatorHeapBase_T<Test_I_Stream_Message,
                                                 Test_I_Stream_SessionMessage>;

 public:
  Test_I_Stream_Message (unsigned int); // size
  virtual ~Test_I_Stream_Message ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  virtual Stream_CommandType_t command () const; // return value: message type

  static std::string CommandType2String (Stream_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Stream_Message (const Test_I_Stream_Message&);

 private:
//  typedef Stream_DataMessageBase_T<xmlDoc,
//                                   Stream_CommandType_t> inherited;
  typedef Stream_MessageBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Message ())
  // *NOTE*: to be used by message allocators...
  Test_I_Stream_Message (ACE_Data_Block*, // data block
                         ACE_Allocator*,  // message allocator
                         bool = true);    // increment running message counter ?
  Test_I_Stream_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Message& operator= (const Test_I_Stream_Message&))
};

#endif
