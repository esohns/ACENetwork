/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef HTTP_NETWORK_H
#define HTTP_NETWORK_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Connector.h"
#endif // SSL_SUPPORT

#include "common_timer_manager_common.h"

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_tcpconnection_base.h"

#include "http_common.h"

// forward declarations
//struct HTTP_StreamConfiguration;

//class HTTP_ConnectionConfiguration
// : public Net_ConnectionConfiguration_T<struct Common_FlexParserAllocatorConfiguration,
//                                        struct HTTP_StreamConfiguration,
//                                        NET_TRANSPORTLAYER_TCP>
//{
// public:
//  HTTP_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration_T ()
//  {}
//};

struct HTTP_ConnectionState
 : Net_StreamConnectionState
{
  HTTP_ConnectionState ()
   : Net_StreamConnectionState ()
   //, configuration (NULL)
  {}

  //HTTP_ConnectionConfiguration* configuration;
};

#endif
