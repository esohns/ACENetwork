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

#ifndef TEST_I_ICECAST_CLIENT_COMMON_H
#define TEST_I_ICECAST_CLIENT_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "endpointvolume.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
#ifdef __cplusplus
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif /* __cplusplus */
#endif // ACE_WIN32 || ACE_WIN64

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_SUPPORT

#include "http_common.h"

#include "test_i_common.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT

#include "test_i_common_modules.h"
#include "test_i_icecast_client_stream_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;
class Test_I_SessionMessage_2;

//typedef HTTP_Statistic_t Test_I_IceCastClient_Statistic_t;
//typedef Common_IStatistic_T<Test_I_IceCastClient_Statistic_t> Test_I_IceCastClient_StatisticReportingHandler_t;

struct Test_I_IceCastClient_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_IceCastClient_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
  {}

  HTTP_StatisticReportingHandler_t* statisticReportingHandler;
};

struct Test_I_IceCastClient_ServerConfiguration
{
  std::string name;
  std::string URL; // web-server

  void clear ()
  {
    name.clear ();
    URL.clear ();
  }
};
typedef std::map<unsigned int, struct Test_I_IceCastClient_ServerConfiguration> Test_I_IceCastClient_ServerConfigurations_t;
typedef Test_I_IceCastClient_ServerConfigurations_t::iterator Test_I_IceCastClient_ServerConfigurationsIterator_t;
typedef Test_I_IceCastClient_ServerConfigurations_t::const_iterator Test_I_IceCastClient_ServerConfigurationsConstIterator_t;

struct Test_I_IceCastClient_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_IceCastClient_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , parserConfiguration ()
   , parserConfiguration_3 ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {}

  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                        parserConfiguration;
  struct HTTP_ParserConfiguration                        parserConfiguration_3;
  // **************************** signal data **********************************
  struct Test_I_IceCastClient_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                         connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_IceCastClient_StreamConfiguration_t             streamConfiguration;   // m3u parsing
  Test_I_IceCastClient_StreamConfiguration_2_t           streamConfiguration_2; // media stream
  Test_I_IceCastClient_StreamConfiguration_3_t           streamConfiguration_3; // html scraping
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_SessionMessage> Test_I_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_SessionMessage_2> Test_I_MessageAllocator_2_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message_3,
                                          Test_I_SessionMessage> Test_I_MessageAllocator_3_t;

//typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

//////////////////////////////////////////

struct Test_I_IceCastClient_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_IceCastClient_UI_ProgressData ()
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

struct Test_I_IceCastClient_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_IceCastClient_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE || WXWIDGETS_USE
   , servers (NULL)
   , handle (ACE_INVALID_HANDLE)
   , progressData ()
#if defined (PROJECTM_SUPPORT)
   , projectMConfiguration (NULL)
#endif // PROJECTM_SUPPORT
#if defined (GTK_SUPPORT)
   , eventSourceId (0)
   , fft (NULL)
   , spectrumAnalyzerCBData ()
#endif // GTK_SUPPORT
   , subscribers ()
   , URL ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , volumeControl (NULL)
#else
   , mixerHandle (NULL)
   , volumeControl (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , nextChannelSet (false)
  {}

  struct Test_I_IceCastClient_Configuration*          configuration;
  Test_I_IceCastClient_ServerConfigurations_t*        servers;

  ACE_HANDLE                                          handle;
  struct Test_I_IceCastClient_UI_ProgressData         progressData;
#if defined (PROJECTM_SUPPORT)
  struct Stream_Visualization_ProjectM_Configuration* projectMConfiguration;
#endif // PROJECTM_SUPPORT
#if defined (GTK_SUPPORT)
  guint                                               eventSourceId; // display-
  Common_Math_FFT_t*                                  fft;
  struct acestream_visualization_gtk_cairo_cbdata     spectrumAnalyzerCBData;
#endif // GTK_SUPPORT
  Test_I_Subscribers_t                                subscribers;
  std::string                                         URL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IAudioEndpointVolume*                               volumeControl;
#else
  snd_mixer_t*                                        mixerHandle;
  snd_mixer_elem_t*                                   volumeControl;
#endif // ACE_WIN32 || ACE_WIN64
  bool                                                nextChannelSet;
};

//struct Test_I_IceCastClient_ThreadData
//{
//  Test_I_IceCastClient_ThreadData ()
//   : CBData (NULL)
//   , eventSourceID (0)
//  {}

//  struct Test_I_IceCastClient_UI_CBData* CBData;
//  guint                                  eventSourceID;
//};

#endif
