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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include <algorithm>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>

#include <ace/INET_Addr.h>
#include <ace/Synch_Traits.h>
#include <ace/Singleton.h>
#include <ace/Time_Value.h>

#include <gtk/gtk.h>

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "dhcp_common.h"
#include "dhcp_configuration.h"
#include "dhcp_defines.h"

#include "test_u_connection_common.h"
#include "test_u_defines.h"
#include "test_u_stream_common.h"

// forward declarations
class Stream_IAllocator;
struct Test_U_ConnectionConfiguration;
class Test_U_Message;
class Test_U_SessionMessage;
struct Test_U_StreamConfiguration;

struct Test_U_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_U_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = DHCP_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Test_U_UserData
 : Stream_UserData
{
  inline Test_U_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  struct Test_U_ConnectionConfiguration* configuration;
  struct Test_U_StreamConfiguration*     streamConfiguration;
};

//struct Test_U_MessageData
//{
//  inline Test_U_MessageData ()
//   : DHCPRecord (NULL)
//  {};
//  inline ~Test_U_MessageData ()
//  {
//    if (DHCPRecord)
//      delete DHCPRecord;
//  };

//  struct DHCP_Record* DHCPRecord;
//};
//typedef Stream_DataBase_T<Test_U_MessageData> Test_U_MessageData_t;

// forward declarations
struct Test_U_ConnectionConfiguration;
struct Test_U_ConnectionState;

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_U_ConnectionConfiguration,
                          struct Test_U_ConnectionState,
                          DHCP_RuntimeStatistic_t> Test_U_IConnection_t;

struct Test_U_DHCPClient_SessionData
 : Test_U_StreamSessionData
{
  inline Test_U_DHCPClient_SessionData ()
   : Test_U_StreamSessionData ()
   , broadcastConnection (NULL)
   , connection (NULL)
   , targetFileName ()
   , serverAddress (static_cast<u_short> (0),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , timeStamp (ACE_Time_Value::zero)
   , xid (0)
  {};
  inline struct Test_U_DHCPClient_SessionData& operator= (struct Test_U_DHCPClient_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    broadcastConnection =
        (broadcastConnection ? broadcastConnection
                             : rhs_in.broadcastConnection);
    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_U_IConnection_t* broadcastConnection; // DISCOVER/REQUEST/INFORM
  Test_U_IConnection_t* connection; // RELEASE
  std::string           targetFileName; // file writer module

  ACE_INET_Addr         serverAddress;
  ACE_Time_Value        timeStamp; // lease timeout
  unsigned int          xid;       // session ID
};
typedef Stream_SessionData_T<struct Test_U_DHCPClient_SessionData> Test_U_DHCPClient_SessionData_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_DHCPClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

struct Test_U_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {
    PDUSize = DHCP_MESSAGE_SIZE;
  };

  struct Test_U_UserData* userData;
};

//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 Test_U_Configuration,
//                                 Test_U_ConnectionState,
//                                 DHCP_RuntimeStatistic_t,
//                                 Test_U_UserData> Test_U_IConnectionManager_t;

// forward declarations
struct Test_U_StreamModuleHandlerConfiguration;
struct Test_U_StreamState;

//typedef Stream_Base_T<ACE_SYNCH_MUTEX,
//                      ACE_MT_SYNCH,
//                      Common_TimePolicy_t,
//                      Stream_StateMachine_ControlState,
//                      Test_U_StreamState,
//                      Test_U_StreamConfiguration,
//                      DHCP_RuntimeStatistic_t,
//                      Stream_ModuleConfiguration,
//                      Test_U_StreamModuleHandlerConfiguration,
//                      Test_U_StreamSessionData,   // session data
//                      Test_U_StreamSessionData_t, // session data container (reference counted)
//                      Test_U_SessionMessage,
//                      Test_U_Message> Test_U_StreamBase_t;

struct Test_U_StreamModuleHandlerConfiguration
 : DHCP_ModuleHandlerConfiguration
{
  inline Test_U_StreamModuleHandlerConfiguration ()
   : DHCP_ModuleHandlerConfiguration ()
   , broadcastConnection (NULL)
   , configuration (NULL)
   , contextID (0)
   , inbound (true)
   , passive (false)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
  {};

  Test_U_IConnection_t*                     broadcastConnection; // UDP target/net IO module
  struct Test_U_Configuration*              configuration;
  guint                                     contextID;
  bool                                      inbound; // net IO module
  bool                                      passive; // UDP target module
  struct Net_SocketConfiguration*           socketConfiguration;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_U_ISessionNotify_t*                  subscriber;
  Test_U_Subscribers_t*                     subscribers;
  std::string                               targetFileName; // dump module
};

typedef DHCP_ProtocolConfiguration Test_U_ProtocolConfiguration_t;

struct Test_U_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Test_U_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , messageAllocator (NULL)
   , networkInterface ()
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
                                 0)
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {
    int result = address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
                              static_cast<ACE_UINT32> (INADDR_ANY),
                              1,
                              0);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  };

  Stream_IAllocator*                        messageAllocator;
  std::string                               networkInterface;
  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                            statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                      useLoopBackDevice;
};
typedef Net_IListener_T<struct Test_U_ListenerConfiguration,
                        struct Test_U_SocketHandlerConfiguration> Test_U_IListener_t;

typedef Common_IStatistic_T<DHCP_RuntimeStatistic_t> Test_U_StatisticReportingHandler_t;

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_U_IListener_t*                 listener;
  Test_U_StatisticReportingHandler_t* statisticReportingHandler;
  long                                statisticReportingTimerID;
};

struct Test_U_StreamConfiguration
 : DHCP_StreamConfiguration
{
  inline Test_U_StreamConfiguration ()
   : DHCP_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
  {
    bufferSize = DHCP_MESSAGE_SIZE;
  };

  struct Test_U_StreamModuleHandlerConfiguration* moduleHandlerConfiguration; // stream module handler configuration
};

struct Test_U_DHCPClient_StreamState
 : Test_U_StreamState
{
  inline Test_U_DHCPClient_StreamState ()
   : Test_U_StreamState ()
   , currentSessionData (NULL)
  {};

  struct Test_U_DHCPClient_SessionData* currentSessionData;
};

struct Test_U_ConnectionConfiguration;
struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , parserConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , listenerConfiguration ()
   , userData ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** signal data **********************************
  struct Test_U_SignalHandlerConfiguration       signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Net_SocketConfiguration                 socketConfiguration;
  struct Test_U_SocketHandlerConfiguration       socketHandlerConfiguration;
  struct Test_U_ConnectionConfiguration          connectionConfiguration;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration              parserConfiguration;
  // **************************** stream data **********************************
  struct Stream_ModuleConfiguration              moduleConfiguration;
  struct Test_U_StreamModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_U_StreamConfiguration              streamConfiguration;
  // *************************** protocol data *********************************
  Test_U_ProtocolConfiguration_t                 protocolConfiguration;
  // *************************** listener data *********************************
  struct Test_U_ListenerConfiguration            listenerConfiguration;

  struct Test_U_UserData                         userData;
  bool                                           useReactor;
};

//typedef Stream_IModuleHandler_T<Test_U_StreamModuleHandlerConfiguration> Test_U_IModuleHandler_t;

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_U_IStreamNotify_t;

//////////////////////////////////////////

typedef std::map<guint, ACE_Thread_ID> Test_U_PendingActions_t;
typedef Test_U_PendingActions_t::iterator Test_U_PendingActionsIterator_t;
typedef std::set<guint> Test_U_CompletedActions_t;
typedef Test_U_CompletedActions_t::iterator Test_U_CompletedActionsIterator_t;
struct Test_U_GTK_ProgressData
{
  inline Test_U_GTK_ProgressData ()
   : completedActions ()
//   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , statistic ()
   , transferred (0)
  {};

  Test_U_CompletedActions_t  completedActions;
//  GdkCursorType                      cursorType;
  struct Common_UI_GTKState* GTKState;
  Test_U_PendingActions_t    pendingActions;
  DHCP_RuntimeStatistic_t    statistic;
  unsigned int               transferred; // byte(s)
};

enum Test_U_GTK_Event
{
  TEST_U_GKTEVENT_INVALID = -1,
  // ------------------------------------
  TEST_U_GTKEVENT_START = 0,
  TEST_U_GTKEVENT_DATA,
  TEST_U_GTKEVENT_END,
  TEST_U_GTKEVENT_STATISTIC,
  // ------------------------------------
  TEST_U_GTKEVENT_MAX
};
typedef std::deque<enum Test_U_GTK_Event> Test_U_GTK_Events_t;
typedef Test_U_GTK_Events_t::const_iterator Test_U_GTK_EventsIterator_t;

struct Test_U_GTK_CBData
 : Common_UI_GTKState
{
  inline Test_U_GTK_CBData ()
   : Common_UI_GTKState ()
   , configuration (NULL)
   , eventStack ()
   , progressData ()
   , progressEventSourceID (0)
  {};

  struct Test_U_Configuration*   configuration;
  Test_U_GTK_Events_t            eventStack;
  struct Test_U_GTK_ProgressData progressData;
  guint                          progressEventSourceID;
};

struct Test_U_ThreadData
{
  inline Test_U_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  struct Test_U_GTK_CBData* CBData;
  guint                     eventSourceID;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_U_GTK_CBData> Test_U_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_U_GTK_CBData> Test_U_GTK_Manager_t;
typedef ACE_Singleton<Test_U_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_U_UI_GTK_MANAGER_SINGLETON;

#endif
