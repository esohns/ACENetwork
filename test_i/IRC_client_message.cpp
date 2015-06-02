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

#include "IRC_client_message.h"

#include <sstream>

#include "ace/Message_Block.h"
#include "ace/Malloc_Base.h"

#include "net_macros.h"

#include "IRC_client_tools.h"

IRC_Client_Message::IRC_Client_Message (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::IRC_Client_Message"));

}

IRC_Client_Message::IRC_Client_Message (const IRC_Client_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::IRC_Client_Message"));

}

IRC_Client_Message::IRC_Client_Message (ACE_Data_Block* dataBlock_in,
                                                    ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in) // use this when destruction is imminent...
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::IRC_Client_Message"));

}

// IRC_Client_Message::IRC_Client_Message(ACE_Allocator* messageAllocator_in)
//  : inherited(messageAllocator_in,
//              true), // usually, we want to increment the running message counter...
//    myIsInitialized(false) // not initialized --> call init() !
// {
//   NETWORK_TRACE(ACE_TEXT("IRC_Client_Message::IRC_Client_Message"));
//
// }

IRC_Client_Message::~IRC_Client_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::~IRC_Client_Message"));

  // *NOTE*: will be called just BEFORE we're passed back to the allocator
}

IRC_Client_CommandType_t
IRC_Client_Message::getCommand () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::getCommand"));

  // sanity check(s)
  ACE_ASSERT (inherited::getData ());

  switch (inherited::getData ()->command.discriminator)
  {
    case IRC_Client_IRCMessage::Command::STRING:
      return IRC_Client_Tools::IRCCommandString2Type (*inherited::getData ()->command.string);
    case IRC_Client_IRCMessage::Command::NUMERIC:
      return static_cast<IRC_Client_CommandType_t> (inherited::getData ()->command.numeric);
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command type (was: %d), aborting\n"),
                  inherited::getData ()->command.discriminator));

      break;
    }
  } // end SWITCH

  return IRC_Client_IRCMessage::IRC_CLIENT_COMMANDTYPE_INVALID;
}

void
IRC_Client_Message::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::dump_state"));

  std::ostringstream converter;

  // count continuations
  unsigned int count = 1;
  std::string info;
  for (const ACE_Message_Block* source = this;
       source != NULL;
       source = source->cont (), count++)
  {
    converter.str (ACE_TEXT (""));
    converter << count;
    info += converter.str ();
    info += ACE_TEXT ("# [");
    converter.str (ACE_TEXT (""));
    converter << source->length ();
    info += converter.str ();
    info += ACE_TEXT (" byte(s)]: \"");
    info.append (source->rd_ptr (), source->length ());
    info += ACE_TEXT ("\"\n");
  } // end FOR
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("***** Message (ID: %u, %u byte(s)) *****\n%s"),
              getID (),
              total_length (),
              ACE_TEXT (info.c_str ())));
  // delegate to base
  inherited::dump_state ();
}

void
IRC_Client_Message::crunch ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::crunch"));

  // sanity check
  // *WARNING*: this is NOT enough, it's a race.
  // Anyway, there may be trailing messages and/or pieces referencing the same
  // buffer...
  // --> in fact, that should be the norm
//   ACE_ASSERT(reference_count() == 1);
  // ... assuming stream processing is indeed single-threaded, then the
  // reference count at this stage SHOULD be 2: us, and the next,
  // trailing "message head". (Of course, it COULD be just "us"...)
  if (reference_count() <= 2)
  {  // step1: align rd_ptr() with base()
    if (inherited::crunch())
    {
      ACE_DEBUG((LM_ERROR,
                ACE_TEXT("failed to ACE_Message_Block::crunch(): \"%m\", aborting\n")));

      return;
    } // end IF
  } // end IF

  // step2: copy the data
  ACE_Message_Block* source = NULL;
  size_t amount = 0;
  for (source = cont();
       source != NULL;
       source = source->cont())
  {
    amount = (space() < source->length() ? space()
                                         : source->length());
    if (copy(source->rd_ptr(), amount))
    {
      ACE_DEBUG((LM_ERROR,
                 ACE_TEXT("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));

      return;
    } // end IF

    // adjust read pointer accordingly
    source->rd_ptr(amount);
  } // end FOR

  // step3: release any thus obsoleted continuations
  source = cont();
  ACE_Message_Block* prev = this;
  ACE_Message_Block* obsolete = NULL;
  do
  {
    // finished ?
    if (source == NULL)
      break;

    if (source->length() == 0)
    {
      obsolete = source;
      source = source->cont();
      prev->cont(source);
      obsolete->release();
    } // end IF
  } while (true);
}

ACE_Message_Block*
IRC_Client_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::duplicate"));

  IRC_Client_Message* message_p = NULL;

  // create a new IRC_Client_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      IRC_Client_Message (*this)); // invoke copy ctor
  else // otherwise, use the existing message_block_allocator
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
allocate:
    try
    {
      // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
      //         a "shallow" copy which just references our data block...
      // *IMPORTANT NOTE*: cached allocators require the object size as argument
      //                   to malloc() (instead of its internal "capacity()" !)
      // *TODO*: (depending on the allocator implementation) this senselessly
      // allocates a datablock anyway, only to immediately release it again...
      ACE_NEW_MALLOC_NORETURN (message_p,
                               //                           static_cast<IRC_Client_Message*>(message_block_allocator_->malloc(capacity())),
                               static_cast<IRC_Client_Message*> (inherited::message_block_allocator_->calloc (sizeof (IRC_Client_Message))),
                               IRC_Client_Message (*this));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), aborting\n"),
                  sizeof (IRC_Client_Message)));
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
                    ACE_TEXT ("failed to allocate IRC_Client_Message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate IRC_Client_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

std::string
IRC_Client_Message::CommandType2String(const IRC_Client_CommandType_t& commandType_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Message::CommandType2String"));

  std::string result = ACE_TEXT ("INVALID/UNKNOWN");

  switch (commandType_in)
  {
    case IRC_Client_IRCMessage::PASS:
      result = ACE_TEXT ("PASS"); break;
    case IRC_Client_IRCMessage::NICK:
      result = ACE_TEXT ("NICK"); break;
    case IRC_Client_IRCMessage::USER:
      result = ACE_TEXT ("USER"); break;
    case IRC_Client_IRCMessage::SERVER:
      result = ACE_TEXT ("SERVER"); break;
    case IRC_Client_IRCMessage::OPER:
      result = ACE_TEXT ("OPER"); break;
    case IRC_Client_IRCMessage::QUIT:
      result = ACE_TEXT ("QUIT"); break;
    case IRC_Client_IRCMessage::SQUIT:
      result = ACE_TEXT ("SQUIT"); break;
    case IRC_Client_IRCMessage::JOIN:
      result = ACE_TEXT ("JOIN"); break;
    case IRC_Client_IRCMessage::PART:
      result = ACE_TEXT ("PART"); break;
    case IRC_Client_IRCMessage::MODE:
      result = ACE_TEXT ("MODE"); break;
    case IRC_Client_IRCMessage::TOPIC:
      result = ACE_TEXT ("TOPIC"); break;
    case IRC_Client_IRCMessage::NAMES:
      result = ACE_TEXT ("NAMES"); break;
    case IRC_Client_IRCMessage::LIST:
      result = ACE_TEXT ("LIST"); break;
    case IRC_Client_IRCMessage::INVITE:
      result = ACE_TEXT ("INVITE"); break;
    case IRC_Client_IRCMessage::KICK:
      result = ACE_TEXT ("KICK"); break;
    case IRC_Client_IRCMessage::SVERSION:
      result = ACE_TEXT ("VERSION"); break;
    case IRC_Client_IRCMessage::STATS:
      result = ACE_TEXT ("STATS"); break;
    case IRC_Client_IRCMessage::LINKS:
      result = ACE_TEXT ("LINKS"); break;
    case IRC_Client_IRCMessage::TIME:
      result = ACE_TEXT ("TIME"); break;
    case IRC_Client_IRCMessage::CONNECT:
      result = ACE_TEXT ("CONNECT"); break;
    case IRC_Client_IRCMessage::TRACE:
      result = ACE_TEXT ("TRACE"); break;
    case IRC_Client_IRCMessage::ADMIN:
      result = ACE_TEXT ("ADMIN"); break;
    case IRC_Client_IRCMessage::INFO:
      result = ACE_TEXT ("INFO"); break;
    case IRC_Client_IRCMessage::PRIVMSG:
      result = ACE_TEXT ("PRIVMSG"); break;
    case IRC_Client_IRCMessage::NOTICE:
      result = ACE_TEXT ("NOTICE"); break;
    case IRC_Client_IRCMessage::WHO:
      result = ACE_TEXT ("WHO"); break;
    case IRC_Client_IRCMessage::WHOIS:
      result = ACE_TEXT ("WHOIS"); break;
    case IRC_Client_IRCMessage::WHOWAS:
      result = ACE_TEXT ("WHOWAS"); break;
    case IRC_Client_IRCMessage::KILL:
      result = ACE_TEXT ("KILL"); break;
    case IRC_Client_IRCMessage::PING:
      result = ACE_TEXT ("PING"); break;
    case IRC_Client_IRCMessage::PONG:
      result = ACE_TEXT ("PONG"); break;
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler...")
    case IRC_Client_IRCMessage::__QUIRK__ERROR:
#else
    case IRC_Client_IRCMessage::ERROR:
#endif
      result = ACE_TEXT ("ERROR"); break;
    case IRC_Client_IRCMessage::AWAY:
      result = ACE_TEXT ("AWAY"); break;
    case IRC_Client_IRCMessage::REHASH:
      result = ACE_TEXT ("REHASH"); break;
    case IRC_Client_IRCMessage::RESTART:
      result = ACE_TEXT ("RESTART"); break;
    case IRC_Client_IRCMessage::SUMMON:
      result = ACE_TEXT ("SUMMON"); break;
    case IRC_Client_IRCMessage::USERS:
      result = ACE_TEXT ("USERS"); break;
    case IRC_Client_IRCMessage::WALLOPS:
      result = ACE_TEXT ("WALLOPS"); break;
    case IRC_Client_IRCMessage::USERHOST:
      result = ACE_TEXT ("USERHOST"); break;
    case IRC_Client_IRCMessage::ISON:
      result = ACE_TEXT ("ISON"); break;
    default:
    {
      // try numeric conversion
      result =
        IRC_Client_Tools::IRCCode2String (static_cast<IRC_Client_IRCNumeric_t>(commandType_in));

      break;
    }
  } // end SWITCH

  return result;
}
