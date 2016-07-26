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

#ifndef NET_SERVER_COMMON_H
#define NET_SERVER_COMMON_H

#include <list>

#include "common_istatistic.h"
#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "test_u_common.h"

#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"

typedef Common_IStatistic_T<Net_RuntimeStatistic_t> Net_Server_StatisticReportingHandler_t;

// forward declarations
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Net_Configuration,
                                 Net_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 Net_UserData> Net_IInetConnectionManager_t;
class Net_SessionMessage;
class Net_Message;
struct Net_Server_ListenerConfiguration;
typedef Net_IListener_T<Net_Server_ListenerConfiguration,
                        Net_SocketHandlerConfiguration> Net_IListener_t;

//////////////////////////////////////////

struct Net_Server_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Net_Server_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Net_IListener_t*                        listener;
  Net_Server_StatisticReportingHandler_t* statisticReportingHandler;
  long                                    statisticReportingTimerID;
};

struct Net_Server_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Net_Server_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionManager (NULL)
//   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  Net_IInetConnectionManager_t* connectionManager;
//  bool                            useLoopBackDevice;
};

struct Net_Server_Configuration
 : Net_Configuration
{
  inline Net_Server_Configuration ()
   : Net_Configuration ()
   , listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
  {};

  Net_IListener_t*                      listener;
  Net_Server_ListenerConfiguration      listenerConfiguration;

  Net_Server_SignalHandlerConfiguration signalHandlerConfiguration;
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

struct Net_Server_GTK_CBData
 : Net_GTK_CBData
{
  inline Net_Server_GTK_CBData ()
   : Net_GTK_CBData ()
   , subscribers ()
   , subscribersLock ()
  {};

  Net_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX subscribersLock;
};

#endif
