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

#ifndef TEST_I_CONFIGURATION_H
#define TEST_I_CONFIGURATION_H

#include <string>

#include "ace/config-lite.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_configuration.h"
#include "common_file_common.h"

#include "common_parser_common.h"

#include "stream_common.h"
#include "stream_configuration.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_mediafoundation_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"

#include "test_i_defines.h"

// forward declarations
class Stream_IStreamControlBase;

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

struct Test_I_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , statisticReportingInterval (ACE_Time_Value::zero)
   , statisticReportingTimerId (-1)
   , stream (NULL)
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Time_Value                  statisticReportingInterval; // statistic reporting interval (second(s)) [0: off]
  long                            statisticReportingTimerId;
  Stream_IStreamControlBase*      stream;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_MediaFoundationConfiguration
 : Stream_MediaFramework_MediaFoundation_Configuration
{
  Test_I_MediaFoundationConfiguration ()
   : Stream_MediaFramework_MediaFoundation_Configuration ()
  {}
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_Configuration
{
  Test_I_Configuration ()
   : dispatchConfiguration ()
   , parserConfiguration ()
   , timerConfiguration ()
   , signalHandlerConfiguration ()
   , allocatorConfiguration ()
#if defined (GUI_SUPPORT)
   , GUIFramework (COMMON_UI_FRAMEWORK_INVALID)
#endif // GUI_SUPPORT
   , userData ()
  {}

  // **************************** event data **********************************
  struct Common_EventDispatchConfiguration   dispatchConfiguration;
  // **************************** parser data **********************************
  struct Common_FlexBisonParserConfiguration parserConfiguration;
  // **************************** timer data **********************************
  struct Common_TimerConfiguration           timerConfiguration;
  // **************************** signal data **********************************
  struct Test_I_SignalHandlerConfiguration   signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_AllocatorConfiguration       allocatorConfiguration;

  ////////////////////////////////////////
#if defined (GUI_SUPPORT)
  enum Common_UI_FrameworkType               GUIFramework;
#endif // GUI_SUPPORT

  struct Test_I_UserData                     userData;
};

#endif
