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

#ifndef PCP_STREAM_COMMON_H
#define PCP_STREAM_COMMON_H

#include "stream_configuration.h"

#include "pcp_common.h"

struct PCP_Stream_SessionData
 : Stream_SessionData
{
  PCP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , statistic ()
  {}

  struct PCP_ConnectionState* connectionState;

  PCP_Statistic_t             statistic;
};

struct PCP_StreamState
 : Stream_State
{
  PCP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct PCP_Stream_SessionData* sessionData;
};

#endif
