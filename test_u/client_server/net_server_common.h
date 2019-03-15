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
#include "net_iconnector.h"
#include "net_ilistener.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_stream_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;
struct Server_ListenerConfiguration;

typedef Net_IListener_T<Net_TCPListenerConfiguration_t,
                        Test_U_TCPConnectionConfiguration> Server_IListener_t;
struct Server_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Server_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Server_IListener_t*      listener;
  Net_IStatisticHandler_t* statisticReportingHandler;
  long                     statisticReportingTimerId;
};

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_UDPConnectionConfiguration> Test_U_IUDPConnector_t;
struct Server_Configuration
 : ClientServer_Configuration
{
  Server_Configuration ()
   : ClientServer_Configuration ()
   , connector (NULL)
   , listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
  {}

  Test_U_IUDPConnector_t*                  connector;
  Server_IListener_t*                      listener;
  Net_TCPListenerConfiguration_t           listenerConfiguration;

  struct Server_SignalHandlerConfiguration signalHandlerConfiguration;
};

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

#if defined (GUI_SUPPORT)
struct Server_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  Server_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#endif
   , configuration (NULL)
   , subscribers ()
  {}

  struct Server_Configuration* configuration;

  Test_U_Subscribers_t         subscribers;
};
#endif // GUI_SUPPORT

#endif
