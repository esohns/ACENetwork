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

#ifndef TEST_I_SESSION_MESSAGE_H
#define TEST_I_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

// #include "test_i_url_stream_load_common.h"

// forward declaration(s)
class ACE_Allocator;
class Test_I_Message;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

struct Test_I_URLStreamLoad_SessionData
 : Test_I_StreamSessionData
{
  Test_I_URLStreamLoad_SessionData ()
   : Test_I_StreamSessionData ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
  {}

  struct Test_I_URLStreamLoad_SessionData& operator= (const struct Test_I_URLStreamLoad_SessionData& rhs_in)
  {
    Test_I_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                                address;
  Test_I_IConnection_t*                        connection;
  enum Stream_Decoder_CompressionFormatType    format; // HTTP parser module
  std::string                                  targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_URLStreamLoad_SessionData> Test_I_URLStreamLoad_SessionData_t;

class Test_I_SessionMessage
 : public Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_URLStreamLoad_SessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_URLStreamLoad_SessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the third argument !
  Test_I_SessionMessage (Stream_SessionId_t,
                         enum Stream_SessionMessageType,
                         Test_I_URLStreamLoad_SessionData_t*&, // session data container handle
                         struct Stream_UserData*,
                         bool); // expedited ?
  // copy ctor to be used by duplicate()
  Test_I_SessionMessage (const Test_I_SessionMessage&);
  inline virtual ~Test_I_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_SessionMessage (Stream_SessionId_t,
                         ACE_Allocator*); // message allocator
  Test_I_SessionMessage (Stream_SessionId_t,
                         ACE_Data_Block*, // data block to use
                         ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_SessionMessage& operator= (const Test_I_SessionMessage&))
};

//////////////////////////////////////////

struct Test_I_URLStreamLoad_StreamState_2;
struct Test_I_URLStreamLoad_SessionData_2
 : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                 struct _AMMediaType,
#else
#if defined (FFMPEG_SUPPORT)
                                 struct Stream_MediaFramework_FFMPEG_VideoMediaType,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                                 struct Test_I_URLStreamLoad_StreamState_2,
                                 struct Stream_Statistic,
                                 struct Stream_UserData>
{
  Test_I_URLStreamLoad_SessionData_2 ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   struct _AMMediaType,
#else
#if defined (FFMPEG_SUPPORT)
                                   struct Stream_MediaFramework_FFMPEG_VideoMediaType,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                                   struct Test_I_URLStreamLoad_StreamState_2,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
  {}

  struct Test_I_URLStreamLoad_SessionData_2& operator= (const struct Test_I_URLStreamLoad_SessionData_2& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  struct _AMMediaType,
#else
#if defined (FFMPEG_SUPPORT)
                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                                  struct Test_I_URLStreamLoad_StreamState_2,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                                address;
  Test_I_IConnection_t*                        connection;
  enum Stream_Decoder_CompressionFormatType    format; // HTTP parser module
  std::string                                  targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_URLStreamLoad_SessionData_2> Test_I_URLStreamLoad_SessionData_2_t;

class Test_I_SessionMessage_2
 : public Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_URLStreamLoad_SessionData_2_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_URLStreamLoad_SessionData_2_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message,
                                                 Test_I_SessionMessage_2>;

 public:
  // *NOTE*: assumes responsibility for the third argument !
  Test_I_SessionMessage_2 (Stream_SessionId_t,
                           enum Stream_SessionMessageType,
                           Test_I_URLStreamLoad_SessionData_2_t*&, // session data container handle
                           struct Stream_UserData*,
                           bool); // expedited ?
  // copy ctor to be used by duplicate()
  Test_I_SessionMessage_2 (const Test_I_SessionMessage_2&);
  inline virtual ~Test_I_SessionMessage_2 () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_SessionMessage_2 (Stream_SessionId_t,
                           ACE_Allocator*); // message allocator
  Test_I_SessionMessage_2 (Stream_SessionId_t,
                           ACE_Data_Block*, // data block to use
                           ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_SessionMessage_2 ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_SessionMessage_2& operator= (const Test_I_SessionMessage_2&))
};

#endif
