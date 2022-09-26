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

#ifndef BITTORRENT_CLIENT_COMMON_H
#define BITTORRENT_CLIENT_COMMON_H

#include "common_itask.h"

#include "common_signal_common.h"

// forward declarations
#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
struct BitTorrent_Client_CursesState;
#endif // CURSES_USE
#endif // GUI_SUPPORT

struct BitTorrent_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  BitTorrent_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , control (NULL)
#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
   , cursesState (NULL)
#endif // CURSES_USE
#endif // GUI_SUPPORT
  {}

  Common_ITask*                         control;
#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
  struct BitTorrent_Client_CursesState* cursesState;
#endif // CURSES_USE
#endif // GUI_SUPPORT
};

#endif
