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

#include "ace/Log_Msg.h"

#include "net_defines.h"
#include "net_macros.h"

#include "irc_record.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
IRC_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
                    SessionMessageType>::IRC_Module_Parser_T (typename ISTREAM_T* stream_in)
 : inherited (stream_in)
 , crunchMessages_ (IRC_DEFAULT_CRUNCH_MESSAGES) // "crunch" messages ?
 , debugScanner_ (IRC_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (IRC_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (IRC_DEFAULT_LEX_TRACE,  // trace scanning ?
            IRC_DEFAULT_YACC_TRACE) // trace parsing ?
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::IRC_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
IRC_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
                    SessionMessageType>::~IRC_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::~IRC_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
IRC_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
                    SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Module_Parser_T::initialize"));

  // sanity check(s)
  if (inherited::isInitialized_)
  {
    debugScanner_ = IRC_DEFAULT_LEX_TRACE;
    debugParser_ = IRC_DEFAULT_YACC_TRACE;
    crunchMessages_ = IRC_DEFAULT_CRUNCH_MESSAGES;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  debugScanner_ = configuration_in.parserConfiguration->debugScanner;
  debugParser_ = configuration_in.parserConfiguration->debugParser;
  crunchMessages_ = configuration_in.crunchMessages;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
IRC_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
                    SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
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
  DataMessageType* message_p = message_inout;
  if (crunchMessages_)
  {
//     message->dump_state();

    // step1: get a new message buffer
    message_p = inherited::allocateMessage (IRC_MAXIMUM_FRAME_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  IRC_MAXIMUM_FRAME_SIZE));
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
    ACE_ASSERT (message_p->space () >= NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE);
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
  message_p->initialize (*container_p);

  // *NOTE*: message has assumed control over "container"...

  // OK: parse this message

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("parsing message(ID: %u, %u byte(s))\n\"%s\"\n"),
//              message_p->id (),
//              message_p->length (),
//              std::string(message_p->rd_ptr(), message_p->length()).c_str()));

  driver_.initialize (const_cast<IRC_Record&> (message_p->get ()),
                      debugScanner_,
                      debugParser_);
  if (!driver_.parse (message_p,        // data block
                      crunchMessages_)) // has data been crunched ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_ParserDriver::parse(ID: %u), returning\n"),
                message_p->id ()));
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
