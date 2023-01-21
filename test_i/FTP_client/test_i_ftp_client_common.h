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

#ifndef TEST_I_FTP_CLIENT_COMMON_H
#define TEST_I_FTP_CLIENT_COMMON_H

#include <list>
#include <string>

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

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

#include "stream_dec_common.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "ftp_common.h"
#include "ftp_control.h"
#include "ftp_network.h"
#include "ftp_stream_common.h"

#include "test_i_common.h"
#include "test_i_stream_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"

struct FTP_Client_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  FTP_Client_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {
    defaultBufferSize = STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE;
  }
};

struct FTP_Client_MessageData
 : FTP_Record
{
  FTP_Client_MessageData ()
   : FTP_Record ()
  {}

  virtual ~FTP_Client_MessageData ()
  {}
  inline void operator= (const struct FTP_Record& rhs_in) { FTP_Record::operator= (rhs_in); }
  inline void operator+= (struct FTP_Client_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
typedef Stream_DataBase_T<struct FTP_Client_MessageData> FTP_Client_MessageData_t;

typedef Net_IConnection_T<ACE_INET_Addr,
                          //FTP_Client_ConnectionConfiguration,
                          struct Net_StreamConnectionState,
                          FTP_Statistic_t> FTP_Client_IConnection_t;
struct FTP_Client_SessionData
 : Test_I_StreamSessionData
{
  FTP_Client_SessionData ()
   : Test_I_StreamSessionData ()
   , connection (NULL) // outbound
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
  {}
  struct FTP_Client_SessionData& operator= (const struct FTP_Client_SessionData& rhs_in)
  {
    Test_I_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    format = rhs_in.format;
    return *this;
  }

  FTP_Client_IConnection_t*                 connection; // RELEASE
  enum Stream_Decoder_CompressionFormatType format;
};
typedef Stream_SessionData_T<struct FTP_Client_SessionData> FTP_Client_SessionData_t;

class Test_I_Message;
class Test_I_SessionMessage;
typedef Stream_ISessionDataNotify_T<struct FTP_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> FTP_Client_ISessionNotify_t;
typedef std::list<FTP_Client_ISessionNotify_t*> FTP_Client_Subscribers_t;
typedef FTP_Client_Subscribers_t::const_iterator FTP_Client_SubscribersIterator_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct FTP_Client_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_SessionMessage> FTP_Client_MessageAllocator_t;

//extern const char stream_name_string_[];
struct FTP_Client_StreamConfiguration;
struct FTP_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct FTP_Client_StreamConfiguration,
                               struct FTP_Client_ModuleHandlerConfiguration> FTP_Client_StreamConfiguration_t;
struct FTP_Client_ModuleHandlerConfiguration
 : FTP_ModuleHandlerConfiguration
{
  FTP_Client_ModuleHandlerConfiguration ()
   : FTP_ModuleHandlerConfiguration ()
   , connection (NULL)
   //, connectionConfigurations (NULL)
   , subscriber (NULL)
  {
    inbound = true;
    passive = false;
  }

  FTP_Client_IConnection_t*       connection; // TCP target/net IO module
  //Net_ConnectionConfigurations_t* connectionConfigurations;
  FTP_Client_ISessionNotify_t*    subscriber;
};

struct FTP_Client_StreamConfiguration
 : FTP_StreamConfiguration
{
  FTP_Client_StreamConfiguration ()
   : FTP_StreamConfiguration ()
  {}
};

struct FTP_Client_StreamState
 : Test_I_StreamState
{
  FTP_Client_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct FTP_Client_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct FTP_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  FTP_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //, statisticReportingHandler (NULL)
   //, statisticReportingTimerId (-1)
  {}

  //FTP_IStatisticReportingHandler_t* statisticReportingHandler;
  //long                              statisticReportingTimerId;
};

//////////////////////////////////////////
typedef Net_IListener_T<FTP_Client_ConnectionConfiguration> FTP_Client_IListener_t;
struct FTP_Client_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  FTP_Client_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfiguration ()
   , connectionConfiguration_2 ()
   , parserConfiguration ()
   , parserConfiguration_2 ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
   , listener (NULL)
   , loginOptions ()
  {}

  struct FTP_Client_AllocatorConfiguration     allocatorConfiguration;
  // **************************** signal data **********************************
  struct FTP_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  FTP_Client_ConnectionConfiguration           connectionConfiguration;
  FTP_Client_ConnectionConfiguration           connectionConfiguration_2; // listener
  // **************************** parser data **********************************
  struct Common_FlexBisonParserConfiguration   parserConfiguration;
  struct Common_FlexBisonParserConfiguration   parserConfiguration_2; // listener
  // **************************** stream data **********************************
  FTP_Client_StreamConfiguration_t             streamConfiguration;
  FTP_Client_StreamConfiguration_t             streamConfiguration_2; // listener
  // *************************** listener data *********************************
  FTP_Client_IListener_t*                      listener;
  // *************************** protocol data *********************************
  struct FTP_LoginOptions                      loginOptions;
};

typedef FTP_Control_T<FTP_Client_AsynchConnector_t,
                      FTP_Client_Connector_t,
                      FTP_Client_AsynchConnector_2,
                      FTP_Client_Connector_2,
                      struct Net_UserData> FTP_Control_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct FTP_Client_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#endif // GTK_USE
{
  FTP_Client_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#endif // GTK_USE
   , transferred (0)
  {}

  ACE_UINT64 transferred;
};

class Test_I_EventHandler_2;
struct FTP_Client_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#endif // GTK_USE
{
  FTP_Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE
   , control (NULL)
   , progressData ()
   , records ()
   , entries ()
   , fileName ()
#if defined (GTK_USE)
   , treeIter ()
#endif // GTK_USE
  {}

  struct FTP_Client_Configuration*  configuration;
  FTP_IControl*                     control;
  Test_I_EventHandler_2*            eventHandler;
  struct FTP_Client_UI_ProgressData progressData;
  FTP_Records_t                     records; // responses
  Common_File_Entries_t             entries; // list entries
  std::string                       fileName; // current-
#if defined (GTK_USE)
  GtkTreeIter                       treeIter; // current-
#endif // GTK_USE
};

struct FTP_Client_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#endif // GTK_USE
{
  FTP_Client_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
   , CBData (NULL)
#else
   : CBData (NULL)
#endif // GTK_USE
  {}

  struct FTP_Client_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
