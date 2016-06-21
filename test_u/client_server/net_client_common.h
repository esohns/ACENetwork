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

#ifndef NET_CLIENT_COMMON_H
#define NET_CLIENT_COMMON_H

#include "ace/INET_Addr.h"

#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"

#include "net_client_connector_common.h"

// forward declaration(s)
class Stream_IAllocator;
struct Net_SocketHandlerConfiguration;
class Net_Client_TimeoutHandler;

struct Net_Client_ConnectorConfiguration
{
 inline Net_Client_ConnectorConfiguration ()
  : connectionManager (NULL)
  , socketHandlerConfiguration (NULL)
  //, statisticCollectionInterval (0)
  {};

  Net_IInetConnectionManager_t*   connectionManager;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  //unsigned int                    statisticCollectionInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Net_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Net_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , connector (NULL)
   , messageAllocator (NULL)
   , peerAddress ()
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (0)
  {};

  long                            actionTimerId;
  Net_IConnector_t*               connector;
  Stream_IAllocator*              messageAllocator;
  ACE_INET_Addr                   peerAddress;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  unsigned int                    statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Net_Client_Configuration
 : Net_Configuration
{
  inline Net_Client_Configuration ()
   : Net_Configuration ()
   , signalHandlerConfiguration ()
   , timeoutHandler (NULL)
  {};

  Net_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  Net_Client_TimeoutHandler*            timeoutHandler;
};

#endif
