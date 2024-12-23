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

#include "ace/INET_Addr.h"

#include "common_file_common.h"

#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_connection_configuration.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

#include "test_u_stream_common.h"

#include "file_server_defines.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;
class FileServer_TCPConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //FileServer_TCPConnectionConfiguration,
                          struct Net_StreamConnectionState,
                          Net_StreamStatistic_t> FileServer_TCPIConnection_t;

struct FileServer_SessionData
 : Test_U_StreamSessionData
{
  FileServer_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL)
  {}

  FileServer_TCPIConnection_t* connection;
};
typedef Stream_SessionData_T<struct FileServer_SessionData> FileServer_SessionData_t;

struct FileServer_StreamState
 : Test_U_StreamState
{
  FileServer_StreamState ()
   : Test_U_StreamState ()
   , sessionData (NULL)
  {}

  struct FileServer_SessionData* sessionData;
};

typedef Stream_ISessionDataNotify_T<struct FileServer_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> FileServer_ISessionNotify_t;
typedef std::list<FileServer_ISessionNotify_t*> FileServer_Subscribers_t;
typedef FileServer_Subscribers_t::const_iterator FileServer_SubscribersIterator_t;

//extern const char stream_name_string_[];
struct FileServer_StreamConfiguration;
struct FileServer_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct FileServer_StreamConfiguration,
                               struct FileServer_ModuleHandlerConfiguration> FileServer_StreamConfiguration_t;
struct FileServer_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  FileServer_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , fileIdentifier ()
   , outboundQueue (NULL)
   , program (FILE_SERVER_DEFAULT_MPEG_TS_PROGRAM_NUMBER)
   , audioStreamType (0)
   , videoStreamType (FILE_SERVER_DEFAULT_MPEG_TS_STREAM_TYPE)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    inbound = true;
  }

  FileServer_TCPIConnection_t*      connection;               // net target module
  Net_ConnectionConfigurations_t*   connectionConfigurations; // net target module
  Common_File_Identifier            fileIdentifier;           // file reader module
  Stream_IMessageQueue*             outboundQueue;            // event handler module
  unsigned int                      program;                  // MPEG TS decoder module
  unsigned int                      audioStreamType;          // MPEG TS decoder module
  unsigned int                      videoStreamType;          // MPEG TS decoder module
  FileServer_StreamConfiguration_t* streamConfiguration;      // net target module
  FileServer_ISessionNotify_t*      subscriber;               // event handler module
  FileServer_Subscribers_t*         subscribers;              // event handler module
};

struct FileServer_StreamConfiguration
 : Stream_Configuration
{
  FileServer_StreamConfiguration ()
   : Stream_Configuration ()
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   //, userData (NULL)
  {}

  enum Common_EventDispatchType dispatch;

  //struct Net_UserData*   userData;   // user data
};

//////////////////////////////////////////

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

#endif
