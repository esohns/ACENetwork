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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include <ace/config-lite.h>

#include "net_common.h"
#include "net_configuration.h"

// forward declarations
class Stream_IAllocator;
struct Net_Server_Configuration;

extern unsigned int random_seed;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
extern struct random_data random_data;
extern char random_state_buffer[];
#endif

struct Test_U_ConnectionConfiguration;
struct Test_U_UserData
 : Net_UserData
{
  inline Test_U_UserData ()
   : Net_UserData ()
   , configuration (NULL)
  {};

  struct Test_U_ConnectionConfiguration* configuration;
};

struct Test_U_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   , userData (NULL)
  {};

  struct Test_U_UserData* userData;
};

#endif
