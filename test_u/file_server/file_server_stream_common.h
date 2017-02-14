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

#ifndef FILE_SERVER_STREAM_COMMON_H
#define FILE_SERVER_STREAM_COMMON_H

#include <list>

#include <ace/INET_Addr.h>

#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_configuration.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

//#include "test_u_connection_manager_common.h"
#include "test_u_stream_common.h"

//#include "file_server_connection_common.h"
#include "file_server_defines.h"

// forward declarations
struct FileServer_ConnectionState;
class Test_U_Message;
class Test_U_SessionMessage;
struct FileServer_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct FileServer_ConnectionConfiguration,
                          struct FileServer_ConnectionState,
                          Net_RuntimeStatistic_t> FileServer_IConnection_t;

struct FileServer_SessionData
 : Test_U_StreamSessionData
{
  inline FileServer_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL)
   , userData (NULL)
  {};

  FileServer_IConnection_t*   connection;

  struct FileServer_UserData* userData;
};
typedef Stream_SessionData_T<struct FileServer_SessionData> FileServer_SessionData_t;

struct FileServer_StreamState
 : Test_U_StreamState
{
  inline FileServer_StreamState ()
   : Test_U_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct FileServer_SessionData* currentSessionData;

  struct FileServer_UserData*    userData;
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct FileServer_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> FileServer_ISessionNotify_t;
typedef std::list<FileServer_ISessionNotify_t*> FileServer_Subscribers_t;
typedef FileServer_Subscribers_t::const_iterator FileServer_SubscribersIterator_t;

class Test_U_UDPStream;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct FileServer_ConnectionConfiguration,
                                struct FileServer_ConnectionState,
                                Net_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct FileServer_SocketHandlerConfiguration,
                                Test_U_UDPStream,
                                enum Stream_StateMachine_ControlState> FileServer_UDPIStreamConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct FileServer_UserData> FileServer_IInetConnectionManager_t;
struct Test_U_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Test_U_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionManager (NULL)
   , fileName ()
   , inbound (true)
   , outboundQueue (NULL)
   , program (FILE_SERVER_DEFAULT_MPEG_TS_PROGRAM_NUMBER)
   , pushStatisticMessages (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
   , streamType (FILE_SERVER_DEFAULT_MPEG_TS_STREAM_TYPE)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

  FileServer_UDPIStreamConnection_t*            connection;                 // net target module
  FileServer_IInetConnectionManager_t*          connectionManager;          // net target module
  std::string                                   fileName;                   // file reader module
  bool                                          inbound;                    // statistic/IO module
  Stream_IMessageQueue*                         outboundQueue;              // event handler module
  unsigned int                                  program;                    // MPEG TS decoder module
  bool                                          pushStatisticMessages;      // statistic/file source module
  struct Net_SocketConfiguration*               socketConfiguration;        // net target module
  struct FileServer_SocketHandlerConfiguration* socketHandlerConfiguration; // net target module
  Test_U_UDPStream*                             stream;                     // net target module
  unsigned int                                  streamType;                 // MPEG TS decoder module
  FileServer_ISessionNotify_t*                  subscriber;                 // event handler module
  FileServer_Subscribers_t*                     subscribers;                // event handler module
};

struct FileServer_StreamConfiguration
 : Stream_Configuration
{
  inline FileServer_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration (NULL)
   , moduleHandlerConfiguration_2 ()
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData (NULL)
  {};

  struct Stream_ModuleConfiguration         moduleConfiguration_2;        // module configuration
  struct Test_U_ModuleHandlerConfiguration* moduleHandlerConfiguration;   // module handler configuration
  struct Test_U_ModuleHandlerConfiguration  moduleHandlerConfiguration_2; // module handler configuration
  bool                                      useReactor;

  struct FileServer_UserData*               userData;                     // user data
};

#endif
