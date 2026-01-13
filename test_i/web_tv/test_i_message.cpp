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

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"

#include "net_macros.h"

#include "http_tools.h"

Test_I_MessageDataContainer::Test_I_MessageDataContainer ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_MessageDataContainer::Test_I_MessageDataContainer"));

}

Test_I_MessageDataContainer::Test_I_MessageDataContainer (struct Test_I_WebTV_MessageData*& messageData_in,
                                                          bool delete_in)
 : inherited (messageData_in,
              delete_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_MessageDataContainer::Test_I_MessageDataContainer"));

}

void
Test_I_MessageDataContainer::setPR (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_MessageDataContainer::setPR"));

  struct Test_I_WebTV_MessageData* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    struct Test_I_WebTV_MessageData ());
  if (unlikely (!data_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
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

Test_I_Message::Test_I_Message (Stream_SessionId_t sessionId_in,
                                unsigned int size_in)
 : inherited (sessionId_in,
              size_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

Test_I_Message::Test_I_Message (const Test_I_Message& message_in)
 : inherited (message_in)
 , mediaType_ (message_in.mediaType_)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

Test_I_Message::Test_I_Message (Stream_SessionId_t sessionId_in,
                                ACE_Data_Block* dataBlock_in,
                                ACE_Allocator* messageAllocator_in,
                                bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
 , mediaType_ (STREAM_MEDIATYPE_INVALID)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));

}

//Test_I_Message::Test_I_Message (ACE_Allocator* messageAllocator_in)
// : inherited (messageAllocator_in) // message block allocator
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::Test_I_Message"));
//
//}

//DHCP_MessageType_t
//Test_I_Message::command () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::command"));
//
//  // sanity check(s)
//  if (!inherited::initialized_)
//    return DHCP_Codes::DHCP_MESSAGE_INVALID;
////  ACE_ASSERT (inherited::data_);
//
//  DHCP_OptionsIterator_t iterator =
////      inherited::data_.DHCPRecord->options.find (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE);
//      inherited::data_.options.find (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE);
//  ACE_ASSERT (iterator != inherited::data_.options.end ());
////  ACE_ASSERT (iterator != inherited::data_.DHCPRecord->options.end ());
////  Test_I_MessageData& data_r =
////      const_cast<Test_I_MessageData&> (inherited::data_->get ());
////  ACE_ASSERT (data_r.DHCPRecord);
////  DHCP_OptionsIterator_t iterator =
////    data_r.DHCPRecord->options.find (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE);
////  ACE_ASSERT (iterator != data_r.DHCPRecord->options.end ());
//
//  return DHCP_Tools::MessageType2Type ((*iterator).second);
//}

//std::string
//Test_I_Message::Command2String (DHCP_MessageType_t type_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::Command2String"));
//
//  return DHCP_Tools::MessageType2String (type_in);
//}

ACE_Message_Block*
Test_I_Message::clone (ACE_Message_Block::Message_Flags flags_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::clone"));

  ACE_UNUSED_ARG (flags_in);

  int result = -1;
  size_t current_size = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::data_block_);

  // step1: "deep"-copy the fragment chain
  Test_I_Message* result_p = NULL;

  current_size = inherited::data_block_->size ();
  // *NOTE*: ACE_Data_Block::clone() does not retain the value of 'cur_size_'
  //         --> reset it
  // *TODO*: resolve ACE bugzilla issue #4219
  ACE_Data_Block* data_block_p = inherited::data_block_->clone (0);
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::clone(0): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  result = data_block_p->size (current_size);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Data_Block::size(%u): \"%m\", aborting\n"),
                current_size));
    data_block_p->release (); data_block_p = NULL;
    return NULL;
  } // end IF

  // allocate a new message that contains unique copies of the message
  // block fields, and "deep" copie(s) of the data block(s)

  // *NOTE*: if there is no allocator, use the standard new/delete calls

  if (inherited::message_block_allocator_)
  {
    // *NOTE*: the argument to calloc() doesn't matter (as long as it is not 0),
    //         the returned memory is always sizeof(ARDrone_LiveVideoMessage)
    ACE_NEW_MALLOC_NORETURN (result_p,
                             static_cast<Test_I_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_Message),
                                                                                                        '\0')),
                             Test_I_Message (inherited::sessionId_,
                                             data_block_p,
                                             inherited::message_block_allocator_,
                                             true));
  } // end IF
  else
    ACE_NEW_NORETURN (result_p,
                      Test_I_Message (inherited::sessionId_,
                                      data_block_p,
                                      NULL,
                                      true));
  if (unlikely (!result_p))
  {
    Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    data_block_p->release (NULL); data_block_p = NULL;
    return NULL;
  } // end IF
  // set read-/write pointers
  result_p->rd_ptr (inherited::rd_ptr_);
  result_p->wr_ptr (inherited::wr_ptr_);

  // set message type
  result_p->set (inherited::type_);
  result_p->setMediaType (mediaType_);

  // initialize
  if (inherited::isInitialized_)
  {
    Test_I_MessageDataContainer* data_container_p = NULL;
    ACE_NEW_NORETURN (data_container_p,
                      Test_I_MessageDataContainer ());
    if (unlikely (!data_container_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      result_p->release ();
      return NULL;
    } // end IF
    data_container_p->setR (inherited::data_->getR ());
    result_p->initialize (data_container_p,
                          inherited::sessionId_,
                          NULL);
  } // end IF

  result_p->setMediaType (mediaType_);

  // clone any continuations
  if (inherited::cont_)
  {
    try {
      result_p->cont_ = inherited::cont_->clone ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in ACE_Message_Block::clone(): \"%m\", continuing\n")));
    }
    if (unlikely (!result_p->cont_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to ACE_Message_Block::clone(): \"%m\", aborting\n")));

      // clean up
      result_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if 'this' is initialized, so is the "clone"

//  // *NOTE*: the new fragment chain is already 'crunch'ed, i.e. aligned to base_
//  // *TODO*: consider defragment()ing the chain before padding
//
//  // step2: 'pad' the fragment(s)
//  unsigned int padding_bytes =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    AV_INPUT_BUFFER_PADDING_SIZE;
//#else
//    FF_INPUT_BUFFER_PADDING_SIZE;
//#endif
//  for (ACE_Message_Block* message_block_p = result_p;
//       message_block_p;
//       message_block_p = message_block_p->cont ())
//  { ACE_ASSERT ((message_block_p->capacity () - message_block_p->size ()) >= padding_bytes);
//    ACE_OS::memset (message_block_p->wr_ptr (), 0, padding_bytes);
//  } // end FOR

  return result_p;
}

ACE_Message_Block*
Test_I_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Message::duplicate"));

  Test_I_Message* message_p = NULL;

  // create a new Test_I_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Test_I_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Test_I_Message*> (inherited::message_block_allocator_->calloc (sizeof (Test_I_Message),
                                                                                                        '\0')),
                             Test_I_Message (*this));
  } // end ELSE
  if (unlikely (!message_p))
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_I_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
