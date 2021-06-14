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
#elif defined (WXWIDGETS_USE)
#include "test_u_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT
#include "net_client_stream_common.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"

#include "test_u_configuration.h"

// forward declaration(s)
class Stream_IAllocator;
class Test_U_SessionMessage;
class Test_U_Message;
class Client_SignalHandler;
class Client_TimeoutHandler;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_TCPConnectionConfiguration> Test_U_ITCPConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Test_U_TCPConnectionConfiguration> Test_U_ITCPAsynchConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_UDPConnectionConfiguration> Test_U_IUDPConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Test_U_UDPConnectionConfiguration> Test_U_IUDPAsynchConnector_t;

class Net_IPing
{
 public:
  //virtual ~Net_IPing () {}

  virtual void ping () = 0;
};

struct Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , address ()
   , actionTimerId (-1)
   , TCPConnectionConfiguration (NULL)
   , UDPConnectionConfiguration (NULL)
   , messageAllocator (NULL)
   , protocolConfiguration (NULL)
   , statisticReportingInterval (0)
  {}

  ACE_INET_Addr                        address;
  long                                 actionTimerId;
  Test_U_TCPConnectionConfiguration*   TCPConnectionConfiguration;
  Test_U_UDPConnectionConfiguration*   UDPConnectionConfiguration;
  Stream_IAllocator*                   messageAllocator;
  struct Test_U_ProtocolConfiguration* protocolConfiguration;
  unsigned int                         statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Client_Configuration
 : ClientServer_Configuration
{
  Client_Configuration ()
   : ClientServer_Configuration ()
   , signalHandlerConfiguration ()
   , signalHandler (NULL)
   , timeoutHandler (NULL)
  {}

  struct Client_SignalHandlerConfiguration signalHandlerConfiguration;
  Client_SignalHandler*                    signalHandler;

  Client_TimeoutHandler*                   timeoutHandler;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Client_UI_CBData
 : ClientServer_UI_CBData
{
  Client_UI_CBData ()
   : ClientServer_UI_CBData ()
   //, configuration (NULL)
  {}

  //struct Client_Configuration* configuration;
};
#endif // GUI_SUPPORT

#endif
