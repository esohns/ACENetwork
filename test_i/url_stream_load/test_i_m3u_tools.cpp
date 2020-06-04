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
#include "stdafx.h"

#include "test_i_m3u_tools.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

std::string
Test_I_M3U_Tools::ElementToString (const struct M3U_Element& element_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Tools::ElementToString"));

  std::string result;

  return result;
}

std::string
Test_I_M3U_Tools::PlaylistToString (const M3U_Playlist_t& playlist_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Tools::PlaylistToString"));

  std::string result;

  return result;
}
