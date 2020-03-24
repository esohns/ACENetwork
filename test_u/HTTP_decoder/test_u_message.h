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

#ifndef TEST_U_MESSAGE_H
#define TEST_U_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_configuration.h"

#include "stream_common.h"
#include "stream_control_message.h"

#include "http_message.h"

//#include "test_u_HTTP_decoder_common.h"
//#include "test_u_HTTP_decoder_stream_common.h"

// forward declaration(s)
class ACE_Allocator;
class Test_U_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_Parser_FlexAllocatorConfiguration> Test_U_HTTPDecoder_ControlMessage_t;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

class Test_U_Message
 : public HTTP_Message_T<struct Common_Parser_FlexAllocatorConfiguration,
                         enum Stream_MessageType>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_Parser_FlexAllocatorConfiguration,
                                                 Test_U_HTTPDecoder_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;
  friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
                                               struct Common_Parser_FlexAllocatorConfiguration,
                                               Test_U_HTTPDecoder_ControlMessage_t,
                                               Test_U_Message,
                                               Test_U_SessionMessage>;

  typedef HTTP_Message_T<struct Common_Parser_FlexAllocatorConfiguration,
                         enum Stream_MessageType> inherited;

 public:
  Test_U_Message (unsigned int); // size
  inline virtual ~Test_U_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

//  static std::string CommandType2String (HTTP_Method_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_U_Message (const Test_U_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message ())
  // *NOTE*: to be used by message allocators
  Test_U_Message (Stream_SessionId_t,
                  ACE_Data_Block*, // data block to use
                  ACE_Allocator*,  // message allocator
                  bool = true);    // increment running message counter ?
  //Test_U_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message& operator= (const Test_U_Message&))
};

#endif
