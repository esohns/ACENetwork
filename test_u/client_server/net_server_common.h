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

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>

#include "common_istatistic.h"
#include "common_isubscribe.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "test_u_connection_common.h"
#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"
#include "test_u_gtk_common.h"

typedef Common_IStatistic_T<Net_RuntimeStatistic_t> Test_U_Server_StatisticReportingHandler_t;

// forward declarations
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct Test_U_UserData> Test_U_IInetConnectionManager_t;
class Test_U_SessionMessage;
class Test_U_Message;
struct Test_U_Server_ListenerConfiguration;
typedef Net_IListener_T<struct Test_U_Server_ListenerConfiguration,
                        struct Test_U_SocketHandlerConfiguration> Test_U_IListener_t;

//////////////////////////////////////////

struct Test_U_Server_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_U_Server_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_U_IListener_t*                        listener;
  Test_U_Server_StatisticReportingHandler_t* statisticReportingHandler;
  long                                       statisticReportingTimerID;
};

struct Test_U_Server_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Test_U_Server_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
//   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  Test_U_IInetConnectionManager_t*          connectionManager;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
//  bool                                    useLoopBackDevice;
};

struct Test_U_Server_Configuration
 : Test_U_Configuration
{
  inline Test_U_Server_Configuration ()
   : Test_U_Configuration ()
   , listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   //, socketHandlerConfiguration ()
  {};

  Test_U_IListener_t*                             listener;
  struct Test_U_Server_ListenerConfiguration      listenerConfiguration;

  struct Test_U_Server_SignalHandlerConfiguration signalHandlerConfiguration;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_ControlMessage_t;

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

struct Test_U_Server_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_Server_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , subscribers ()
  {};

  struct Test_U_Server_Configuration* configuration;

  Test_U_Subscribers_t                subscribers;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_U_Server_GTK_CBData> Test_U_Server_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_U_Server_GTK_CBData> Test_U_Server_GTK_Manager_t;
typedef ACE_Singleton<Test_U_Server_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> SERVER_UI_GTK_MANAGER_SINGLETON;

#endif
