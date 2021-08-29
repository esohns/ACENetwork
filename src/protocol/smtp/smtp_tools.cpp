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

#include "smtp_tools.h"

#include <sstream>

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "smtp_codes.h"
#include "smtp_defines.h"

std::string
SMTP_Tools::dump (const struct SMTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Tools::dump"));

  std::ostringstream converter;
  std::string string_buffer;

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("code: \t");
  string_buffer += SMTP_Tools::CodeToString (record_in.code);
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("text: (");
  converter << record_in.text.size ();
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  for (SMTP_TextConstIterator_t iterator = record_in.text.begin ();
       iterator != record_in.text.end ();
       ++iterator)
  {
    string_buffer += *iterator;
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end FOR
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("text /END\n");

  return string_buffer;
}

std::string
SMTP_Tools::CodeToString (SMTP_Code_t code_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Tools::CodeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (code_in)
  {
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown code (was: %d), aborting\n"),
                  code_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
SMTP_Tools::StateToString (enum SMTP_ProtocolState state_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Tools::StateToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (state_in)
  {
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown code (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}
