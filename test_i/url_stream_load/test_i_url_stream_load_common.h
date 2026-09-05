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

#if defined (RAPIDJSON_SUPPORT)
#include "document.h"
#endif // RAPIDJSON_SUPPORT

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "http_common.h"

#include "test_i_common.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT

#include "test_i_url_stream_load_stream_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;
class Test_I_AVStream;

struct Test_I_URLStreamLoad_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_URLStreamLoad_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
  {}

  HTTP_StatisticReportingHandler_t* statisticReportingHandler;
};

struct Test_I_URLStreamLoad_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_URLStreamLoad_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , parserConfiguration ()
   , parserConfiguration_1b ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   , streamConfiguration_1b ()
   , streamConfiguration_2 ()
  {
    parserConfiguration.notifyProgress = true;
    parserConfiguration_1b.notifyProgress = true;
  }

  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                        parserConfiguration;    // audio connection-
  struct HTTP_ParserConfiguration                        parserConfiguration_1b; // video connection-
  // **************************** signal data **********************************
  struct Test_I_URLStreamLoad_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                         connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_URLStreamLoad_StreamConfiguration_t             streamConfiguration; // audio connection-
  Test_I_URLStreamLoad_StreamConfiguration_t             streamConfiguration_1b; // video connection-
  Test_I_URLStreamLoad_StreamConfiguration_t             streamConfiguration_2; // A/V
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_SessionMessage> Test_I_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

//////////////////////////////////////////

struct Test_I_URLStreamLoad_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_URLStreamLoad_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
   , transferred (0)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
   , transferred (0)
#else
   : transferred (0)
#endif // GTK_USE || WXWIDGETS_USE
//   , statistic ()
  {}

//  HTTP_Statistic_t statistic;
  unsigned int transferred; // byte(s)
};

struct Test_I_URLStreamLoad_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_URLStreamLoad_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , AVStream (NULL)
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE || WXWIDGETS_USE
   , audioHandle (ACE_INVALID_HANDLE)
   , videoHandle (ACE_INVALID_HANDLE)
   , progressData ()
#if defined (RAPIDJSON_SUPPORT)
   , formats ()
#endif // RAPIDJSON_SUPPORT
  {}

  Test_I_AVStream*                            AVStream;
  struct Test_I_URLStreamLoad_Configuration*  configuration;
  ACE_HANDLE                                  audioHandle;
  ACE_HANDLE                                  videoHandle;
  struct Test_I_URLStreamLoad_UI_ProgressData progressData;

#if defined (RAPIDJSON_SUPPORT)
  rapidjson::Document                         formats;
#endif // RAPIDJSON_SUPPORT
};

//struct Test_I_URLStreamLoad_ThreadData
//{
//  Test_I_URLStreamLoad_ThreadData ()
//   : CBData (NULL)
//   , eventSourceID (0)
//  {}

//  struct Test_I_URLStreamLoad_UI_CBData* CBData;
//  guint                                  eventSourceID;
//};

#endif
