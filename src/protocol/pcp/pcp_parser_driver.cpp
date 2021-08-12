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

#include "pcp_parser_driver.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"

#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_defines.h"
#include "pcp_message.h"
#include "pcp_parser.h"
#include "pcp_scanner.h"

PCP_ParserDriver::PCP_ParserDriver (bool traceScanning_in,
                                    bool traceParsing_in)
 : fragment_ (NULL)
 , offset_ (0)
 , record_ (NULL)
 , trace_ (traceParsing_in)
//, parser_ (this,               // driver
//           &numberOfMessages_, // counter
//           scannerState_)      // scanner
 , scannerState_ (NULL)
 , bufferState_ (NULL)
 , messageQueue_ (NULL)
 , useYYScanBuffer_ (COMMON_PARSER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER)
 , initialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::PCP_ParserDriver"));

  if (PCP_Scanner_lex_init_extra (this, &scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", continuing\n")));
  ACE_ASSERT (scannerState_);
  //parser_.set (scannerState_);

  // trace ?
  PCP_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                          scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (traceParsing_in ? 1 : 0);
#endif
}

PCP_ParserDriver::~PCP_ParserDriver ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::~PCP_ParserDriver"));

  // finalize lex scanner
  if (PCP_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

bool
PCP_ParserDriver::initialize (const struct Common_ParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::initialize"));

  if (initialized_)
  {
    if (fragment_)
      fragment_ = NULL;
    offset_ = 0;
    record_ = NULL;

    configuration_ = NULL;
    trace_ = COMMON_PARSER_DEFAULT_YACC_TRACE;

    if (bufferState_)
    {
      PCP_Scanner__delete_buffer (bufferState_,
                                   scannerState_);
      bufferState_ = NULL;
    } // end IF

    messageQueue_ = NULL;
    useYYScanBuffer_ = COMMON_PARSER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER;

    initialized_ = false;
  } // end IF

  configuration_ =
      &const_cast<struct Common_ParserConfiguration&> (configuration_in);
  trace_ = configuration_->debugScanner;

  messageQueue_ = configuration_->messageQueue;
  useYYScanBuffer_ = configuration_->useYYScanBuffer;

  // trace ?
  PCP_Scanner_set_debug ((configuration_->debugScanner ? 1 : 0),
                         scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (trace_ ? 1 : 0);
#endif

  // OK
  initialized_ = true;

  return true;
}

bool
PCP_ParserDriver::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::parse"));

  // sanity check(s)
  ACE_ASSERT (initialized_);
  ACE_ASSERT (data_in);

  // start with the first fragment...
  fragment_ = data_in;

  // *NOTE*: parse ALL available message fragments
  // *TODO*: yyrestart(), yy_create_buffer/yy_switch_to_buffer, YY_INPUT...
  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to PCP_ParserDriver::scan_begin(), aborting\n")));

    // clean up
    fragment_ = NULL;

    return false;
  } // end IF

  // initialize scanner
  PCP_Scanner_set_column (1, scannerState_);
  PCP_Scanner_set_lineno (1, scannerState_);
  int debug_level = 0;
#if YYDEBUG
  //debug_level = parser_.debug_level ();
  debug_level = yydebug;
#endif
  ACE_UNUSED_ARG (debug_level);

  // parse data fragment
  int result = -1;
  try
  {
    //result = parser_.parse ();
    result = yyparse (this, scannerState_);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ::yyparse(), continuing\n")));
  }
  switch (result)
  {
    case 0:
      break; // done
    case 1:
    default:
    { // *NOTE*: need more data
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("failed to parse message fragment (result was: %d), aborting\n"),
//                    result));

//        // debug info
//        if (debug_level)
//        {
//          try
//          {
//            record_->dump_state ();
//          }
//          catch (...)
//          {
//            ACE_DEBUG ((LM_ERROR,
//                        ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
//          }
//        } // end IF

      break;
    }
  } // end SWITCH

  // finalize buffer/scanner
  scan_end ();

  if (fragment_)
    fragment_ = NULL;

  return (result == 0);
}

bool
PCP_ParserDriver::getDebugScanner () const
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::getDebugScanner"));

  return (PCP_Scanner_get_debug (scannerState_) != 0);
}

void
PCP_ParserDriver::error (const yy::location& location_in,
                         const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::error"));

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

void
PCP_ParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::error"));

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

void
PCP_ParserDriver::error (const YYLTYPE& location_in,
                         const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::error"));

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

bool
PCP_ParserDriver::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (fragment_);

  // reset scanner state
  PCP_Scanner_reset (scannerState_);

  // create/initialize a new buffer state
  if (useYYScanBuffer_)
  {
    bufferState_ =
      PCP_Scanner__scan_buffer (fragment_->rd_ptr (),
                                 fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                 scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      PCP_Scanner__scan_bytes (fragment_->rd_ptr (),
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
//  PCP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

void
PCP_ParserDriver::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_ParserDriver::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  PCP_Scanner__delete_buffer (bufferState_,
                               scannerState_);
  bufferState_ = NULL;
}
