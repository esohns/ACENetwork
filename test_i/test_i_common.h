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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include "ace/OS.h"

#include "common.h"
#include "common_statistic_handler.h"
#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_inotify.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_defines.h"

struct Net_Configuration;
struct Test_I_UserData
 : Net_UserData
{
  Test_I_UserData ()
   : Net_UserData ()
   , configuration (NULL)
  {}

  struct Net_Configuration* configuration;
};

//struct Test_I_ConnectionState
// : Net_ConnectionState
//{
//  Test_I_ConnectionState ()
//   : Net_ConnectionState ()
//   //, configuration (NULL)
//  {}

//  // *TODO*: remove ASAP
//  //struct Net_Configuration* configuration;
//};

//struct Test_I_AllocatorConfiguration
// : Stream_AllocatorConfiguration
//{
//  Test_I_AllocatorConfiguration ()
//   : Stream_AllocatorConfiguration ()
//  {
//    // *NOTE*: this facilitates (message block) data buffers to be scanned with
//    //         'flex's yy_scan_buffer() method
//    paddingBytes = NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
//  }
//};

typedef Stream_Statistic Test_I_Statistic_t;
typedef Common_IStatistic_T<Test_I_Statistic_t> Test_I_StatisticReportingHandler_t;

struct Test_I_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Test_I_StatisticReportingHandler_t* statisticReportingHandler;
  long                                statisticReportingTimerId;
};

struct Test_I_Configuration
{
  Test_I_Configuration ()
   : dispatchConfiguration ()
//   , connectionConfigurations ()
   , parserConfiguration ()
   //, streamConfiguration ()
   , signalHandlerConfiguration ()
   , userData ()
  {}

  struct Common_EventDispatchConfiguration dispatchConfiguration;
  // **************************** socket data **********************************
//  Net_ConnectionConfigurations_t           connectionConfigurations;
  // **************************** parser data **********************************
  struct Common_ParserConfiguration        parserConfiguration;
  //// **************************** stream data **********************************
  //struct Test_I_StreamConfiguration        streamConfiguration;
  // **************************** signal data **********************************
  struct Test_I_SignalHandlerConfiguration signalHandlerConfiguration;

  struct Test_I_UserData                   userData;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_UI_ProgressData
{
  Test_I_UI_ProgressData ()
   : state (NULL)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (Test_I_Statistic_t));
  }

  struct Common_UI_State* state;
  Test_I_Statistic_t      statistic;
};

struct Test_I_UI_CBData
{
  Test_I_UI_CBData ()
   : allowUserRuntimeStatistic (true)
   //, configuration (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
   , UIState (NULL)
  {
    progressData.state = UIState;
  }

  bool                            allowUserRuntimeStatistic;
  //struct Test_U_Configuration*    configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_UI_ProgressData   progressData;
  struct Common_UI_State*         UIState;
};

struct Test_I_UI_ThreadData
{
  Test_I_UI_ThreadData ()
   : CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , sessionId (0)
  {}

  struct Test_I_UI_CBData*        CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  size_t                          sessionId;
};
#endif // GUI_SUPPORT

#endif
