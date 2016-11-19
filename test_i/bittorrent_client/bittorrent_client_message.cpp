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

#include <ace/Synch.h>
#include "bittorrent_client_message.h"

#include <ace/Log_Msg.h>
#include <ace/Malloc_Base.h>

#include "net_macros.h"

BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage (unsigned int size_in)
 : inherited (size_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage"));

}

BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage (const BitTorrent_Client_PeerMessage& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage"));

}

BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage (ACE_Data_Block* dataBlock_in,
                                                              ACE_Allocator* messageAllocator_in,
                                                              bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in, // re-use the same allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::BitTorrent_Client_PeerMessage"));

}

//BitTorrent_Client_PeerMessage::BitTorrent_Client_Message (ACE_Allocator* messageAllocator_in)
// : inherited (messageAllocator_in) // message block allocator
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::BitTorrent_Client_Message"));
//
//}

BitTorrent_Client_PeerMessage::~BitTorrent_Client_PeerMessage ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::~BitTorrent_Client_PeerMessage"));

}

ACE_Message_Block*
BitTorrent_Client_PeerMessage::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::duplicate"));

  BitTorrent_Client_PeerMessage* message_p = NULL;

  // create a new BitTorrent_Client_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      BitTorrent_Client_PeerMessage (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<BitTorrent_Client_PeerMessage*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                                       '\0')),
                             BitTorrent_Client_PeerMessage (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate BitTorrent_Client_PeerMessage: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_Client_PeerMessage::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

//std::string
//BitTorrent_Client_PeerMessage::CommandType2String (HTTP_Method_t method_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerMessage::CommandType2String"));

//  return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
//                                                       : HTTP_Tools::Method2String (method_in));
//}

//////////////////////////////////////////

BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage (unsigned int size_in)
 : inherited (size_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage"));

}

BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage (const BitTorrent_Client_TrackerMessage& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage"));

}

BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage (ACE_Data_Block* dataBlock_in,
                                                                    ACE_Allocator* messageAllocator_in,
                                                                    bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in, // re-use the same allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage"));

}

//BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage (ACE_Allocator* messageAllocator_in)
// : inherited (messageAllocator_in) // message block allocator
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::BitTorrent_Client_TrackerMessage"));
//
//}

BitTorrent_Client_TrackerMessage::~BitTorrent_Client_TrackerMessage ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::~BitTorrent_Client_TrackerMessage"));

}

ACE_Message_Block*
BitTorrent_Client_TrackerMessage::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::duplicate"));

  BitTorrent_Client_TrackerMessage* message_p = NULL;

  // create a new BitTorrent_Client_TrackerMessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      BitTorrent_Client_TrackerMessage (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<BitTorrent_Client_TrackerMessage*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                                          '\0')),
                             BitTorrent_Client_TrackerMessage (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate BitTorrent_Client_TrackerMessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_Client_TrackerMessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

//std::string
//BitTorrent_Client_TrackerMessage::CommandType2String (HTTP_Method_t method_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerMessage::CommandType2String"));

//  return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
//                                                       : HTTP_Tools::Method2String (method_in));
//}
