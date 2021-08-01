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

#include <sstream>

#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"

template <typename SessionDataType,
          typename UserDataType>
BitTorrent_Message_T<SessionDataType,
                     UserDataType>::BitTorrent_Message_T (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::BitTorrent_Message_T"));

}

template <typename SessionDataType,
          typename UserDataType>
BitTorrent_Message_T<SessionDataType,
                     UserDataType>::BitTorrent_Message_T (const BitTorrent_Message_T<SessionDataType,
                                                                                     UserDataType>& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::BitTorrent_Message_T"));

}

template <typename SessionDataType,
          typename UserDataType>
BitTorrent_Message_T<SessionDataType,
                     UserDataType>::BitTorrent_Message_T (Stream_SessionId_t sessionId_in,
                                                          ACE_Data_Block* dataBlock_in,
                                                          ACE_Allocator* messageAllocator_in,
                                                          bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own !) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in) // increment message counter ?
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::BitTorrent_Message_T"));

}

// BitTorrent_Message_T::BitTorrent_Message_T (ACE_Allocator* messageAllocator_in)
//  : inherited (messageAllocator_in,
//               true), // usually, we want to increment the running message counter
//    isInitialized (false) // not initialized --> call init() !
// {
//   NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::BitTorrent_Message_T"));
//
// }

//template <typename SessionDataType,
//          typename UserDataType>
//enum BitTorrent_MessageType
//BitTorrent_Message_T<SessionDataType,
//                     UserDataType>::command () const
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::command"));
//
//  // sanity check(s)
//  if (!inherited::isInitialized_)
//    return BITTORRENT_MESSAGETYPE_INVALID;
//  ACE_ASSERT (inherited::data_);
//
//  const struct BitTorrent_Record& record_r = inherited::data_->get ();
//
//  return record_r.type;
//}

template <typename SessionDataType,
          typename UserDataType>
void
BitTorrent_Message_T<SessionDataType,
                     UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::dump_state"));

  // sanity check(s)
  if (!inherited::isInitialized_)
    return;
  ACE_ASSERT (inherited::data_);

  const struct BitTorrent_PeerRecord& record_r = inherited::data_->getR ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("***** Message (ID: %u, %u byte(s)) *****\n%s\n"),
              inherited::id (),
              inherited::total_length (),
              ACE_TEXT (BitTorrent_Tools::RecordToString (record_r).c_str ())));
}

template <typename SessionDataType,
          typename UserDataType>
ACE_Message_Block*
BitTorrent_Message_T<SessionDataType,
                     UserDataType>::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new BitTorrent_Message_T that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this)); // invoke copy ctor
  else // otherwise, use the existing message_block_allocator
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
allocate:
    try {
      // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
      //         a "shallow" copy which just references our data block...
      // *IMPORTANT NOTE*: cached allocators require the object size as argument
      //                   to malloc() (instead of its internal "capacity()" !)
      // *TODO*: (depending on the allocator implementation) this senselessly
      // allocates a datablock anyway, only to immediately release it again...
      ACE_NEW_MALLOC_NORETURN (message_p,
                               //static_cast<BitTorrent_Message_T*>(message_block_allocator_->malloc(capacity())),
                               static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (sizeof (OWN_TYPE_T))),
                               OWN_TYPE_T (*this));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), aborting\n"),
                  sizeof (OWN_TYPE_T)));
      return NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_p->block ())
      goto allocate;
  } // end ELSE
  if (!message_p)
  {
    if (inherited::message_block_allocator_)
    {
      Stream_IAllocator* allocator_p =
        dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
      ACE_ASSERT (allocator_p);

      if (allocator_p->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate BitTorrent_Message_T: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate BitTorrent_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_Message_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

//////////////////////////////////////////

template <typename SessionDataType,
          typename UserDataType>
BitTorrent_TrackerMessage_T<SessionDataType,
                            UserDataType>::BitTorrent_TrackerMessage_T (const BitTorrent_TrackerMessage_T& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::BitTorrent_TrackerMessage_T"));

}

template <typename SessionDataType,
          typename UserDataType>
BitTorrent_TrackerMessage_T<SessionDataType,
                            UserDataType>::BitTorrent_TrackerMessage_T (Stream_SessionId_t sessionId_in,
                                                                        ACE_Data_Block* dataBlock_in,
                                                                        ACE_Allocator* messageAllocator_in,
                                                                        bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::BitTorrent_TrackerMessage_T"));

}

//BitTorrent_TrackerMessage_T::BitTorrent_TrackerMessage_T (ACE_Allocator* messageAllocator_in)
// : inherited (messageAllocator_in) // message block allocator
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::BitTorrent_TrackerMessage_T"));
//
//}

template <typename SessionDataType,
          typename UserDataType>
HTTP_Method_t
BitTorrent_TrackerMessage_T<SessionDataType,
                            UserDataType>::command () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::command"));

  // sanity check(s)
  if (!inherited::isInitialized_)
    return HTTP_Codes::HTTP_METHOD_INVALID;

  const typename inherited::DATA_T& data_container_r = inherited::getR ();
  const struct HTTP_Record& record_r = data_container_r.getR ();

  return record_r.method;
}

template <typename SessionDataType,
          typename UserDataType>
std::string
BitTorrent_TrackerMessage_T<SessionDataType,
                            UserDataType>::CommandToString (HTTP_Method_t method_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::CommandToString"));

  return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
                                                       : HTTP_Tools::MethodToString (method_in));
}

template <typename SessionDataType,
          typename UserDataType>
ACE_Message_Block*
BitTorrent_TrackerMessage_T<SessionDataType,
                            UserDataType>::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new BitTorrent_TrackerMessage_TBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate BitTorrent_TrackerMessage_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_TrackerMessage_TBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

//std::string
//BitTorrent_TrackerMessage_T::CommandTypeToString (HTTP_Method_t method_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerMessage_T::CommandTypeToString"));

//  return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE)
//                                                       : HTTP_Tools::Method2String (method_in));
//}
