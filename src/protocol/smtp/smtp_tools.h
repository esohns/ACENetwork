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

#ifndef SMTP_TOOLS_H
#define SMTP_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"

#include "smtp_common.h"

class SMTP_Tools
{
 public:
  // debug info
  static std::string dump (const struct SMTP_Record&);

  static std::string CodeToString (SMTP_Code_t);

  static std::string CommandToString (SMTP_Command_t);

  static std::string StateToString (enum SMTP_ProtocolState);

  inline static bool isSuccess (SMTP_Code_t code_in) { return ((code_in >= 200) && (code_in < 300)); }

 private:
  ACE_UNIMPLEMENTED_FUNC (SMTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~SMTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (SMTP_Tools (const SMTP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (SMTP_Tools& operator= (const SMTP_Tools&))
};

#endif
