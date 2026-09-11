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

#if defined (LIBXML2_SUPPORT)
#include "libxml/HTMLParser.h"
#endif // LIBXML2_SUPPORT

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
//#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_window.h"

#include "common_parser_m3u_iparser.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "stream_dev_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//#include "stream_module_htmlparser.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_network.h"
#include "http_stream_common.h"

#include "test_i_defines.h"
#include "test_i_stream_common.h"

#include "test_i_connection_common.h"
#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Message;
// class Test_I_SessionMessage;
// class Test_I_SessionMessage_2;
//struct Test_I_URLStreamLoad_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //Test_I_URLStreamLoad_ConnectionConfiguration_t,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;

struct HTTP_Record;
struct Test_I_URLStreamLoad_MessageData
 : HTTP_Record
{
  Test_I_URLStreamLoad_MessageData ()
   : HTTP_Record ()
#if defined (LIBXML2_SUPPORT)
   //, document (NULL)
#endif // LIBXML2_SUPPORT
 //, M3UPlaylist ()
  {}
  ~Test_I_URLStreamLoad_MessageData ()
  {}
  inline void operator= (const struct HTTP_Record& rhs_in) { HTTP_Record::operator= (rhs_in); }
  inline void operator+= (Test_I_URLStreamLoad_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

#if defined (LIBXML2_SUPPORT)
  //htmlDocPtr document;
#endif // LIBXML2_SUPPORT
  //struct M3U_Playlist M3UPlaylist;
};

typedef Stream_ISessionDataNotify_T<struct Test_I_URLStreamLoad_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_t;

struct Test_I_URLStreamLoad_UI_CBData;
struct Test_I_URLStreamLoad_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_URLStreamLoad_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , ALSAConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , CBData (NULL)
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , connectionConfigurations (NULL)
   , delayConfiguration (NULL)
   , deviceIdentifier ()
   , inputFormat ()
#if defined (FFMPEG_SUPPORT)
   , outputFormat ()
#endif // FFMPEG_SUPPORT
   , queue (NULL)
   , subscriber (NULL)
   , targetFileName ()
   , window ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_MediaFramework_ALSA_Configuration*        ALSAConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_URLStreamLoad_UI_CBData*                  CBData; // HTTP Get module
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  Net_ConnectionConfigurations_t*                         connectionConfigurations;
  struct Stream_Miscellaneous_DelayConfiguration*         delayConfiguration;
  struct Stream_Device_Identifier                         deviceIdentifier;
  std::string                                             inputFormat; // demuxer module
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType           outputFormat;
#endif // FFMPEG_SUPPORT
  ACE_Message_Queue_Base*                                 queue;
  Test_I_ISessionNotify_t*                                subscriber;
  std::string                                             targetFileName; // dump module
  Common_UI_Window                                        window;
};

struct Test_I_URLStreamLoad_StreamConfiguration
 : HTTP_StreamConfiguration
{
  Test_I_URLStreamLoad_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , mediaType ()
#endif // FFMPEG_SUPPORT
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
   , useHardwareDecoder (false)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    renderer = STREAM_DEVICE_RENDERER_WASAPI;
#else
    renderer = STREAM_DEVICE_RENDERER_ALSA;
#endif // ACE_WIN32 || ACE_WIN64
  }

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType mediaType; // input-
#endif // FFMPEG_SUPPORT
  enum Stream_Device_Renderer                   renderer;
  bool                                          useHardwareDecoder;
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_URLStreamLoad_StreamConfiguration,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration> Test_I_URLStreamLoad_StreamConfiguration_t;

struct Test_I_URLStreamLoad_StreamState
 : Test_I_StreamState
{
  Test_I_URLStreamLoad_StreamState ()
   : Test_I_StreamState ()
  {}
};

typedef Stream_MessageQueue_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Test_I_SessionMessage> Test_I_URLStreamLoad_MessageQueue_t;

#endif
