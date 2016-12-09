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

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>

#include "stream_common.h"

#include "net_macros.h"

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::Net_CppParserBase_T (bool traceScanning_in,
                                                           bool traceParsing_in)
 : fragment_ (NULL)
 , offset_ (0)
 , trace_ (traceParsing_in)
 , parser_ (dynamic_cast<ParserInterfaceType*> (this), // parser
            &scanner_)                                 // scanner
// , argument_ ()
 , scanner_ ()
 , blockInParse_ (false)
 , isFirst_ (true)
 , buffer_ (NULL)
 , streamBuffer_ ()
 , stream_ (&streamBuffer_)
 , messageQueue_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::Net_CppParserBase_T"));

  scanner_.set (this);

  scanner_.set_debug (traceScanning_in ? 1 : 0);
  parser_.set_debug_level (traceParsing_in ? 1 : 0);
#if YYDEBUG
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::~Net_CppParserBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::~Net_CppParserBase_T"));

  // finalize lex scanner
  if (buffer_)
    scanner_.yy_delete_buffer (buffer_);
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::initialize (bool traceScanning_in,
                                                  bool traceParsing_in,
                                                  ACE_Message_Queue_Base* messageQueue_in,
                                                  bool blockInParse_in,
                                                  bool)
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::initialize"));

  if (isInitialized_)
  {
    fragment_ = NULL;
    offset_ = 0;
    trace_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;

    blockInParse_ = false;
    isFirst_ = true;

    if (buffer_)
    {
      scanner_.yy_delete_buffer (buffer_);
      buffer_ = NULL;
    } // end IF

    messageQueue_ = NULL;

    isInitialized_ = false;
  } // end IF

  trace_ = traceParsing_in;

  blockInParse_ = blockInParse_in;

  messageQueue_ = messageQueue_in;

  scanner_.set_debug (traceScanning_in ? 1 : 0);
#if defined (YYDEBUG)
  parser_.set_debug_level (traceParsing_in ? 1 : 0);
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif

  isInitialized_ = true;
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
//Net_CppParserBase_T<SessionMessageType>::error (const YYLTYPE& location_in,
//                                                      const std::string& message_in)
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::error"));

//  std::ostringstream converter;
//  converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
//              location_in.begin.line, location_in.begin.column,
//              location_in.end.line, location_in.end.column,
//              ACE_TEXT (message_in.c_str ())));
  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
              ACE_TEXT ("failed to BitTorrent_Parser::parse(): \"%s\"\n"),
//              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
//              converter.str ().c_str (),
              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = fragment_;
  while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //std::clog << location_in << ": " << message_in << std::endl;
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  // retain a handle to the 'current' fragment
  fragment_ = data_in;
  offset_ = 0;

  int result = -1;
  bool do_scan_end = false;
  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_CppParserBase_T::scan_begin(), aborting\n")));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;

//    bittorrent_set_column (1, state_);
//    bittorrent_set_lineno (1, state_);
  } // end IF

  // parse data fragment
  try {
//    result = ::yyparse (this, state_);
    result = parser_.parse ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in ::yyparse(), continuing\n")));
                ACE_TEXT ("caught exception in ParserType::parse(), continuing\n")));
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
                  ACE_TEXT ("failed to parse BitTorrent PDU (result was: %d), aborting\n"),
                  result));
      goto error;
    }
  } // end SWITCH

  // finalize buffer/scanner
  scan_end ();
  do_scan_end = false;

  goto continue_;

error:
  if (do_scan_end)
    scan_end ();
  fragment_ = NULL;

continue_:
  return (result == 0);
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (fragment_);

  ACE_Message_Block* message_block_p = fragment_;
  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!blockInParse_)
      return false; // not enough data, cannot proceed

    wait (); // <-- wait for data
    if (!fragment_->cont ())
    {
      // *NOTE*: most probable reason: received session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("no data after Net_CppParserBase_T::wait(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();
  offset_ = 0;

  // unlink ?
  if (unlink_in)
    message_block_p->cont (NULL);

  // switch to the next fragment

  // clean state
  scan_end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (fragment_->capacity () - fragment_->length () >=
              NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_CppParserBase_T::scan_begin(), aborting\n")));
    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("switched input buffers...\n")));

  return true;
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::wait"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  enum Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (blockInParse_);
  if (!messageQueue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("message queue not set - cannot wait, returning\n")));
      return;
  } // end IF

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

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data = true; break;
      case ACE_Message_Block::MB_USER:
      {
        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
        if (session_message_p)
        {
          session_message_type = session_message_p->type ();
          if (session_message_type == STREAM_SESSION_MESSAGE_END)
            done = true; // session has finished --> abort
        } // end IF
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data) break;

    // requeue message
    result = messageQueue_->enqueue_tail (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));
      return;
    } // end IF

    message_block_p = NULL;
  } while (!done);

  // 2. append data ?
  if (message_block_p)
  {
    // sanity check(s)
    ACE_ASSERT (fragment_);

    ACE_Message_Block* message_block_2 = fragment_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ());
    message_block_2->cont (message_block_p);
  } // end IF
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::scan_begin"));

//  static int counter = 1;

  // sanity check(s)
  ACE_ASSERT (!buffer_);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  streamBuffer_.set (fragment_->rd_ptr (),
                     fragment_->length () + NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE );
//  stream_.rdbuf (&streamBuffer_);
  scanner_.switch_streams (&stream_, NULL);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment #%d --> %d byte(s)...\n"),
//              counter++,
//              fragment_->length ()));

  return true;
}

template <typename ScannerType,
          typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_CppParserBase_T<ScannerType,
                 ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_CppParserBase_T::scan_end"));

  // sanity check(s)
  if (!buffer_)
    return;

  // clean state
  scanner_.yy_delete_buffer (buffer_);
  buffer_ = NULL;
}

//////////////////////////////////////////

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::Net_ParserBase_T (bool traceScanning_in,
                                                        bool traceParsing_in)
 : fragment_ (NULL)
 , offset_ (0)
 , trace_ (traceParsing_in)
 , parser_ (dynamic_cast<ParserInterfaceType*> (this), // parser
            NULL)                                      // scanner
// , argument_ ()
 , state_ (NULL)
 , useYYScanBuffer_ (NET_PROTOCOL_DEFAULT_USE_YY_SCAN_BUFFER)
 , blockInParse_ (false)
 , isFirst_ (true)
 , buffer_ (NULL)
 , messageQueue_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::Net_ParserBase_T"));

  bool result = false;
  try {
    initialize (state_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IScanner_T::initialize(): \"%m\", continuing\n")));
  }
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IScanner_T::initialize(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (state_);
  parser_.set (state_);

  // trace ?
  try {
    debug (state_,
           traceScanning_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IScanner_T::debug(): \"%m\", continuing\n")));
  }
#if YYDEBUG
  parser_.set_debug_level (traceParsing_in ? 1 : 0);
  //  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::~Net_ParserBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::~Net_ParserBase_T"));

  // finalize lex scanner
  if (buffer_)
  {
    ACE_ASSERT (state_);
    try {
      destroy (state_,
               buffer_);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IScanner_T::destroy(): \"%m\", continuing\n")));
    }
  } // end IF

  if (state_)
  {
    try {
      finalize (state_);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IScanner_T::finalize(): \"%m\", continuing\n")));
    }
  } // end IF
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::initialize (bool traceScanning_in,
                                                  bool traceParsing_in,
                                                  ACE_Message_Queue_Base* messageQueue_in,
                                                  bool blockInParse_in,
                                                  bool useYYScanBuffer_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::initialize"));

  if (isInitialized_)
  {
    fragment_ = NULL;
    offset_ = 0;
    trace_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;

    blockInParse_ = false;
    isFirst_ = true;

    if (buffer_)
    {
      ACE_ASSERT (state_);
      try {
        destroy (state_,
                 buffer_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IScanner_T::destroy(): \"%m\", continuing\n")));
      }
      buffer_ = NULL;
    } // end IF
    if (state_)
    {
      try {
        finalize (state_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IScanner_T::finalize(): \"%m\", continuing\n")));
      }
      state_ = NULL;
    } // end IF

    messageQueue_ = NULL;
    useYYScanBuffer_ = NET_PROTOCOL_DEFAULT_USE_YY_SCAN_BUFFER;

    isInitialized_ = false;
  } // end IF

  trace_ = traceParsing_in;

  blockInParse_ = blockInParse_in;

  messageQueue_ = messageQueue_in;
  useYYScanBuffer_ = useYYScanBuffer_in;

  try {
    debug (state_,
           traceScanning_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IScanner_T::debug(): \"%m\", continuing\n")));
  }
#if defined (YYDEBUG)
  parser_.set_debug_level (traceParsing_in ? 1 : 0);
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif

  isInitialized_ = true;
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
//Net_ParserBase_T<SessionMessageType>::error (const YYLTYPE& location_in,
//                                                      const std::string& message_in)
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::error"));

//  std::ostringstream converter;
//  converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
//              location_in.begin.line, location_in.begin.column,
//              location_in.end.line, location_in.end.column,
//              ACE_TEXT (message_in.c_str ())));
  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
              ACE_TEXT ("failed to BitTorrent_Parser::parse(): \"%s\"\n"),
//              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
//              converter.str ().c_str (),
              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = fragment_;
  while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
  }

  //std::clog << location_in << ": " << message_in << std::endl;
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  // retain a handle to the 'current' fragment
  fragment_ = data_in;
  offset_ = 0;

  int result = -1;
  bool do_scan_end = false;
  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ParserBase_T::scan_begin(), aborting\n")));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;

//    bittorrent_set_column (1, state_);
//    bittorrent_set_lineno (1, state_);
  } // end IF

  // parse data fragment
  try {
//    result = ::yyparse (this, state_);
    result = parser_.parse ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in ::yyparse(), continuing\n")));
                ACE_TEXT ("caught exception in ParserType::parse(), continuing\n")));
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
                  ACE_TEXT ("failed to parse BitTorrent PDU (result was: %d), aborting\n"),
                  result));
      goto error;
    }
  } // end SWITCH

  // finalize buffer/scanner
  scan_end ();
  do_scan_end = false;

  goto continue_;

error:
  if (do_scan_end)
    scan_end ();
  fragment_ = NULL;

continue_:
  return (result == 0);
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (fragment_);

  ACE_Message_Block* message_block_p = fragment_;
  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!blockInParse_)
      return false; // not enough data, cannot proceed

    wait (); // <-- wait for data
    if (!fragment_->cont ())
    {
      // *NOTE*: most probable reason: received session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("no data after Net_ParserBase_T::wait(), aborting\n")));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();
  offset_ = 0;

  // unlink ?
  if (unlink_in)
    message_block_p->cont (NULL);

  // switch to the next fragment

  // clean state
  scan_end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (fragment_->capacity () - fragment_->length () >=
              NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ParserBase_T::scan_begin(), aborting\n")));
    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("switched input buffers...\n")));

  return true;
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::wait"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  enum Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (blockInParse_);
  if (!messageQueue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("message queue not set - cannot wait, returning\n")));
      return;
  } // end IF

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

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data = true; break;
      case ACE_Message_Block::MB_USER:
      {
        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
        if (session_message_p)
        {
          session_message_type = session_message_p->type ();
          if (session_message_type == STREAM_SESSION_MESSAGE_END)
            done = true; // session has finished --> abort
        } // end IF
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data) break;

    // requeue message
    result = messageQueue_->enqueue_tail (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));
      return;
    } // end IF

    message_block_p = NULL;
  } while (!done);

  // 2. append data ?
  if (message_block_p)
  {
    // sanity check(s)
    ACE_ASSERT (fragment_);

    ACE_Message_Block* message_block_2 = fragment_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ());
    message_block_2->cont (message_block_p);
  } // end IF
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
bool
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::scan_begin"));

  static int counter = 1;

  // sanity check(s)
  ACE_ASSERT (!buffer_);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  try {
    buffer_ = create (state_,
                      fragment_->rd_ptr (),
                      fragment_->length ());
  }
  catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IScanner_T::create(): \"%m\", continuing\n")));
  }
  if (!buffer_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IScanner_T::create(): \"%m\", aborting\n")));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("parsing fragment #%d --> %d byte(s)...\n"),
                counter++,
                fragment_->length ()));

  return (buffer_ != NULL);
}

template <typename ParserType,
          typename ParserInterfaceType,
          typename ArgumentType,
          typename SessionMessageType>
void
Net_ParserBase_T<ParserType,
                 ParserInterfaceType,
                 ArgumentType,
                 SessionMessageType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ParserBase_T::scan_end"));

  // sanity check(s)
  if (!buffer_)
    return;

  // clean buffer
  try {
    destroy (state_,
             buffer_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IScanner_T::destroy(): \"%m\", continuing\n")));
  }
  buffer_ = NULL;
}
