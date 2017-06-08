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

#ifndef TEST_I_URLSTREAMLOAD_COMMON_H
#define TEST_I_URLSTREAMLOAD_COMMON_H

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "test_i_common.h"
#include "test_i_gtk_common.h"

#include "test_i_url_stream_load_stream_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;

typedef HTTP_RuntimeStatistic_t Test_I_URLStreamLoad_RuntimeStatistic_t;
typedef Common_IStatistic_T<Test_I_URLStreamLoad_RuntimeStatistic_t> Test_I_URLStreamLoad_StatisticReportingHandler_t;

struct Test_I_URLStreamLoad_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  inline Test_I_URLStreamLoad_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
  {};

  Test_I_URLStreamLoad_StatisticReportingHandler_t* statisticReportingHandler;
};

struct Test_I_URLStreamLoad_Configuration
 : Test_I_Configuration
{
  inline Test_I_URLStreamLoad_Configuration ()
   : Test_I_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   //, protocolConfiguration ()
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Test_I_URLStreamLoad_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_URLStreamLoad_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  struct Test_I_URLStreamLoad_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  //struct HTTP_ProtocolConfiguration        protocolConfiguration;

  struct HTTP_Stream_UserData                            userData;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_I_AllocatorConfiguration> Test_I_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_SessionMessage> Test_I_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

//////////////////////////////////////////

struct Test_I_URLStreamLoad_GTK_ProgressData
 : Test_I_GTK_ProgressData
{
  inline Test_I_URLStreamLoad_GTK_ProgressData ()
   : Test_I_GTK_ProgressData ()
//   , statistic ()
   , transferred (0)
  {};

//  HTTP_RuntimeStatistic_t statistic;
  unsigned int transferred; // byte(s)
};

struct Test_I_URLStreamLoad_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_URLStreamLoad_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , handle (ACE_INVALID_HANDLE)
   , progressData ()
   , subscribers ()
  {};

  struct Test_I_URLStreamLoad_Configuration*   configuration;

  ACE_HANDLE                                   handle;
  struct Test_I_URLStreamLoad_GTK_ProgressData progressData;
  Test_I_Subscribers_t                         subscribers;
};

//struct Test_I_URLStreamLoad_ThreadData
//{
//  inline Test_I_URLStreamLoad_ThreadData ()
//   : CBData (NULL)
//   , eventSourceID (0)
//  {};

//  struct Test_I_URLStreamLoad_GTK_CBData* CBData;
//  guint                                   eventSourceID;
//};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_URLStreamLoad_GTK_CBData> Test_I_URLStreamLoad_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_URLStreamLoad_GTK_CBData> Test_I_URLStreamLoad_GTK_Manager_t;
typedef ACE_Singleton<Test_I_URLStreamLoad_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_URLSTREAMLOAD_UI_GTK_MANAGER_SINGLETON;

#endif
