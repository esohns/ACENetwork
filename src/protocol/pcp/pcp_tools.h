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

#ifndef PCP_TOOLS_H
#define PCP_TOOLS_H

#include <string>

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "net_defines.h"

#include "pcp_codes.h"
#include "pcp_common.h"

class PCP_Tools
{
 public:
  // debug info
  static std::string dump (const struct PCP_Record&);

  static std::string VersionToString (PCP_Version_t);
  static std::string OpcodeToString (PCP_Opcode_t);
  static std::string ResultCodeToString (PCP_ResultCode_t);
  static std::string OptionToString (PCP_Option_t);

  inline static bool isRequest (const struct PCP_Record& record_in) { return (record_in.opcode & 0x80); }
  static void mapAddress (const ACE_INET_Addr&, // address
                          ACE_UINT8[]);         // return value: mapped address
  static ACE_UINT64 generateNonce ();

 private:
  ACE_UNIMPLEMENTED_FUNC (PCP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~PCP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (PCP_Tools (const PCP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (PCP_Tools& operator= (const PCP_Tools&))
};

#endif
