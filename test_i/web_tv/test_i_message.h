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

#include "stream_lib_imediatype.h"

#include "http_message.h"

#include "test_i_web_tv_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_I_SessionMessage;
class Test_I_SessionMessage_3;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Test_I_MessageDataContainer
 : public Stream_DataBase_T<struct Test_I_WebTV_MessageData>
 , public Common_ISetPR_T<struct HTTP_Record>
{
  typedef Stream_DataBase_T<struct Test_I_WebTV_MessageData> inherited;

 public:
   Test_I_MessageDataContainer ();
  // *IMPORTANT NOTE*: fire-and-forget API
  Test_I_MessageDataContainer (struct Test_I_WebTV_MessageData*&, // data handle
                               bool = true);                              // delete in dtor ?
  inline virtual ~Test_I_MessageDataContainer () {}

  // implement Common_ISetPP_T
  virtual void setPR (struct HTTP_Record*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_MessageDataContainer (const Test_I_MessageDataContainer&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MessageDataContainer& operator= (const Test_I_MessageDataContainer&))
};

//////////////////////////////////////////

class Test_I_Message
 : public Stream_DataMessageBase_2<Test_I_MessageDataContainer,
                                   enum Stream_MessageType,
                                   HTTP_Method_t>
 , public Stream_IMediaType
{
  typedef Stream_DataMessageBase_2<Test_I_MessageDataContainer,
                                   enum Stream_MessageType,
                                   HTTP_Method_t> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage>;
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage_3>;

 public:
  Test_I_Message (Stream_SessionId_t, // session id
                  unsigned int);      // size
  inline virtual ~Test_I_Message () {}

  // overrides from ACE_Message_Block
  // *NOTE*: these use the allocator (if any)
  // create a "deep" copy
  virtual ACE_Message_Block* clone (ACE_Message_Block::Message_Flags = 0) const;
  // create a "shallow" copy that references the current block(s) of data
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_IMediaType
  inline virtual enum Stream_MediaType_Type getMediaType () const { return mediaType_; }
  inline virtual void setMediaType (enum Stream_MediaType_Type mediaType_in) { mediaType_ = mediaType_in; }

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Message (const Test_I_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message block allocator
                  bool = true);       // increment running message counter ?
  //Test_I_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message& operator= (const Test_I_Message&))

  enum Stream_MediaType_Type mediaType_;
};

#endif
