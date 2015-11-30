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

#include "irc_record.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "irc_tools.h"

IRC_Record::IRC_Record ()
 : inherited (1,
              true)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::IRC_Record"));

  command.string = NULL;
  command.discriminator = Command::INVALID;
}

IRC_Record::~IRC_Record ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::~IRC_Record"));

  switch (command.discriminator)
  {
    case IRC_Record::Command::STRING:
    {
      if (command.string)
        delete command.string;

      break;
    }
    default:
      break;
  } // end SWITCH
}

void
IRC_Record::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Record::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s"),
              ACE_TEXT (IRC_Tools::dump (*this).c_str ())));
}
