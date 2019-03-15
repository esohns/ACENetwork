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
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_common.h"
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

#include "test_u_common.h"
#include "test_u_configuration.h"
#include "test_u_connection_common.h"

// forward declaration(s)
class Stream_IAllocator;
class Test_U_SessionMessage;
class Test_U_Message;
class Client_TimeoutHandler;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_TCPConnectionConfiguration> Test_U_ITCPConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Test_U_TCPConnectionConfiguration> Test_U_ITCPAsynchConnector_t;

class Net_IPing
{
 public:
  //virtual ~Net_IPing () {}

  virtual void ping () = 0;
};

//struct Test_U_ConnectorConfiguration
//{
//  Test_U_ConnectorConfiguration ()
//   : connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//  {}

//  Test_U_ITCPConnectionManager_t*     connectionManager;
//  struct Net_SocketConfigurationBase* socketHandlerConfiguration;
//};

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

  ACE_INET_Addr                      address;
  long                               actionTimerId;
  Test_U_TCPConnectionConfiguration* connectionConfiguration;
  Test_U_ITCPConnector_t*            connector;
  Stream_IAllocator*                 messageAllocator;
  unsigned int                       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
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
//                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
//typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
//typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

//typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

struct Client_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#endif
   , configuration (NULL)
   , subscribers ()
  {}

  struct Client_Configuration* configuration;

  Test_U_Subscribers_t         subscribers;
};
#endif // GUI_SUPPORT

#endif
