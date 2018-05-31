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

#include "ace/INET_Addr.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif
#include "ace/Time_Value.h"

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
#include "test_u_stream_common.h"

// forward declarations
struct ClientServer_ConnectionConfiguration;
struct ClientServer_ConnectionState;
class Test_U_Message;
class Test_U_SessionMessage;

struct ClientServer_ProtocolConfiguration
{
  ClientServer_ProtocolConfiguration ()
   : PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , pingInterval (ACE_Time_Value::zero)
   , pingAutoAnswer (true)
   , printPongMessages (true)
  {}

  unsigned int   PDUSize; // pdu size (if fixed)
  ACE_Time_Value pingInterval; // {0: off}
  bool           pingAutoAnswer;
  bool           printPongMessages;
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct ClientServer_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> ClientServer_ISessionNotify_t;
typedef std::list<ClientServer_ISessionNotify_t*> ClientServer_Subscribers_t;
typedef ClientServer_Subscribers_t::const_iterator ClientServer_SubscribersIterator_t;

//extern const char stream_name_string_[];
struct ClientServer_StreamConfiguration;
struct ClientServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Net_AllocatorConfiguration,
                               struct ClientServer_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct ClientServer_ModuleHandlerConfiguration> ClientServer_StreamConfiguration_t;
struct ClientServer_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  ClientServer_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //, printProgressDot (false)
   , protocolConfiguration (NULL)
   , streamConfiguration (NULL)
   //, sessionData (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  }

  //bool                             printProgressDot; // file writer module
  struct ClientServer_ProtocolConfiguration* protocolConfiguration; // protocol handler
  ClientServer_StreamConfiguration_t*        streamConfiguration;
  //// *TODO*: remove this (--> session message data)
  //struct ClientServer_StreamSessionData*     sessionData;
  ClientServer_ISessionNotify_t*             subscriber;
  ClientServer_Subscribers_t*                subscribers;
};

struct ClientServer_StreamConfiguration
 : Stream_Configuration
{
  ClientServer_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {}

  struct Test_U_UserData* userData; // user data
};

struct ClientServer_Configuration
 : Test_U_Configuration
{
  ClientServer_Configuration ()
   : Test_U_Configuration ()
   , allocatorConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   , protocolConfiguration ()
  {}

  struct Net_AllocatorConfiguration         allocatorConfiguration;
  // **************************** socket data **********************************
  ClientServer_ConnectionConfigurations_t   connectionConfigurations;
  // **************************** stream data **********************************
  ClientServer_StreamConfiguration_t        streamConfiguration;
  // *************************** protocol data *********************************
  struct ClientServer_ProtocolConfiguration protocolConfiguration;
};

#endif
