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

#ifndef TEST_U_SESSIONMESSAGE_H
#define TEST_U_SESSIONMESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_message_base.h"

#include "net_configuration.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_U_Message;

class Test_U_SessionMessage
 : public Stream_SessionMessageBase_T<struct Net_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      ClientServer_StreamSessionData_t,
                                      struct Test_U_UserData>
{
  // enable access to private ctor(s)
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Net_AllocatorConfiguration,
                                                 Test_U_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;

  typedef Stream_SessionMessageBase_T<struct Net_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      ClientServer_StreamSessionData_t,
                                      struct Test_U_UserData> inherited;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  Test_U_SessionMessage (Stream_SessionId_t,
                         enum Stream_SessionMessageType,
                         ClientServer_StreamSessionData_t*&, // session data handle
                         struct Test_U_UserData*);
    // *NOTE*: to be used by message allocators
  Test_U_SessionMessage (Stream_SessionId_t,
                         ACE_Allocator*); // message allocator
  Test_U_SessionMessage (Stream_SessionId_t,
                         ACE_Data_Block*, // data block to use
                         ACE_Allocator*); // message allocator
  inline virtual ~Test_U_SessionMessage () {}

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_SessionMessage ())
  // copy ctor (to be used by duplicate())
  Test_U_SessionMessage (const Test_U_SessionMessage&);
  ACE_UNIMPLEMENTED_FUNC (Test_U_SessionMessage& operator= (const Test_U_SessionMessage&))
};

#endif
