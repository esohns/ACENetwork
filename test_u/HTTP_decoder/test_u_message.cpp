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

//#include "ace/Synch.h"
//#include "test_u_HTTP_decoder_stream_common.h"
#include "test_u_message.h"

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"

//#include "http_tools.h"

#include "net_macros.h"

Test_U_Message::Test_U_Message (unsigned int size_in)
 : inherited (size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::Test_U_Message"));

}

Test_U_Message::Test_U_Message (const Test_U_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::Test_U_Message"));

}

Test_U_Message::Test_U_Message (Stream_SessionId_t sessionId_in,
                                ACE_Data_Block* dataBlock_in,
                                ACE_Allocator* messageAllocator_in,
                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::Test_U_Message"));

}

//Test_U_Message::Test_U_Message (ACE_Allocator* messageAllocator_in)
// : inherited (messageAllocator_in) // message block allocator
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::Test_U_Message"));
//
//}

ACE_Message_Block*
Test_U_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::duplicate"));

  Test_U_Message* message_p = NULL;

  // create a new Test_U_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_U_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_U_Message*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                        '\0')),
                             Test_U_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_U_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_U_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

//std::string
//Test_U_Message::CommandTypeToString (HTTP_Method_t method_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_U_Message::CommandTypeToString"));

//  return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
//                                                       : HTTP_Tools::MethodToString (method_in));
//}
