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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "test_u_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "file_server_connection_common.h"
#include "file_server_stream_common.h"

// forward declarations
struct FileServer_ConnectionConfiguration;
struct FileServer_ConnectionState;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_TCPConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct Net_UserData> FileServer_ITCPConnectionManager_t;

//typedef Net_ListenerConfiguration_T<FileServer_TCPConnectionConfiguration,
//                                    NET_TRANSPORTLAYER_TCP> FileServer_TCPListenerConfiguration_t;
typedef Net_IListener_T<FileServer_TCPConnectionConfiguration> Test_U_IListener_t;

//////////////////////////////////////////

struct FileServer_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  FileServer_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Test_U_IListener_t*            listener;
  Net_IStreamStatisticHandler_t* statisticReportingHandler;
  long                           statisticReportingTimerId;
};

struct FileServer_Configuration
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
  FileServer_Configuration ()
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
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   , listener (NULL)
   //, listenerConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  struct Common_Parser_FlexAllocatorConfiguration            allocatorConfiguration;
  Net_ConnectionConfigurations_t               connectionConfigurations;
  ACE_HANDLE                                   handle;
  Test_U_IListener_t*                          listener;
  //FileServer_TCPListenerConfiguration_t        listenerConfiguration;
  struct FileServer_SignalHandlerConfiguration signalHandlerConfiguration;
  FileServer_StreamConfiguration_t             streamConfiguration;

  struct Net_UserData                          userData;
};

//typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
//                                          struct Stream_AllocatorConfiguration,
//                                          Test_U_ControlMessage_t,
//                                          Test_U_Message,
//                                          Test_U_SessionMessage> Test_U_StreamMessageAllocator_t;

//////////////////////////////////////////

struct FileServer_SessionData;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct FileServer_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> FileServer_ISessionNotify_t;
typedef std::list<FileServer_ISessionNotify_t*> FileServer_Subscribers_t;
typedef FileServer_Subscribers_t::const_iterator FileServer_SubscribersIterator_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct FileServer_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  FileServer_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
  : Test_U_wxWidgets_CBData ()
#endif
   , configuration (NULL)
   , subscribers ()
  {}

  struct FileServer_Configuration* configuration;
  FileServer_Subscribers_t         subscribers;
};
#endif // GUI_SUPPORT

#endif
