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

#ifndef PCP_CODES_H
#define PCP_CODES_H

#include "ace/Global_Macros.h"

class PCP_Codes
{
 public:
  enum VersionType
  {
    //PCP_VERSION_1 = 1, 
    PCP_VERSION_2,
    /////////////////////////////////////
    PCP_VERSION_MAX,
    PCP_VERSION_INVALID
  };
   
  enum OpcodeType
  {
    PCP_OPCODE_ANNOUNCE = 0,
    PCP_OPCODE_MAP = 1,
    PCP_OPCODE_PEER = 2,
    PCP_OPCODE_STANDARDS_ACTION_BEGIN = 3,
    PCP_OPCODE_STANDARDS_ACTION_END = 31,
    PCP_OPCODE_SPECIFICATION_REQUIRED_BEGIN = 32,
    PCP_OPCODE_SPECIFICATION_REQUIRED_END = 63,
    PCP_OPCODE_RESERVED_PRIVATE_BEGIN = 96,
    PCP_OPCODE_RESERVED_PRIVATE_END = 126,
    PCP_OPCODE_RESERVED_STANDARDS_ACTION = 127,
    /////////////////////////////////////
    PCP_OPCODE_MAX,
    PCP_OPCODE_INVALID
  };

  enum ResultCodeType
  {
    PCP_RESULTCODE_SUCCESS = 0,
    PCP_RESULTCODE_UNSUPP_VERSION,
    PCP_RESULTCODE_NOT_AUTHORIZED,
    PCP_RESULTCODE_MALFORMED_REQUEST,
    PCP_RESULTCODE_UNSUPP_OPCODE,
    PCP_RESULTCODE_UNSUPP_OPTION,
    PCP_RESULTCODE_MALFORMED_OPTION,
    PCP_RESULTCODE_NETWORK_FAILURE,
    PCP_RESULTCODE_NO_RESOURCES,
    PCP_RESULTCODE_UNSUPP_PROTOCOL,
    PCP_RESULTCODE_USER_EX_QUOTA,
    PCP_RESULTCODE_CANNOT_PROVIDE_EXTERNAL,
    PCP_RESULTCODE_ADDRESS_MISMATCH,
    PCP_RESULTCODE_EXCESSIVE_REMOTE_PEERS,
    PCP_RESULTCODE_STANDARDS_ACTION_BEGIN = 14,
    PCP_RESULTCODE_STANDARDS_ACTION_END = 127,
    PCP_RESULTCODE_SPECIFICATION_REQUIRED_BEGIN = 128,
    PCP_RESULTCODE_SPECIFICATION_REQUIRED_END = 191,
    PCP_RESULTCODE_PRIVATE_USE_BEGIN = 192,
    PCP_RESULTCODE_PRIVATE_USE_END = 254,
    /////////////////////////////////////
    PCP_RESULTCODE_STANDARDS_ACTION_RESERVED = 255,
    /////////////////////////////////////
    PCP_RESULTCODE_MAX,
    PCP_RESULTCODE_INVALID
  };

  enum OptionType
  {
    PCP_OPTION_THIRD_PARTY = 1,
    PCP_OPTION_PREFER_FAILURE,
    PCP_OPTION_FILTER,
    /////////////////////////////////////
    PCP_OPTION_MAX,
    PCP_OPTION_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (PCP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (PCP_Codes (const PCP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (PCP_Codes& operator= (const PCP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~PCP_Codes ())
};

#endif
