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

#ifndef TEST_I_AVSTREAM_SERVER_SESSION_MESSAGE_H
#define TEST_I_AVSTREAM_SERVER_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_message_base.h"
#include "stream_messageallocatorheap_base.h"

// #include "test_i_av_stream_server_common.h"

// forward declaration(s)
class ACE_Allocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_Message;
class Test_I_AVStream_Server_MediaFoundation_Message;
#endif
class Test_I_AVStream_Server_Message;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                        struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                        struct Test_I_AVStream_Server_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Server_DirectShow_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                   struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                   struct Test_I_AVStream_Server_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
   , targetFileName ()
   , windowController (NULL)
  {}

  Test_I_AVStream_Server_DirectShow_StreamSessionData& operator+= (const Test_I_AVStream_Server_DirectShow_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                  struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                  struct Test_I_AVStream_Server_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
  std::string            targetFileName;
  IVideoWindow*          windowController;
};
typedef Stream_SessionData_T<Test_I_AVStream_Server_DirectShow_StreamSessionData> Test_I_AVStream_Server_DirectShow_StreamSessionData_t;

class Test_I_AVStream_Server_MediaFoundation_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                        struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Server_MediaFoundation_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                   struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                   struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
   , outputFormat (NULL)
   , sourceFormat (NULL)
  {}

  Test_I_AVStream_Server_MediaFoundation_StreamSessionData& operator+= (const Test_I_AVStream_Server_MediaFoundation_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                  struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                  struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
  IMFMediaType*          outputFormat;
  IMFMediaType*          sourceFormat;
};
typedef Stream_SessionData_T<Test_I_AVStream_Server_MediaFoundation_StreamSessionData> Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t;
#else
class Test_I_AVStream_Server_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                        struct Stream_MediaFramework_ALSA_V4L_Format,
                                        struct Test_I_AVStream_Server_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Server_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                   struct Stream_MediaFramework_ALSA_V4L_Format,
                                   struct Test_I_AVStream_Server_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
  {}

  Test_I_AVStream_Server_StreamSessionData& operator+= (const Test_I_AVStream_Server_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                  struct Stream_MediaFramework_ALSA_V4L_Format,
                                  struct Test_I_AVStream_Server_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    targetFileName =
      (targetFileName.empty () ? rhs_in.targetFileName : targetFileName);

    return *this;
  }

  Net_IINETConnection_t* connection;
};
typedef Stream_SessionData_T<Test_I_AVStream_Server_StreamSessionData> Test_I_AVStream_Server_StreamSessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_SessionMessage
 : public Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_DirectShow_Message,
                                                 Test_I_AVStream_Server_DirectShow_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the third argument !
  Test_I_AVStream_Server_DirectShow_SessionMessage (Stream_SessionId_t,
                                                    enum Stream_SessionMessageType,
                                                    Test_I_AVStream_Server_DirectShow_StreamSessionData_t*&, // session data container handle
                                                    struct Stream_UserData*,
                                                    bool); // expedited ?
  inline virtual ~Test_I_AVStream_Server_DirectShow_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_AVStream_Server_DirectShow_SessionMessage (const Test_I_AVStream_Server_DirectShow_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_AVStream_Server_DirectShow_SessionMessage (Stream_SessionId_t,
                                                    ACE_Allocator*); // message allocator
  Test_I_AVStream_Server_DirectShow_SessionMessage (Stream_SessionId_t,
                                                    ACE_Data_Block*, // data block to use
                                                    ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_SessionMessage& operator= (const Test_I_AVStream_Server_DirectShow_SessionMessage&))
};

class Test_I_AVStream_Server_MediaFoundation_SessionMessage
 : public Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_MediaFoundation_Message,
                                                 Test_I_AVStream_Server_MediaFoundation_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the third argument !
  Test_I_AVStream_Server_MediaFoundation_SessionMessage (Stream_SessionId_t,
                                                         enum Stream_SessionMessageType,
                                                         Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t*&,   // session data container handle
                                                         struct Stream_UserData*,
                                                         bool); // expedited ?
  inline virtual ~Test_I_AVStream_Server_MediaFoundation_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_AVStream_Server_MediaFoundation_SessionMessage (const Test_I_AVStream_Server_MediaFoundation_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_AVStream_Server_MediaFoundation_SessionMessage (Stream_SessionId_t,
                                                         ACE_Allocator*); // message allocator
  Test_I_AVStream_Server_MediaFoundation_SessionMessage (Stream_SessionId_t,
                                                         ACE_Data_Block*, // data block to use
                                                         ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_SessionMessage& operator= (const Test_I_AVStream_Server_MediaFoundation_SessionMessage&))
};
#else
class Test_I_AVStream_Server_SessionMessage
 : public Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_StreamSessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_AVStream_Server_StreamSessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_AVStream_Server_Message,
                                                 Test_I_AVStream_Server_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the third argument !
  Test_I_AVStream_Server_SessionMessage (Stream_SessionId_t,
                                         enum Stream_SessionMessageType,
                                         Test_I_AVStream_Server_StreamSessionData_t*&, // session data container handle
                                         struct Stream_UserData*,
                                         bool); // expedited ?
  inline virtual ~Test_I_AVStream_Server_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_AVStream_Server_SessionMessage (const Test_I_AVStream_Server_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_AVStream_Server_SessionMessage (Stream_SessionId_t,
                                         ACE_Allocator*); // message allocator
  Test_I_AVStream_Server_SessionMessage (Stream_SessionId_t,
                                         ACE_Data_Block*, // data block to use
                                         ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_SessionMessage& operator= (const Test_I_AVStream_Server_SessionMessage&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
