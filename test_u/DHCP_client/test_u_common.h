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

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "dhcp_common.h"
#include "dhcp_configuration.h"
#include "dhcp_defines.h"

#include "test_u_defines.h"
//#include "test_u_message.h"
//#include "test_u_session_message.h"

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

// forward declarations
class Stream_IAllocator;
class Test_U_Message;
class Test_U_SessionMessage;
struct Test_U_Configuration;
struct Test_U_StreamConfiguration;
struct Test_U_UserData
 : Stream_UserData
{
  inline Test_U_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_U_Configuration*       configuration;
  Test_U_StreamConfiguration* streamConfiguration;
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

//  DHCP_Record* DHCPRecord;
//};
//typedef Stream_DataBase_T<Test_U_MessageData> Test_U_MessageData_t;

struct Test_U_StreamSessionData
 : Stream_SessionData
{
  inline Test_U_StreamSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , targetFileName ()
   , userData (NULL)
  {};
  inline Test_U_StreamSessionData& operator= (Test_U_StreamSessionData& rhs_in)
  {
    Stream_SessionData::operator= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  DHCP_ConnectionState* connectionState;
  std::string           targetFileName; // file writer module
  Test_U_UserData*      userData;
};
typedef Stream_SessionData_T<Test_U_StreamSessionData> Test_U_StreamSessionData_t;

struct Test_U_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {
    PDUSize = DHCP_MESSAGE_SIZE;
  };

  Test_U_UserData* userData;
};

// forward declarations
struct Test_U_Configuration;
struct Test_U_ConnectionState;
typedef DHCP_RuntimeStatistic_t Test_U_RuntimeStatistic_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_U_Configuration,
                          Test_U_ConnectionState,
                          Test_U_RuntimeStatistic_t> Test_U_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_U_Configuration,
                                 Test_U_ConnectionState,
                                 Test_U_RuntimeStatistic_t,
                                 ////////
                                 Test_U_UserData> Test_U_IConnectionManager_t;
struct Test_U_StreamModuleHandlerConfiguration;
struct Test_U_StreamState;
typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                      ////////////////////
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      ////////////////////
                      Stream_StateMachine_ControlState,
                      Test_U_StreamState,
                      ////////////////////
                      Test_U_StreamConfiguration,
                      ////////////////////
                      Test_U_RuntimeStatistic_t,
                      ////////////////////
                      Stream_ModuleConfiguration,
                      Test_U_StreamModuleHandlerConfiguration,
                      ////////////////////
                      Test_U_StreamSessionData,   // session data
                      Test_U_StreamSessionData_t, // session data container (reference counted)
                      Test_U_SessionMessage,
                      Test_U_Message> Test_U_StreamBase_t;
struct Test_U_StreamModuleHandlerConfiguration
 : DHCP_ModuleHandlerConfiguration
{
  inline Test_U_StreamModuleHandlerConfiguration ()
   : DHCP_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , contextID (0)
   , inbound (true)
   , passive (false)
   , printProgressDot (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
   , targetFileName ()
  {};

  Test_U_Configuration*              configuration;
  Test_U_IConnection_t*              connection; // UDP target/net IO module
  Test_U_IConnectionManager_t*       connectionManager; // UDP IO module
  guint                              contextID;
  bool                               inbound; // net IO module
  bool                               passive; // UDP target module
  bool                               printProgressDot; // dump module
  Net_SocketConfiguration*           socketConfiguration;
  Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_U_StreamBase_t*               stream; // UDP target/net IO module
  std::string                        targetFileName; // dump module
};

typedef DHCP_ProtocolConfiguration Test_U_ProtocolConfiguration_t;

struct Test_U_ListenerConfiguration
{
  inline Test_U_ListenerConfiguration ()
   : address (DHCP_DEFAULT_CLIENT_PORT, static_cast<ACE_UINT32> (INADDR_ANY))
   , addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , networkInterface ()
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
                                 0)
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  ACE_INET_Addr                      address;
  int                                addressFamily;
  Test_U_IConnectionManager_t*       connectionManager;
  Stream_IAllocator*                 messageAllocator;
  std::string                        networkInterface;
  Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                     statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                               useLoopBackDevice;
};
typedef Net_IListener_T<Test_U_ListenerConfiguration,
                        Test_U_SocketHandlerConfiguration> Test_U_IListener_t;

typedef Common_IStatistic_T<Test_U_RuntimeStatistic_t> Test_U_StatisticReportingHandler_t;

struct Test_U_SignalHandlerConfiguration
{
  inline Test_U_SignalHandlerConfiguration ()
   : listener (NULL)
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

  Test_U_StreamModuleHandlerConfiguration* moduleHandlerConfiguration; // stream module handler configuration
};

struct Test_U_StreamState
 : Stream_State
{
  inline Test_U_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_U_StreamSessionData* currentSessionData;
  Test_U_UserData*          userData;
};

struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
   , streamConfiguration ()
   , listenerConfiguration ()
   , userData ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** signal data **********************************
  Test_U_SignalHandlerConfiguration       signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_SocketConfiguration                 socketConfiguration;
  Test_U_SocketHandlerConfiguration       socketHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_ModuleConfiguration              moduleConfiguration_2;
  Test_U_StreamModuleHandlerConfiguration moduleHandlerConfiguration_2;
  Test_U_StreamConfiguration              streamConfiguration;
  // *************************** protocol data *********************************
  Test_U_ProtocolConfiguration_t          protocolConfiguration;
  // *************************** listener data *********************************
  Test_U_ListenerConfiguration            listenerConfiguration;

  Test_U_UserData                         userData;
  bool                                    useReactor;
};

typedef Stream_IModuleHandler_T<Test_U_StreamModuleHandlerConfiguration> Test_U_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,

                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

typedef Common_INotify_T<Test_U_StreamSessionData,
                         Test_U_Message,
                         Test_U_SessionMessage> Test_U_IStreamNotify_t;
typedef std::list<Test_U_IStreamNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_U_IStreamNotify_t> Test_U_ISubscribe_t;

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

  Test_U_CompletedActions_t completedActions;
//  GdkCursorType                      cursorType;
  Common_UI_GTKState*       GTKState;
  Test_U_PendingActions_t   pendingActions;
  Test_U_RuntimeStatistic_t statistic;
  unsigned int              transferred; // byte(s)
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
typedef std::deque<Test_U_GTK_Event> Test_U_GTK_Events_t;
typedef Test_U_GTK_Events_t::const_iterator Test_U_GTK_EventsIterator_t;

struct Test_U_GTK_CBData
 : Common_UI_GTKState
{
  inline Test_U_GTK_CBData ()
   : Common_UI_GTKState ()
   , configuration (NULL)
   , eventStack ()
   , logStack ()
   , progressData ()
   , progressEventSourceID (0)
   , stream (NULL)
   , subscribers ()
//   , subscribersLock ()
  {};

  Test_U_Configuration*     configuration;
  Test_U_GTK_Events_t       eventStack;
  Common_MessageStack_t     logStack;
  Test_U_GTK_ProgressData   progressData;
  guint                     progressEventSourceID;
  Test_U_StreamBase_t*      stream;
  Test_U_Subscribers_t      subscribers;
  // *NOTE*: use Common_UI_GTKState.lock instead
//  ACE_SYNCH_RECURSIVE_MUTEX subscribersLock;
};

struct Test_U_ThreadData
{
  inline Test_U_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  Test_U_GTK_CBData* CBData;
  guint              eventSourceID;
};

#endif
