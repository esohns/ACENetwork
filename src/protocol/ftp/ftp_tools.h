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

#ifndef FTP_TOOLS_H
#define FTP_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "ftp_common.h"

class FTP_Tools
{
 public:
  // debug info
  static std::string dump (const struct FTP_Record&);

  static std::string CodeToString (FTP_Code_t);
  static std::string CommandToString (FTP_Command_t);
  static std::string StateToString (enum FTP_ProtocolState);
  static std::string DataStateToString (enum FTP_ProtocolDataState);

  inline static bool isSuccess (FTP_Code_t code_in) { return ((code_in >= 200) && (code_in < 300)); }

  static ACE_INET_Addr parsePASVResponse (const std::string&); // PASV response line #1

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~FTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Tools (const FTP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Tools& operator= (const FTP_Tools&))
};

#endif
