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

#include "http_message.h"

#include <sstream>

#include "ace/Message_Block.h"
#include "ace/Malloc_Base.h"

#include "net_macros.h"

#include "http_tools.h"

HTTP_Message::HTTP_Message (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::HTTP_Message"));

}

HTTP_Message::HTTP_Message (const HTTP_Message& message_in)
 : inherited (message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::HTTP_Message"));

}

HTTP_Message::HTTP_Message (ACE_Data_Block* dataBlock_in,
                            ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in) // use this when destruction is imminent...
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::HTTP_Message"));

}

// HTTP_Message::HTTP_Message(ACE_Allocator* messageAllocator_in)
//  : inherited(messageAllocator_in,
//              true), // usually, we want to increment the running message counter...
//    myIsInitialized(false) // not initialized --> call init() !
// {
//   NETWORK_TRACE(ACE_TEXT("HTTP_Message::HTTP_Message"));
//
// }

HTTP_Message::~HTTP_Message ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::~HTTP_Message"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator
}

HTTP_CommandType_t
HTTP_Message::command () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::command"));

  const HTTP_Record* data_p = inherited::getData ();

  // sanity check(s)
  ACE_ASSERT (data_p);

  switch (data_p->command.discriminator)
  {
    case HTTP_Record::Command::STRING:
      return HTTP_Tools::Command2Type (*data_p->command.string);
    case HTTP_Record::Command::NUMERIC:
      return static_cast<IRC_CommandType_t> (data_p->command.numeric);
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command type (was: %d), aborting\n"),
                  data_p->command.discriminator));
      break;
    }
  } // end SWITCH

  return HTTP_Record::IRC_COMMANDTYPE_INVALID;
}

void
HTTP_Message::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::dump_state"));

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
HTTP_Message::crunch ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::crunch"));

  int result = -1;

//  // sanity check(s)
//  // *IMPORTANT NOTE*: this check is NOT enough (race condition). Also, there
//  //                   may be trailing messages (in fact, that should be the
//  //                   norm), and/or (almost any) number(s) of fragments
//  //                   referencing the same buffer
//  ACE_ASSERT (inherited::reference_count () <= 2);
  // ... assuming stream processing is indeed single-threaded (CHECK !!!), then
  // the reference count at this stage should be <=2: "this", and (most
  // probably), the next, trailing "message head" (of course, it could be just
  // "this")
  // step1: align rd_ptr() with base()
  result = inherited::crunch ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", returning\n")));
    return;
  } // end IF

  // step2: copy (part of) the data
  ACE_Message_Block* message_block_p = NULL;
  size_t amount, space, length = 0;
  for (message_block_p = inherited::cont ();
       message_block_p;
       message_block_p = message_block_p->cont ())
  {
    space = inherited::space ();
    length = message_block_p->length ();
    amount = (space < length ? space : length);
    result = inherited::copy (message_block_p->rd_ptr (), amount);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
      return;
    } // end IF

    // adjust read pointer accordingly
    message_block_p->rd_ptr (amount);
  } // end FOR

  // step3: release any thusly obsoleted continuations
  message_block_p = inherited::cont ();
  ACE_Message_Block* previous_p = this;
  ACE_Message_Block* obsolete_p = NULL;
  do
  {
    // finished ?
    if (!message_block_p)
      break;

    if (message_block_p->length () == 0)
    {
      obsolete_p = message_block_p;
      message_block_p = message_block_p->cont ();
      previous_p->cont (message_block_p);
      obsolete_p->release ();
    } // end IF
  } while (true);
}

ACE_Message_Block*
HTTP_Message::duplicate (void) const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::duplicate"));

  HTTP_Message* message_p = NULL;

  // create a new HTTP_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      HTTP_Message (*this)); // invoke copy ctor
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
                               //                           static_cast<HTTP_Message*>(message_block_allocator_->malloc(capacity())),
                               static_cast<HTTP_Message*> (inherited::message_block_allocator_->calloc (sizeof (HTTP_Message))),
                               HTTP_Message (*this));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(%u), aborting\n"),
                  sizeof (HTTP_Message)));
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
                    ACE_TEXT ("failed to allocate HTTP_Message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate HTTP_Message: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_Message::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

std::string
HTTP_Message::CommandType2String(const IRC_CommandType_t& commandType_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Message::CommandType2String"));

  std::string result = ACE_TEXT ("INVALID/UNKNOWN");

  switch (commandType_in)
  {
    case HTTP_Record::PASS:
      result = ACE_TEXT ("PASS"); break;
    case HTTP_Record::NICK:
      result = ACE_TEXT ("NICK"); break;
    case HTTP_Record::USER:
      result = ACE_TEXT ("USER"); break;
    case HTTP_Record::SERVER:
      result = ACE_TEXT ("SERVER"); break;
    case HTTP_Record::OPER:
      result = ACE_TEXT ("OPER"); break;
    case HTTP_Record::QUIT:
      result = ACE_TEXT ("QUIT"); break;
    case HTTP_Record::SQUIT:
      result = ACE_TEXT ("SQUIT"); break;
    case HTTP_Record::JOIN:
      result = ACE_TEXT ("JOIN"); break;
    case HTTP_Record::PART:
      result = ACE_TEXT ("PART"); break;
    case HTTP_Record::MODE:
      result = ACE_TEXT ("MODE"); break;
    case HTTP_Record::TOPIC:
      result = ACE_TEXT ("TOPIC"); break;
    case HTTP_Record::NAMES:
      result = ACE_TEXT ("NAMES"); break;
    case HTTP_Record::LIST:
      result = ACE_TEXT ("LIST"); break;
    case HTTP_Record::INVITE:
      result = ACE_TEXT ("INVITE"); break;
    case HTTP_Record::KICK:
      result = ACE_TEXT ("KICK"); break;
    case HTTP_Record::SVERSION:
      result = ACE_TEXT ("VERSION"); break;
    case HTTP_Record::STATS:
      result = ACE_TEXT ("STATS"); break;
    case HTTP_Record::LINKS:
      result = ACE_TEXT ("LINKS"); break;
    case HTTP_Record::TIME:
      result = ACE_TEXT ("TIME"); break;
    case HTTP_Record::CONNECT:
      result = ACE_TEXT ("CONNECT"); break;
    case HTTP_Record::TRACE:
      result = ACE_TEXT ("TRACE"); break;
    case HTTP_Record::ADMIN:
      result = ACE_TEXT ("ADMIN"); break;
    case HTTP_Record::INFO:
      result = ACE_TEXT ("INFO"); break;
    case HTTP_Record::PRIVMSG:
      result = ACE_TEXT ("PRIVMSG"); break;
    case HTTP_Record::NOTICE:
      result = ACE_TEXT ("NOTICE"); break;
    case HTTP_Record::WHO:
      result = ACE_TEXT ("WHO"); break;
    case HTTP_Record::WHOIS:
      result = ACE_TEXT ("WHOIS"); break;
    case HTTP_Record::WHOWAS:
      result = ACE_TEXT ("WHOWAS"); break;
    case HTTP_Record::KILL:
      result = ACE_TEXT ("KILL"); break;
    case HTTP_Record::PING:
      result = ACE_TEXT ("PING"); break;
    case HTTP_Record::PONG:
      result = ACE_TEXT ("PONG"); break;
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler...")
    case HTTP_Record::__QUIRK__ERROR:
#else
    case HTTP_Record::ERROR:
#endif
      result = ACE_TEXT ("ERROR"); break;
    case HTTP_Record::AWAY:
      result = ACE_TEXT ("AWAY"); break;
    case HTTP_Record::REHASH:
      result = ACE_TEXT ("REHASH"); break;
    case HTTP_Record::RESTART:
      result = ACE_TEXT ("RESTART"); break;
    case HTTP_Record::SUMMON:
      result = ACE_TEXT ("SUMMON"); break;
    case HTTP_Record::USERS:
      result = ACE_TEXT ("USERS"); break;
    case HTTP_Record::WALLOPS:
      result = ACE_TEXT ("WALLOPS"); break;
    case HTTP_Record::USERHOST:
      result = ACE_TEXT ("USERHOST"); break;
    case HTTP_Record::ISON:
      result = ACE_TEXT ("ISON"); break;
    default:
    {
      // try numeric conversion
      result =
        HTTP_Tools::Command2String (static_cast<IRC_NumericCommand_t> (commandType_in));
      break;
    }
  } // end SWITCH

  return result;
}
