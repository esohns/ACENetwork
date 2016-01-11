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

#ifndef HTTP_SessionMessage_H
#define HTTP_SessionMessage_H

#include "ace/Global_Macros.h"

#include "stream_session_message_base.h"

#include "http_exports.h"
#include "http_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

class HTTP_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      HTTP_Stream_SessionData_t,
                                      HTTP_Stream_UserData>
{
//  // enable access to private ctor(s)...
//  friend class Net_StreamMessageAllocator;
//  friend class Stream_MessageAllocatorHeapBase<Net_Message, Net_SessionMessage>;

 public:
  // *NOTE*: assume lifetime responsibility for the second argument !
  HTTP_SessionMessage (Stream_SessionMessageType,  // session message type
                       HTTP_Stream_SessionData_t*&, // session data container handle
                       HTTP_Stream_UserData*);      // user data handle
    // *NOTE*: to be used by message allocators...
  HTTP_SessionMessage (ACE_Allocator*); // message allocator
  HTTP_SessionMessage (ACE_Data_Block*, // data block
                       ACE_Allocator*); // message allocator
  virtual ~HTTP_SessionMessage ();

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      HTTP_Stream_SessionData_t,
                                      HTTP_Stream_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTP_SessionMessage ())
  // copy ctor (to be used by duplicate())
  HTTP_SessionMessage (const HTTP_SessionMessage&);
  ACE_UNIMPLEMENTED_FUNC (HTTP_SessionMessage& operator= (const HTTP_SessionMessage&))
};

#endif