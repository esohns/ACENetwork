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

#include "irc_message.h"

#include <sstream>

#include "ace/Message_Block.h"
#include "ace/Malloc_Base.h"

#include "net_macros.h"

#include "irc_tools.h"

IRC_Message::IRC_Message (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::IRC_Message"));

}

IRC_Message::IRC_Message (const IRC_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::IRC_Message"));

}

IRC_Message::IRC_Message (ACE_Data_Block* dataBlock_in,
                          ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in) // use this when destruction is imminent...
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::IRC_Message"));

}

// IRC_Message::IRC_Message(ACE_Allocator* messageAllocator_in)
//  : inherited(messageAllocator_in,
//              true), // usually, we want to increment the running message counter...
//    myIsInitialized(false) // not initialized --> call init() !
// {
//   NETWORK_TRACE(ACE_TEXT("IRC_Message::IRC_Message"));
//
// }

IRC_Message::~IRC_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::~IRC_Message"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator
}

IRC_CommandType_t
IRC_Message::command () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::command"));

  // sanity check(s)
  if (!inherited::isInitialized_)
    return IRC_Record::IRC_COMMANDTYPE_INVALID;

  switch (data_.command_.discriminator)
  {
    case IRC_Record::Command::STRING:
      return IRC_Tools::Command2Type (*data_.command_.string);
    case IRC_Record::Command::NUMERIC:
      return static_cast<IRC_CommandType_t> (data_.command_.numeric);
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command type (was: %d), aborting\n"),
                  data_.command_.discriminator));
      break;
    }
  } // end SWITCH

  return IRC_Record::IRC_COMMANDTYPE_INVALID;
}

void
IRC_Message::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::dump_state"));

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

ACE_Message_Block*
IRC_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::duplicate"));

  IRC_Message* message_p = NULL;

  // create a new IRC_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      IRC_Message (*this)); // invoke copy ctor
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
//                           static_cast<IRC_Message*>(message_block_allocator_->malloc(capacity())),
                               static_cast<IRC_Message*> (inherited::message_block_allocator_->calloc (sizeof (IRC_Message))),
                               IRC_Message (*this));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), aborting\n"),
                  sizeof (IRC_Message)));
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
                    ACE_TEXT ("failed to allocate IRC_Message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate IRC_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

std::string
IRC_Message::CommandType2String(const IRC_CommandType_t& commandType_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Message::CommandType2String"));

  std::string result = ACE_TEXT ("INVALID/UNKNOWN");

  switch (commandType_in)
  {
    case IRC_Record::PASS:
      result = ACE_TEXT_ALWAYS_CHAR ("PASS"); break;
    case IRC_Record::NICK:
      result = ACE_TEXT_ALWAYS_CHAR ("NICK"); break;
    case IRC_Record::USER:
      result = ACE_TEXT_ALWAYS_CHAR ("USER"); break;
    case IRC_Record::SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVER"); break;
    case IRC_Record::OPER:
      result = ACE_TEXT_ALWAYS_CHAR ("OPER"); break;
    case IRC_Record::QUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("QUIT"); break;
    case IRC_Record::SQUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("SQUIT"); break;
    case IRC_Record::JOIN:
      result = ACE_TEXT_ALWAYS_CHAR ("JOIN"); break;
    case IRC_Record::PART:
      result = ACE_TEXT_ALWAYS_CHAR ("PART"); break;
    case IRC_Record::MODE:
      result = ACE_TEXT_ALWAYS_CHAR ("MODE"); break;
    case IRC_Record::TOPIC:
      result = ACE_TEXT_ALWAYS_CHAR ("TOPIC"); break;
    case IRC_Record::NAMES:
      result = ACE_TEXT_ALWAYS_CHAR ("NAMES"); break;
    case IRC_Record::LIST:
      result = ACE_TEXT_ALWAYS_CHAR ("LIST"); break;
    case IRC_Record::INVITE:
      result = ACE_TEXT_ALWAYS_CHAR ("INVITE"); break;
    case IRC_Record::KICK:
      result = ACE_TEXT_ALWAYS_CHAR ("KICK"); break;
    case IRC_Record::SVERSION:
      result = ACE_TEXT_ALWAYS_CHAR ("VERSION"); break;
    case IRC_Record::STATS:
      result = ACE_TEXT_ALWAYS_CHAR ("STATS"); break;
    case IRC_Record::LINKS:
      result = ACE_TEXT_ALWAYS_CHAR ("LINKS"); break;
    case IRC_Record::TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("TIME"); break;
    case IRC_Record::CONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECT"); break;
    case IRC_Record::TRACE:
      result = ACE_TEXT_ALWAYS_CHAR ("TRACE"); break;
    case IRC_Record::ADMIN:
      result = ACE_TEXT_ALWAYS_CHAR ("ADMIN"); break;
    case IRC_Record::INFO:
      result = ACE_TEXT_ALWAYS_CHAR ("INFO"); break;
    case IRC_Record::PRIVMSG:
      result = ACE_TEXT_ALWAYS_CHAR ("PRIVMSG"); break;
    case IRC_Record::NOTICE:
      result = ACE_TEXT_ALWAYS_CHAR ("NOTICE"); break;
    case IRC_Record::WHO:
      result = ACE_TEXT_ALWAYS_CHAR ("WHO"); break;
    case IRC_Record::WHOIS:
      result = ACE_TEXT_ALWAYS_CHAR ("WHOIS"); break;
    case IRC_Record::WHOWAS:
      result = ACE_TEXT_ALWAYS_CHAR ("WHOWAS"); break;
    case IRC_Record::KILL:
      result = ACE_TEXT_ALWAYS_CHAR ("KILL"); break;
    case IRC_Record::PING:
      result = ACE_TEXT_ALWAYS_CHAR ("PING"); break;
    case IRC_Record::PONG:
      result = ACE_TEXT_ALWAYS_CHAR ("PONG"); break;
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler...")
    case IRC_Record::__QUIRK__ERROR:
#else
    case IRC_Record::ERROR:
#endif
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR"); break;
    case IRC_Record::AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("AWAY"); break;
    case IRC_Record::REHASH:
      result = ACE_TEXT_ALWAYS_CHAR ("REHASH"); break;
    case IRC_Record::RESTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RESTART"); break;
    case IRC_Record::SUMMON:
      result = ACE_TEXT_ALWAYS_CHAR ("SUMMON"); break;
    case IRC_Record::USERS:
      result = ACE_TEXT_ALWAYS_CHAR ("USERS"); break;
    case IRC_Record::WALLOPS:
      result = ACE_TEXT_ALWAYS_CHAR ("WALLOPS"); break;
    case IRC_Record::USERHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("USERHOST"); break;
    case IRC_Record::ISON:
      result = ACE_TEXT_ALWAYS_CHAR ("ISON"); break;
    default:
    {
      // try numeric conversion
      result =
        IRC_Tools::Command2String (static_cast<IRC_NumericCommand_t> (commandType_in));
      break;
    }
  } // end SWITCH

  return result;
}
