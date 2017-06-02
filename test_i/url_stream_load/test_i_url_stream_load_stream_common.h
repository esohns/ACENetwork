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

#ifndef TEST_I_URLSTREAMLOAD_STREAM_COMMON_H
#define TEST_I_URLSTREAMLOAD_STREAM_COMMON_H

#include <list>
#include <map>
#include <set>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "gtk/gtk.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_network.h"

#include "test_i_connection_common.h"
#include "test_i_defines.h"
#include "test_i_stream_common.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Message;
class Test_I_SessionMessage;
struct Test_I_URLStreamLoad_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_URLStreamLoad_ConnectionConfiguration,
                          struct HTTP_ConnectionState,
                          HTTP_RuntimeStatistic_t> Test_I_IConnection_t;

struct Test_I_URLStreamLoad_SessionData
 : Test_I_StreamSessionData
{
  inline Test_I_URLStreamLoad_SessionData ()
   : Test_I_StreamSessionData ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
   , userData (NULL)
  {};
  inline struct Test_I_URLStreamLoad_SessionData& operator= (struct Test_I_URLStreamLoad_SessionData& rhs_in)
  {
    Test_I_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                             address;
  Test_I_IConnection_t*                     connection;
  enum Stream_Decoder_CompressionFormatType format; // HTTP parser module
  std::string                               targetFileName; // file writer module

  struct HTTP_Stream_UserData*              userData;
};
typedef Stream_SessionData_T<struct Test_I_URLStreamLoad_SessionData> Test_I_URLStreamLoad_SessionData_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_URLStreamLoad_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::const_iterator Test_I_SubscribersIterator_t;

struct Test_I_URLStreamLoad_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  inline Test_I_URLStreamLoad_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , connectionConfiguration (NULL)
   , contextID (0)
   , inbound (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  };

  struct Net_ConnectionConfiguration*     connectionConfiguration;
  guint                                   contextID;
  bool                                    inbound; // net IO module
  struct Net_SocketConfiguration*         socketConfiguration;
  struct HTTP_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_ISessionNotify_t*                subscriber;
  Test_I_Subscribers_t*                   subscribers;
  std::string                             targetFileName; // dump module
};

typedef std::map<std::string,
                 struct Test_I_URLStreamLoad_ModuleHandlerConfiguration*> Test_I_URLStreamLoad_ModuleHandlerConfigurations_t;
typedef Test_I_URLStreamLoad_ModuleHandlerConfigurations_t::iterator Test_I_URLStreamLoad_ModuleHandlerConfigurationsIterator_t;
struct Test_I_URLStreamLoad_StreamConfiguration
 : HTTP_StreamConfiguration
{
  inline Test_I_URLStreamLoad_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
   , moduleHandlerConfigurations ()
   , userData (NULL)
  {};

  Test_I_URLStreamLoad_ModuleHandlerConfigurations_t moduleHandlerConfigurations; // stream module handler configuration

  struct HTTP_Stream_UserData*                       userData;
};

struct Test_I_URLStreamLoad_StreamState
 : Test_I_StreamState
{
  inline Test_I_URLStreamLoad_StreamState ()
   : Test_I_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_URLStreamLoad_SessionData* currentSessionData;

  struct HTTP_Stream_UserData*             userData;
};

//typedef Stream_IModuleHandler_T<Test_I_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_I_IStreamNotify_t;

#endif
