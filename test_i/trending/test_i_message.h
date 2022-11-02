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

#ifndef TEST_I_MESSAGE_H
#define TEST_I_MESSAGE_H

#include "ace/Global_Macros.h"

#include "common_parser_common.h"

#include "stream_data_message_base.h"

#include "http_codes.h"
#include "http_defines.h"
#include "http_tools.h"

#include "test_i_trending_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_I_Stream_SessionMessage;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Test_I_Stream_MessageData
 : public Stream_DataBase_T<struct Test_I_Trending_MessageData>
 , public Common_ISetPR_T<struct HTTP_Record>
{
  typedef Stream_DataBase_T<struct Test_I_Trending_MessageData> inherited;

 public:
  Test_I_Stream_MessageData ();
  // *IMPORTANT NOTE*: fire-and-forget API
  Test_I_Stream_MessageData (struct Test_I_Trending_MessageData*&, // data handle
                             bool = true);                        // delete in dtor ?
  inline virtual ~Test_I_Stream_MessageData () {}

  // implement Common_ISetPR_T
  virtual void setPR (struct HTTP_Record*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_MessageData (const Test_I_Stream_MessageData&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_MessageData& operator= (const Test_I_Stream_MessageData&))
};

//////////////////////////////////////////

class Test_I_Stream_Message
 : public Stream_DataMessageBase_2<Test_I_Stream_MessageData,
                                   enum Stream_MessageType,
                                   HTTP_Method_t>
{
  typedef Stream_DataMessageBase_2<Test_I_Stream_MessageData,
                                   enum Stream_MessageType,
                                   HTTP_Method_t> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Stream_Message,
                                                 Test_I_Stream_SessionMessage>;

 public:
  Test_I_Stream_Message (Stream_SessionId_t, // session id
                         unsigned int);      // size
  inline virtual ~Test_I_Stream_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  virtual HTTP_Method_t command () const; // return value: message type
  inline static std::string CommandTypeToString (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
                                                                                                                                 : HTTP_Tools::MethodToString (method_in)); }

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Stream_Message (const Test_I_Stream_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Stream_Message (Stream_SessionId_t, // session id
                         ACE_Data_Block*,    // data block
                         ACE_Allocator*,     // message allocator
                         bool = true);       // increment running message counter ?
  Test_I_Stream_Message (Stream_SessionId_t, // session id
                         ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Message& operator= (const Test_I_Stream_Message&))
};

#endif
