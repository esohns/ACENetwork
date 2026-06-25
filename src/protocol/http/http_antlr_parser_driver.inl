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

//#include "http_antlr_parser.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::HTTP_ANTLRParserDriver_T (Stream_ITask* itask_in)
 : inherited ()
 , configuration_ (NULL)
 , finished_ (false)
 , fragment_ (NULL)
#if (USE_UNBUFFERED)
 , inputBuffer_ (this)
 , inputStream_ (&inputBuffer_)
 , input_ (inputStream_)
 , tokenFactory_ (true)
 , lexer_ (&input_)
 , tokens_ (&lexer_)
#else
 , inputStream_ ()
 , lexer_ (&inputStream_)
 , tokens_ (&lexer_)
#endif // USE_UNBUFFERED
 , itask_ (itask_in)
 , parser_ (&tokens_)
 , isFirst_ (true)
 , isInitialized_ (false)
 , messageQueue_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::HTTP_ANTLRParserDriver_T"));

  // sanity check(s)
  ACE_ASSERT (itask_);

  lexer_.parser = this;
  lexer_.removeErrorListeners ();
  lexer_.addErrorListener (this);
#if (USE_UNBUFFERED)
  //lexer_.setTokenFactory(antlr4::CommonTokenFactory::DEFAULT.get ());
  lexer_.setTokenFactory (&tokenFactory_);
#endif // USE_UNBUFFERED

  parser_.parser_ = this;
  parser_.setBuildParseTree (false);
  parser_.removeErrorListeners ();
  parser_.addErrorListener (this);
  parser_.addParseListener (this);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::~HTTP_ANTLRParserDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::~HTTP_ANTLRParserDriver_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::exitChunks (http_antlr_parser::ChunksContext* context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::exitChunks"));

  ACE_UNUSED_ARG (context_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::exitBody (http_antlr_parser::BodyContext* context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::exitBody"));

  ACE_UNUSED_ARG (context_in);

  // sanity check: finished ?
  ACE_Message_Block* head_fragment_p = this->head ();
  ACE_ASSERT (head_fragment_p);
  size_t total_length_i = head_fragment_p->total_length ();
  if (total_length_i < lexer_.content_length)
    return;

  // process any chunks
  //size_t offset_i = lexer_.offset;
  //for (HTTP_ChunksConstIterator_t iterator = parser_.chunks_.begin ();
  //     iterator != parser_.chunks_.end ();
  //     ++iterator)
  //{
  //  lexer_.offset = (*iterator).first;
  //  chunk ((*iterator).second);
  //} // end FOR
  //lexer_.offset = offset_i;

  struct HTTP_Record* record_p = &parser_.record_;
  record (record_p);

  // *TODO*: set to finished only iff total_length >= content_length !
  finished_ = true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::enterEveryRule (antlr4::ParserRuleContext* context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::enterEveryRule"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  if (unlikely (configuration_->debugParser))
  {
    std::vector<std::string> rules = parser_.getRuleInvocationStack (context_in);
    std::reverse (rules.begin (), rules.end ());
    std::string rulesStr = antlrcpp::arrayToString (rules);
    std::string info_string = rulesStr;
#if (USE_UNBUFFERED)
#else
    info_string += ACE_TEXT_ALWAYS_CHAR (" {start=") +
                   (context_in->start ? std::to_string (context_in->start->getTokenIndex ()) : ACE_TEXT_ALWAYS_CHAR ("0x0")) +
                   ACE_TEXT_ALWAYS_CHAR (", stop=") +
                   (context_in->stop ? std::to_string (context_in->stop->getTokenIndex ()) : ACE_TEXT_ALWAYS_CHAR ("0x0")) +
                   '}';
#endif // USE_UNBUFFERED
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("entering rule: \"%s\"...\n"),
                ACE_TEXT (info_string.c_str ())));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::syntaxError (antlr4::Recognizer* recognizer_in,
                                                           antlr4::Token* token_in,
                                                           size_t line_in,
                                                           size_t column_in,
                                                           const std::string& message_in,
                                                           std::exception_ptr exception_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::syntaxError"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%B/%B: \"%s\"...\n"),
              line_in, column_in,
              ACE_TEXT (message_in.c_str ())));

  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::reportAmbiguity (antlr4::Parser* recognizer_in,
                                                               const antlr4::dfa::DFA& DFA_in,
                                                               size_t startIndex_in,
                                                               size_t stopIndex_in,
                                                               bool exact_in,
                                                               const antlrcpp::BitSet& ambigAlts_in,
                                                               antlr4::atn::ATNConfigSet* configs_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::reportAmbiguity"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%B/%B: \"found ambiguity\"...\n"),
              startIndex_in, stopIndex_in));

  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::reportAttemptingFullContext (antlr4::Parser* recognizer_in,
                                                                           const antlr4::dfa::DFA& DFA_in,
                                                                           size_t startIndex_in,
                                                                           size_t stopIndex_in,
                                                                           const antlrcpp::BitSet& conflictingAlts_in,
                                                                           antlr4::atn::ATNConfigSet* configs_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::reportAttemptingFullContext"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%B/%B: \"found SLL conflict\"...\n"),
              startIndex_in, stopIndex_in));

  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::reportContextSensitivity (antlr4::Parser* recognizer_in,
                                                                        const antlr4::dfa::DFA& DFA_in,
                                                                        size_t startIndex_in,
                                                                        size_t stopIndex_in,
                                                                        size_t prediction_in,
                                                                        antlr4::atn::ATNConfigSet* configs_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::reportContextSensitivity"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%B/%B: \"found full-context prediction with unique result\"...\n"),
              startIndex_in, stopIndex_in));

  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (fragment_);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::initialize (const struct HTTP_ParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::initialize"));

  if (isInitialized_)
  {
    configuration_ = NULL;
    finished_ = false;
    fragment_ = NULL;
    parser_.reset ();

    isFirst_ = true;

    isInitialized_ = false;
    messageQueue_ = NULL;
  } // end IF

  configuration_ =
    &const_cast<struct HTTP_ParserConfiguration&> (configuration_in);
  messageQueue_ = configuration_->messageQueue;
  ACE_ASSERT (messageQueue_);

  if (configuration_->debugParser)
    parser_.setTrace (true);

  isInitialized_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  int result = 0;

  finished_ = false;
  // retain a handle to the 'current' fragment
  fragment_ = data_in;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;

    lexer_.reset_2 ();
    lexer_.parser = this;

    parser_.reset_2 ();
    parser_.parser_ = this;
  } // end IF

  begin (fragment_->rd_ptr (),
         fragment_->length ());

  if (unlikely (configuration_->debugParser))
  {
#if (USE_UNBUFFERED)
#else
    tokens_.fill ();
    antlr4::Token* token_p = NULL;
    for (size_t i = 0;
         i < tokens_.size ();
         ++i)
    {
      token_p = tokens_.get (i);
      ACE_ASSERT (token_p);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("#%B: @%B/%B:\t%d -->\t\"%s\"\n"),
                  i + 1,
                  token_p->getLine (), token_p->getCharPositionInLine (),
                  token_p->getType (), ACE_TEXT (token_p->getText ().c_str ())));
    } // end FOR
#endif // USE_UNBUFFERED
  } // end IF
//#if (USE_UNBUFFERED)
//  tokens_.fill (1);
//#endif // USE_UNBUFFERED

  // parse data fragment
  try {
    parser_.document ();
  } catch (const antlr4::RuntimeException& e) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught ANTLR exception in http_antlr_parser::document(): \"%s\", continuing\n"),
                ACE_TEXT (e.what ())));
    result = 1;
  } catch (const std::runtime_error& e) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in http_antlr_parser::document(): \"%s\", continuing\n"),
                ACE_TEXT (e.what ())));
    result = 1;
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in http_antlr_parser::document(), continuing\n")));
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

  return true;

error:
  end ();
  fragment_ = NULL;

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::switchBuffer (bool begin_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

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
//                  ACE_TEXT ("no data after HTTP_ANTLRParserDriver_T::waitBuffer(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();

  // switch to the next fragment

  // clean state
  end ();

  if (likely (begin_in))
    if (!begin (fragment_->rd_ptr (),
                fragment_->length ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_ANTLRParserDriver_T::begin(), aborting\n")));
      return false;
    } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::waitBuffer"));

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
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
bool
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::begin (const char* buffer_in,
                                                     size_t bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::begin"));

  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

#if (USE_UNBUFFERED)
  // inputBuffer_.pubsetbuf (fragment_->rd_ptr (),
  //                         fragment_->length ());
  inputBuffer_.append_chunk (fragment_->rd_ptr (),
                             fragment_->length ());
  inputStream_.clear ();
#else
  inputStream_.load (fragment_->rd_ptr (),
                     fragment_->length ());
#endif
  // lexer_.setInputStream (&input_);
  // tokens_.setTokenSource (&lexer_);
  // parser_.setInputStream (&tokens_);

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
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::end ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::end"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
void
HTTP_ANTLRParserDriver_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         SessionMessageType>::chunk_2 (ACE_UINT64 offset_in,
                                                       ACE_UINT32 size_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ANTLRParserDriver_T::chunk_2"));

  size_t temp_i = lexer_.offset;
  lexer_.offset = offset_in;
  chunk (size_in);

  lexer_.offset = temp_i;
}
