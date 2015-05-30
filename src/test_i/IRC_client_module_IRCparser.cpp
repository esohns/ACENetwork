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

#include "IRC_client_module_IRCparser.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "IRC_client_tools.h"

IRC_Client_Module_IRCParser::IRC_Client_Module_IRCParser ()
 : inherited ()
 , allocator_ (NULL)
 , debugScanner_ (IRC_CLIENT_DEF_LEX_TRACE) // trace scanning ?
 , debugParser_ (IRC_CLIENT_DEF_YACC_TRACE) // trace parsing ?
 , driver_ (IRC_CLIENT_DEF_LEX_TRACE,  // trace scanning ?
            IRC_CLIENT_DEF_YACC_TRACE) // trace parsing ?
 , crunchMessages_ (IRC_CLIENT_DEF_CRUNCH_MESSAGES) // "crunch" messages ?
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCParser::IRC_Client_Module_IRCParser"));

}

IRC_Client_Module_IRCParser::~IRC_Client_Module_IRCParser ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCParser::~IRC_Client_Module_IRCParser"));

}

bool
IRC_Client_Module_IRCParser::initialize (Stream_IAllocator* allocator_in,
                                               bool crunchMessages_in,
                                               bool debugScanner_in,
                                               bool debugParser_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);
  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    allocator_ = NULL;
    debugScanner_ = debugScanner_in;
    debugParser_ = debugParser_in;
    crunchMessages_ = IRC_CLIENT_DEF_CRUNCH_MESSAGES;
    isInitialized_ = false;
  } // end IF

  allocator_ = allocator_in;
  debugScanner_ = debugScanner_in;
  debugParser_ = debugParser_in;
  crunchMessages_ = crunchMessages_in;

  // OK: all's well...
  isInitialized_ = true;

  return isInitialized_;
}

void
IRC_Client_Module_IRCParser::handleDataMessage (IRC_Client_Message*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCParser::handleDataMessage"));

// according to RFC1459:
//  <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
//  <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
//  <command>  ::= <letter> { <letter> } | <number> <number> <number>
//  <SPACE>    ::= ' ' { ' ' }
//  <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
//  <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                 or NUL or CR or LF, the first of which may not be ':'>
//  <trailing> ::= <Any, possibly *empty*, sequence of octets not including
//                   NUL or CR or LF>

  // sanity check(s)
//   ACE_ASSERT(message_inout->getData() == NULL);

  // "crunch" messages for easier parsing ?
  IRC_Client_Message* message_p = message_inout;
  if (crunchMessages_)
  {
//     // debug info
//     message->dump_state();

    // step1: get a new message buffer
    message_p = allocateMessage (IRC_CLIENT_BUFFER_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  IRC_CLIENT_BUFFER_SIZE));
      return;
    } // end IF

    // step2: copy available data
    for (ACE_Message_Block* source = message_inout;
         source;
         source = source->cont ())
    {
      ACE_ASSERT (source->length () <= message_p->space ());
      if (message_p->copy (source->rd_ptr (),
                           source->length ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

        // clean up
        message_p->release ();

        return;
      } // end IF
    } // end FOR

    // step3: append the "\0\0"-sequence, as required by flex
    ACE_ASSERT (message_p->space () >= IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE);
    ACE_ASSERT (IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE == 2);
    *(message_p->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
    *(message_p->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
    // *NOTE*: DO NOT adjust the write pointer --> length() should stay as it was !
  } // end IF

  // allocate the target data container and attach it to our current message
  IRC_Client_IRCMessage* container_p = NULL;
  ACE_NEW_NORETURN (container_p,
                    IRC_Client_IRCMessage ());
  if (!container_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  // *NOTE*: at this time, the parser does not bisect (','-separated) list-items
  // (i.e. items which are themselves lists) from the (' '-separated) list of
  // parameters. This means that any list-items are extracted as a "whole" - a
  // list-item will just be a single (long) string...
  // *TODO*: check whether any messages actually use this feature on the client side
  // and either:
  // - make the parser more intelligent
  // - bisect any list items in a post-processing step...
  message_inout->initialize (container_p);

  // *NOTE*: message has assumed control over "container"...

  // OK: parse this message

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("parsing message(ID: %u, %u byte(s))\n\"%s\"\n"),
//              message_inout->getID(),
//              message_inout->length(),
//              std::string(message_inout->rd_ptr(), message_inout->length()).c_str()));

  driver_.initialize (const_cast<IRC_Client_IRCMessage&> (*message_inout->getData ()),
                      debugScanner_,
                      debugParser_);
  if (!driver_.parse (message_p,        // data block
                      crunchMessages_)) // has data been crunched ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IRCParserDriver::parse(ID: %u), returning\n"),
                message_inout->getID ()));

    // clean up
    if (message_p != message_inout)
      message_p->release ();
    passMessageDownstream_out = false;
    message_inout->release ();

    return;
  } // end IF

  // clean up
  if (message_p != message_inout)
    message_p->release ();
}

IRC_Client_Message*
IRC_Client_Module_IRCParser::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCParser::allocateMessage"));

  // initialize return value(s)
  IRC_Client_Message* message_p = NULL;

  if (allocator_)
  {
allocate:
    try
    {
      message_p =
        static_cast<IRC_Client_Message*> (allocator_->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_->block ())
        goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      IRC_Client_Message (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}
