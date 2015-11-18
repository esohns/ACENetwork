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

#ifndef TEST_I_SESSION_MESSAGE_H
#define TEST_I_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_i_common.h"

// forward declaration(s)
class ACE_Allocator;
class Test_I_Stream_Message;
template <typename MessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Test_I_Stream_SessionMessage
 : public Stream_SessionMessageBase_T<Test_I_Stream_SessionData_t,
                                      Test_I_UserData>
{
  // grant access to specific private ctors...
  friend class Stream_MessageAllocatorHeapBase_T<Test_I_Stream_Message,
                                                 Test_I_Stream_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason...
  Test_I_Stream_SessionMessage (Stream_SessionMessageType,     // session message type
                                Test_I_Stream_SessionData_t*&, // session data container handle
                                Test_I_UserData*);             // user data handle
  virtual ~Test_I_Stream_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Test_I_Stream_SessionData_t,
                                      Test_I_UserData> inherited;

  // copy ctor to be used by duplicate()
  Test_I_Stream_SessionMessage (const Test_I_Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators...
  // *WARNING*: these ctors are NOT threadsafe...
  Test_I_Stream_SessionMessage (ACE_Allocator*); // message allocator
  Test_I_Stream_SessionMessage (ACE_Data_Block*, // data block
                                ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SessionMessage& operator= (const Test_I_Stream_SessionMessage&))
};

#endif
