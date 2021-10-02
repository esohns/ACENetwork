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

#ifndef TEST_U_CONFIGURATION_H
#define TEST_U_CONFIGURATION_H

#include <list>

#include "ace/config-lite.h"
#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Time_Value.h"

#include "common_parser_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_iallocator.h"
#include "stream_isessionnotify.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "net_server_defines.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"
#include "net_client_stream_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

// forward declarations
struct Test_U_ConnectionConfiguration;
class Test_U_Message;
class Test_U_SessionMessage;

struct Test_U_ProtocolConfiguration
{
  Test_U_ProtocolConfiguration ()
   : PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , pingInterval (ACE_Time_Value::zero)
   , pingAutoAnswer (true)
   , printPongMessages (true)
   , transportLayer (NET_TRANSPORTLAYER_TCP)
  {}

  unsigned int                PDUSize; // pdu size (if fixed)
  ACE_Time_Value              pingInterval; // {0: off}
  bool                        pingAutoAnswer;
  bool                        printPongMessages;
  enum Net_TransportLayerType transportLayer;
};

typedef Stream_ISessionDataNotify_T<struct Test_U_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::const_iterator Test_U_SubscribersIterator_t;

//extern const char stream_name_string_[];
struct Test_U_StreamConfiguration;
struct ClientServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_StreamConfiguration,
                               struct ClientServer_ModuleHandlerConfiguration> ClientServer_StreamConfiguration_t;
struct ClientServer_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  ClientServer_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   //, printProgressDot (false)
   , protocolConfiguration (NULL)
   , streamConfiguration (NULL)
   //, sessionData (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  }

  struct Test_U_ProtocolConfiguration* protocolConfiguration; // protocol handler
  ClientServer_StreamConfiguration_t*  streamConfiguration;
  Test_U_ISessionNotify_t*             subscriber;
  Test_U_Subscribers_t*                subscribers;
};

struct ClientServer_Configuration
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
  ClientServer_Configuration ()
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
   , streamConfiguration ()
   , protocolConfiguration ()
  {}

  struct Common_Parser_FlexAllocatorConfiguration allocatorConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                  connectionConfigurations;
  // **************************** stream data **********************************
  ClientServer_StreamConfiguration_t              streamConfiguration;
  // *************************** protocol data *********************************
  struct Test_U_ProtocolConfiguration             protocolConfiguration;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct ClientServer_ProgressData
#if defined (GTK_USE)
  : Test_U_GTK_ProgressData
#endif
{
  ClientServer_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE
  {}

  struct ClientServer_Configuration* configuration;
};

struct ClientServer_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  ClientServer_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif
   , progressData ()
   , subscribers ()
  {}

  struct ClientServer_Configuration* configuration;
  struct ClientServer_ProgressData   progressData;
  Test_U_Subscribers_t               subscribers;
};
#endif // GUI_SUPPORT

#endif
