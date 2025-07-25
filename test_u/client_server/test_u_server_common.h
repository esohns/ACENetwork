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

#ifndef TEST_U_SERVER_COMMON_H
#define TEST_U_SERVER_COMMON_H

#include <list>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_istatistic.h"
#include "common_isubscribe.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_ilistener.h"

#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_u_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "net_client_stream_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;
struct Server_ListenerConfiguration;

//typedef Net_ListenerConfiguration_T<Test_U_TCPConnectionConfiguration,
//                                    NET_TRANSPORTLAYER_TCP> Server_ListenerConfiguration_t;

typedef Net_IListener_T<Test_U_TCPConnectionConfiguration> Test_U_ITCPListener_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_UDPConnectionConfiguration> Test_U_IUDPConnector_t;

struct Server_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Server_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (SSL_SUPPORT)
   , SSLListener (NULL)
#endif // SSL_SUPPORT
   , TCPListener (NULL)
   , UDPConnector (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

#if defined (SSL_SUPPORT)
  Test_U_ITCPListener_t*         SSLListener;
#endif // SSL_SUPPORT
  Test_U_ITCPListener_t*         TCPListener;
  Test_U_IUDPConnector_t*        UDPConnector;

  Net_IStreamStatisticHandler_t* statisticReportingHandler;
  long                           statisticReportingTimerId;
};

struct Server_Configuration
 : ClientServer_Configuration
{
  Server_Configuration ()
   : ClientServer_Configuration ()
#if defined (SSL_SUPPORT)
   , SSLListener (NULL)
#endif // SSL_SUPPORT
   , TCPListener (NULL)
   , UDPConnector (NULL)
   //, listenerConfiguration ()
   , signalHandlerConfiguration ()
  {}

#if defined (SSL_SUPPORT)
  Test_U_ITCPListener_t*                   SSLListener;
#endif // SSL_SUPPORT
  Test_U_ITCPListener_t*                   TCPListener;
  Test_U_IUDPConnector_t*                  UDPConnector;
  //Server_ListenerConfiguration_t           listenerConfiguration;

  struct Server_SignalHandlerConfiguration signalHandlerConfiguration;
};

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<struct Test_U_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

struct Server_UI_CBData
 : ClientServer_UI_CBData
{
  Server_UI_CBData ()
   : ClientServer_UI_CBData ()
  {}

};

#endif
