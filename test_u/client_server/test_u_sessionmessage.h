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

#ifndef NET_SESSIONMESSAGE_H
#define NET_SESSIONMESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_session_message_base.h"

#include "test_u_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
// class Net_StreamMessageAllocator;
//template <typename MessageType,
//          typename SessionMessageType> class Stream_MessageAllocatorHeapBase;

class Net_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      ///
                                      Net_StreamSessionData_t,
                                      Net_UserData>
{
//  // enable access to private ctor(s)...
//  friend class Net_StreamMessageAllocator;
//  friend class Stream_MessageAllocatorHeapBase<Net_Message, Net_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  Net_SessionMessage (Stream_SessionMessageType, // session message type
                      Net_StreamSessionData_t*&, // session data handle
                      Net_UserData*);            // user data handle
    // *NOTE*: to be used by message allocators...
  Net_SessionMessage (ACE_Allocator*); // message allocator
  Net_SessionMessage (ACE_Data_Block*, // data block
                      ACE_Allocator*); // message allocator
  virtual ~Net_SessionMessage ();

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      ///
                                      Net_StreamSessionData_t,
                                      Net_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SessionMessage ())
  // copy ctor (to be used by duplicate())
  Net_SessionMessage (const Net_SessionMessage&);
  ACE_UNIMPLEMENTED_FUNC (Net_SessionMessage& operator= (const Net_SessionMessage&))
};

#endif
