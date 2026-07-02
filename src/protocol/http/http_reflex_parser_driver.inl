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
#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"
#include "ace/OS.h"

#include "common_file_tools.h"

#include "stream_itask.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_message.h"
#include "http_reflex_parser.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::HTTP_ReflexParserDriver_T (Stream_ITask* itask_in)
 : configuration_ (NULL)
 , finished_ (false)
 , fragment_ (NULL)
 , itask_ (itask_in)
 , matcher_ (NULL)
 , offset_ (0)
 , record_ ()
 , bufferState_ (NULL)
 , isFirst_ (true)
 , isInitialized_ (false)
 , messageQueue_ (NULL)
 , scannerState_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::HTTP_ReflexParserDriver_T"));

  // sanity check(s)
  ACE_ASSERT (itask_);

  int result = -1;

  // step1: initialize flex state
  yylex_init_extra (this,
                    &scannerState_);
  if (!scannerState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (scannerState_);

  //parser_.set (scannerState_);

  // trace ?
  HTTP_Reflex_Scanner_set_debug ((COMMON_PARSER_DEFAULT_LEX_TRACE ? 1 : 0),
                                 scannerState_);
  //parser_.set_debug_level (trace_ ? 1 : 0); // binary (see bison manual)
//  yysetdebug (trace_ ? 1 : 0);
  http_debug = (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1 : 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::~HTTP_ReflexParserDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::~HTTP_ReflexParserDriver_T"));

  if (scannerState_)
    yylex_destroy (scannerState_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::initialize (const struct HTTP_ParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (scannerState_);

  if (isInitialized_)
  {
    configuration_ = NULL;
    finished_ = false;
    fragment_ = NULL;
    offset_ = 0;
    record_.reset ();

    isFirst_ = true;

    if (bufferState_)
    { ACE_ASSERT (scannerState_);
      yy_delete_buffer (bufferState_,
                        scannerState_);
      bufferState_ = NULL;
    } // end IF

    isInitialized_ = false;
    if (matcher_)
    { ACE_ASSERT (scannerState_);
      yy_delete_buffer (matcher_, scannerState_);
      matcher_ = NULL;
    } // end IF
    messageQueue_ = NULL;
  } // end IF

  configuration_ =
    &const_cast<struct HTTP_ParserConfiguration&> (configuration_in);
  messageQueue_ = configuration_->messageQueue;
  ACE_ASSERT (messageQueue_);

#if defined (_DEBUG)
  HTTP_Reflex_Scanner_set_debug ((configuration_->debugScanner ? 1 : 0),
                                 scannerState_);
  //parser_.set_debug_level (trace_ ? 1 : 0); // binary (see bison manual)
  //yysetdebug (trace_ ? 1 : 0);
  http_debug = (configuration_->debugParser ? 1 : 0);
#endif // _DEBUG

  isInitialized_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::error (const struct YYLTYPE& location_in,
                                                      const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::error"));

  //std::ostringstream converter;
  //converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
              location_in.first_line, location_in.first_column,
              location_in.last_line, location_in.last_column,
              ACE_TEXT (message_in.c_str ())));
//  ACE_DEBUG ((LM_ERROR,
////              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              ACE_TEXT ("failed to HTTP_Parser::parse(): \"%s\"\n"),
////              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
////              converter.str ().c_str (),
//              message_in.c_str ()));

  // dump message
  //ACE_Message_Block* message_block_p = fragment_;
  //while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  //ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //std::clog << location_in << ": " << message_in << std::endl;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::error (const yy::location& location_in,
                                                      const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::error"));

  std::ostringstream converter;
  converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
              location_in.begin.line, location_in.begin.column,
              location_in.end.line, location_in.end.column,
              ACE_TEXT (message_in.c_str ())));
//  ACE_DEBUG ((LM_ERROR,
////              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              ACE_TEXT ("failed to HTTP_Parser::parse(): \"%s\"\n"),
////              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
////              converter.str ().c_str (),
//              message_in.c_str ()));

  // dump message
  //ACE_Message_Block* message_block_p = fragment_;
  //while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  //ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //std::clog << location_in << ": " << message_in << std::endl;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::error"));

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("\": \"%s\"...\n"),
              ACE_TEXT (message_in.c_str ())));
//   ACE_DEBUG((LM_ERROR,
//              ACE_TEXT("failed to parse \"%s\": \"%s\"...\n"),
//              std::string(fragment_->rd_ptr(), fragment_->length()).c_str(),
//              message_in.c_str()));

//   std::clog << message_in << std::endl;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  int result = -1;
  bool do_scan_end = false;

  finished_ = false;
  // retain a handle to the 'current' fragment
  fragment_ = data_in;
  offset_ = 0;
  record_.reset ();

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;
    // HTTP_Reflex_Scanner_set_column (1, scannerState_);
    // HTTP_Reflex_Scanner_set_lineno (1, scannerState_);

    extern void reflex_code_HTTP_Reflex_Scanner_INITIAL (reflex::Matcher&);
    static reflex::Pattern PATTERN_INITIAL (reflex_code_HTTP_Reflex_Scanner_INITIAL);
    ACE_NEW_NORETURN (matcher_,
                      FlexLexer::AbstractBaseLexer::Matcher (PATTERN_INITIAL,
                                                             reflex::Input (),
                                                             static_cast<yyscanner_t*> (scannerState_),
                                                             NULL));
    ACE_ASSERT (matcher_);
    static_cast<yyscanner_t*> (scannerState_)->buffer (fragment_->rd_ptr (),
                                                       fragment_->length () + 1);
    static_cast<yyscanner_t*> (scannerState_)->matcher (matcher_);
    // matcher_->buffer (fragment_->length ());
    // matcher_->set_reserve (fragment_->size ());
  } // end IF

  // parse data fragment
  try {
    //result = parser_.parse ();
    result = http_parse (this, scannerState_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in http_parse(), continuing\n")));
    result = 1;
  }
  switch (result)
  {
    case 0:
      break; // done/need more data
    case 1:
    default:
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to parse HTTP PDU (result was: %d), aborting\n"),
                  result));
      goto error;
    }
  } // end SWITCH

  // finalize buffer/scanner
  end ();
  do_scan_end = false;

  goto continue_;

error:
  if (do_scan_end)
    end ();
  fragment_ = NULL;

continue_:
  return (result == 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);
  ACE_ASSERT (matcher_);

  // need to adjust buffer ?
  //handleRealloc (fragment_);

  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!configuration_->block)
      return false; // not enough data, cannot proceed

    waitBuffer (); // <-- wait for data
    if (!fragment_->cont ())
    {
      // *NOTE*: most probable reason: received session end
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("no data after HTTP_ReflexParserDriver_T::waitBuffer(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();

  // switch to the next fragment

  // clean state
  end ();

  // initialize next buffer
  // *TODO*: what if this clobbers data of the next message ?
  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((fragment_->capacity () - fragment_->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!begin (NULL, 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_ReflexParserDriver_T::begin(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::waitBuffer"));

  int result_i = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done_b = false;
  bool is_data_b = false;
  bool handle_message_inline_b = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently in yylex() context

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->block);
  ACE_ASSERT (messageQueue_);

  // 1. wait for data
  do
  {
    result_i = messageQueue_->dequeue_head (message_block_p,
                                            NULL);
    if (unlikely (result_i == -1))
    { int error = ACE_OS::last_error ();
      if (unlikely (error != ESHUTDOWN))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      { // *NOTE*: some other error occured: connection failed ?
        done_b = true;
        break;
      }
      //case STREAM_MESSAGE_CONTROL:
      //{
      //  handle_message_inline_b = true;
      //  break;
      //}
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data_b = true;
        break;
      //case STREAM_MESSAGE_SESSION:
      //{
      //  SessionMessageType* session_message_p =
      //    static_cast<SessionMessageType*> (message_block_p);
      //  if (unlikely (session_message_p->type () == STREAM_SESSION_MESSAGE_END))
      //    done_b = true; // session has finished --> leave
      //  else
      //    handle_message_inline_b = true;
      //  break;
      //}
      default:
        break;
    } // end SWITCH
    if (likely (is_data_b))
      break;

    // requeue message ?
    if (unlikely (handle_message_inline_b))
    { ACE_ASSERT (itask_);
      itask_->handleMessage (message_block_p,
                             done_b);
      handle_message_inline_b = false;
    } // end IF
    else
    {
      result_i = messageQueue_->enqueue_tail (message_block_p,
                                              NULL);
      if (unlikely (result_i == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));
        return;
      } // end IF
    } // end ELSE
    message_block_p = NULL;

    if (done_b)
      break;
  } while (true);

  // 2. append data ?
  if (likely (message_block_p))
  { // sanity check(s)
    ACE_ASSERT (fragment_);

    ACE_Message_Block* message_block_2 = fragment_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ());
    message_block_2->cont (message_block_p);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::begin (const char* buffer_in,
                                                      unsigned int bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::begin"));

//  static int counter = 1;

  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);

  // sanity check(s)
  ACE_ASSERT (!bufferState_);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  if (configuration_->useYYScanBuffer)
  {
    bufferState_ = yy_scan_buffer (fragment_->rd_ptr (),
                                   fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                   scannerState_);
    //ACE_ASSERT (matcher_);
    //matcher_->buffer (fragment_->rd_ptr (),
    //                  fragment_->length () + 1);
    // matcher_->buffer (fragment_->length ());
    // matcher_->set_reserve (fragment_->size ());
  } // end IF
  else
    bufferState_ = yy_scan_bytes (fragment_->rd_ptr (),
                                  static_cast<int> (fragment_->length ()),
                                  scannerState_);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment #%d --> %d byte(s)\n"),
//              counter++,
//              fragment_->length ()));

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::end ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::end"));

  // sanity check(s)
  ACE_ASSERT (scannerState_);

  // clean state
  if (bufferState_)
  {
    yy_delete_buffer (bufferState_, scannerState_); bufferState_ = NULL;
  } // end IF
  //else
    //yy_delete_buffer (YY_CURRENT_BUFFER);
    //yy_delete_buffer (static_cast<yyscanner_t*> (scannerState_)->ptr_matcher (), scannerState_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ReflexParserDriver_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          SessionMessageType>::handleRealloc (ACE_Message_Block* fragment_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ReflexParserDriver_T::handleRealloc"));

  // sanity check(s)
  ACE_ASSERT (fragment_in);
  ACE_ASSERT (matcher_);

  // adjust buffer ?
  /*struct FlexLexer::AbstractBaseLexer::Matcher::Context context_s =
    matcher_->after ();
  if (fragment_in->base () != context_s.buf)
  {
    ACE_Data_Block* data_block_p = fragment_in->data_block ();
    data_block_p->set_flags (ACE_Message_Block::DONT_DELETE); // do NOT free the previous memory buffer
    data_block_p->base (const_cast<char*> (context_s.buf),
                        fragment_in->capacity (),
                        0); // <-- own the reallocated memory buffer
    data_block_p->size (fragment_in->size ());
  } // end IF*/
}
