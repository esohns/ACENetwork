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

#ifndef FILE_SERVER_COMMON_H
#define FILE_SERVER_COMMON_H

#include <list>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

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

#include "test_u_configuration.h"
#include "test_u_gtk_common.h"

#include "file_server_connection_common.h"
#include "file_server_stream_common.h"

// forward declarations
struct FileServer_ConnectionConfiguration;
struct FileServer_ConnectionState;
struct FileServer_UserData;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct FileServer_UserData> FileServer_IInetConnectionManager_t;
class Test_U_SessionMessage;
class Test_U_Message;
struct FileServer_ListenerConfiguration;
typedef Net_IListener_T<struct FileServer_ListenerConfiguration,
                        struct FileServer_ConnectionConfiguration> Test_U_IListener_t;

//////////////////////////////////////////

struct FileServer_ConnectionConfiguration;
struct FileServer_UserData
 : Net_UserData
{
  FileServer_UserData ()
   : Net_UserData ()
   //, connectionConfiguration (NULL)
  {};

  //struct FileServer_ConnectionConfiguration* connectionConfiguration;
};

struct FileServer_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  FileServer_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {};

  Test_U_IListener_t*      listener;
  Net_IStatisticHandler_t* statisticReportingHandler;
  long                     statisticReportingTimerId;
};

struct FileServer_Configuration
 : Test_U_Configuration
{
  FileServer_Configuration ()
   : Test_U_Configuration ()
   , allocatorConfiguration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   , listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  struct Stream_AllocatorConfiguration         allocatorConfiguration;
  FileServer_ConnectionConfigurations_t        connectionConfigurations;
  ACE_HANDLE                                   handle;
  Test_U_IListener_t*                          listener;
  struct FileServer_ListenerConfiguration      listenerConfiguration;
  struct FileServer_SignalHandlerConfiguration signalHandlerConfiguration;
  FileServer_StreamConfiguration_t             streamConfiguration;

  struct FileServer_UserData                   userData;
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

struct FileServer_SessionData;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct FileServer_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> FileServer_ISessionNotify_t;
typedef std::list<FileServer_ISessionNotify_t*> FileServer_Subscribers_t;
typedef FileServer_Subscribers_t::const_iterator FileServer_SubscribersIterator_t;

struct FileServer_GTK_CBData
 : Test_U_GTK_CBData
{
  FileServer_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , subscribers ()
  {};

  struct FileServer_Configuration* configuration;
  FileServer_Subscribers_t         subscribers;
};

typedef Common_UI_GtkBuilderDefinition_T<struct FileServer_GTK_CBData> FileServer_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct FileServer_GTK_CBData> FileServer_GTK_Manager_t;
typedef ACE_Singleton<FileServer_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> FILESERVER_UI_GTK_MANAGER_SINGLETON;

#endif
