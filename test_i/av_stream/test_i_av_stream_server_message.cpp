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

#include "test_i_av_stream_server_message.h"

#include "ace/Malloc_Base.h"

#include "net_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      size_t size_in)
 : inherited (sessionId_in,
              size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message"));

}

Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message (const Test_I_AVStream_Server_DirectShow_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message"));

}

Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      ACE_Data_Block* dataBlock_in,
                                                                                      ACE_Allocator* messageAllocator_in,
                                                                                      bool incrementMessageCounter_in)
 : inherited (sessionId_in,               // session id
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message"));

}

Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message (Stream_SessionId_t sessionId_in,
                                                                                      ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,        // session id
              messageAllocator_in) // message allocator
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::Test_I_AVStream_Server_DirectShow_Message"));

}

ACE_Message_Block*
Test_I_AVStream_Server_DirectShow_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::duplicate"));

  Test_I_AVStream_Server_DirectShow_Message* message_p = NULL;

  // create a new Test_I_AVStream_Server_DirectShow_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_AVStream_Server_DirectShow_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_AVStream_Server_DirectShow_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_AVStream_Server_DirectShow_Message),
                                                                                                                                   '\0')),
                             Test_I_AVStream_Server_DirectShow_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_AVStream_Server_DirectShow_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_AVStream_Server_DirectShow_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
ACE_Message_Block*
Test_I_AVStream_Server_DirectShow_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::release"));

  // release any continuations
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  return inherited::release ();
}

std::string
Test_I_AVStream_Server_DirectShow_Message::CommandTypeToString (int command_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}

//////////////////////////////////////////

Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message"));

}

Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message (const Test_I_AVStream_Server_MediaFoundation_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message"));

}

Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                ACE_Data_Block* dataBlock_in,
                                                                                                ACE_Allocator* messageAllocator_in,
                                                                                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,               // session id
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message"));

}

Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message (Stream_SessionId_t sessionId_in,
                                                                                                ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,        // session id
              messageAllocator_in) // message allocator
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::Test_I_AVStream_Server_MediaFoundation_Message"));

}

ACE_Message_Block*
Test_I_AVStream_Server_MediaFoundation_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::duplicate"));

  Test_I_AVStream_Server_MediaFoundation_Message* message_p = NULL;

  // create a new Test_I_AVStream_Server_MediaFoundation_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_AVStream_Server_MediaFoundation_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_AVStream_Server_MediaFoundation_Message*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                                                      '\0')),
                             Test_I_AVStream_Server_MediaFoundation_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_AVStream_Server_MediaFoundation_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_AVStream_Server_MediaFoundation_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

ACE_Message_Block*
Test_I_AVStream_Server_MediaFoundation_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::release"));

  // release any continuations
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  return inherited::release ();
}

std::string
Test_I_AVStream_Server_MediaFoundation_Message::CommandTypeToString (int command_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}
#else
Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message (Stream_SessionId_t sessionId_in,
                                                                unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message"));

}

Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message (const Test_I_AVStream_Server_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message"));

}

Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message (Stream_SessionId_t sessionId_in,
                                                                ACE_Data_Block* dataBlock_in,
                                                                ACE_Allocator* messageAllocator_in,
                                                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,               // session id
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message"));

}

Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message (Stream_SessionId_t sessionId_in,
                                                                ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,        // session id
              messageAllocator_in) // message allocator
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::Test_I_AVStream_Server_Message"));

}

ACE_Message_Block*
Test_I_AVStream_Server_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::duplicate"));

  Test_I_AVStream_Server_Message* message_p = NULL;

  // create a new Test_I_AVStream_Server_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (unlikely (inherited::message_block_allocator_ == NULL))
    ACE_NEW_NORETURN (message_p,
                      Test_I_AVStream_Server_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_AVStream_Server_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_AVStream_Server_Message),
                                                                                                                        '\0')),
                             Test_I_AVStream_Server_Message (*this));
  } // end ELSE
  if (unlikely (!message_p))
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_AVStream_Server_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (unlikely (inherited::cont_))
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_AVStream_Server_MessageBase::duplicate(): \"%m\", aborting\n")));
      message_p->release (); message_p = NULL;
      return NULL;
    } // end IF
  } // end IF

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

ACE_Message_Block*
Test_I_AVStream_Server_Message::release (void)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::release"));

  // release any continuations
  if (inherited::cont_)
  {
    inherited::cont_->release (); inherited::cont_ = NULL;
  } // end IF

  return inherited::release ();
}

std::string
Test_I_AVStream_Server_Message::CommandTypeToString (int command_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Server_Message::CommandTypeToString"));

  ACE_UNUSED_ARG (command_in);

  return ACE_TEXT_ALWAYS_CHAR ("MB_DATA");
}
#endif // ACE_WIN32 || ACE_WIN64
