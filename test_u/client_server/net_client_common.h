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

#include <ace/INET_Addr.h>
#include <ace/Synch_Traits.h>

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_iconnector.h"

#include "test_u_common.h"
#include "test_u_configuration.h"
#include "test_u_gtk_common.h"
#include "test_u_stream_common.h"

// forward declaration(s)
class Stream_IAllocator;
struct Test_U_SocketHandlerConfiguration;
class Test_U_SessionMessage;
class Test_U_Message;
class Test_U_Client_TimeoutHandler;
typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_U_SocketHandlerConfiguration> Test_U_IConnector_t;

struct Test_U_Client_ConnectorConfiguration
{
  inline Test_U_Client_ConnectorConfiguration ()
   : connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   //, statisticCollectionInterval (0)
  {};

  Test_U_IInetConnectionManager_t*          connectionManager;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  //unsigned int                    statisticCollectionInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_U_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_U_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , connector (NULL)
   , messageAllocator (NULL)
   , peerAddress ()
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (0)
  {};

  long                                      actionTimerId;
  Test_U_IConnector_t*                      connector;
  Stream_IAllocator*                        messageAllocator;
  ACE_INET_Addr                             peerAddress;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  unsigned int                              statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_U_Client_Configuration
 : Test_U_Configuration
{
  inline Test_U_Client_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , timeoutHandler (NULL)
   , userData ()
  {};

  struct Test_U_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  Test_U_Client_TimeoutHandler*                   timeoutHandler;

  struct Test_U_UserData                          userData;
};

typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_U_Message,
                                Test_U_SessionMessage> Test_U_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_StreamMessageAllocator_t;

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

struct Test_U_Client_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_Client_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , subscribers ()
  {};

  struct Test_U_Client_Configuration* configuration;

  Test_U_Subscribers_t                subscribers;
};

#endif
