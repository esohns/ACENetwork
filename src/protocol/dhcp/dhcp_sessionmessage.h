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

#ifndef DHCP_SessionMessage_T_H
#define DHCP_SessionMessage_T_H

#include <ace/Global_Macros.h>

#include "stream_session_message_base.h"

#include "dhcp_common.h"
#include "dhcp_exports.h"
#include "dhcp_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionDataType>
class DHCP_SessionMessage_T
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      SessionDataType,
                                      DHCP_Stream_UserData,
                                      ControlMessageType,
                                      DataMessageType>
{
//  // enable access to private ctor(s)
//  friend class Net_StreamMessageAllocator;
//  friend class Stream_MessageAllocatorHeapBase<Net_Message, Net_SessionMessage>;

 public:
  // convenient types
  typedef DHCP_SessionMessage_T<ControlMessageType,
                                DataMessageType,
                                SessionDataType> OWN_TYPE_T;

  // *NOTE*: assume lifetime responsibility for the second argument !
  DHCP_SessionMessage_T (Stream_SessionMessageType, // session message type
                         SessionDataType*&,         // session data container handle
                         DHCP_Stream_UserData*);    // user data handle
  // *NOTE*: to be used by message allocators
  DHCP_SessionMessage_T (ACE_Allocator*); // message allocator
  DHCP_SessionMessage_T (ACE_Data_Block*, // data block
                         ACE_Allocator*); // message allocator
  virtual ~DHCP_SessionMessage_T ();

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      SessionDataType,
                                      DHCP_Stream_UserData,
                                      ControlMessageType,
                                      DataMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (DHCP_SessionMessage_T ())
  // copy ctor (to be used by duplicate())
  DHCP_SessionMessage_T (const DHCP_SessionMessage_T&);
  ACE_UNIMPLEMENTED_FUNC (DHCP_SessionMessage_T& operator= (const DHCP_SessionMessage_T&))
};

// include template definition
#include "dhcp_sessionmessage.inl"

#endif
