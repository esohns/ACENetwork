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

#ifndef BITTORRENT_MESSAGE_H
#define BITTORRENT_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_data_message_base.h"

#include "http_message.h"

#include "bittorrent_common.h"
#include "bittorrent_tools.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
//template <typename SessionDataType,
//          typename UserDataType>
//class BitTorrent_SessionMessage_T;
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

struct BitTorrent_PeerMessageData
{
  BitTorrent_PeerMessageData ()
   : handShakeRecord (NULL)
   , peerRecord (NULL)
  {}
  ~BitTorrent_PeerMessageData ()
  {
    if (handShakeRecord)
      delete handShakeRecord;
    if (peerRecord)
      delete peerRecord;
  }
  inline void operator+= (BitTorrent_PeerMessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }
  inline operator struct BitTorrent_PeerHandShake& () const { ACE_ASSERT (handShakeRecord); return *handShakeRecord; }
  inline operator struct BitTorrent_PeerRecord& () const { ACE_ASSERT (peerRecord); return *peerRecord; }

  struct BitTorrent_PeerHandShake* handShakeRecord;
  struct BitTorrent_PeerRecord*    peerRecord;
};

template <typename SessionDataType, // *NOTE*: this implements Common_IReferenceCount !
          typename UserDataType>
class BitTorrent_Message_T
 : public Stream_DataMessageBase_2<Stream_DataBase_T<struct BitTorrent_PeerMessageData>,
                                   struct BitTorrent_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   enum BitTorrent_MessageType>
{
  typedef Stream_DataMessageBase_2<Stream_DataBase_T<struct BitTorrent_PeerMessageData>,
                                   struct BitTorrent_AllocatorConfiguration,
                                   enum Stream_MessageType,
                                   enum BitTorrent_MessageType> inherited;

    // enable access to specific private ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               struct BitTorrent_AllocatorConfiguration,
  //                                               Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                       struct BitTorrent_AllocatorConfiguration,
  //                                                                       BitTorrent_Message_T<SessionDataType,
  //                                                                                            UserDataType>,
  //                                                                       BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                                                   UserDataType> >,
  //                                               BitTorrent_Message_T<SessionDataType,
  //                                                                    UserDataType>,
  //                                               BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                           UserDataType> >;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             struct BitTorrent_AllocatorConfiguration,
  //                                             Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                     struct BitTorrent_AllocatorConfiguration,
  //                                                                     BitTorrent_Message_T<SessionDataType,
  //                                                                                          UserDataType>,
  //                                                                     BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                                                 UserDataType> >,
  //                                             BitTorrent_Message_T<SessionDataType,
  //                                                                  UserDataType>,
  //                                             BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                         UserDataType> >;

 public:
  BitTorrent_Message_T (unsigned int); // size
  // *NOTE*: to be used by allocators
  BitTorrent_Message_T (Stream_SessionId_t,
                        ACE_Data_Block*, // data block to use
                        ACE_Allocator*,  // message allocator
                        bool = true);    // increment running message counter ?
  //   BitTorrent_Message (ACE_Allocator*); // message allocator
  inline virtual ~BitTorrent_Message_T () {}

  //virtual enum BitTorrent_MessageType command () const; // return value: message type
  inline static std::string CommandTypeToString (enum BitTorrent_MessageType type_in) { return BitTorrent_Tools::TypeToString (type_in); }

  // implement Common_IDumpState
  virtual void dump_state () const;

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  BitTorrent_Message_T (const BitTorrent_Message_T<SessionDataType,
                                                   UserDataType>&);

 private:
  // convenient types
  typedef BitTorrent_Message_T<SessionDataType,
                               UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Message_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Message_T& operator= (const BitTorrent_Message_T&))
};

//////////////////////////////////////////

template <typename SessionDataType, // *NOTE*: this implements Common_IReferenceCount !
          typename UserDataType>
class BitTorrent_TrackerMessage_T
 : public HTTP_Message_T<struct BitTorrent_AllocatorConfiguration,
                         enum Stream_MessageType>
{
//  typedef Stream_DataMessageBase_T<xmlDoc,
//                                   Stream_CommandType_t> inherited;
  typedef HTTP_Message_T<struct BitTorrent_AllocatorConfiguration,
                         enum Stream_MessageType> inherited;

  // grant access to specific private ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               struct BitTorrent_AllocatorConfiguration,
  //                                               Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                       struct BitTorrent_AllocatorConfiguration,
  //                                                                       BitTorrent_TrackerMessage_T<SessionDataType,
  //                                                                                                   UserDataType>,
  //                                                                       BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                                                   UserDataType> >,
  //                                               BitTorrent_TrackerMessage_T<SessionDataType,
  //                                                                           UserDataType>,
  //                                               BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                           UserDataType> >;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             struct BitTorrent_AllocatorConfiguration,
  //                                             Stream_ControlMessage_T<enum Stream_ControlMessageType,
  //                                                                     struct BitTorrent_AllocatorConfiguration,
  //                                                                     BitTorrent_TrackerMessage_T<SessionDataType,
  //                                                                                                 UserDataType>,
  //                                                                     BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                                                 UserDataType> >,
  //                                             BitTorrent_TrackerMessage_T<SessionDataType,
  //                                                                         UserDataType>,
  //                                             BitTorrent_SessionMessage_T<typename SessionDataType::DATA_T,
  //                                                                         UserDataType> >;

 public:
  BitTorrent_TrackerMessage_T (unsigned int); // size
  // *NOTE*: to be used by message allocators
  BitTorrent_TrackerMessage_T (Stream_SessionId_t,
                               ACE_Data_Block*,    // data block to use
                               ACE_Allocator*,     // message allocator
                               bool = true);       // increment running message counter ?
  //BitTorrent_TrackerMessage_T (ACE_Allocator*); // message allocator
  inline virtual ~BitTorrent_TrackerMessage_T () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

//  static std::string CommandTypeToString (HTTP_Method_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  BitTorrent_TrackerMessage_T (const BitTorrent_TrackerMessage_T<SessionDataType,
                                                                 UserDataType>&);

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerMessage_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerMessage_T& operator= (const BitTorrent_TrackerMessage_T&))

  // convenient types
  typedef BitTorrent_TrackerMessage_T<SessionDataType,
                                      UserDataType> OWN_TYPE_T;
};

// include template definition
#include "bittorrent_message.inl"

#endif
