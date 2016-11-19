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

#ifndef BitTorrent_Client_PeerMessage_H
#define BitTorrent_Client_PeerMessage_H

#include <ace/Global_Macros.h>

#include "stream_control_message.h"

#include "http_message.h"

#include "bittorrent_message.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class BitTorrent_Client_SessionMessage;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

class BitTorrent_Client_PeerMessage
 : public BitTorrent_Message_T<struct BitTorrent_Client_SessionData>
{
  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<struct BitTorrent_AllocatorConfiguration,
                                                 Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                                         struct BitTorrent_AllocatorConfiguration,
                                                                         BitTorrent_Client_PeerMessage,
                                                                         BitTorrent_Client_SessionMessage>,
                                                 BitTorrent_Client_PeerMessage,
                                                 BitTorrent_Client_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<struct BitTorrent_AllocatorConfiguration,
                                               Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                                       struct BitTorrent_AllocatorConfiguration,
                                                                       BitTorrent_Client_PeerMessage,
                                                                       BitTorrent_Client_SessionMessage>,
                                               BitTorrent_Client_PeerMessage,
                                               BitTorrent_Client_SessionMessage>;

 public:
  BitTorrent_Client_PeerMessage (unsigned int); // size
  virtual ~BitTorrent_Client_PeerMessage ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

//  static std::string CommandType2String (HTTP_Method_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  BitTorrent_Client_PeerMessage (const BitTorrent_Client_PeerMessage&);

 private:
  typedef BitTorrent_Message_T<struct BitTorrent_Client_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerMessage ())
  // *NOTE*: to be used by message allocators
  BitTorrent_Client_PeerMessage (ACE_Data_Block*, // data block
                                 ACE_Allocator*,  // message allocator
                                 bool = true);    // increment running message counter ?
  //BitTorrent_Client_PeerMessage (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerMessage& operator= (const BitTorrent_Client_PeerMessage&))
};

//////////////////////////////////////////

class BitTorrent_Client_TrackerMessage
 : public HTTP_Message_T<struct BitTorrent_AllocatorConfiguration,
                         Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                 struct BitTorrent_AllocatorConfiguration,
                                                 BitTorrent_Client_TrackerMessage,
                                                 BitTorrent_Client_SessionMessage>,
                         BitTorrent_Client_SessionMessage>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<struct BitTorrent_AllocatorConfiguration,
                                                 Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                                         struct BitTorrent_AllocatorConfiguration,
                                                                         BitTorrent_Client_TrackerMessage,
                                                                         BitTorrent_Client_SessionMessage>,
                                                 BitTorrent_Client_TrackerMessage,
                                                 BitTorrent_Client_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<struct BitTorrent_AllocatorConfiguration,
                                               Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                                       struct BitTorrent_AllocatorConfiguration,
                                                                       BitTorrent_Client_TrackerMessage,
                                                                       BitTorrent_Client_SessionMessage>,
                                               BitTorrent_Client_TrackerMessage,
                                               BitTorrent_Client_SessionMessage>;

 public:
  BitTorrent_Client_TrackerMessage (unsigned int); // size
  virtual ~BitTorrent_Client_TrackerMessage ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

//  static std::string CommandType2String (HTTP_Method_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  BitTorrent_Client_TrackerMessage (const BitTorrent_Client_TrackerMessage&);

 private:
//  typedef Stream_DataMessageBase_T<xmlDoc,
//                                   Stream_CommandType_t> inherited;
  typedef HTTP_Message_T<struct BitTorrent_AllocatorConfiguration,
                         Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                                 struct BitTorrent_AllocatorConfiguration,
                                                 BitTorrent_Client_TrackerMessage,
                                                 BitTorrent_Client_SessionMessage>,
                         BitTorrent_Client_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerMessage ())
  // *NOTE*: to be used by message allocators
  BitTorrent_Client_TrackerMessage (ACE_Data_Block*, // data block
                                    ACE_Allocator*,  // message allocator
                                    bool = true);    // increment running message counter ?
  //BitTorrent_Client_TrackerMessage (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerMessage& operator= (const BitTorrent_Client_TrackerMessage&))
};

#endif
