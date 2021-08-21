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

#ifndef TEST_U_PCPCLIENT_COMMON_H
#define TEST_U_PCPCLIENT_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "pcp_common.h"
#include "pcp_configuration.h"
#include "pcp_defines.h"
#include "pcp_isession.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_defines.h"

struct PCP_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //PCPClient_ConnectionConfiguration,
                          struct PCP_ConnectionState,
                          PCP_Statistic_t> PCPClient_IConnection_t;
struct PCPClient_SessionData
 : Test_U_StreamSessionData
{
  PCPClient_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL) // outbound
   //, userData (NULL)
  {}
  struct PCPClient_SessionData& operator= (struct PCPClient_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    return *this;
  }

  PCPClient_IConnection_t* connection;          // RELEASE
  std::string              targetFileName;      // file writer module

  //struct Stream_UserData*   userData;
};
typedef Stream_SessionData_T<struct PCPClient_SessionData> PCPClient_SessionData_t;

class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct PCPClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> PCPClient_ISessionNotify_t;
typedef std::list<PCPClient_ISessionNotify_t*> PCPClient_Subscribers_t;
typedef PCPClient_Subscribers_t::const_iterator PCPClient_SubscribersIterator_t;

//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 PCPClient_ConnectionConfiguration_t,
//                                 struct PCP_ConnectionState,
//                                 PCP_Statistic_t,
//                                 Test_U_UserData> PCPClient_IConnectionManager_t;

//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                enum Stream_ControlMessageType,
//                                struct PCP_AllocatorConfiguration> PCPClient_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct PCP_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> PCPClient_MessageAllocator_t;

//extern const char stream_name_string_[];
//struct PCPClient_StreamConfiguration;
struct PCPClient_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct PCP_StreamConfiguration,
                               struct PCPClient_ModuleHandlerConfiguration> PCPClient_StreamConfiguration_t;
struct PCPClient_ModuleHandlerConfiguration
 : PCP_ModuleHandlerConfiguration
{
  PCPClient_ModuleHandlerConfiguration ()
   : PCP_ModuleHandlerConfiguration ()
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

  PCPClient_IConnection_t*         connection; // UDP target/net IO module
  Net_ConnectionConfigurations_t*  connectionConfigurations;
  PCPClient_StreamConfiguration_t* streamConfiguration; // PCP discover module
  PCPClient_ISessionNotify_t*      subscriber;
  PCPClient_Subscribers_t*         subscribers;
  std::string                      targetFileName; // dump module
};

//struct PCPClient_StreamConfiguration
// : PCP_StreamConfiguration
//{
//  PCPClient_StreamConfiguration ()
//   : PCP_StreamConfiguration ()
//   //, userData (NULL)
//  {}
//
//  //struct Net_UserData* userData;
//};

struct PCPClient_StreamState
 : Test_U_StreamState
{
  PCPClient_StreamState ()
   : Test_U_StreamState ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct PCPClient_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct PCPClient_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  PCPClient_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  PCP_IStatistic_t* statisticReportingHandler;
  long              statisticReportingTimerId;
};

//////////////////////////////////////////

struct PCPClient_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#else
 : Test_U_Configuration
#endif // GUI_SUPPORT
{
  PCPClient_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#else
   : Test_U_Configuration ()
#endif // GUI_SUPPORT
   , allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   , multicastHandle (ACE_INVALID_HANDLE)
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , handle (ACE_INVALID_HANDLE)
  {}

  struct PCP_AllocatorConfiguration           allocatorConfiguration;
  // **************************** signal data **********************************
  struct PCPClient_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t              connectionConfigurations;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration           parserConfiguration;
  // **************************** stream data **********************************
  PCPClient_StreamConfiguration_t             streamConfiguration;
  // *************************** protocol data *********************************
  struct PCP_ProtocolConfiguration            protocolConfiguration;
  // *************************** listener data *********************************

  ACE_HANDLE                                  multicastHandle; // listen handle (multicast)
  enum Common_EventDispatchType               dispatch;
  ACE_HANDLE                                  handle;          // listen handle (unicast)
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct PCPClient_UI_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#endif // GTK_USE
{
  PCPClient_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#endif // GTK_USE
  {}
};

struct PCPClient_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#endif // GTK_USE
{
  PCPClient_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE
   , externalAddress ()
   , gatewayAddress ()
   , interfaceAddress ()
   , progressData ()
   , session (NULL)
   , subscribers ()
  {}

  struct PCPClient_Configuration*  configuration;
  ACE_INET_Addr                    externalAddress;
  ACE_INET_Addr                    gatewayAddress;
  ACE_INET_Addr                    interfaceAddress;
  struct PCPClient_UI_ProgressData progressData;
  PCP_ISession_t*                  session;
  PCPClient_Subscribers_t          subscribers;
};

struct PCPClient_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#endif // GTK_USE
{
  PCPClient_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
   , CBData (NULL)
#else
   : CBData (NULL)
#endif // GTK_USE
  {}

  struct PCPClient_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
