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

#include "ace/Synch.h"
#include "test_u_stream.h"
#include "test_u_sessionmessage.h"
#include "test_u_module_headerparser.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "test_u_defines.h"
#include "test_u_message.h"

Test_U_Module_HeaderParser::Test_U_Module_HeaderParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_HeaderParser::Test_U_Module_HeaderParser"));

}

bool
Test_U_Module_HeaderParser::initialize (const struct Test_U_ModuleHandlerConfiguration& configuration_in,
                                              Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_HeaderParser::initialize"));

  // sanity check(s)
  if (inherited::isInitialized_)
  {
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
Test_U_Module_HeaderParser::handleDataMessage (Test_U_Message*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_HeaderParser::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

//  // interpret the message header...
//  Net_Remote_Comm::MessageHeader message_header = message_inout->getHeader ();

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("received protocol message (ID: %u): [length: %u; type: \"%s\"]...\n"),
  //            message_inout->getID (),
  //            message_header.messageLength,
  //            ACE_TEXT (Net_Message::CommandType2String (message_header.messageType).c_str ())));
}

void
Test_U_Module_HeaderParser::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_HeaderParser::dump_state"));

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\n"),
//               ACE_TEXT (inherited::name ())));
//
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\\END\n"),
//               ACE_TEXT (inherited::name ())));
}

//////////////////////////////////////////

//Net_Export const char libacenetwork_default_test_u_headerparser_module_name_string[] =
//  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_MODULE_HEADERPARSER_NAME);
const char libacenetwork_default_test_u_headerparser_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_MODULE_HEADERPARSER_NAME);
