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

#ifndef TEST_U_UPNP_CLIENT_COMMON_H
#define TEST_U_UPNP_CLIENT_COMMON_H

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

#include "http_common.h"
#include "http_network.h"
#include "http_stream_common.h"

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

typedef Net_IConnection_T<ACE_INET_Addr,
                          //UPnP_Client_ConnectionConfiguration,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> UPnP_Client_IConnection_t;
struct UPnP_Client_SessionData
 : Test_U_StreamSessionData
{
  UPnP_Client_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL) // outbound
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
  {}
  struct UPnP_Client_SessionData& operator= (const struct UPnP_Client_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    format = rhs_in.format;
    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    return *this;
  }

  enum Stream_Decoder_CompressionFormatType format;
  UPnP_Client_IConnection_t*                connection;          // RELEASE
  std::string                               targetFileName;      // file writer module
};
typedef Stream_SessionData_T<struct UPnP_Client_SessionData> UPnP_Client_SessionData_t;

class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ISessionDataNotify_T<struct UPnP_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> UPnP_Client_ISessionNotify_t;
typedef std::list<UPnP_Client_ISessionNotify_t*> UPnP_Client_Subscribers_t;
typedef UPnP_Client_Subscribers_t::const_iterator UPnP_Client_SubscribersIterator_t;

//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 UPnP_Client_ConnectionConfiguration_t,
//                                 struct HTTP_ConnectionState,
//                                 UPnP_Statistic_t,
//                                 Test_U_UserData> UPnP_Client_IConnectionManager_t;

//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                enum Stream_ControlMessageType,
//                                struct Common_Parser_FlexAllocatorConfiguration> UPnP_Client_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_Parser_FlexAllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> UPnP_Client_MessageAllocator_t;

//extern const char stream_name_string_[];
struct UPnP_Client_StreamConfiguration;
struct UPnP_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct UPnP_Client_StreamConfiguration,
                               struct UPnP_Client_ModuleHandlerConfiguration> UPnP_Client_StreamConfiguration_t;
struct UPnP_Client_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  UPnP_Client_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
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

  UPnP_Client_IConnection_t*         connection; // UDP target/net IO module
  Net_ConnectionConfigurations_t*    connectionConfigurations;
  UPnP_Client_StreamConfiguration_t* streamConfiguration; // PCP discover module
  UPnP_Client_ISessionNotify_t*      subscriber;
  UPnP_Client_Subscribers_t*         subscribers;
  std::string                        targetFileName; // dump module
};

struct UPnP_Client_StreamConfiguration
 : HTTP_StreamConfiguration
{
  UPnP_Client_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
   //, userData (NULL)
  {}

  //struct Net_UserData* userData;
};

struct UPnP_Client_StreamState
 : Test_U_StreamState
{
  UPnP_Client_StreamState ()
   : Test_U_StreamState ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct UPnP_Client_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct UPnP_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  UPnP_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  HTTP_IStatisticReportingHandler_t* statisticReportingHandler;
  long                               statisticReportingTimerId;
};

//////////////////////////////////////////

struct UPnP_Client_Configuration
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
  UPnP_Client_Configuration ()
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
   , outboundHandle(ACE_INVALID_HANDLE)
   , multicastHandle (ACE_INVALID_HANDLE)
   , handle (ACE_INVALID_HANDLE)
  {
    parserConfiguration.headerOnly = true; // SSDP is header-only HTTP
  }

  struct Common_Parser_FlexAllocatorConfiguration allocatorConfiguration;
  // **************************** signal data **********************************
  struct UPnP_Client_SignalHandlerConfiguration   signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                  connectionConfigurations;
  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                 parserConfiguration;
  // **************************** stream data **********************************
  UPnP_Client_StreamConfiguration_t               streamConfiguration;
  // *************************** listener data *********************************

  ACE_HANDLE                                      outboundHandle;  // output handle (multicast)
  ACE_HANDLE                                      multicastHandle; // listen handle (multicast)
  ACE_HANDLE                                      handle;          // listen handle (unicast)
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct UPnP_Client_UI_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#endif // GTK_USE
{
  UPnP_Client_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#endif // GTK_USE
  {}
};

struct UPnP_Client_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#endif // GTK_USE
{
  UPnP_Client_UI_CBData ()
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
   //, session (NULL)
   , subscribers ()
  {}

  struct UPnP_Client_Configuration*  configuration;
  ACE_INET_Addr                      externalAddress;
  ACE_INET_Addr                      gatewayAddress;
  ACE_INET_Addr                      interfaceAddress;
  struct UPnP_Client_UI_ProgressData progressData;
  //UPnP_ISession_t*                   session;
  UPnP_Client_Subscribers_t          subscribers;
};

struct UPnP_Client_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#endif // GTK_USE
{
  UPnP_Client_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
   , CBData (NULL)
#else
   : CBData (NULL)
#endif // GTK_USE
  {}

  struct UPnP_Client_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
