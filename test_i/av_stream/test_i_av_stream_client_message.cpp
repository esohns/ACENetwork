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
#include "stdafx.h"

#include "test_i_av_stream_client_message.h"

#include "ace/Malloc_Base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#else
#include "libv4l2.h"
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message"));

}

Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message (const Test_I_AVStream_Client_DirectShow_Message& message_in)
 : inherited (message_in)
 , mediaType_ (message_in.mediaType_)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message"));

}

Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      ACE_Data_Block* dataBlock_in,
                                                                                      ACE_Allocator* messageAllocator_in,
                                                                                      bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message"));

}

Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::Test_I_AVStream_Client_DirectShow_Message"));

}

Test_I_AVStream_Client_DirectShow_Message::~Test_I_AVStream_Client_DirectShow_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::~Test_I_AVStream_Client_DirectShow_Message"));

  // release media sample ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release (); inherited::data_.sample = NULL;
  } // end IF
}

ACE_Message_Block*
Test_I_AVStream_Client_DirectShow_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::duplicate"));

  Test_I_AVStream_Client_DirectShow_Message* message_p = NULL;

  // create a new Test_I_AVStream_Client_DirectShow_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_AVStream_Client_DirectShow_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_AVStream_Client_DirectShow_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_AVStream_Client_DirectShow_Message),
                                                                                                                                   '\0')),
                             Test_I_AVStream_Client_DirectShow_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_AVStream_Client_DirectShow_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_AVStream_Client_DirectShow_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  ULONG reference_count = 0;
  if (inherited::data_.sample)
    reference_count = inherited::data_.sample->AddRef ();

  return message_p;
}
ACE_Message_Block*
Test_I_AVStream_Client_DirectShow_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::release"));

  ULONG reference_count = 0;
  if (inherited::data_.sample)
    reference_count = inherited::data_.sample->Release ();
  if (reference_count == 0)
    inherited::data_.sample = NULL;

  return inherited::release ();
}

std::string
Test_I_AVStream_Client_DirectShow_Message::CommandTypeToString (enum Stream_MediaType_Type mediaType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Message::CommandTypeToString"));

  std::string result;

  switch (mediaType_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
      result = ACE_TEXT_ALWAYS_CHAR ("AUDIO"); break;
    case STREAM_MEDIATYPE_VIDEO:
      result = ACE_TEXT_ALWAYS_CHAR ("VIDEO"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type (was: %d), aborting\n"),
                  mediaType_in));
      break;
    }
  } // end SWITCH

  return result;
}

//////////////////////////////////////////

Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message"));

}

Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message (const Test_I_AVStream_Client_MediaFoundation_Message& message_in)
 : inherited (message_in)
 , mediaType_ (message_in.mediaType_)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message"));

}

Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                ACE_Data_Block* dataBlock_in,
                                                                                                ACE_Allocator* messageAllocator_in,
                                                                                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message"));

}

Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::Test_I_AVStream_Client_MediaFoundation_Message"));

}

Test_I_AVStream_Client_MediaFoundation_Message::~Test_I_AVStream_Client_MediaFoundation_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::~Test_I_AVStream_Client_MediaFoundation_Message"));

  // release media sample ?
  if (inherited::data_.sample)
  {
    inherited::data_.sample->Release (); inherited::data_.sample = NULL;
  } // end IF
}

ACE_Message_Block*
Test_I_AVStream_Client_MediaFoundation_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::duplicate"));

  Test_I_AVStream_Client_MediaFoundation_Message* message_p = NULL;

  // create a new Test_I_AVStream_Client_MediaFoundation_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
      Test_I_AVStream_Client_MediaFoundation_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to alloc() does not really matter, as this creates
    //         a shallow copy of the existing data block
    ACE_NEW_MALLOC_NORETURN (message_p,
      static_cast<Test_I_AVStream_Client_MediaFoundation_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_AVStream_Client_MediaFoundation_Message),
                                                                                                                 '\0')),
      Test_I_AVStream_Client_MediaFoundation_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_AVStream_Client_MediaFoundation_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
        ACE_TEXT ("failed to Test_I_AVStream_Client_MediaFoundation_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  ULONG reference_count = 0;
  if (inherited::data_.sample)
    reference_count = inherited::data_.sample->AddRef ();

  return message_p;
}

ACE_Message_Block*
Test_I_AVStream_Client_MediaFoundation_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::release"));

  ULONG reference_count = 0;
  if (inherited::data_.sample)
    reference_count = inherited::data_.sample->Release ();
  if (reference_count == 0)
    inherited::data_.sample = NULL;

  return inherited::release ();
}

std::string
Test_I_AVStream_Client_MediaFoundation_Message::CommandTypeToString (enum Stream_MediaType_Type mediaType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Message::CommandTypeToString"));

  std::string result;

  switch (mediaType_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
      result = ACE_TEXT_ALWAYS_CHAR ("AUDIO"); break;
    case STREAM_MEDIATYPE_VIDEO:
      result = ACE_TEXT_ALWAYS_CHAR ("VIDEO"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type (was: %d), aborting\n"),
                  mediaType_in));
      break;
    }
  } // end SWITCH

  return result;
}
#else
Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message (Stream_SessionId_t sessionId_in,
                                                                        unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
 , inherited2 (1, false)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message"));

}

Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message (const Test_I_AVStream_Client_V4L_Message& message_in)
 : inherited (message_in)
 , inherited2 (1, false)
 , mediaType_ (message_in.mediaType_)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message"));

}

Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message (Stream_SessionId_t sessionId_in,
                                                                        ACE_Data_Block* dataBlock_in,
                                                                        ACE_Allocator* messageAllocator_in,
                                                                        bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
 , inherited2 (1, false)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message"));

}

Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message (Stream_SessionId_t sessionId_in,
                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
 , inherited2 (1, false)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::Test_I_AVStream_Client_V4L_Message"));

}

ACE_Message_Block*
Test_I_AVStream_Client_V4L_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::duplicate"));

  Test_I_AVStream_Client_V4L_Message* message_p = NULL;

  message_p = const_cast<Test_I_AVStream_Client_V4L_Message*> (this);
  message_p->increase ();

  return message_p;
}

ACE_Message_Block*
Test_I_AVStream_Client_V4L_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::release"));

  int result = -1;
  int error = 0;

  // release any continuations first
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  unsigned int reference_count = inherited2::decrease ();
  if (reference_count > 0)
    return NULL; // done

  if ((inherited::data_.device == -1) || // not a device data buffer
      inherited::data_.release)          // clean up (device data)
    return inherited::release ();

  // reset reference counter/message
  reference_count = inherited2::increase ();
  ACE_ASSERT (reference_count == 1);
  //  inherited::rd_ptr (inherited::base ());

  // *NOTE*: this is a device data buffer
  //         --> return it to the pool

  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.index = inherited::data_.index;
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = inherited::data_.method;
  switch (inherited::data_.method)
  {
    case V4L2_MEMORY_USERPTR:
    {
      buffer.m.userptr =
          reinterpret_cast<unsigned long> (inherited::rd_ptr ());
      buffer.length = inherited::size ();
      break;
    }
    case V4L2_MEMORY_MMAP:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown method (was: %d), returning\n"),
                  inherited::data_.method));
      return NULL;
    }
  } // end SWITCH
    // *NOTE*: in oder to retrieve the buffer instance handle from the device
    //         buffer when it has written the frame data, the address of the
    //         ACE_Message_Block (!) could be embedded in the 'reserved' field(s).
    //         Unfortunately this does not work, the fields are 0-ed by the driver
    //         --> maintain a mapping: buffer index <--> buffer handle
    //        buffer.reserved = reinterpret_cast<unsigned long> (message_block_p);
  result = v4l2_ioctl (inherited::data_.device,
                       VIDIOC_QBUF,
                       &buffer);
  if (result == -1)
  {
    error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to V4L_ioctl(%d,%s): \"%m\", continuing\n"),
                  inherited::data_.device, ACE_TEXT ("VIDIOC_QBUF")));
  } // end IF

    //  unsigned int done = 0;
    //  unsigned int queued =
    //      Stream_Module_Device_Tools::queued (inherited::data_.device,
    //                                          32,
    //                                          done);
    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("returned device buffer %d to the pool (queued/done: %u/%u)...\n"),
    //              inherited::data_.index,
    //              queued, done));

  return NULL;
}

std::string
Test_I_AVStream_Client_V4L_Message::CommandTypeToString (enum Stream_MediaType_Type mediaType_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Message::CommandTypeToString"));

  std::string result;

  switch (mediaType_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
      result = ACE_TEXT_ALWAYS_CHAR ("AUDIO"); break;
    case STREAM_MEDIATYPE_VIDEO:
      result = ACE_TEXT_ALWAYS_CHAR ("VIDEO"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type (was: %d), aborting\n"),
                  mediaType_in));
      break;
    }
  } // end SWITCH

  return result;
}
#endif // ACE_WIN32 || ACE_WIN64
