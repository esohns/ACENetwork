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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include <algorithm>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data_base.h"

#include "net_defines.h"

#include "http_common.h"
#include "http_configuration.h"
#include "http_defines.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_defines.h"
//#include "test_u_message.h"
//#include "test_u_session_message.h"

struct Test_U_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_U_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = HTTP_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

// forward declarations
class Stream_IAllocator;
class Test_U_Message;
class Test_U_SessionMessage;
//typedef Common_IStatistic_T<HTTP_RuntimeStatistic_t> Test_U_StatisticReportingHandler_t;
struct Test_U_Configuration;
struct Test_U_StreamConfiguration;
struct Test_U_UserData
 : Stream_UserData
{
  inline Test_U_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_U_Configuration*       configuration;
  Test_U_StreamConfiguration* streamConfiguration;
};

struct Test_U_StreamSessionData
 : Stream_SessionData
{
  inline Test_U_StreamSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , targetFileName ()
   , userData (NULL)
  {};
  inline Test_U_StreamSessionData& operator= (Test_U_StreamSessionData& rhs_in)
  {
    Stream_SessionData::operator= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  HTTP_ConnectionState* connectionState;
  std::string           targetFileName; // file writer module
  Test_U_UserData*      userData;
};
typedef Stream_SessionDataBase_T<Test_U_StreamSessionData> Test_U_StreamSessionData_t;

struct Test_U_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {};

  Test_U_UserData* userData;
};

// forward declarations
struct Test_U_Configuration;
struct Test_U_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  inline Test_U_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , hostName ()
   , inbound (true)
   , passive (false)
   , printProgressDot (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , targetFileName ()
   , URL ()
  {};

  Test_U_Configuration*              configuration;
  Test_U_IConnection_t*              connection; // TCP target/IO module
  Test_U_IConnectionManager_t*       connectionManager; // TCP IO module
  std::string                        hostName; // net source module
  bool                               inbound; // net io module
  bool                               passive; // net source module
  bool                               printProgressDot; // file writer module
  Net_SocketConfiguration*           socketConfiguration;
  Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
  std::string                        targetFileName; // file writer module
  std::string                        URL; // HTTP get module
};

struct Test_U_SignalHandlerConfiguration
{
  inline Test_U_SignalHandlerConfiguration ()
   : //messageAllocator (NULL)
   /*,*/ statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistics collecting interval (second(s)) [0: off]
};

struct Test_U_StreamConfiguration
 : HTTP_StreamConfiguration
{
  inline Test_U_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_U_ModuleHandlerConfiguration* moduleHandlerConfiguration; // stream module handler configuration
};

struct Test_U_StreamState
 : Stream_State
{
  inline Test_U_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_U_StreamSessionData* currentSessionData;
  Test_U_UserData*          userData;
};

struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** signal data **********************************
  Test_U_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_SocketConfiguration           socketConfiguration;
  Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_ModuleConfiguration        moduleConfiguration;
  Test_U_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_U_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  Test_U_UserData                   userData;
  bool                              useReactor;
};

typedef Stream_IModuleHandler_T<Test_U_ModuleHandlerConfiguration> Test_U_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,
                                          
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

#endif
