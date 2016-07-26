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

#include <list>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "test_u_common.h"

#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"

#include "net_client_connector_common.h"

// forward declaration(s)
class Stream_IAllocator;
struct Net_SocketHandlerConfiguration;
class Net_SessionMessage;
class Net_Message;
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

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Net_Message,
                                Net_SessionMessage> Net_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Net_ControlMessage_t,
                                          Net_Message,
                                          Net_SessionMessage> Net_StreamMessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Net_StreamSessionData,
                                    Stream_SessionMessageType,
                                    Net_Message,
                                    Net_SessionMessage> Net_ISessionNotify_t;
typedef std::list<Net_ISessionNotify_t*> Net_Subscribers_t;
typedef Net_Subscribers_t::iterator Net_SubscribersIterator_t;

typedef Common_ISubscribe_T<Net_ISessionNotify_t> Net_ISubscribe_t;

struct Net_Client_GTK_CBData
 : Net_GTK_CBData
{
  inline Net_Client_GTK_CBData ()
   : Net_GTK_CBData ()
   , subscribers ()
   , subscribersLock ()
  {};

  Net_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX subscribersLock;
};

#endif
