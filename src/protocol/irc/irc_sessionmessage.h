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

#ifndef IRC_SessionMessage_T_H
#define IRC_SessionMessage_T_H

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

//#include "irc_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType>
//class Stream_MessageAllocatorHeapBase_T;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType>
//class Stream_CachedMessageAllocator_T;

template <typename SessionDataType, // reference-counted
          ////////////////////////////////
          typename UserDataType>
class IRC_SessionMessage_T
 : public Stream_SessionMessageBase_T<struct IRC_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      SessionDataType,
                                      UserDataType>
{
  // convenient types
  typedef IRC_SessionMessage_T<SessionDataType,
                               UserDataType> OWN_TYPE_T;

 // enable access to specific private ctors
 //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
 //                                               struct Stream_AllocatorConfiguration,
 //                                               ControlMessageType,
 //                                               DataMessageType,
 //                                               OWN_TYPE_T>;
 //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
 //                                             struct Stream_AllocatorConfiguration,
 //                                             ControlMessageType,
 //                                             DataMessageType,
 //                                             OWN_TYPE_T>;

 public:
  // *NOTE*: assume lifetime responsibility for the third argument !
  IRC_SessionMessage_T (Stream_SessionId_t,
                        enum Stream_SessionMessageType,
                        SessionDataType*&,              // session data container handle
                        UserDataType*);
    // *NOTE*: to be used by message allocators
  IRC_SessionMessage_T (Stream_SessionId_t,
                        ACE_Allocator*); // message allocator
  IRC_SessionMessage_T (Stream_SessionId_t,
                        ACE_Data_Block*, // data block to use
                        ACE_Allocator*); // message allocator
  inline virtual ~IRC_SessionMessage_T () {};

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // copy ctor (to be used by duplicate())
  IRC_SessionMessage_T (const IRC_SessionMessage_T&);

 private:
  typedef Stream_SessionMessageBase_T<struct IRC_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      SessionDataType,
                                      UserDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_SessionMessage_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_SessionMessage_T& operator= (const IRC_SessionMessage_T&))
};

// include template definition
#include "irc_sessionmessage.inl"

#endif
