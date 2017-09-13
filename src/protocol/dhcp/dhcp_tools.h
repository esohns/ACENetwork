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

#ifndef DHCP_TOOLS_H
#define DHCP_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"

#include "dhcp_codes.h"
#include "dhcp_common.h"
#include "dhcp_exports.h"

class NET_PROTOCOL_DHCP_Export DHCP_Tools
{
 public:
  inline virtual ~DHCP_Tools () {};

  // debug info
  static std::string dump (const DHCP_Record&);

  static std::string OpToString (DHCP_Op_t);
  static std::string OptionToString (DHCP_Option_t);
  static std::string MessageTypeToString (DHCP_MessageType_t);

  static DHCP_MessageType_t MessageTypeToType (const std::string&);
  static DHCP_OptionFieldType_t OptionToFieldType (DHCP_Option_t);

  static bool isRequest (const DHCP_Record&);
  static DHCP_MessageType_t type (const DHCP_Record&);

  static unsigned int generateXID ();

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~DHCP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools (const DHCP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools& operator= (const DHCP_Tools&))
};

#endif
