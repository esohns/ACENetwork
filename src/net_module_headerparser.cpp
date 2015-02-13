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

#include "net_module_headerparser.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"
#include "net_message.h"
#include "net_sessionmessage.h"

Net_Module_HeaderParser::Net_Module_HeaderParser ()
 : //inherited(),
   isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_HeaderParser::Net_Module_HeaderParser"));

}

Net_Module_HeaderParser::~Net_Module_HeaderParser()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_HeaderParser::~Net_Module_HeaderParser"));

}

bool
Net_Module_HeaderParser::init ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_HeaderParser::init"));

  // sanity check(s)
  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    isInitialized_ = false;
  } // end IF

  isInitialized_ = true;

  return isInitialized_;
}

void
Net_Module_HeaderParser::handleDataMessage (Net_Message*& message_inout,
                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_HeaderParser::handleDataMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // interpret the message header...

  if (message_inout->length () < sizeof (Net_MessageHeader_t))
  {
    if (!message_inout->crunchForHeader (sizeof (Net_MessageHeader_t)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("[%u]: failed to RPG_Net_Message::crunchForHeader(%u), aborting\n"),
                  message_inout->getID (),
                  sizeof (Net_MessageHeader_t)));

      // clean up
      message_inout->release ();
      passMessageDownstream_out = false;

      return;
    } // end IF
  } // end IF

//   // OK: retrieve type of message and other details...
//   Net_MessageHeader* message_header = reinterpret_cast<Net_MessageHeader*> (//                                                                message_inout->rd_ptr());

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("received protocol message (ID: %u): [length: %u; type: \"%s\"]...\n"),
//               message_inout->getID (),
//               message_header->messageLength,
//               ACE_TEXT (Net_Common_Tools::messageType2String (message_header->messageType).c_str ())));
}

void
Net_Module_HeaderParser::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_HeaderParser::dump_state"));

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\n"),
//               ACE_TEXT (inherited::name ())));
//
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT (" ***** MODULE: \"%s\" state *****\\END\n"),
//               ACE_TEXT (inherited::name ())));
}
