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

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_message.h"
#include "http_scanner.h"

template <typename SessionMessageType>
HTTP_ParserDriver_T<SessionMessageType>::HTTP_ParserDriver_T (const std::string& scannerTables_in,
                                                              bool traceScanning_in,
                                                              bool traceParsing_in)
 : configuration_ (NULL)
 , finished_ (false)
 , fragment_ (NULL)
 , offset_ (0)
 , record_ (NULL)
 , trace_ (traceParsing_in)
 , blockInParse_ (false)
 , isFirst_ (true)
 , parser_ (this,          // driver
            scannerState_) // scanner
 , scannerState_ (NULL)
 , scannerTables_ (scannerTables_in)
 , bufferState_ (NULL)
 , messageQueue_ (NULL)
 , useYYScanBuffer_ (HTTP_DEFAULT_USE_YY_SCAN_BUFFER)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::HTTP_ParserDriver_T"));

  int result = -1;

  // step1: initialize flex state
  result = HTTP_Scanner_lex_init_extra (this,
                                        &scannerState_);
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (scannerState_);

//  // step2: load tables ?
//  FILE* file_p = NULL;
//  if (!scannerTables_.empty ())
//  {
//    std::string filename_string;
//    std::string package_name;
//#ifdef HAVE_CONFIG_H
//    package_name = ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME);
//#else
//#error "package name not available, set HAVE_CONFIG_H"
//#endif
//#if defined (DEBUG_DEBUGGER)
//  filename_string = Common_File_Tools::getWorkingDirectory ();
//  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//  filename_string += ACE_TEXT_ALWAYS_CHAR ("../../src/protocol/http");
//#else
//    filename_string =
//        Common_File_Tools::getConfigurationDataDirectory (package_name,
//                                                          true);
//#endif // #ifdef DEBUG_DEBUGGER
//    filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//    filename_string +=
//        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
//    filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//    filename_string += scannerTables_;
//    file_p = ACE_OS::fopen (filename_string.c_str (),
//                            ACE_TEXT_ALWAYS_CHAR ("rb"));
//    if (!file_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::fopen(\"%s\"): \"%m\", returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      return;
//    } // end IF
//    result = HTTP_Scanner_tables_fload (file_p,
//                                        scannerState_);
//    if (result)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to yy_tables_fload(\"%s\"): \"%m\", returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));

//      // clean up
//      result = ACE_OS::fclose (file_p);
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fclose(\"%s\"): \"%m\", continuing\n"),
//                    ACE_TEXT (filename_string.c_str ())));

//      return;
//    } // end IF
//    result = ACE_OS::fclose (file_p);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::fclose(\"%s\"): \"%m\", continuing\n"),
//                  ACE_TEXT (filename_string.c_str ())));

//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("loaded \"%s\"...\n"),
//                ACE_TEXT (scannerTables_.c_str ())));
//  } // end IF

  parser_.set (scannerState_);

  // trace ?
  HTTP_Scanner_set_debug ((traceScanning_in ? 1 : 0),
                          scannerState_);
  parser_.set_debug_level (trace_ ? 1 : 0); // binary (see bison manual)
  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
}

template <typename SessionMessageType>
HTTP_ParserDriver_T<SessionMessageType>::~HTTP_ParserDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::~HTTP_ParserDriver_T"));

//  int result = -1;

//  // finalize lex scanner
//  if (!scannerTables_.empty ())
//  {
//    result = HTTP_Scanner_tables_destroy (scannerState_);
//    if (result)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to yy_tables_destroy(): \"%m\", continuing\n")));
//  } // end IF

  if (HTTP_Scanner_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy(): \"%m\", continuing\n")));

  if (record_)
    delete record_;
}

template <typename SessionMessageType>
bool
HTTP_ParserDriver_T<SessionMessageType>::initialize (const struct Common_ParserConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::initialize"));

  //int result = -1;

  if (isInitialized_)
  {
    configuration_ = NULL;
    finished_ = false;
    fragment_ = NULL;
    offset_ = 0;
    if (record_)
      delete record_;
    record_ = NULL;

    blockInParse_ = false;
    isFirst_ = true;
    trace_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;

    //if (!scannerTables_.empty ())
    //{
    //  result = HTTP_Scanner_tables_destroy (scannerState_);
    //  if (result)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to yy_tables_destroy(): \"%m\", continuing\n")));
    //  scannerTables_ = scannerTables_in;
    //} // end IF

    if (bufferState_)
    {
      ACE_ASSERT (scannerState_);
      HTTP_Scanner__delete_buffer (bufferState_,
                                   scannerState_);
      bufferState_ = NULL;
    } // end IF
    //if (scannerState_)
    //{
    //  if (HTTP_Scanner_lex_destroy (scannerState_))
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
    //  scannerState_ = NULL;
    //} // end IF

    messageQueue_ = NULL;
    useYYScanBuffer_ = HTTP_DEFAULT_USE_YY_SCAN_BUFFER;

    isInitialized_ = false;
  } // end IF

  configuration_ =
      &const_cast<struct Common_ParserConfiguration&> (configuration_in);
  blockInParse_ = configuration_->block;
  trace_ = configuration_->debugParser;
  messageQueue_ = configuration_->messageQueue;
  useYYScanBuffer_ = configuration_->useYYScanBuffer;

  HTTP_Scanner_set_debug ((configuration_->debugScanner ? 1 : 0),
                          scannerState_);
  parser_.set_debug_level (trace_ ? 1
                                  : 0); // binary (see bison manual)
  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);

  isInitialized_ = true;

  return true;
}

template <typename SessionMessageType>
void
//HTTP_ParserDriver_T<SessionMessageType>::error (const struct YYLTYPE& location_in,
HTTP_ParserDriver_T<SessionMessageType>::error (const yy::location& location_in,
                                                const std::string& message_in)
//HTTP_ParserDriver_T<SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::error"));

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
template <typename SessionMessageType>
void
HTTP_ParserDriver_T<SessionMessageType>::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::error"));

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

template <typename SessionMessageType>
bool
HTTP_ParserDriver_T<SessionMessageType>::parse (ACE_Message_Block* data_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::parse"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  int result = -1;
  bool do_scan_end = false;

  // retain a handle to the 'current' fragment
  fragment_ = data_in;
  offset_ = 0;
//  if (record_)
//  {
//    delete record_;
//    record_ = NULL;
//  } // end IF
  record_ = NULL;
  ACE_NEW_NORETURN (record_,
                    struct HTTP_Record);
  if (!record_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (!scan_begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_ParserDriver_T::scan_begin(), aborting\n")));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;
    HTTP_Scanner_set_column (1, scannerState_);
    HTTP_Scanner_set_lineno (1, scannerState_);
  } // end IF

  // parse data fragment
  try {
    result = parser_.parse ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in yy::HTTP_Parser::parse(), continuing\n")));
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

template <typename SessionMessageType>
bool
HTTP_ParserDriver_T<SessionMessageType>::switchBuffer (bool unlink_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (fragment_);
  ACE_ASSERT (scannerState_);

  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!blockInParse_)
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
                ACE_TEXT ("failed to HTTP_ParserDriver_T::scan_begin(), aborting\n")));
    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("switched input buffers...\n")));

  return true;
}

template <typename SessionMessageType>
void
HTTP_ParserDriver_T<SessionMessageType>::waitBuffer ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (blockInParse_);
  ACE_ASSERT (messageQueue_);

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

template <typename SessionMessageType>
void
HTTP_ParserDriver_T<SessionMessageType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionMessageType>
bool
HTTP_ParserDriver_T<SessionMessageType>::scan_begin ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::scan_begin"));

//  static int counter = 1;

  // sanity check(s)
  ACE_ASSERT (!bufferState_);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  if (useYYScanBuffer_)
  {
    bufferState_ =
      HTTP_Scanner__scan_buffer (fragment_->rd_ptr (),
                                 fragment_->length () + NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE,
                                 scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      HTTP_Scanner__scan_bytes (fragment_->rd_ptr (),
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
//  HTTP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

template <typename SessionMessageType>
void
HTTP_ParserDriver_T<SessionMessageType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  HTTP_Scanner__delete_buffer (bufferState_,
                               scannerState_);
  bufferState_ = NULL;
}
