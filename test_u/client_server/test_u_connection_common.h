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
#include "test_u_stream_common.h"

//extern const char stream_name_string_[];
//struct Test_U_StreamConfiguration;
struct ClientServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_StreamConfiguration,
                               struct ClientServer_ModuleHandlerConfiguration> ClientServer_StreamConfiguration_t;

class Test_U_TCPConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<ClientServer_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  Test_U_TCPConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
   ///////////////////////////////////////
  {}
};

class Test_U_UDPConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<ClientServer_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_UDP>
{
 public:
  Test_U_UDPConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
   ///////////////////////////////////////
  {}
};

#endif
