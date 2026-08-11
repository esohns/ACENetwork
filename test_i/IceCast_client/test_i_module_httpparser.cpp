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

#include "test_i_module_httpparser.h"

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "net_macros.h"

Test_I_HTTPParser::Test_I_HTTPParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPParser::Test_I_HTTPParser"));

}

void
Test_I_HTTPParser::handleDataMessage (Test_I_Message*& message_inout,
                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPParser::handleDataMessage"));

  if (unlikely (inherited::multiBody_))
    return;

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);
}

//////////////////////////////////////////

Test_I_HTTPParser_2::Test_I_HTTPParser_2 (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPParser_2::Test_I_HTTPParser_2"));

}

//void
//Test_I_HTTPParser_2::handleDataMessage (Test_I_Message*& message_inout,
//                                        bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPParser::handleDataMessage_2"));
//
////  if (likely (inherited::multiBody_))
////  {
////    if (inherited::headFragment_)
////      goto continue_;
////    return;
////  } // end IF
////
////continue_:
//  inherited::handleDataMessage (message_inout,
//                                passMessageDownstream_out);
//}
