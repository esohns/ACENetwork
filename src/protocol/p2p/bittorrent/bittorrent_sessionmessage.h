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

#ifndef BITTORRENT_SESSIONMESSAGE_H
#define BITTORRENT_SESSIONMESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_message_base.h"
#include "stream_session_data.h"

#include "bittorrent_common.h"
#include "bittorrent_exports.h"
//#include "bittorrent_stream_common.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
//template <typename AllocatorType,
//          typename SessionDataType,
//          typename UserDataType>
//class BitTorrent_Message_T;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

template <typename SessionDataType, // not (!) reference-counted
          typename UserDataType>
class BitTorrent_SessionMessage_T
 : public Stream_SessionMessageBase_T<struct Net_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Stream_SessionData_T<SessionDataType>,
                                      UserDataType>
{
  typedef Stream_SessionMessageBase_T<struct Net_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Stream_SessionData_T<SessionDataType>,
                                      UserDataType> inherited;

  // enable access to specific private ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               struct Net_AllocatorConfiguration,
  //                                               Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                       struct Net_AllocatorConfiguration,
  //                                                                       BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
  //                                                                                            UserDataType>,
  //                                                                       BitTorrent_SessionMessage_T<SessionDataType,
  //                                                                                                   UserDataType> >,
  //                                               BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
  //                                                                    UserDataType>,
  //                                               BitTorrent_SessionMessage_T<SessionDataType,
  //                                                                           UserDataType> >;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             struct Net_AllocatorConfiguration,
  //                                             Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                     struct Net_AllocatorConfiguration,
  //                                                                     BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
  //                                                                                          UserDataType>,
  //                                                                     BitTorrent_SessionMessage_T<SessionDataType,
  //                                                                                                 UserDataType> >,
  //                                             BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
  //                                                                  UserDataType>,
  //                                             BitTorrent_SessionMessage_T<SessionDataType,
  //                                                                         UserDataType> >;

 public:
  // *NOTE*: assume lifetime responsibility for the third argument !
  BitTorrent_SessionMessage_T (Stream_SessionId_t,
                               enum Stream_SessionMessageType,
                               Stream_SessionData_T<SessionDataType>*&, // session data container handle
                               UserDataType*);
    // *NOTE*: to be used by message allocators
  BitTorrent_SessionMessage_T (Stream_SessionId_t,
                               ACE_Allocator*); // message allocator
  BitTorrent_SessionMessage_T (Stream_SessionId_t,
                               ACE_Data_Block*, // data block to use
                               ACE_Allocator*); // message allocator
  inline virtual ~BitTorrent_SessionMessage_T () {}

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // convenient types
  typedef BitTorrent_SessionMessage_T<SessionDataType,
                                      UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_SessionMessage_T ())
  // copy ctor (to be used by duplicate())
  BitTorrent_SessionMessage_T (const BitTorrent_SessionMessage_T&);
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_SessionMessage_T& operator= (const BitTorrent_SessionMessage_T&))
};

// include template definition
#include "bittorrent_sessionmessage.inl"

#endif
