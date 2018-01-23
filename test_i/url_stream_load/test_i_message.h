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

#include "stream_data_message_base.h"

#include "http_message.h"

#include "test_i_url_stream_load_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_I_SessionMessage;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Test_I_MessageDataContainer
 : public Stream_DataBase_T<struct Test_I_MessageData>
 , public Common_ISetPR_T<struct HTTP_Record>
{
 public:
   Test_I_MessageDataContainer ();
  // *IMPORTANT NOTE*: fire-and-forget API
  Test_I_MessageDataContainer (struct Test_I_MessageData*&, // data handle
                               bool = true);                // delete in dtor ?
  inline virtual ~Test_I_MessageDataContainer () {};

  // implement Common_ISetPP_T
  virtual void setPR (struct HTTP_Record*&);

 private:
  typedef Stream_DataBase_T<struct Test_I_MessageData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_MessageDataContainer (const Test_I_MessageDataContainer&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MessageDataContainer& operator= (const Test_I_MessageDataContainer&))
};

//////////////////////////////////////////

class Test_I_Message
 : public Stream_DataMessageBase_2<struct Common_FlexParserAllocatorConfiguration,
                                   enum Stream_MessageType,
                                   Test_I_MessageDataContainer,
                                   HTTP_Method_t>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_FlexParserAllocatorConfiguration,
                                                 Test_I_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage>;

 public:
  Test_I_Message (unsigned int); // size
  inline virtual ~Test_I_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Message (const Test_I_Message&);

 private:
  typedef Stream_DataMessageBase_2<struct Common_FlexParserAllocatorConfiguration,
                                   enum Stream_MessageType,
                                   Test_I_MessageDataContainer,
                                   HTTP_Method_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Message (Stream_SessionId_t,
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message block allocator
                  bool = true);       // increment running message counter ?
  //Test_I_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message& operator= (const Test_I_Message&))
};

#endif
