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

#ifndef TEST_U_DHCPCLIENT_COMMON_H
#define TEST_U_DHCPCLIENT_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "gtk/gtk.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
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

#include "test_u_common.h"
#include "test_u_stream_common.h"

#include "test_u_gtk_common.h"

#include "test_u_connection_common.h"
#include "test_u_defines.h"

struct DHCPClient_MessageData
{
  DHCPClient_MessageData ()
   : record (NULL)
  {}
  ~DHCPClient_MessageData ()
  {
    if (record)
      delete record;
  }

  struct DHCP_Record* record;
};
typedef Stream_DataBase_T<struct DHCPClient_MessageData> DHCPClient_MessageData_t;

struct DHCPClient_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          DHCPClient_ConnectionConfiguration_t,
                          struct DHCPClient_ConnectionState,
                          DHCP_Statistic_t> DHCPClient_IConnection_t;
struct DHCPClient_SessionData
 : Test_U_StreamSessionData
{
  DHCPClient_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL) // inbound
   , targetFileName ()
   , serverAddress (static_cast<u_short> (0),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , timeStamp (ACE_Time_Value::zero)
   , xid (0)
  {}
  struct DHCPClient_SessionData& operator= (struct DHCPClient_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

//  DHCPClient_IConnection_t* broadcastConnection; // DISCOVER/REQUEST/INFORM
  DHCPClient_IConnection_t* connection;          // RELEASE
  std::string               targetFileName;      // file writer module

  ACE_INET_Addr             serverAddress;
  ACE_Time_Value            timeStamp;           // lease timeout
  ACE_UINT32                xid;                 // session id
};
typedef Stream_SessionData_T<struct DHCPClient_SessionData> DHCPClient_SessionData_t;

class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct DHCPClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> DHCPClient_ISessionNotify_t;
typedef std::list<DHCPClient_ISessionNotify_t*> DHCPClient_Subscribers_t;
typedef DHCPClient_Subscribers_t::const_iterator DHCPClient_SubscribersIterator_t;

//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 DHCPClient_ConnectionConfiguration_t,
//                                 struct DHCPClient_ConnectionState,
//                                 DHCP_Statistic_t,
//                                 Test_U_UserData> DHCPClient_IConnectionManager_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_FlexParserAllocatorConfiguration> DHCPClient_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_FlexParserAllocatorConfiguration,
                                          DHCPClient_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> DHCPClient_MessageAllocator_t;

//extern const char stream_name_string_[];
struct DHCPClient_StreamConfiguration;
struct DHCPClient_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Common_FlexParserAllocatorConfiguration,
                               struct DHCPClient_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct DHCPClient_ModuleHandlerConfiguration> DHCPClient_StreamConfiguration_t;
struct DHCPClient_ModuleHandlerConfiguration
 : DHCP_ModuleHandlerConfiguration
{
  DHCPClient_ModuleHandlerConfiguration ()
   : DHCP_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
  {
    inbound = true;
    passive = false;
  }

  DHCPClient_IConnection_t*              connection; // UDP target/net IO module
  DHCPClient_ConnectionConfigurations_t* connectionConfigurations;
  DHCPClient_StreamConfiguration_t*      streamConfiguration; // dhcp discover module
  DHCPClient_ISessionNotify_t*           subscriber;
  DHCPClient_Subscribers_t*              subscribers;
  std::string                            targetFileName; // dump module
};

struct DHCPClient_StreamConfiguration
 : DHCP_StreamConfiguration
{
  DHCPClient_StreamConfiguration ()
   : DHCP_StreamConfiguration ()
   , userData (NULL)
  {}

  struct Test_U_UserData* userData;
};

struct DHCPClient_StreamState
 : Test_U_StreamState
{
  DHCPClient_StreamState ()
   : Test_U_StreamState ()
   , sessionData (NULL)
  {}

  struct DHCPClient_SessionData* sessionData;
};

//struct DHCPClient_SocketHandlerConfiguration;
//struct DHCPClient_ListenerConfiguration
// : Net_ListenerConfiguration
//{
//  DHCPClient_ListenerConfiguration ()
//   : Net_ListenerConfiguration ()
//   , socketHandlerConfiguration ()
//   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
//                                 0)
//  {
//    int result =
//      socketHandlerConfiguration.socketConfiguration_2.peerAddress.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
//                                                                        static_cast<ACE_UINT32> (INADDR_ANY),
//                                                                        1,
//                                                                        0);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
//    result =
//      socketHandlerConfiguration.socketConfiguration_2.listenAddress.set (static_cast<u_short> (DHCP_DEFAULT_SERVER_PORT),
//                                                                          static_cast<ACE_UINT32> (INADDR_ANY),
//                                                                          1,
//                                                                          0);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
//  }

//  struct DHCPClient_SocketHandlerConfiguration socketHandlerConfiguration;
//  ACE_Time_Value                               statisticReportingInterval; // [ACE_Time_Value::zero: off]
//};
//typedef Net_IListener_T<struct DHCPClient_ListenerConfiguration,
//                        struct DHCPClient_SocketHandlerConfiguration> DHCPClient_IListener_t;

struct DHCPClient_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  DHCPClient_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
//   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

//  DHCPClient_IListener_t*           listener;
  DHCP_StatisticReportingHandler_t* statisticReportingHandler;
  long                              statisticReportingTimerId;
};

//////////////////////////////////////////

struct DHCPClient_Configuration
 : Test_U_Configuration
{
  DHCPClient_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
//   , allocatorConfiguration ()
   , streamConfiguration ()
//   , listenerConfiguration ()
   , broadcastHandle (ACE_INVALID_HANDLE)
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , handle (ACE_INVALID_HANDLE)
  {}

  // **************************** signal data **********************************
  struct DHCPClient_SignalHandlerConfiguration   signalHandlerConfiguration;
  // **************************** socket data **********************************
  DHCPClient_ConnectionConfigurations_t          connectionConfigurations;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration              parserConfiguration;
  // **************************** stream data **********************************
//  struct Common_FlexParserAllocatorConfiguration allocatorConfiguration;
  DHCPClient_StreamConfiguration_t               streamConfiguration;
  // *************************** protocol data *********************************
  struct DHCP_ProtocolConfiguration              protocolConfiguration;
  // *************************** listener data *********************************
//  struct DHCPClient_ListenerConfiguration        listenerConfiguration;

  ACE_HANDLE                                     broadcastHandle; // listen handle (broadcast)
  enum Common_EventDispatchType                  dispatch;
  ACE_HANDLE                                     handle;          // listen handle (unicast)
};

//typedef Common_ISubscribe_T<DHCPClient_ISessionNotify_t> DHCPClient_ISubscribe_t;

//////////////////////////////////////////

struct DHCPClient_GTK_ProgressData
 : Test_U_GTK_ProgressData
{
  DHCPClient_GTK_ProgressData ()
   : Test_U_GTK_ProgressData ()
   , statistic ()
   , transferred (0)
  {}

  DHCP_Statistic_t statistic;
  unsigned int     transferred; // byte(s)
};

struct DHCPClient_GTK_CBData
 : Test_U_GTK_CBData
{
  DHCPClient_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , progressData ()
   , progressEventSourceId (0)
   , subscribers ()
  {}

  struct DHCPClient_Configuration*   configuration;
  struct DHCPClient_GTK_ProgressData progressData;
  guint                              progressEventSourceId;
  DHCPClient_Subscribers_t           subscribers;
};

struct DHCPClient_ThreadData
 : Test_U_GTK_ThreadData
{
  DHCPClient_ThreadData ()
   : Test_U_GTK_ThreadData ()
   , CBData (NULL)
  {}

  struct DHCPClient_GTK_CBData* CBData;
};

//////////////////////////////////////////

typedef Common_UI_GtkBuilderDefinition_T<struct DHCPClient_GTK_CBData> DHCPClient_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct DHCPClient_GTK_CBData> DHCPClient_GTK_Manager_t;
typedef ACE_Singleton<DHCPClient_GTK_Manager_t,
                      typename ACE_MT_SYNCH::MUTEX> DHCPCLIENT_GTK_MANAGER_SINGLETON;

#endif
