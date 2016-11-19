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

#ifndef BITTORRENT_CLIENT_SESSIONMESSAGE_H
#define BITTORRENT_CLIENT_SESSIONMESSAGE_H

#include <ace/Global_Macros.h>

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"
#include "stream_session_message_base.h"

#include "bittorrent_stream_common.h"

#include "bittorrent_client_common.h"

// forward declarations
struct BitTorrent_Client_SessionData;
typedef Stream_SessionData_T<struct BitTorrent_Client_SessionData> BitTorrent_Client_SessionData_t;
struct BitTorrent_Client_UserData;
class BitTorrent_Client_PeerMessage;
class BitTorrent_Client_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration,
                                BitTorrent_Client_PeerMessage,
                                BitTorrent_Client_SessionMessage> BitTorrent_Client_ControlMessage_t;
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

class BitTorrent_Client_SessionMessage
 : public Stream_SessionMessageBase_T<struct BitTorrent_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      BitTorrent_Client_SessionData_t,
                                      struct BitTorrent_Client_UserData,
                                      BitTorrent_Client_ControlMessage_t,
                                      BitTorrent_Client_PeerMessage>
{
  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<struct BitTorrent_AllocatorConfiguration,
                                                 BitTorrent_Client_ControlMessage_t,
                                                 BitTorrent_Client_PeerMessage,
                                                 BitTorrent_Client_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<struct BitTorrent_AllocatorConfiguration,
                                               BitTorrent_Client_ControlMessage_t,
                                               BitTorrent_Client_PeerMessage,
                                               BitTorrent_Client_SessionMessage>;

 public:
  // *NOTE*: assume lifetime responsibility for the second argument !
  BitTorrent_Client_SessionMessage (enum Stream_SessionMessageType,      // session message type
                                    BitTorrent_Client_SessionData_t*&,   // session data container handle
                                    struct BitTorrent_Client_UserData*); // user data handle
  // *NOTE*: to be used by message allocators
  BitTorrent_Client_SessionMessage (ACE_Allocator*); // message allocator
  BitTorrent_Client_SessionMessage (ACE_Data_Block*, // data block
                                    ACE_Allocator*); // message allocator
  virtual ~BitTorrent_Client_SessionMessage ();

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<struct BitTorrent_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      BitTorrent_Client_SessionData_t,
                                      struct BitTorrent_Client_UserData,
                                      BitTorrent_Client_ControlMessage_t,
                                      BitTorrent_Client_PeerMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_SessionMessage ())
  // copy ctor (to be used by duplicate())
  BitTorrent_Client_SessionMessage (const BitTorrent_Client_SessionMessage&);
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_SessionMessage& operator= (const BitTorrent_Client_SessionMessage&))
};

#endif
