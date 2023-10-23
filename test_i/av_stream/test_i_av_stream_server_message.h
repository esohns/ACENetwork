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

#ifndef TEST_I_AVSTREAM_SERVER_MESSAGE_H
#define TEST_I_AVSTREAM_SERVER_MESSAGE_H

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"

#include "stream_data_message_base.h"
#include "stream_messageallocatorheap_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_message_base.h"
#include "stream_lib_mediafoundation_message_base.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_av_stream_common.h"
#include "test_i_av_stream_server_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_SessionMessage;
class Test_I_AVStream_Server_MediaFoundation_SessionMessage;
#else
class Test_I_AVStream_Server_SessionMessage;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_Message
 : public Stream_MediaFramework_DirectShow_DataMessageBase_T<struct Test_I_AVStream_Server_DirectShow_MessageData,
                                                             enum Stream_MessageType,
                                                             int>
{
  typedef Stream_MediaFramework_DirectShow_DataMessageBase_T<struct Test_I_AVStream_Server_DirectShow_MessageData,
                                                             enum Stream_MessageType,
                                                             int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_DirectShow_Message,
                                                 Test_I_AVStream_Server_DirectShow_SessionMessage>;

 public:
  Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t, // session id
                                             unsigned int);      // size
  inline virtual ~Test_I_AVStream_Server_DirectShow_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy that references the same buffer
  // *NOTE*: uses the allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual int command () const { return ACE_Message_Block::MB_DATA; }
  static std::string CommandTypeToString (int);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_AVStream_Server_DirectShow_Message (const Test_I_AVStream_Server_DirectShow_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t, // session id
                                             ACE_Allocator*);    // message allocator
  Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t, // session id
                                             ACE_Data_Block*,    // data block
                                             ACE_Allocator*,     // message allocator
                                             bool = true);       // increment running message counter ?
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_Message& operator= (const Test_I_AVStream_Server_DirectShow_Message&))
};

//////////////////////////////////////////

class Test_I_AVStream_Server_MediaFoundation_Message
 : public Stream_MediaFramework_MediaFoundation_DataMessageBase_T<struct Test_I_AVStream_Server_MediaFoundation_MessageData,
                                                                  enum Stream_MessageType>
{
  typedef Stream_MediaFramework_MediaFoundation_DataMessageBase_T<struct Test_I_AVStream_Server_MediaFoundation_MessageData,
                                                                  enum Stream_MessageType> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_MediaFoundation_Message,
                                                 Test_I_AVStream_Server_MediaFoundation_SessionMessage>;

 public:
  Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t, // session id
                                                  unsigned int);      // size
  inline virtual ~Test_I_AVStream_Server_MediaFoundation_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual int command () const { return ACE_Message_Block::MB_DATA; }
  static std::string CommandTypeToString (int);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_AVStream_Server_MediaFoundation_Message (const Test_I_AVStream_Server_MediaFoundation_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t, // session id
                                                  ACE_Allocator*);    // message allocator
  Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t, // session id
                                                  ACE_Data_Block*,    // data block
                                                  ACE_Allocator*,     // message allocator
                                                  bool = true);       // increment running message counter ?
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_Message& operator= (const Test_I_AVStream_Server_MediaFoundation_Message&))
};
#else
class Test_I_AVStream_Server_Message
 : public Stream_DataMessageBase_T<Test_I_AVStream_Server_MessageData_t,
                                   enum Stream_MessageType,
                                   int>
{
  typedef Stream_DataMessageBase_T<Test_I_AVStream_Server_MessageData_t,
                                   enum Stream_MessageType,
                                   int> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_Message,
                                                 Test_I_AVStream_Server_SessionMessage>;

 public:
  Test_I_AVStream_Server_Message (Stream_SessionId_t, // session id
                                  unsigned int);      // size
  inline virtual ~Test_I_AVStream_Server_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;
  virtual ACE_Message_Block* release (void);

  // implement Stream_MessageBase_T
  inline virtual int command () const { return ACE_Message_Block::MB_DATA; }
  static std::string CommandTypeToString (int);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_AVStream_Server_Message (const Test_I_AVStream_Server_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_Message ())
  // *NOTE*: to be used by message allocators
  Test_I_AVStream_Server_Message (Stream_SessionId_t, // session id
                                  ACE_Allocator*);    // message allocator
  Test_I_AVStream_Server_Message (Stream_SessionId_t, // session id
                                  ACE_Data_Block*,    // data block
                                  ACE_Allocator*,     // message allocator
                                  bool = true);       // increment running message counter ?
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_Message& operator= (const Test_I_AVStream_Server_Message&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
