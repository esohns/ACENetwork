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

#ifndef NET_MESSAGE_BASE_H
#define NET_MESSAGE_BASE_H

#include "ace/Global_Macros.h"

#include "stream_message_base.h"

// forward declaration(s)
class ACE_Data_Block;
class ACE_Allocator;

template <typename HeaderType,
          typename ProtocolCommandType>
class Net_MessageBase_T
 : public Stream_MessageBase
{
 public:
  virtual ~Net_MessageBase_T ();

  // used for pre-allocated messages...
  virtual void initialize (// Stream_MessageBase members
                           ACE_Data_Block*); // data block to use

  virtual ProtocolCommandType getCommand () const = 0; // return value: message type
//  static std::string CommandType2String (ProtocolCommandType);

  // implement Common_IDumpState
  virtual void dump_state () const;

  HeaderType getHeader () const;

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Net_MessageBase_T (const Net_MessageBase_T&);
  // *NOTE*: to be used by allocators...
  Net_MessageBase_T (ACE_Data_Block*, // data block to use
                     ACE_Allocator*); // message allocator

 private:
  typedef Stream_MessageBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_MessageBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_MessageBase_T& operator= (const Net_MessageBase_T&));

  bool isInitialized_;
};

#include "net_message_base.inl"

#endif
