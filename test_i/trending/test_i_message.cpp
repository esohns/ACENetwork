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

#include "test_i_message.h"

#include "ace/Malloc_Base.h"

#include "stream_macros.h"

#include "http_defines.h"
#include "http_tools.h"

Test_I_Stream_MessageData::Test_I_Stream_MessageData ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_MessageData::Test_I_Stream_MessageData"));

}

Test_I_Stream_MessageData::Test_I_Stream_MessageData (struct Test_I_Trending_MessageData*& messageData_in,
                                                      bool delete_in)
 : inherited (messageData_in,
              delete_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_MessageData::Test_I_Stream_MessageData"));

}

void
Test_I_Stream_MessageData::setPR (struct HTTP_Record*& record_inout)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_MessageData::setPR"));

  Test_I_Trending_MessageData* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    Test_I_Trending_MessageData ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_I_MessageData: \"%m\", returning\n")));
    return;
  } // end IF
  data_p->form = record_inout->form; // request
  data_p->headers = record_inout->headers;
  data_p->method = record_inout->method;
  data_p->reason = record_inout->reason; // response
  data_p->status = record_inout->status; // response
  data_p->URI = record_inout->URI;
  data_p->version = record_inout->version;

  delete record_inout; record_inout= NULL;

  inherited::setPR (data_p);
}

//////////////////////////////////////////

Test_I_Stream_Message::Test_I_Stream_Message (Stream_SessionId_t sessionId_in,
                                              unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::Test_I_Stream_Message"));

}

Test_I_Stream_Message::Test_I_Stream_Message (const Test_I_Stream_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::Test_I_Stream_Message"));

}

Test_I_Stream_Message::Test_I_Stream_Message (Stream_SessionId_t sessionId_in,
                                              ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in,
                                              bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::Test_I_Stream_Message"));

}

Test_I_Stream_Message::Test_I_Stream_Message (Stream_SessionId_t sessionId_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::Test_I_Stream_Message"));

}

ACE_Message_Block*
Test_I_Stream_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::duplicate"));

  Test_I_Stream_Message* message_p = NULL;

  // create a new Test_I_Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_Stream_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_Stream_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_Stream_Message),
                                                                                                               '\0')),
                             Test_I_Stream_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_Stream_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_Stream_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

HTTP_Method_t
Test_I_Stream_Message::command () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::command"));

  // sanity check(s)
  if (!inherited::isInitialized_)
    return HTTP_Codes::HTTP_METHOD_INVALID;
  ACE_ASSERT (inherited::data_);

  const Test_I_Trending_MessageData& data_r = inherited::data_->getR ();

  return (data_r.method);
}

void
Test_I_Stream_Message::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Message::dump_state"));

  // sanity check(s)
  if (!inherited::data_)
    return;
  const Test_I_Trending_MessageData& data_r = inherited::data_->getR ();

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s"),
              ACE_TEXT (HTTP_Tools::dump (data_r).c_str ())));
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("ISIN: %s / symbol: %s / WKN: %s\n"),
              ACE_TEXT (data_r.stockItem.ISIN.c_str ()),
              ACE_TEXT (data_r.stockItem.symbol.c_str ()),
              ACE_TEXT (data_r.stockItem.WKN.c_str ())));
}
