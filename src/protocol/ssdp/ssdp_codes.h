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

#ifndef SSDP_CODES_H
#define SSDP_CODES_H

#include "ace/Global_Macros.h"

class SSDP_Codes
{
 public:
  enum MethodType
  {
    SSDP_METHOD_M_SEARCH = 0,
    SSDP_METHOD_NOTIFY,
    /////////////////////////////////////
    SSDP_METHOD_MAX,
    SSDP_METHOD_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (SSDP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (SSDP_Codes (const SSDP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (SSDP_Codes& operator= (const SSDP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~SSDP_Codes ())
};

// convenience typedefs
typedef SSDP_Codes::MethodType SSDP_Method_t;

#endif
