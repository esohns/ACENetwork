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

#ifndef POP_TOOLS_H
#define POP_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"

#include "pop_common.h"

class POP_Tools
{
 public:
  // debug info
  static std::string dump (const struct POP_Record&);

  static std::string CommandToString (POP_Command_t);

  static std::string StateToString (enum POP_ProtocolState);

  inline static bool isSuccess (unsigned char code_in) { return (code_in == 0x2B); }

 private:
  ACE_UNIMPLEMENTED_FUNC (POP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~POP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (POP_Tools (const POP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (POP_Tools& operator= (const POP_Tools&))
};

#endif
