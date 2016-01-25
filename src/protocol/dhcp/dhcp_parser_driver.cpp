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

#include "dhcp_parser_driver.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"

#include "net_macros.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_message.h"
#include "dhcp_parser.h"
#include "dhcp_scanner.h"

DHCP_ParserDriver::DHCP_ParserDriver (bool traceScanning_in,
                                      bool traceParsing_in)
 : finished_ (false)
 , fragment_ (NULL)
 , offset_ (0)
 , record_ (NULL)
 , trace_ (traceParsing_in)
//, parser_ (this,               // driver
//           &numberOfMessages_, // counter
//           scannerState_)      // scanner
 , scannerState_ (NULL)
 , bufferState_ (NULL)
 , messageQueue_ (NULL)
 , useYYScanBuffer_ (DHCP_DEFAULT_USE_YY_SCAN_BUFFER)
 , initialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::DHCP_ParserDriver"));

  if (DHCP_Scanner_lex_init_extra (this, &scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", continuing\n")));
  ACE_ASSERT (scannerState_);
  //parser_.set (scannerState_);

  // trace ?
  DHCP_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                          scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (traceParsing_in ? 1 : 0);
#endif
}

DHCP_ParserDriver::~DHCP_ParserDriver ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::~DHCP_ParserDriver"));

  // finalize lex scanner
  if (DHCP_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

void
DHCP_ParserDriver::initialize (DHCP_Record& record_in,
                               bool traceScanning_in,
                               bool traceParsing_in,
                               ACE_Message_Queue_Base* messageQueue_in,
                               bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::initialize"));

  if (initialized_)
  {
    finished_ = false;
    if (fragment_)
    {
      fragment_->release ();
      fragment_ = NULL;
    } // end IF
    offset_ = 0;

    initialized_ = false;
  } // end IF

  // set parse target
  record_ = &record_in;

  // trace ?
  DHCP_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                          scannerState_);
#if YYDEBUG
  //parser_.set_debug_level (traceParsing_in ? 1
  //                                         : 0); // binary (see bison manual)
  yydebug = (traceParsing_in ? 1 : 0);
#endif
  messageQueue_ = messageQueue_in;
  useYYScanBuffer_ = useYYScanBuffer_in;

  // OK
  initialized_ = true;
}

bool
DHCP_ParserDriver::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::parse"));

  // sanity check(s)
  ACE_ASSERT (initialized_);
  ACE_ASSERT (data_in);

  // start with the first fragment...
  fragment_ = data_in;

  // *NOTE*: parse ALL available message fragments
  // *TODO*: yyrestart(), yy_create_buffer/yy_switch_to_buffer, YY_INPUT...
  int result = -1;
//   do
//   { // initialize scan buffer
    if (!scan_begin ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to DHCP_ParserDriver::scan_begin(), aborting\n")));

      // clean up
      fragment_ = NULL;

//       break;
      return false;
    } // end IF

    // initialize scanner
    DHCP_Scanner_set_column (1, scannerState_);
    DHCP_Scanner_set_lineno (1, scannerState_);
    int debug_level = 0;
#if YYDEBUG
    //debug_level = parser_.debug_level ();
    debug_level = yydebug;
#endif
    ACE_UNUSED_ARG (debug_level);

    // parse data fragment
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
//   } while (fragment_);

  return (result == 0);
}

bool
DHCP_ParserDriver::getDebugScanner () const
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::getDebugScanner"));

  return (DHCP_Scanner_get_debug (scannerState_) != 0);
}

bool
DHCP_ParserDriver::switchBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (scannerState_);

  if (fragment_->cont () == NULL)
    wait (); // <-- wait for data
  if (!fragment_->cont ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no data after DHCP_ParserDriver::wait(), aborting\n")));
    return false;
  } // end IF
  fragment_ = fragment_->cont ();

  // switch to the next fragment

  // clean state
  scan_end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (fragment_->capacity () - fragment_->length () >= DHCP_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DHCP_ParserDriver::scan_begin(), aborting\n")));
    return false;
  } // end IF

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("switched input buffers...\n")));

  return true;
}

void
DHCP_ParserDriver::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::wait"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  if (!messageQueue_)
    return;

  // 1. wait for data
  do
  {
    result = messageQueue_->dequeue_head (message_block_p,
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

    if (message_block_p->msg_type () >= STREAM_MESSAGE_MAP_2)
      break;

    // session message --> put it back
    result = messageQueue_->enqueue_tail (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", aborting\n")));
      return;
    } // end IF
  } while (true);

  // 2. enqueue data
  ACE_Message_Block* message_block_2 = fragment_;
  for (;
       message_block_2->cont ();
       message_block_2 = message_block_2->cont ());
  message_block_2->cont (message_block_p);
}

void
DHCP_ParserDriver::error (const yy::location& location_in,
                          const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::error"));

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
DHCP_ParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::error"));

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
DHCP_ParserDriver::error (const YYLTYPE& location_in,
                          const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::error"));

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
DHCP_ParserDriver::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::scan_begin"));

//  static int counter = 1;

  // sanity check(s)
  ACE_ASSERT (bufferState_ == NULL);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  if (useYYScanBuffer_)
  {
    bufferState_ =
      DHCP_Scanner__scan_buffer (fragment_->rd_ptr (),
                                fragment_->length () + DHCP_FLEX_BUFFER_BOUNDARY_SIZE,
                                scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      DHCP_Scanner__scan_bytes (fragment_->rd_ptr (),
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
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment #%d --> %d byte(s)...\n"),
//              counter++,
//              fragment_->length ()));

//  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
//  DHCP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

void
DHCP_ParserDriver::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_ParserDriver::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  DHCP_Scanner__delete_buffer (bufferState_,
                               scannerState_);
  bufferState_ = NULL;
}
