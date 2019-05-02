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

#include "ace/Synch_Traits.h"
#include "ace/INET_Addr.h"

#include "stream_common.h"
#include "stream_configuration.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_iconnectionmanager.h"

#include "test_u_common.h"

//extern const char stream_name_string_[];
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Net_AllocatorConfiguration,
                               struct Test_U_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;

struct Test_U_ConnectionState
 : Net_ConnectionState
{
  Test_U_ConnectionState ()
   : Net_ConnectionState ()
  {}
};

class Test_U_TCPConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct Net_AllocatorConfiguration,
                                        Test_U_StreamConfiguration_t,
                                        NET_TRANSPORTLAYER_TCP>
{
 public:
  Test_U_TCPConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
//   , connectionManager (NULL)
  {}

//  Test_U_IInetConnectionManager_t*         connectionManager;
};
class Test_U_UDPConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct Net_AllocatorConfiguration,
                                        Test_U_StreamConfiguration_t,
                                        NET_TRANSPORTLAYER_UDP>
{
 public:
  Test_U_UDPConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
//   , connectionManager (NULL)
  {}

//  Test_U_IInetConnectionManager_t*         connectionManager;
};
#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
class Test_U_NetlinkConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct Net_AllocatorConfiguration,
                                        Test_U_StreamConfiguration_t,
                                        NET_TRANSPORTLAYER_NETLINK>
{
 public:
  Test_U_NetlinkConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
//   , connectionManager (NULL)
  {}

//  Test_U_IInetConnectionManager_t*         connectionManager;
};
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

//typedef std::map<std::string,
//                 Test_U_TCPConnectionConfiguration> Test_U_TCPConnectionConfigurations_t;
//typedef Test_U_TCPConnectionConfigurations_t::iterator Test_U_TCPConnectionConfigurationIterator_t;
//typedef std::map<std::string,
//                 Test_U_UDPConnectionConfiguration> Test_U_UDPConnectionConfigurations_t;
//typedef Test_U_UDPConnectionConfigurations_t::iterator Test_U_UDPConnectionConfigurationIterator_t;

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_U_TCPConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 Net_Statistic_t,
                                 struct Net_UserData> Test_U_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_U_UDPConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 Net_Statistic_t,
                                 struct Net_UserData> Test_U_IUDPConnectionManager_t;

#endif
