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

//#include "ace/Synch.h"
#include "irc_parser_driver.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_parser_defines.h"

#include "net_defines.h"
#include "net_macros.h"

#include "irc_defines.h"
#include "irc_record.h"
#include "irc_parser.h"
#include "irc_scanner.h"
#include "irc_message.h"

IRC_ParserDriver::IRC_ParserDriver (bool traceScanning_in,
                                    bool traceParsing_in)
 : trace_ (traceParsing_in)
 , numberOfMessages_ (0)
 , scannerState_ (NULL)
 , bufferState_ (NULL)
 , fragment_ (NULL)
 , fragmentIsResized_ (false)
 , parser_ (this,               // driver
            &numberOfMessages_, // counter
            scannerState_)      // scanner
 , record_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::IRC_ParserDriver"));

  if (IRC_Scanner_lex_init_extra (this, &scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", continuing\n")));
  ACE_ASSERT (scannerState_);
  parser_.set (scannerState_);

  // trace ?
  IRC_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                         scannerState_);
#if YYDEBUG
  parser_.set_debug_level (traceParsing_in ? 1
                                           : 0); // binary (see bison manual)
#endif
}

IRC_ParserDriver::~IRC_ParserDriver ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::~IRC_ParserDriver"));

  // finalize lex scanner
  if (IRC_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

void
IRC_ParserDriver::initialize (IRC_Record& record_in,
                              bool traceScanning_in,
                              bool traceParsing_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  // set parse target
  record_ = &record_in;

  // trace ?
  IRC_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                         scannerState_);
#if YYDEBUG
  parser_.set_debug_level (traceParsing_in ? 1
                                           : 0); // binary (see bison manual)
#endif

  // OK
  isInitialized_ = true;
}

bool
IRC_ParserDriver::parse (ACE_Message_Block* data_in,
                         bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  // start with the first fragment...
  fragment_ = data_in;

  // *NOTE*: we parse ALL available message fragments
  // *TODO*: yyrestart(), yy_create_buffer/yy_switch_to_buffer, YY_INPUT...
  int result = -1;
//   do
//   { // initialize scan buffer
    if (!scan_begin (useYYScanBuffer_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_ParserDriver::scan_begin(), aborting\n")));

      // clean up
      fragment_ = NULL;

//       break;
      return false;
    } // end IF

    // parse data
    result = parser_.parse ();
    if (result)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to parse message fragment, aborting\n")));

    // finalize buffer/scanner
    scan_end ();

    int debug_level = 0;
#if YYDEBUG
    debug_level = parser_.debug_level ();
#endif
    // debug info
    if (debug_level)
    {
      try {
        record_->dump_state ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
      }
    } // end IF
//   } while (fragment_);

  // reset state
  isInitialized_ = false;

  return (result == 0);
}

bool
IRC_ParserDriver::switchBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (scannerState_);
  if (fragment_->cont () == NULL)
    return false; // <-- nothing to do

  // switch to the next fragment
  fragment_ = fragment_->cont ();

  // clean state
  ACE_ASSERT (bufferState_);
  IRC_Scanner__delete_buffer (bufferState_,
                              scannerState_);
  bufferState_ = NULL;

  // initialize next buffer
  //bufferState_ = IRCScanner_scan_bytes(fragment_->rd_ptr(),
  //                                             fragment_->length(),
  //                                             myScannerContext);
//   if (!bufferState_)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to ::IRCScanner_scan_bytes(%@, %d), aborting\n"),
//                fragment_->rd_ptr(),
//                fragment_->length()));
//     return false;
//   } // end IF

//  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  IRC_Scanner__switch_to_buffer (bufferState_,
                                 scannerState_);

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("switched to next buffer...\n")));

  return true;
}

void
IRC_ParserDriver::error (const yy::location& location_in,
                         const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::error"));

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
IRC_ParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::error"));

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
IRC_ParserDriver::scan_begin (bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (fragment_);

  // create/init a new buffer state
  if (useYYScanBuffer_in)
  {
    bufferState_ =
      IRC_Scanner__scan_buffer (fragment_->rd_ptr (),
                                fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      IRC_Scanner__scan_bytes (fragment_->rd_ptr (),
                               fragment_->length (),
                               scannerState_);
  } // end ELSE
  if (!bufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yy_scan_buffer/bytes(%@, %d), aborting\n"),
                fragment_->rd_ptr (),
                fragment_->length ()));
    return false;
  } // end IF

  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  IRC_Scanner__switch_to_buffer (bufferState_,
                                 scannerState_);

  return true;
}

void
IRC_ParserDriver::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_ParserDriver::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

//   // clean buffer
//   if (fragmentIsResized_)
//   {
//     if (fragment_->size(fragment_->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("failed to ACE_Message_Block::size(%u), continuing\n"),
//                  (fragment_->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)));
//     fragmentIsResized_ = false;
//   } // end IF

  // clean state
  IRC_Scanner__delete_buffer (bufferState_,
                              scannerState_);
  bufferState_ = NULL;

//   // switch to the next fragment (if any)
//   fragment_ = fragment_->cont();
}

