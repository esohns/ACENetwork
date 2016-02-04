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

#include "test_u_session_message.h"

#include "ace/Malloc_Base.h"

#include "stream_macros.h"

Test_U_SessionMessage::Test_U_SessionMessage (Stream_SessionMessageType messageType_in,
                                              Test_U_StreamSessionData_t*& sessionData_in,
                                              Test_U_UserData* userData_in)
 : inherited (messageType_in,
              sessionData_in,
              userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::Test_U_SessionMessage"));

}

Test_U_SessionMessage::Test_U_SessionMessage (const Test_U_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::Test_U_SessionMessage"));

}

Test_U_SessionMessage::Test_U_SessionMessage (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::Test_U_SessionMessage"));

}

Test_U_SessionMessage::Test_U_SessionMessage (ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // re-use the same allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::Test_U_SessionMessage"));

}

Test_U_SessionMessage::~Test_U_SessionMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::~Test_U_SessionMessage"));

}

ACE_Message_Block*
Test_U_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SessionMessage::duplicate"));

  Test_U_SessionMessage* message_p = NULL;

  // create a new <Test_U_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_RETURN (message_p,
                    Test_U_SessionMessage (*this),
                    NULL);

  // *WARNING*: the allocator returns a Test_U_SessionMessageBase<ConfigurationType>
  //            when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<Test_U_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Test_U_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (message_p->cont_ == 0)
    {
      message_p->release ();
      message_p = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
