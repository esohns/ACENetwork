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

#ifndef TEST_I_M3U_TOOLS_H
#define TEST_I_M3U_TOOLS_H

#include "ace/Global_Macros.h"

#include "m3u_iparser.h"

class Test_I_M3U_Tools
{
 public:
  static std::string ElementToString (const struct M3U_Element&);
  static std::string PlaylistToString (const M3U_Playlist_t&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Tools (const Test_I_M3U_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_M3U_Tools& operator= (const Test_I_M3U_Tools&))
};

#endif
