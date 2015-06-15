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

#include "IRC_client_IRCparser_driver.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "net_macros.h"

#include "IRC_client_defines.h"
#include "IRC_client_IRCmessage.h"
#include "IRC_client_IRCparser.h"
#include "IRC_client_IRCscanner.h"
#include "IRC_client_message.h"

IRC_Client_IRCParserDriver::IRC_Client_IRCParserDriver (bool traceScanning_in,
                                                        bool traceParsing_in)
 : traceParsing_ (traceParsing_in)
 , currentNumMessages_ (0)
 , currentScannerState_ (NULL)
 , currentBufferState_ (NULL)
 , currentFragment_ (NULL)
 , fragmentIsResized_ (false)
 , parser_ (this,                 // driver
            &currentNumMessages_, // counter
            currentScannerState_) // scanner
 , currentMessage_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::IRC_Client_IRCParserDriver"));

  if (IRC_Client_IRCScanner_lex_init_extra (this, &currentScannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", continuing\n")));
  ACE_ASSERT (currentScannerState_);
  parser_.set (currentScannerState_);

  // trace ?
  IRC_Client_IRCScanner_set_debug ((traceScanning_in ? 1 : 0),
                                   currentScannerState_);
#if YYDEBUG
  parser_.set_debug_level (traceParsing_in ? 1
                                           : 0); // binary (see bison manual)
#endif
}

IRC_Client_IRCParserDriver::~IRC_Client_IRCParserDriver ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::~IRC_Client_IRCParserDriver"));

  // finalize lex scanner
  if (IRC_Client_IRCScanner_lex_destroy (currentScannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
}

void
IRC_Client_IRCParserDriver::initialize (IRC_Client_IRCMessage& message_in,
                                        bool traceScanning_in,
                                        bool traceParsing_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  // set parse target
  currentMessage_ = &message_in;

  // trace ?
  IRC_Client_IRCScanner_set_debug ((traceScanning_in ? 1 : 0),
                                   currentScannerState_);
#if YYDEBUG
  parser_.set_debug_level (traceParsing_in ? 1 
                                           : 0); // binary (see bison manual)
#endif

  // OK
  isInitialized_ = true;
}

bool
IRC_Client_IRCParserDriver::parse (ACE_Message_Block* data_in,
                                   bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  // start with the first fragment...
  currentFragment_ = data_in;

  // *NOTE*: we parse ALL available message fragments
  // *TODO*: yyrestart(), yy_create_buffer/yy_switch_to_buffer, YY_INPUT...
  int result = -1;
//   do
//   { // init scan buffer
    if (!scan_begin (useYYScanBuffer_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_IRCParserDriver::scan_begin(), aborting\n")));

      // clean up
      currentFragment_ = NULL;

//       break;
      return false;
    } // end IF

    // parse our data
    result = parser_.parse ();
    if (result)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to parse IRC message fragment, aborting\n")));

    // finalize buffer/scanner
    scan_end ();

    int debug_level = 0;
#if YYDEBUG
    debug_level = parser_.debug_level ();
#endif
    // debug info
    if (debug_level)
    {
      try
      {
        currentMessage_->dump_state ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
      }
    } // end IF
//   } while (currentFragment_);

  // reset state
  isInitialized_ = false;

  return (result == 0);
}

bool
IRC_Client_IRCParserDriver::switchBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (currentScannerState_);
  if (currentFragment_->cont () == NULL)
    return false; // <-- nothing to do

  // switch to the next fragment
  currentFragment_ = currentFragment_->cont ();

  // clean state
  ACE_ASSERT (currentBufferState_);
  IRC_Client_IRCScanner__delete_buffer (currentBufferState_,
                                        currentScannerState_);
  currentBufferState_ = NULL;

  // initialize next buffer
  //currentBufferState_ = IRCScanner_scan_bytes(currentFragment_->rd_ptr(),
  //                                             currentFragment_->length(),
  //                                             myScannerContext);
//   if (!currentBufferState_)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to ::IRCScanner_scan_bytes(%@, %d), aborting\n"),
//                currentFragment_->rd_ptr(),
//                currentFragment_->length()));
//     return false;
//   } // end IF

//  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  IRC_Client_IRCScanner__switch_to_buffer (currentBufferState_,
                                           currentScannerState_);

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("switched to next buffer...\n")));

  return true;
}

bool
IRC_Client_IRCParserDriver::moreData ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::moreData"));

  return (currentFragment_->cont () != NULL);
}

bool
IRC_Client_IRCParserDriver::getDebugScanner () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::getDebugScanner"));

  return (IRC_Client_IRCScanner_get_debug (currentScannerState_) != 0);
}

void
IRC_Client_IRCParserDriver::error (const yy::location& location_in,
                                   const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::error"));

  std::ostringstream converter;
  converter << location_in;
  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%s): %s\n"),
              ACE_TEXT (converter.str ().c_str ()),
              ACE_TEXT (message_in.c_str ())));
//   ACE_DEBUG((LM_ERROR,
//              ACE_TEXT("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              std::string(currentFragment_->rd_ptr(), currentFragment_->length()).c_str(),
//              converter.str().c_str(),
//              message_in.c_str()));

  // dump message
  ACE_Message_Block* head = currentFragment_;
  while (head->prev ())
    head = head->prev ();
  ACE_ASSERT (head);
  IRC_Client_Message* message = NULL;
  message = dynamic_cast<IRC_Client_Message*> (head);
  ACE_ASSERT (message);
  try
  {
    message->dump_state ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

//   std::clog << location_in << ": " << message_in << std::endl;
}

void
IRC_Client_IRCParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::error"));

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("\": \"%s\"...\n"),
              ACE_TEXT (message_in.c_str ())));
//   ACE_DEBUG((LM_ERROR,
//              ACE_TEXT("failed to parse \"%s\": \"%s\"...\n"),
//              std::string(currentFragment_->rd_ptr(), currentFragment_->length()).c_str(),
//              message_in.c_str()));

//   std::clog << message_in << std::endl;
}

bool
IRC_Client_IRCParserDriver::scan_begin (bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (currentBufferState_ == NULL);
  ACE_ASSERT (currentFragment_);

  // create/init a new buffer state
  if (useYYScanBuffer_in)
  {
    currentBufferState_ =
      IRC_Client_IRCScanner__scan_buffer (currentFragment_->rd_ptr (),
                                          (currentFragment_->length () + IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE),
                                          currentScannerState_);
  } // end IF
  else
  {
    currentBufferState_ =
      IRC_Client_IRCScanner__scan_bytes (currentFragment_->rd_ptr (),
                                         currentFragment_->length (),
                                         currentScannerState_);
  } // end ELSE
  if (!currentBufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yy_scan_buffer/bytes(%@, %d), aborting\n"),
                currentFragment_->rd_ptr (),
                currentFragment_->length ()));
    return false;
  } // end IF

  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  IRC_Client_IRCScanner__switch_to_buffer (currentBufferState_,
                                           currentScannerState_);

  return true;
}

void
IRC_Client_IRCParserDriver::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCParserDriver::scan_end"));

  // sanity check(s)
  ACE_ASSERT (currentBufferState_);

//   // clean buffer
//   if (fragmentIsResized_)
//   {
//     if (currentFragment_->size(currentFragment_->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("failed to ACE_Message_Block::size(%u), continuing\n"),
//                  (currentFragment_->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)));
//     fragmentIsResized_ = false;
//   } // end IF

  // clean state
  IRC_Client_IRCScanner__delete_buffer (currentBufferState_,
                                        currentScannerState_);
  currentBufferState_ = NULL;

//   // switch to the next fragment (if any)
//   currentFragment_ = currentFragment_->cont();
}
