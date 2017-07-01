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
#include "test_u_module_protocolhandler.h"

#include <iostream>

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "test_u_message.h"

Test_U_Module_ProtocolHandler::Test_U_Module_ProtocolHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , counter_ (1)
 , sessionID_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::Test_U_Module_ProtocolHandler"));

}

Test_U_Module_ProtocolHandler::~Test_U_Module_ProtocolHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::~Test_U_Module_ProtocolHandler"));

}

bool
Test_U_Module_ProtocolHandler::initialize (const struct Stream_ModuleHandlerConfiguration& configuration_in,
                                           Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::initialize"));

  if (inherited::isInitialized_)
  {
    counter_ = 1;
    sessionID_ = 0;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
Test_U_Module_ProtocolHandler::handleDataMessage (Test_U_Message*& message_inout,
                                                  bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::handleDataMessage"));

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

void
Test_U_Module_ProtocolHandler::handleSessionMessage (Test_U_SessionMessage*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::handleSessionMessage"));

  // don't care (implies yes per default, when part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::isInitialized_);

//  int result = -1;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // retain session ID for reporting
      const FileServer_SessionData_t& session_data_container_r =
          message_inout->get ();
      const struct FileServer_SessionData& session_data_r =
          session_data_container_r.get ();
      sessionID_ = session_data_r.sessionID;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    default:
      break;
  } // end SWITCH
}

void
Test_U_Module_ProtocolHandler::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Module_ProtocolHandler::dump_state"));

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\n"),
//               ACE_TEXT (inherited::name ())));
//
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\\END\n"),
//               ACE_TEXT (inherited::name ())));
}
