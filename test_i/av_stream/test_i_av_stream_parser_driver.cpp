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

#include "test_i_av_stream_parser_driver.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_parser_defines.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_i_av_stream_parser.h"
#include "test_i_av_stream_scanner.h"

AVStream_ParserDriver::AVStream_ParserDriver ()
 : buffer_ (NULL)
 , header_ ()
 , missing_ (0)
 , configuration_ (NULL)
 , scannerState_ (NULL)
 , bufferState_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::AVStream_ParserDriver"));

  if (AVStream_Scanner_lex_init_extra (this, &scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", continuing\n")));
  ACE_ASSERT (scannerState_);
  //parser_.set (scannerState_);

  // trace ?
  AVStream_Scanner_set_debug ((COMMON_PARSER_DEFAULT_LEX_TRACE ? 1 : 0),
                              scannerState_);
#if defined (YYDEBUG)
  //parser_.set_debug_level (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1
  //                                                          : 0); // binary (see bison manual)
  yydebug = (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1
                                              : 0); // binary (see bison manual)
#endif // YYDEBUG

  ACE_OS::memset (&header_, 0, sizeof (struct acestream_av_stream_header));
}

AVStream_ParserDriver::~AVStream_ParserDriver ()
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::~AVStream_ParserDriver"));

  // finalize lex scanner
  if (scannerState_ &&
      AVStream_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

bool
AVStream_ParserDriver::initialize (const struct Common_FlexBisonParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::initialize"));

  ACE_OS::memset (&header_, 0, sizeof (struct acestream_av_stream_header));
  missing_ = 0;

  configuration_ =
    &const_cast<struct Common_FlexBisonParserConfiguration&> (configuration_in);

#if defined (_DEBUG)
  // sanity check(s)
  ACE_ASSERT (scannerState_);

  AVStream_Scanner_set_debug ((configuration_in.debugScanner ? 1 : 0),
                              scannerState_);
#if defined (YYDEBUG)
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (configuration_in.debugParser ? 1
                                          : 0); // binary (see bison manual)
#endif // YYDEBUG
#endif // _DEBUG

  return true;
}

bool
AVStream_ParserDriver::parse (ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::parse"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);
  //ACE_ASSERT (!buffer_);

  // *NOTE*: we parse ALL available message fragments
  // *TODO*: yyrestart(), yy_create_buffer/yy_switch_to_buffer, YY_INPUT...
  int result = -1;

  buffer_ = messageBlock_in;
  if (!scan_begin (COMMON_PARSER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AVStream_ParserDriver::scan_begin(), aborting\n")));
    return false;
  } // end IF

  // parse data
  //result = parser_.parse ();
  result = yyparse (this,
                    scannerState_);
  if (unlikely (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse message fragment, aborting\n")));

  // finalize buffer/scanner
  scan_end ();

#if defined (_DEBUG)
  int debug_level = 0;
#if defined (YYDEBUG)
  //debug_level = parser_.debug_level ();
  debug_level = yydebug;
#endif // YYDEBUG
  if (debug_level)
  {
  } // end IF
#endif // _DEBUG

  return (result == 0);
}

void
AVStream_ParserDriver::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool is_data_b = true;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently in yylex() context

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->messageQueue);

  // 1. wait for data
  result = configuration_->messageQueue->dequeue_head (message_block_p, NULL);
  if (unlikely (result == -1))
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
      is_data_b = false;
      break;
    case ACE_Message_Block::MB_DATA:
    case ACE_Message_Block::MB_PROTO:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), returning\n"),
                  message_block_p->msg_type ()));
      return;
    }
  } // end SWITCH

  // 2. append data ?
  if (likely (is_data_b))
  { // sanity check(s)
    ACE_ASSERT (buffer_);

    ACE_Message_Block* message_block_2 = buffer_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ())
      ;
    message_block_2->cont (message_block_p);
  } // end IF
  else
    message_block_p->release ();
}

bool
AVStream_ParserDriver::switchBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (scannerState_);
  ACE_ASSERT (bufferState_);

  // clean state
  scan_end ();

  // need more data ?
  if (buffer_->cont () == NULL)
  {
    waitBuffer (); // <-- wait for data
    if (buffer_->cont () == NULL)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("parser received MB_STOP, aborting\n")));
      return false;
    } // end IF
  } // end IF

  // load next buffer
  buffer_ = buffer_->cont ();
  if (!scan_begin (configuration_->useYYScanBuffer))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AVStream_ParserDriver::scan_begin(), aborting\n")));
    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("switched to next buffer...\n")));

  return true;
}

void
//AVStream_ParserDriver::error (const yy::location& location_in,
//                              const std::string& message_in)
AVStream_ParserDriver::error (const YYLTYPE& location_in,
                              const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::error"));

  std::ostringstream converter;
  //converter << location_in;
  converter << location_in.first_column << ACE_TEXT_ALWAYS_CHAR (":") << location_in.first_line;
  converter << ACE_TEXT_ALWAYS_CHAR ("-");
  converter << location_in.last_column << ACE_TEXT_ALWAYS_CHAR (":") << location_in.last_line;
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
  ACE_Message_Block* head = buffer_;
  while (head->prev ())
    head = head->prev ();
  ACE_ASSERT (head);
  Common_IDumpState* idump_state_p = dynamic_cast<Common_IDumpState*> (head);
  if (!idump_state_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_IDumpState*>(0x%@), returning\n"),
                head));
    return;
  } // end IF
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

//   std::clog << location_in << ": " << message_in << std::endl;
}

void
AVStream_ParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::error"));

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

bool
AVStream_ParserDriver::scan_begin (bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (buffer_);

  // create/init a new buffer state
  if (likely (useYYScanBuffer_in))
  {
    bufferState_ =
      AVStream_Scanner__scan_buffer (buffer_->rd_ptr (),
                                     buffer_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                     scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      AVStream_Scanner__scan_bytes (buffer_->rd_ptr (),
                                    static_cast<int> (buffer_->length ()),
                                    scannerState_);
  } // end ELSE
  if (unlikely (!bufferState_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to %s(%@,%Q), aborting\n"),
                (useYYScanBuffer_in ? ACE_TEXT ("yy_scan_buffer") : ACE_TEXT ("yy_scan_bytes")),
                buffer_->rd_ptr (),
                buffer_->length ()));
    return false;
  } // end IF

  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  AVStream_Scanner__switch_to_buffer (bufferState_,
                                      scannerState_);

  return true;
}

void
AVStream_ParserDriver::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_ParserDriver::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  AVStream_Scanner__delete_buffer (bufferState_,
                                   scannerState_);
  bufferState_ = NULL;
}
