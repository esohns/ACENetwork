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

#ifndef DHCP_STREAM_COMMON_H
#define DHCP_STREAM_COMMON_H

#include "stream_configuration.h"

#include "dhcp_common.h"

struct DHCP_Stream_SessionData
 : Stream_SessionData
{
  DHCP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , statistic ()
  {}

  struct DHCP_ConnectionState* connectionState;

  DHCP_Statistic_t             statistic;
};

struct DHCP_StreamState
 : Stream_State
{
  DHCP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct DHCP_Stream_SessionData* sessionData;
};

#endif
