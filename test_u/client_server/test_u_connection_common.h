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

#ifndef TEST_U_CONNECTION_COMMON_H
#define TEST_U_CONNECTION_COMMON_H

#include <map>
#include <string>

#include "stream_common.h"
#include "stream_configuration.h"

#include "net_common.h"
#include "net_configuration.h"

#include "test_u_common.h"
//#include "test_u_configuration.h"

struct Test_U_ConnectionConfiguration;
struct Test_U_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ConnectionConfiguration* connectionConfiguration;

  struct Test_U_UserData*                userData;
};

extern const char stream_name_string_[];
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Test_U_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct Test_U_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline Test_U_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_U_StreamConfiguration_t*            streamConfiguration;

  struct Test_U_UserData*                  userData;
};
typedef std::map<std::string,
                 struct Test_U_ConnectionConfiguration> Test_U_ConnectionConfigurations_t;
typedef Test_U_ConnectionConfigurations_t::iterator Test_U_ConnectionConfigurationIterator_t;

struct Test_U_ConnectionState
 : Net_ConnectionState
{
  inline Test_U_ConnectionState ()
   : Net_ConnectionState ()
   , userData (NULL)
  {};

  struct Test_U_UserData* userData;
};

#endif
