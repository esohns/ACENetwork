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

#ifndef NET_CLIENT_COMMON_H
#define NET_CLIENT_COMMON_H

#include <list>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#elif defined (WXWIDGETS_USE)
//#include "common_ui_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_iconnector.h"

#include "test_u_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#else
#include "test_u_wxwidgets_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_u_stream_common.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"

// forward declaration(s)
class Stream_IAllocator;
struct ClientServer_SocketHandlerConfiguration;
class Test_U_SessionMessage;
class Test_U_Message;
class Client_TimeoutHandler;
typedef Net_IConnector_T<ACE_INET_Addr,
                         ClientServer_ConnectionConfiguration_t> Client_IConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               ClientServer_ConnectionConfiguration_t> Client_IAsynchConnector_t;

class Net_IPing
{
 public:
  //virtual ~Net_IPing () {}

  virtual void ping () = 0;
};

struct Client_ConnectorConfiguration
{
  Client_ConnectorConfiguration ()
   : connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
  {}

  ClientServer_IInetConnectionManager_t*          connectionManager;
  struct ClientServer_SocketHandlerConfiguration* socketHandlerConfiguration;
};

struct Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , address ()
   , actionTimerId (-1)
   , connectionConfiguration (NULL)
   , connector (NULL)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  ACE_INET_Addr                           address;
  long                                    actionTimerId;
  ClientServer_ConnectionConfiguration_t* connectionConfiguration;
  Client_IConnector_t*                    connector;
  Stream_IAllocator*                      messageAllocator;
  unsigned int                            statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Client_Configuration
 : ClientServer_Configuration
{
  Client_Configuration ()
   : ClientServer_Configuration ()
   , signalHandlerConfiguration ()
   , timeoutHandler (NULL)
  {}

  struct Client_SignalHandlerConfiguration signalHandlerConfiguration;
  Client_TimeoutHandler*                   timeoutHandler;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
//typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
//                                    struct Test_U_StreamSessionData,
//                                    enum Stream_SessionMessageType,
//                                    Test_U_Message,
//                                    Test_U_SessionMessage> ClientServer_ISessionNotify_t;
//typedef std::list<ClientServer_ISessionNotify_t*> ClientServer_Subscribers_t;
//typedef ClientServer_Subscribers_t::const_iterator ClientServer_SubscribersIterator_t;

//typedef Common_ISubscribe_T<ClientServer_ISessionNotify_t> ClientServer_ISubscribe_t;

struct Client_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_WxWidgets_CBData
#endif
{
  Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_WxWidgets_CBData ()
#endif
   , configuration (NULL)
   , subscribers ()
  {}

  struct Client_Configuration* configuration;

  ClientServer_Subscribers_t   subscribers;
};

//////////////////////////////////////////

#if defined (GTK_USE)
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Client_UI_CBData> Client_GtkBuilderDefinition_t;
#endif // GTK_USE
#endif // GUI_SUPPORT

#endif
