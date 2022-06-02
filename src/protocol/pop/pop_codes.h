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

#ifndef POP_CODES_H
#define POP_CODES_H

#include "ace/Global_Macros.h"

class POP_Codes
{
 public:
  enum CommandType
  {
    // AUTHENTICATION
    POP_COMMAND_AUTH_USER = 0,
    POP_COMMAND_AUTH_PASS,
    POP_COMMAND_AUTH_APOP,
    POP_COMMAND_AUTH_QUIT,
    // TRANSACTION
    POP_COMMAND_TRANS_STAT,
    POP_COMMAND_TRANS_LIST,
    POP_COMMAND_TRANS_RETR,
    POP_COMMAND_TRANS_DELE,
    POP_COMMAND_TRANS_NOOP,
    POP_COMMAND_TRANS_RSET,
    POP_COMMAND_TRANS_QUIT,
    POP_COMMAND_TRANS_TOP,
    POP_COMMAND_TRANS_UIDL,
    // UPDATE
    POP_COMMAND_UPDAT_QUIT,
    //////////////////////////////////////
    POP_COMMAND_MAX,
    POP_COMMAND_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (POP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (POP_Codes (const POP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (POP_Codes& operator= (const POP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~POP_Codes ())
};

#endif
