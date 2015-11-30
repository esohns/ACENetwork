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
#include "stream_iallocator.h"

#include "net_macros.h"

#include "irc_record.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
IRC_Module_Parser_T<TaskSynchType,
                    TimePolicyType,
                    SessionMessageType,
                    ProtocolMessageType>::IRC_Module_Parser_T ()
 : inherited ()
 , allocator_ (NULL)
 , debugScanner_ (IRC_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (IRC_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (IRC_DEFAULT_LEX_TRACE,  // trace scanning ?
            IRC_DEFAULT_YACC_TRACE) // trace parsing ?
 , crunchMessages_ (IRC_DEFAULT_CRUNCH_MESSAGES) // "crunch" messages ?
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::IRC_Module_Parser_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
IRC_Module_Parser_T<TaskSynchType,
                    TimePolicyType,
                    SessionMessageType,
                    ProtocolMessageType>::~IRC_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::~IRC_Module_Parser_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
IRC_Module_Parser_T<TaskSynchType,
                    TimePolicyType,
                    SessionMessageType,
                    ProtocolMessageType>::initialize (Stream_IAllocator* allocator_in,
                                                      bool crunchMessages_in,
                                                      bool debugScanner_in,
                                                      bool debugParser_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);
  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    allocator_ = NULL;
    debugScanner_ = IRC_DEFAULT_LEX_TRACE;
    debugParser_ = IRC_DEFAULT_YACC_TRACE;
    crunchMessages_ = IRC_DEFAULT_CRUNCH_MESSAGES;
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

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
IRC_Module_Parser_T<TaskSynchType,
                    TimePolicyType,
                    SessionMessageType,
                    ProtocolMessageType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::handleDataMessage"));

  int result = -1;
  IRC_Record* container_p = NULL;

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
  ProtocolMessageType* message_p = message_inout;
  if (crunchMessages_)
  {
//     message->dump_state();

    // step1: get a new message buffer
    message_p = allocateMessage (IRC_BUFFER_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  IRC_BUFFER_SIZE));
      goto error;
    } // end IF

    // step2: copy available data
    for (ACE_Message_Block* message_block_p = message_inout;
         message_block_p;
         message_block_p = message_block_p->cont ())
    {
      ACE_ASSERT (message_block_p->length () <= message_p->space ());
      result = message_p->copy (message_block_p->rd_ptr (),
                                message_block_p->length ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

        // clean up
        message_p->release ();

        goto error;
      } // end IF
    } // end FOR

    // step3: append the "\0\0"-sequence, as required by flex
    ACE_ASSERT (message_p->space () >= IRC_FLEX_BUFFER_BOUNDARY_SIZE);
    //ACE_ASSERT (IRC_FLEX_BUFFER_BOUNDARY_SIZE == 2);
    *(message_p->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
    *(message_p->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
    // *NOTE*: DO NOT adjust the write pointer --> length() should stay as it
    //         was !

    // clean up
    message_inout->release ();
    message_inout = NULL;
    passMessageDownstream_out = false;
  } // end IF

  // allocate the target data container and attach it to the current message
  ACE_NEW_NORETURN (container_p,
                    IRC_Record ());
  if (!container_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  // *NOTE*: at this time, the parser does not bisect (','-separated) list-items
  //         (i.e. items which are themselves lists) from the (' '-separated)
  //         list of parameters. This means that any list-items are extracted as
  //         a "whole" - a list-item will just be a single (long) string...
  // *TODO*: check whether any messages actually use this feature on the client
  //         side and either:
  //         - make the parser more intelligent
  //         - bisect any list items in a post-processing step...
  message_p->initialize (container_p);

  // *NOTE*: message has assumed control over "container"...

  // OK: parse this message

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("parsing message(ID: %u, %u byte(s))\n\"%s\"\n"),
//              message_p->getID(),
//              message_p->length(),
//              std::string(message_p->rd_ptr(), message_p->length()).c_str()));

  driver_.initialize (const_cast<IRC_Record&> (message_p->get ()),
                      debugScanner_,
                      debugParser_);
  if (!driver_.parse (message_p,        // data block
                      crunchMessages_)) // has data been crunched ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_ParserDriver::parse(ID: %u), returning\n"),
                message_p->getID ()));
    goto error;
  } // end IF

  if (!passMessageDownstream_out)
  {
    result = inherited::put_next (message_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_T::put_next(): \"%m\", continuing\n")));
      goto error;
    } // end IF
  } // end IF

  return;

error:
  message_p->release ();
  message_p = NULL;
  passMessageDownstream_out = false;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
ProtocolMessageType*
IRC_Module_Parser_T<TaskSynchType,
                    TimePolicyType,
                    SessionMessageType,
                    ProtocolMessageType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::allocateMessage"));

  // initialize return value(s)
  ProtocolMessageType* message_p = NULL;

  if (allocator_)
  {
allocate:
    try
    {
      message_p =
        static_cast<ProtocolMessageType*> (allocator_->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (requestedSize_in));
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
