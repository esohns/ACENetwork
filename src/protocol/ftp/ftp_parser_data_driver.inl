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

#include "ftp_common.h"
#include "ftp_defines.h"
#include "ftp_message.h"
#include "ftp_parser_data.h"
#include "ftp_scanner_data.h"

template <typename SessionMessageType>
FTP_ParserDataDriver_T<SessionMessageType>::FTP_ParserDataDriver_T ()
 : finished_ (false)
 , fragment_ (NULL)
 , offset_ (0)
 , configuration_ (NULL)
//, parser_ (this,               // driver
//           &numberOfMessages_, // counter
//           scannerState_)      // scanner
 , scannerState_ (NULL)
 , bufferState_ (NULL)
 , state_ (FTP_STATE_DATA_INITIAL)
 , initialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::FTP_ParserDataDriver_T"));

}

template <typename SessionMessageType>
FTP_ParserDataDriver_T<SessionMessageType>::~FTP_ParserDataDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::~FTP_ParserDataDriver_T"));

  // finalize lex scanner
  if (FTP_Scanner_Data_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

template <typename SessionMessageType>
bool
FTP_ParserDataDriver_T<SessionMessageType>::initialize (const struct Common_FlexBisonParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::initialize"));

  if (initialized_)
  {
    if (fragment_)
    {
      fragment_->release (); fragment_ = NULL;
    } // end IF
    offset_ = 0;

    configuration_ = NULL;
    finished_ = false;

    if (bufferState_)
    { ACE_ASSERT (scannerState_);
      FTP_Scanner_Data__delete_buffer (bufferState_,
                                       scannerState_);
      bufferState_ = NULL;
    } // end IF

    if (scannerState_)
    {
      if (FTP_Scanner_Data_lex_destroy (scannerState_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
      scannerState_ = NULL;
    } // end IF

    initialized_ = false;
  } // end IF

  configuration_ =
      &const_cast<struct Common_FlexBisonParserConfiguration&> (configuration_in);

  if (FTP_Scanner_Data_lex_init_extra (this, &scannerState_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (scannerState_);
  //parser_.set (scannerState_);

#if defined (_DEBUG)
  // trace ?
  FTP_Scanner_Data_set_debug ((configuration_->debugScanner ? 1 : 0),
                              scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  zzdebug = (configuration_->debugParser ? 1 : 0);
#endif // YYDEBUG
#endif // _DEBUG

  // OK
  initialized_ = true;

  return true;
}

template <typename SessionMessageType>
bool
FTP_ParserDataDriver_T<SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::parse"));

  // sanity check(s)
  ACE_ASSERT (initialized_);
  ACE_ASSERT (!fragment_);
  ACE_ASSERT (data_in);

  // start with the first fragment...
  fragment_ = data_in;
  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_ParserDataDriver_T::scan_begin(), aborting\n")));

    // clean up
    fragment_ = NULL;

    return false;
  } // end IF

  // initialize scanner
  offset_ = 0;
  FTP_Scanner_Data_set_column (1, scannerState_);
  FTP_Scanner_Data_set_lineno (1, scannerState_);
  int debug_level = 0;
#if YYDEBUG
  //debug_level = parser_.debug_level ();
  debug_level = zzdebug;
#endif // YYDEBUG

  // parse data fragment
  ACE_ASSERT (!finished_);
  int result = -1;
  try {
    //result = parser_.parse ();
    result = zzparse (this, scannerState_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ::zzparse(), continuing\n")));
  }
  switch (result)
  {
    case 0:
    case 1: // *NOTE*: for some reason, YYACCEPT returns 1; *TODO*: find out why
      break; // done
    default:
    { // *NOTE*: need more data ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to parse message fragment (result was: %d), aborting\n"),
                  result));
      break;
    }
  } // end SWITCH

  // finalize buffer/scanner
  scan_end ();

  if (fragment_)
    fragment_ = NULL;

  return (result == 0);
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::error (const yy::location& location_in,
                                                   const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::error"));

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
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

//   std::clog << location_in << ": " << message_in << std::endl;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::error (const YYLTYPE& location_in,
                                                   const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::error"));

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
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //   std::clog << location_in << ": " << message_in << std::endl;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::error"));

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
//FTP_ParserDataDriver_T<SessionMessageType>::getDebugScanner () const
//{
//  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::getDebugScanner"));
//
//  return (FTP_Scanner_get_debug (scannerState_) != 0);
//}

template <typename SessionMessageType>
bool
FTP_ParserDataDriver_T<SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (configuration_);

  bool get_next_b = fragment_ != NULL;

  if (!fragment_ ||
      (get_next_b && !fragment_->cont ()))
  {
    // sanity check(s)
    if (!configuration_->block)
      return false; // not enough data, cannot proceed

    waitBuffer (); // <-- wait for data
    if (!fragment_ ||
        (get_next_b && !fragment_->cont ()))
    {
      // *NOTE*: most probable reason: received session end
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("no data after HTTP_ParserDriver_T::waitBuffer(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  if (get_next_b)
    fragment_ = fragment_->cont ();
  //setP (fragment_);

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
                ACE_TEXT ("failed to FTP_ParserDataDriver_T::begin(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  SessionMessageType* session_message_p = NULL;
//  Stream_SessionMessageType session_message_type =
//      STREAM_SESSION_MESSAGE_INVALID;
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
        finished_ = true;
        break;
      case STREAM_MESSAGE_SESSION_TYPE:
      {
        session_message_p = static_cast<SessionMessageType*> (message_block_p);
        if (session_message_p->type () == STREAM_SESSION_MESSAGE_END)
          finished_ = true; // session has finished --> abort
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
  {
    if (fragment_)
    { 
      ACE_Message_Block* message_block_2 = fragment_;
      for (;
           message_block_2->cont ();
           message_block_2 = message_block_2->cont ());
      message_block_2->cont (message_block_p);
    } // end IF
    else
      fragment_ = message_block_p;
  } // end IF
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::setDebug (yyscan_t context_in,
                                                      bool debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::setDebug"));

  ACE_ASSERT (context_in);

  FTP_Scanner_Data_set_debug ((debug_in ? 1 : 0), context_in);
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::reset"));

  ACE_ASSERT (scannerState_);

  FTP_Scanner_Data_set_column (1, scannerState_);
  FTP_Scanner_Data_set_lineno (1, scannerState_);
}

template <typename SessionMessageType>
bool
FTP_ParserDataDriver_T<SessionMessageType>::initialize (yyscan_t& context_out,
                                                        FTP_IParserData* extra_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::initialize"));

  ACE_ASSERT (!context_out);

  int result = FTP_Scanner_Data_lex_init_extra (extra_in, &context_out);
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_Scanner_lex_init_extra(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::finalize (yyscan_t& context_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::finalize"));

  ACE_ASSERT (context_in);

  int result = FTP_Scanner_Data_lex_destroy (context_in);
  if (result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_Scanner_Data_lex_destroy(): \"%m\", returning\n")));
}

template <typename SessionMessageType>
struct yy_buffer_state*
FTP_ParserDataDriver_T<SessionMessageType>::create (yyscan_t context_in,
                                                    char* buffer_in,
                                                    size_t size_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::create"));

  ACE_ASSERT (context_in);

  struct yy_buffer_state* result = FTP_Scanner_Data__scan_buffer (buffer_in,
                                                                  size_in,
                                                                  context_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_Scanner_Data__scan_buffer(): \"%m\", aborting\n")));

  return result;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::destroy (yyscan_t context_in,
                                                     struct yy_buffer_state*& buffer_inout)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::destroy"));

  ACE_ASSERT (context_in);

  FTP_Scanner_Data__delete_buffer (buffer_inout,
                                   context_in);
  buffer_inout = NULL;
}

template <typename SessionMessageType>
bool
FTP_ParserDataDriver_T<SessionMessageType>::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

  // reset scanner state
  FTP_Scanner_Data_reset (scannerState_);

  // create/initialize a new buffer state
  if (configuration_->useYYScanBuffer)
  {
    bufferState_ =
      FTP_Scanner_Data__scan_buffer (fragment_->rd_ptr (),
                                     fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                     scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      FTP_Scanner_Data__scan_bytes (fragment_->rd_ptr (),
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
//  FTP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

template <typename SessionMessageType>
void
FTP_ParserDataDriver_T<SessionMessageType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_ParserDataDriver_T::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  FTP_Scanner_Data__delete_buffer (bufferState_,
                                   scannerState_);
  bufferState_ = NULL;
}
