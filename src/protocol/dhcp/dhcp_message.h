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

#ifndef DHCP_MESSAGE_H
#define DHCP_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"

#include "dhcp_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
//class DHCP_SessionMessage;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CachedMessageAllocator_T;

template <typename AllocatorConfigurationType,
          typename MessageType>
class DHCP_Message_T
 : public Stream_DataMessageBase_T<struct DHCP_Record,
                                   AllocatorConfigurationType,
                                   MessageType,
                                   DHCP_MessageType_t>
{
  typedef Stream_DataMessageBase_T<struct DHCP_Record,
                                   AllocatorConfigurationType,
                                   MessageType,
                                   DHCP_MessageType_t> inherited;

    // enable access to specific private ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               AllocatorConfigurationType,
  //                                               ControlMessageType,
  //                                               DHCP_Message_T<AllocatorConfigurationType,
  //                                                              MessageType>,
  //                                               SessionMessageType>;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             AllocatorConfigurationType,
  //                                             ControlMessageType,
  //                                             DHCP_Message_T<AllocatorConfigurationType,
  //                                                            MessageType>,
  //                                             SessionMessageType>;

 public:
  // convenient types
  typedef DHCP_Message_T<AllocatorConfigurationType,
                         MessageType> OWN_TYPE_T;

  DHCP_Message_T (unsigned int); // size
  inline virtual ~DHCP_Message_T () {};

  virtual DHCP_MessageType_t command () const; // return value: message type
  static std::string CommandToString (DHCP_MessageType_t);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // *NOTE*: to be used by allocators
  DHCP_Message_T (Stream_SessionId_t,
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
//   DHCP_Message_T (ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  DHCP_Message_T (const OWN_TYPE_T&);

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Message_T ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Message_T& operator= (const DHCP_Message_T&))
};

// include template definition
#include "dhcp_message.inl"

#endif
