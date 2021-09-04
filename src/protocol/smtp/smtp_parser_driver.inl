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

#include "net_macros.h"

#include "smtp_common.h"
#include "smtp_defines.h"
#include "smtp_message.h"
#include "smtp_parser.h"
#include "smtp_scanner.h"

template <typename SessionMessageType>
SMTP_ParserDriver_T<SessionMessageType>::SMTP_ParserDriver_T ()
 : fragment_ (NULL)
 , offset_ (0)
 , record_ (NULL)
 , configuration_ (NULL)
 , finished_ (false)
//, parser_ (this,               // driver
//           &numberOfMessages_, // counter
//           scannerState_)      // scanner
 , scannerState_ (NULL)
 , bufferState_ (NULL)
 , initialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::SMTP_ParserDriver_T"));

}

template <typename SessionMessageType>
SMTP_ParserDriver_T<SessionMessageType>::~SMTP_ParserDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::~SMTP_ParserDriver_T"));

  // finalize lex scanner
  if (SMTP_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

template <typename SessionMessageType>
bool
SMTP_ParserDriver_T<SessionMessageType>::initialize (const struct Common_FlexBisonParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::initialize"));

  if (initialized_)
  {
    if (fragment_)
    {
      fragment_->release (); fragment_ = NULL;
    } // end IF
    offset_ = 0;
    if (record_)
    {
      delete record_;  record_ = NULL;
    } // end IF

    configuration_ = NULL;
    finished_ = false;

    if (bufferState_)
    { ACE_ASSERT (scannerState_);
      SMTP_Scanner__delete_buffer (bufferState_,
                                   scannerState_);
      bufferState_ = NULL;
    } // end IF

    if (scannerState_)
    {
      if (SMTP_Scanner_lex_destroy (scannerState_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
      scannerState_ = NULL;
    } // end IF

    initialized_ = false;
  } // end IF

  configuration_ =
      &const_cast<struct Common_FlexBisonParserConfiguration&> (configuration_in);

  if (SMTP_Scanner_lex_init_extra (this, &scannerState_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (scannerState_);
  //parser_.set (scannerState_);

  // trace ?
  SMTP_Scanner_set_debug ((configuration_->debugScanner ? 1 : 0),
                          scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (configuration_->debugParser ? 1 : 0);
#endif // YYDEBUG

  // OK
  initialized_ = true;

  return true;
}

template <typename SessionMessageType>
bool
SMTP_ParserDriver_T<SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::parse"));

  // sanity check(s)
  ACE_ASSERT (initialized_);
  ACE_ASSERT (!fragment_);
  ACE_ASSERT (!record_);
  ACE_ASSERT (data_in);

  ACE_NEW_NORETURN (record_,
                    struct SMTP_Record ());
  if (!record_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return false;
  } // end IF

  // start with the first fragment...
  fragment_ = data_in;
  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SMTP_ParserDriver_T::scan_begin(), aborting\n")));

    // clean up
    delete record_; record_ = NULL;
    fragment_ = NULL;

    return false;
  } // end IF

  // initialize scanner
  offset_ = 0;
  SMTP_Scanner_set_column (1, scannerState_);
  SMTP_Scanner_set_lineno (1, scannerState_);
  int debug_level = 0;
#if YYDEBUG
  //debug_level = parser_.debug_level ();
  debug_level = yydebug;
#endif // YYDEBUG

  // parse data fragment
  ACE_ASSERT (!finished_);
  int result = -1;
  try {
    //result = parser_.parse ();
    result = yyparse (this, scannerState_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ::yyparse(), continuing\n")));
  }
  switch (result)
  {
    case 0:
    case 1: // *NOTE*: for some reason, YYACCEPT returns 1; *TODO*: find out why
      ACE_ASSERT (record_);
      record_ = NULL;
      break; // done
    default:
    { // *NOTE*: need more data ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to parse message fragment (result was: %d), aborting\n"),
                  result));
      if (debug_level)
      { ACE_ASSERT (record_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s\n"),
                    ACE_TEXT (SMTP_Tools::dump (*record_).c_str ())));
      } // end IF
      break;
    }
  } // end SWITCH

  // finalize buffer/scanner
  scan_end ();

  if (fragment_)
    fragment_ = NULL;
  if (record_)
  {
    delete record_; record_ = NULL;
  } // end IF

  return (result == 0);
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::error (const yy::location& location_in,
                                                const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::error"));

  std::ostringstream converter;
  converter << location_in;
  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%s): %s\n"),
              ACE_TEXT (converter.str ().c_str ()),
              ACE_TEXT (message_in.c_str ())));
//   ACE_DEBUG((LM_ERROR,
//              ACE_TEXT("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              std::string(fragment_->rd_ptr(), fragment_->length()).c_str(),
//              converter.str().c_str(),
//              message_in.c_str()));

  // dump message
  ACE_Message_Block* head = fragment_;
  while (head->prev ())
    head = head->prev ();
  ACE_ASSERT (head);
  Common_IDumpState* idump_state_p = dynamic_cast<Common_IDumpState*> (head);
  ACE_ASSERT (idump_state_p);
  try
  {
    idump_state_p->dump_state ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

//   std::clog << location_in << ": " << message_in << std::endl;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::error (const YYLTYPE& location_in,
                                                const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::error"));

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
              location_in.first_line, location_in.first_column, location_in.last_line, location_in.last_column,
              ACE_TEXT (message_in.c_str ())));
  //   ACE_DEBUG((LM_ERROR,
  //              ACE_TEXT("failed to parse \"%s\" (@%s): \"%s\"\n"),
  //              std::string(fragment_->rd_ptr(), fragment_->length()).c_str(),
  //              converter.str().c_str(),
  //              message_in.c_str()));

  // dump message
  ACE_Message_Block* head = fragment_;
  while (head->prev ())
    head = head->prev ();
  ACE_ASSERT (head);
  Common_IDumpState* idump_state_p = dynamic_cast<Common_IDumpState*> (head);
  ACE_ASSERT (idump_state_p);
  try
  {
    idump_state_p->dump_state ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //   std::clog << location_in << ": " << message_in << std::endl;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::error"));

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

//template <typename SessionMessageType>
//bool
//SMTP_ParserDriver_T<SessionMessageType>::getDebugScanner () const
//{
//  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::getDebugScanner"));
//
//  return (SMTP_Scanner_get_debug (scannerState_) != 0);
//}

template <typename SessionMessageType>
bool
SMTP_ParserDriver_T<SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

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
//                  ACE_TEXT ("no data after HTTP_ParserDriver_T::waitBuffer(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();

  // switch to the next fragment

  // clean state
  end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((fragment_->capacity () - fragment_->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!begin (NULL, 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SMTP_ParserDriver_T::begin(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  SessionMessageType* session_message_p = NULL;
  Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data_b = false;
  bool requeue_b = true;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->block);
  ACE_ASSERT (configuration_->messageQueue);
  ACE_ASSERT (!finished_);

  // 1. wait for data
  do
  {
    result = configuration_->messageQueue->dequeue_head (message_block_p,
                                                         NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data_b = true;
        break;
      case ACE_Message_Block::MB_STOP:
        finished_ = true; requeue_b = false;
        break;
      case ACE_Message_Block::MB_USER:
      {
        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
        if (session_message_p)
        {
          session_message_type = session_message_p->type ();
          if (session_message_type == STREAM_SESSION_MESSAGE_END)
            finished_ = true; // session has finished --> abort
        } // end IF
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data_b)
      break;

    // requeue message ?
    if (requeue_b)
    {
      result = configuration_->messageQueue->enqueue_tail (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));
        message_block_p->release ();
        return;
      } // end IF
    } // end IF
    else
      message_block_p->release ();
    message_block_p = NULL;
  } while (!finished_);

  // 2. append data ?
  if (message_block_p)
  { ACE_ASSERT (fragment_);
    ACE_Message_Block* message_block_2 = fragment_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ());
    message_block_2->cont (message_block_p);
  } // end IF
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::debug (yyscan_t context_in,
                                                bool debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::debug"));

  ACE_ASSERT (context_in);

  SMTP_Scanner_set_debug ((debug_in ? 1 : 0), context_in);
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::reset"));

  ACE_ASSERT (scannerState_);

  SMTP_Scanner_set_column (1, scannerState_);
  SMTP_Scanner_set_lineno (1, scannerState_);
}

template <typename SessionMessageType>
bool
SMTP_ParserDriver_T<SessionMessageType>::initialize (yyscan_t& context_out,
                                                     SMTP_IParser* extra_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::initialize"));

  ACE_ASSERT (!context_out);

  int result = SMTP_Scanner_lex_init_extra (extra_in, &context_out);
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SMTP_Scanner_lex_init_extra(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::finalize (yyscan_t& context_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::finalize"));

  ACE_ASSERT (context_in);

  int result = SMTP_Scanner_lex_destroy (context_in);
  if (result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SMTP_Scanner_lex_destroy(): \"%m\", returning\n")));
}

template <typename SessionMessageType>
struct yy_buffer_state*
SMTP_ParserDriver_T<SessionMessageType>::create (yyscan_t context_in,
                                                 char* buffer_in,
                                                 size_t size_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::create"));

  ACE_ASSERT (context_in);

  struct yy_buffer_state* result = SMTP_Scanner__scan_buffer (buffer_in,
                                                              size_in,
                                                              context_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SMTP_Scanner__scan_buffer(): \"%m\", aborting\n")));

  return result;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::destroy (yyscan_t context_in,
                                                  struct yy_buffer_state*& buffer_inout)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::destroy"));

  ACE_ASSERT (context_in);

  SMTP_Scanner__delete_buffer (buffer_inout,
                               context_in);
  buffer_inout = NULL;
}

template <typename SessionMessageType>
bool
SMTP_ParserDriver_T<SessionMessageType>::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

  // reset scanner state
  SMTP_Scanner_reset (scannerState_);

  // create/initialize a new buffer state
  if (configuration_->useYYScanBuffer)
  {
    bufferState_ =
      SMTP_Scanner__scan_buffer (fragment_->rd_ptr (),
                                 fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                 scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      SMTP_Scanner__scan_bytes (fragment_->rd_ptr (),
                                fragment_->length (),
                                scannerState_);
  } // end ELSE
  if (!bufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yy_scan_buffer/bytes(0x%@, %d), aborting\n"),
                fragment_->rd_ptr (),
                fragment_->length ()));
    return false;
  } // end IF

//  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
//  SMTP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

template <typename SessionMessageType>
void
SMTP_ParserDriver_T<SessionMessageType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_ParserDriver_T::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  SMTP_Scanner__delete_buffer (bufferState_,
                               scannerState_);
  bufferState_ = NULL;
}
