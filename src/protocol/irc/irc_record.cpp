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

#include <ace/Synch.h>
#include "irc_record.h"

#include <ace/Log_Msg.h>

#include "net_macros.h"

#include "irc_tools.h"

IRC_Record::IRC_Record ()
 : inherited (1,
              true)
 , prefix_ ()
 , command_ ()
 , parameters_ ()
 , parameterRanges_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::IRC_Record"));

  command_.string = NULL;
  command_.discriminator = Command::INVALID;
}

IRC_Record::IRC_Record (IRC_Record& record_in)
 : inherited (1,
              true)
 , prefix_ ()
 , command_ ()
 , parameters_ ()
 , parameterRanges_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::IRC_Record"));

  *this = record_in;
}

IRC_Record::~IRC_Record ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::~IRC_Record"));

  switch (command_.discriminator)
  {
    case IRC_Record::Command::STRING:
    {
      if (command_.string)
        delete command_.string;
      break;
    }
    default:
      break;
  } // end SWITCH
}

IRC_Record&
IRC_Record::operator= (IRC_Record& lhs_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::operator="));

  prefix_ = lhs_in.prefix_;
  if (command_.discriminator == Command::STRING)
  {
    delete command_.string;
    command_.string = NULL;
  } // end IF
  if (lhs_in.command_.discriminator == Command::STRING)
  {
    ACE_NEW_NORETURN (command_.string,
                      std::string (*lhs_in.command_.string));
    if (!command_.string)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing")));
    command_.discriminator = Command::STRING;
  } // end IF
  else
    command_ = lhs_in.command_;
  parameters_ = lhs_in.parameters_;
  parameterRanges_ = lhs_in.parameterRanges_;

  return *this;
}

void
IRC_Record::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s"),
              ACE_TEXT (IRC_Tools::dump (*this).c_str ())));
}
