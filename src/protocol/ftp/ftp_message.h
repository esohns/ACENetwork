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

#ifndef FTP_MESSAGE_H
#define FTP_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_message_base.h"
#include "stream_session_data.h"

#include "ftp_common.h"
#include "ftp_tools.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
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
typedef Stream_DataBase_T<struct FTP_Record> FTP_MessageData_t;

struct FTP_Stream_SessionData;
typedef Stream_SessionData_T<struct FTP_Stream_SessionData> FTP_Stream_SessionData_t;
template <typename MessageData,
          ////////////////////////////////
          typename SessionDataType, // reference-counted
          ////////////////////////////////
          typename UserDataType>
class FTP_SessionMessage_T;

template <typename MessageData = FTP_MessageData_t,
          typename MessageType = enum Stream_MessageType>
class FTP_Message_T
 : public Stream_DataMessageBase_2<MessageData,
                                   MessageType,
                                   FTP_Code_t>
{
  typedef Stream_DataMessageBase_2<MessageData,
                                   MessageType,
                                   FTP_Code_t> inherited;

  // enable access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct FTP_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 FTP_Message_T<MessageData,
                                                               MessageType>,
                                                 FTP_SessionMessage_T<MessageData,
                                                                      FTP_Stream_SessionData_t,
                                                                      struct Stream_UserData> >;
  friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
                                               struct FTP_AllocatorConfiguration,
                                               Stream_ControlMessage_t,
                                               FTP_Message_T<MessageData,
                                                             MessageType>,
                                               FTP_SessionMessage_T<MessageData,
                                                                    FTP_Stream_SessionData_t,
                                                                    struct Stream_UserData> >;

 public:
  // convenient types
  typedef FTP_Message_T<MessageData,
                        MessageType> OWN_TYPE_T;

  FTP_Message_T (Stream_SessionId_t, // session id
                 unsigned int);      // size
  inline virtual ~FTP_Message_T () {}

  virtual FTP_Code_t command () const; // return value: message type
  inline static std::string CommandToString (FTP_Code_t code_in) { return FTP_Tools::CodeToString (code_in); }

  // implement Common_IDumpState
  virtual void dump_state () const;

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // *NOTE*: to be used by allocators
  FTP_Message_T (Stream_SessionId_t, // session id
                 ACE_Data_Block*,    // data block to use
                 ACE_Allocator*,     // message allocator
                 bool = true);       // increment running message counter ?
//   FTP_Message_T (ACE_Allocator*); // message allocator

  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  FTP_Message_T (const OWN_TYPE_T&);

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Message_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Message_T& operator= (const FTP_Message_T&))
};

// include template definition
#include "ftp_message.inl"

#endif
