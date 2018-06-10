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

struct ClientServer_ConnectionConfiguration;
struct ClientServer_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  ClientServer_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   , socketConfiguration_2 ()
   , socketConfiguration_3 ()
   , connectionConfiguration (NULL)
   , userData (NULL)
  {
    socketConfiguration = &socketConfiguration_2; // default: TCP socket
  };

  struct Net_TCPSocketConfiguration            socketConfiguration_2;
  struct Net_UDPSocketConfiguration            socketConfiguration_3;
  struct ClientServer_ConnectionConfiguration* connectionConfiguration;

  struct Test_U_UserData*                      userData;
};

//extern const char stream_name_string_[];
struct ClientServer_StreamConfiguration;
struct ClientServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Net_AllocatorConfiguration,
                               struct ClientServer_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct ClientServer_ModuleHandlerConfiguration> ClientServer_StreamConfiguration_t;
struct ClientServer_ConnectionConfiguration;
typedef Net_ConnectionConfiguration_T<struct ClientServer_ConnectionConfiguration,
                                      struct Net_AllocatorConfiguration,
                                      ClientServer_StreamConfiguration_t> ClientServer_ConnectionConfiguration_t;
struct ClientServer_ConnectionState
 : Net_ConnectionState
{
  ClientServer_ConnectionState ()
   : Net_ConnectionState ()
   , userData (NULL)
  {};

  struct Test_U_UserData* userData;
};
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 ClientServer_ConnectionConfiguration_t,
                                 struct ClientServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Test_U_UserData> ClientServer_IInetConnectionManager_t;
struct ClientServer_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  ClientServer_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , userData (NULL)
  {};

  ClientServer_IInetConnectionManager_t*         connectionManager;
  struct ClientServer_SocketHandlerConfiguration socketHandlerConfiguration;

  struct Test_U_UserData*                        userData;
};
typedef std::map<std::string,
                 ClientServer_ConnectionConfiguration_t> ClientServer_ConnectionConfigurations_t;
typedef ClientServer_ConnectionConfigurations_t::iterator ClientServer_ConnectionConfigurationIterator_t;

#endif
