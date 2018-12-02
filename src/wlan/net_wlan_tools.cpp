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

#include "net_wlan_tools.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "net_wlan_defines.h"

std::string
Net_WLAN_Tools::decodeSSID (void* data_in,
                            size_t length_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::decodeSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (data_in);
  ACE_ASSERT (length_in && length_in <= NET_WLAN_ESSID_MAX_SIZE);

  uint8_t* data_p = static_cast<uint8_t*> (data_in);
  char buffer_a[4 + 1]; // '\\xAB\0'
  int result_2 = -1;
  for (size_t i = 0;
       i < length_in;
       ++i)
  {
    if (::isprint (data_p[i]) &&
        data_p[i] != ' '      &&
        data_p[i] != '\\')
      result += static_cast<char> (data_p[i]);
    else if (data_p[i] == ' ')
      result += ' ';
    else
    { ACE_ASSERT (data_p[i] == '\\'); // --> unicode character
      ACE_OS::memset (buffer_a, 0, sizeof (char[4 + 1]));
      result_2 =
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("\\x%.2x"),
                           data_p[i + 1]);
      ACE_ASSERT (result_2 == 4);
      result += buffer_a;
    } // end ELSE
  } // end FOR

  return result;
}
