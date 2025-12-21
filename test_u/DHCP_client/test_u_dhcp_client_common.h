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

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT

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

#include "dhcp_common.h"
#include "dhcp_configuration.h"
#include "dhcp_defines.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT

#include "test_u_connection_common.h"
#include "test_u_connection_stream.h"
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

class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ISessionDataNotify_T<struct DHCPClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> DHCPClient_ISessionNotify_t;
typedef std::list<DHCPClient_ISessionNotify_t*> DHCPClient_Subscribers_t;
typedef DHCPClient_Subscribers_t::const_iterator DHCPClient_SubscribersIterator_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct DHCP_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> DHCPClient_MessageAllocator_t;

//extern const char stream_name_string_[];
struct DHCPClient_StreamConfiguration;
struct DHCPClient_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct DHCPClient_StreamConfiguration,
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
    passive = false;
  }

  DHCPClient_IConnection_t*         connection; // UDP target/net IO module
  Net_ConnectionConfigurations_t*   connectionConfigurations;
  DHCPClient_StreamConfiguration_t* streamConfiguration; // dhcp discover module
  DHCPClient_ISessionNotify_t*      subscriber;
  DHCPClient_Subscribers_t*         subscribers;
  std::string                       targetFileName; // dump module
};

struct DHCPClient_StreamConfiguration
 : DHCP_StreamConfiguration
{
  DHCPClient_StreamConfiguration ()
   : DHCP_StreamConfiguration ()
   //, userData (NULL)
  {}

  //struct Net_UserData* userData;
};

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
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
{
  DHCPClient_Configuration ()
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
   , allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   , protocolConfiguration ()
   , connector (true)
   , asynchConnector (true)
   , connectorBcast (true)
   , asynchConnectorBcast (true)
   , broadcastHandle (ACE_INVALID_HANDLE)
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , handle (ACE_INVALID_HANDLE)
  {
    // *NOTE*: on linux systems, port 68 is taken (by systemd ?; see: `netstat -u --numeric-ports -p`)
    //         --> request broadcast replies
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    protocolConfiguration.requestBroadcastReplies = true;
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct DHCP_AllocatorConfiguration           allocatorConfiguration;
  // **************************** signal data **********************************
  struct DHCPClient_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t               connectionConfigurations;
  // **************************** parser data **********************************
  struct Common_FlexBisonParserConfiguration   parserConfiguration;
  // **************************** stream data **********************************
  DHCPClient_StreamConfiguration_t             streamConfiguration;
  // *************************** protocol data *********************************
  struct DHCP_ProtocolConfiguration            protocolConfiguration;
  // *************************** listener data *********************************
  DHCPClient_InboundConnector_t                connector;
  DHCPClient_InboundAsynchConnector_t          asynchConnector;
  DHCPClient_InboundConnectorBcast_t           connectorBcast;
  DHCPClient_InboundAsynchConnectorBcast_t     asynchConnectorBcast;

  ACE_HANDLE                                   broadcastHandle; // listen handle (broadcast)
  enum Common_EventDispatchType                dispatch;
  ACE_HANDLE                                   handle;          // listen handle (unicast)
};

//////////////////////////////////////////

struct DHCPClient_UI_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#endif // GTK_USE
{
  DHCPClient_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#endif // GTK_USE
  {}
};

struct DHCPClient_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#endif // GTK_USE
{
  DHCPClient_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE
   , progressData ()
   , subscribers ()
  {}

  struct DHCPClient_Configuration*  configuration;
  struct DHCPClient_UI_ProgressData progressData;
  DHCPClient_Subscribers_t          subscribers;
};

struct DHCPClient_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#endif // GTK_USE
{
  DHCPClient_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
   , CBData (NULL)
#else
   : CBData (NULL)
#endif // GTK_USE
  {}

  struct DHCPClient_UI_CBData* CBData;
};

#endif
