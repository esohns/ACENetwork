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
#include <string>

#include "ace/Message_Block.h"
#include "ace/Malloc_Base.h"

#include "net_macros.h"

#include "pop_defines.h"

template <typename MessageType>
POP_Message_T<MessageType>::POP_Message_T (Stream_SessionId_t sessionId_in,
                                             unsigned int requestedSize_in)
 : inherited (sessionId_in,
              requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::POP_Message_T"));

}

template <typename MessageType>
POP_Message_T<MessageType>::POP_Message_T (const POP_Message_T<MessageType>& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::POP_Message_T"));

}

template <typename MessageType>
POP_Message_T<MessageType>::POP_Message_T (Stream_SessionId_t sessionId_in,
                                             ACE_Data_Block* dataBlock_in,
                                             ACE_Allocator* messageAllocator_in,
                                             bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own !) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in) // increment message counter ?
{
  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::POP_Message_T"));

}

// POP_Message_T::POP_Message_T(ACE_Allocator* messageAllocator_in)
//  : inherited(messageAllocator_in,
//              true), // usually, we want to increment the running message counter
//    myIsInitialized(false) // not initialized --> call init() !
// {
//   NETWORK_TRACE(ACE_TEXT("POP_Message_T::POP_Message_T"));
//
// }

//template <typename MessageType>
//POP_Code_t
//POP_Message_T<MessageType>::command () const
//{
//  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::command"));
//
//  // sanity check(s)
//  if (!inherited::isInitialized_)
//    return POP_Codes::POP_CODE_INVALID;
//  ACE_ASSERT (inherited::data_);
//
//  return inherited::data_->getR ().code;
//}

template <typename MessageType>
void
POP_Message_T<MessageType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::dump_state"));

  std::ostringstream converter;

  // count continuations
  unsigned int count = 1;
  std::string info;
  for (const ACE_Message_Block* source = this;
       source != NULL;
       source = source->cont (), count++)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << count;
    info += converter.str ();
    info += ACE_TEXT_ALWAYS_CHAR ("# [");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << source->length ();
    info += converter.str ();
    info += ACE_TEXT_ALWAYS_CHAR (" byte(s)]\n");
  } // end FOR
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("***** Message (ID: %u, %u byte(s)) *****\n%s"),
              inherited::id (),
              inherited::total_length (),
              ACE_TEXT (info.c_str ())));
  if (inherited::data_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("\n%s"),
                ACE_TEXT (POP_Tools::dump (inherited::data_->getR ()).c_str ())));
}

template <typename MessageType>
ACE_Message_Block*
POP_Message_T<MessageType>::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("POP_Message_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new POP_Message_T that contains unique copies of
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
      //         a "shallow" copy which just references the data block
      // *IMPORTANT NOTE*: cached allocators require the object size as argument
      //                   to malloc() (instead of its internal "capacity()" !)
      // *TODO*: (depending on the allocator implementation) this senselessly
      // allocates a datablock anyway, only to immediately release it again
      ACE_NEW_MALLOC_NORETURN (message_p,
                               //static_cast<POP_Message_T*>(message_block_allocator_->malloc(capacity())),
                               static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (sizeof (OWN_TYPE_T))),
                               OWN_TYPE_T (*this));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), continuing\n"),
                  sizeof (OWN_TYPE_T)));
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
                    ACE_TEXT ("failed to allocate POP_Message_T: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate POP_Message_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to POP_Message_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}
