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

#ifndef NET_OS_TOOLS_H
#define NET_OS_TOOLS_H

#include <string>

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"

class Net_OS_Tools
{
 public:
  // --- services ---
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//  static bool ifUpDownManageInterface (const std::string&,
//                                       bool); // toggle

  static bool networkManagerManageInterface (const std::string&,
                                             bool); // toggle
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_OS_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_OS_Tools())
  ACE_UNIMPLEMENTED_FUNC (Net_OS_Tools(const Net_OS_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_OS_Tools& operator= (const Net_OS_Tools&))

  // helper methods
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static bool isIfUpDownManagingInterface (const std::string&);

  static bool isNetworkManagerManagingInterface (const std::string&);
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
