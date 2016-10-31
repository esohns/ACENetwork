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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include "stream_common.h"
#include "stream_inotify.h"

#include "net_common.h"

struct Net_Configuration;
struct Test_I_UserData
 : Net_UserData
{
  inline Test_I_UserData ()
   : Net_UserData ()
   , configuration (NULL)
  {};

  Net_Configuration* configuration;
};

struct Test_I_ConnectionState
 : Net_ConnectionState
{
  inline Test_I_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Net_Configuration* configuration;

  Test_I_UserData*   userData;
};

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_I_IStreamNotify_t;

#endif
