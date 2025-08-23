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

#include "ace/Log_Msg.h"

#include "stream_stat_defines.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_defines.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_common_modules.h"

template <typename ListenerType>
Test_U_Stream_2<ListenerType>::Test_U_Stream_2 ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_2::Test_U_Stream_2"));

}

template <typename ListenerType>
bool
Test_U_Stream_2<ListenerType>::load (Stream_ILayout* layout_inout,
                                     bool& delete_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_2::load"));

  if (!inherited::load (layout_inout,
                        delete_out))
    return false;

  delete_out = true;

  return true;
}
