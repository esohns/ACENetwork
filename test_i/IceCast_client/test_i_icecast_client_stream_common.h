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

#ifndef TEST_I_ICECAST_CLIENT_STREAM_COMMON_H
#define TEST_I_ICECAST_CLIENT_STREAM_COMMON_H

#include <list>
#include <map>
#include <set>
#include <string>

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

#include "stream_misc_common.h"

#include "stream_vis_common.h"
#include "stream_vis_iresize.h"

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
#include "test_i_message.h"
#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
//class Test_I_Message;
// class Test_I_SessionMessage;
// class Test_I_SessionMessage_2;
//struct Test_I_IceCastClient_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //Test_I_IceCastClient_ConnectionConfiguration_t,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;

struct Test_I_IceCastClient_MessageData
 : HTTP_Record
{
  Test_I_IceCastClient_MessageData ()
   : HTTP_Record ()
   , M3UPlaylist ()
  {}
  inline void operator= (const struct HTTP_Record& rhs_in) { HTTP_Record::operator= (rhs_in); }
  inline void operator+= (Test_I_IceCastClient_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  struct M3U_Playlist M3UPlaylist;
};

typedef Stream_ISessionDataNotify_T<struct Test_I_IceCastClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::const_iterator Test_I_SubscribersIterator_t;

struct Test_I_IceCastClient_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_IceCastClient_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   , subscriber (NULL)
   , targetFileName ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

  Net_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_ISessionNotify_t*        subscriber;
  std::string                     targetFileName; // dump module
};

struct Test_I_IceCastClient_StreamConfiguration
 : HTTP_StreamConfiguration
{
  Test_I_IceCastClient_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
  {}
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_IceCastClient_StreamConfiguration,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration> Test_I_IceCastClient_StreamConfiguration_t;

struct Test_I_IceCastClient_StreamState
 : Test_I_StreamState
{
  Test_I_IceCastClient_StreamState ()
   : Test_I_StreamState ()
   //, sessionData (NULL)
  {}

  //struct Test_I_IceCastClient_SessionData* sessionData;
};

//////////////////////////////////////////

typedef Stream_ISessionDataNotify_T<struct Test_I_IceCastClient_SessionData_2,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2> Test_I_ISessionNotify_2_t;
typedef std::list<Test_I_ISessionNotify_2_t*> Test_I_Subscribers_2_t;
typedef Test_I_Subscribers_2_t::const_iterator Test_I_SubscribersIterator_2_t;

struct Test_I_IceCastClient_ModuleHandlerConfiguration_2
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_IceCastClient_ModuleHandlerConfiguration_2 ()
   : HTTP_ModuleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , ALSAConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , connectionConfigurations (NULL)
   , delayConfiguration (NULL)
   , deviceIdentifier ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , subscriber (NULL)
   , targetFileName ()
   , outputFormat ()
#if defined (PROJECTM_SUPPORT)
   , projectMConfiguration (NULL)
#endif // PROJECTM_SUPPORT
   , queue (NULL)
   , resize (NULL)
#if defined (GTK_SUPPORT)
   , spectrumAnalyzerConfiguration (NULL)
#endif // GTK_SUPPORT
   , window ()
#if defined (WAYLAND_SUPPORT)
   , surface (NULL)
   , waylandDisplay (NULL)
#endif // WAYLAND_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::memset (&outputFormat, 0, sizeof (struct _AMMediaType));
#endif // ACE_WIN32 || ACE_WIN64
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_MediaFramework_ALSA_Configuration*                      ALSAConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration*               codecConfiguration;
#endif // FFMPEG_SUPPORT
  Net_ConnectionConfigurations_t*                                       connectionConfigurations;
  struct Stream_Miscellaneous_DelayConfiguration*                       delayConfiguration;
  struct Stream_Device_Identifier                                       deviceIdentifier; // render
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration*                  direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  Test_I_ISessionNotify_2_t*                                            subscriber;
  std::string                                                           targetFileName; // dump module
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType                         outputFormat;
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                                                   outputFormat;
#else
  struct Stream_MediaFramework_ALSA_MediaType                           outputFormat;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT
#if defined (PROJECTM_SUPPORT)
  struct Stream_Visualization_ProjectM_Configuration*                   projectMConfiguration;
#endif // PROJECTM_SUPPORT
  ACE_Message_Queue_Base*                                               queue;
  Stream_Visualization_IResize*                                         resize;
#if defined (GTK_SUPPORT)
  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration* spectrumAnalyzerConfiguration;
#endif // GTK_SUPPORT
  struct Common_UI_Window                                               window;
#if defined (WAYLAND_SUPPORT)
  struct wl_shell_surface*                                              surface;
  struct wl_display*                                                    waylandDisplay;
#endif // WAYLAND_SUPPORT
};

struct Test_I_IceCastClient_StreamConfiguration_2
 : HTTP_StreamConfiguration
{
  Test_I_IceCastClient_StreamConfiguration_2 ()
   : HTTP_StreamConfiguration ()
   , displayVideo (true)
   //, format ()
   , URL ()
  {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_OS::memset (&format, 0, sizeof (struct _AMMediaType));
//#endif // ACE_WIN32 || ACE_WIN64
  }

  bool                                        displayVideo;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct _AMMediaType                         format;
//#else
//  struct Stream_MediaFramework_ALSA_MediaType format;
//#endif // ACE_WIN32 || ACE_WIN64
  std::string                                 URL;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_IceCastClient_StreamConfiguration_2,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2> Test_I_IceCastClient_StreamConfiguration_2_t;

struct Test_I_IceCastClient_StreamState_2
 : Test_I_StreamState
{
  Test_I_IceCastClient_StreamState_2 ()
   : Test_I_StreamState ()
   //, sessionData (NULL)
  {}

  //struct Test_I_IceCastClient_SessionData_2* sessionData;
};

typedef Stream_MessageQueue_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Test_I_SessionMessage_2> Test_I_IceCastClient_MessageQueue_2_t;

//////////////////////////////////////////

typedef std::vector<std::string> Test_I_IceCastClient_ScrapedURLs_t;
typedef Test_I_IceCastClient_ScrapedURLs_t::iterator Test_I_IceCastClient_ScrapedURLsIterator_t;

struct Test_I_IceCastClient_MessageData_3
 : HTTP_Record
{
  Test_I_IceCastClient_MessageData_3 ()
   : HTTP_Record ()
   , document (NULL)
   , xPathObject (NULL)
  {}
  ~Test_I_IceCastClient_MessageData_3 ()
  {
    if (xPathObject)
      xmlXPathFreeObject (xPathObject);
    if (document)
      xmlFreeDoc (document);
  }

  inline void operator= (const struct HTTP_Record& rhs_in) { HTTP_Record::operator= (rhs_in); }
  inline void operator+= (struct Test_I_IceCastClient_MessageData_3 rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  htmlDocPtr        document;
  xmlXPathObjectPtr xPathObject;
};

typedef Stream_ISessionDataNotify_T<struct Test_I_IceCastClient_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message_3,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_3_t;
typedef std::list<Test_I_ISessionNotify_3_t*> Test_I_Subscribers_3_t;
typedef Test_I_Subscribers_3_t::const_iterator Test_I_SubscribersIterator_3_t;

struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , URL ()
  {}

  std::string URL;
};

struct Test_I_IceCastClient_ModuleHandlerConfiguration_3
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_IceCastClient_ModuleHandlerConfiguration_3 ()
   : HTTP_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   , mode (STREAM_MODULE_HTMLPARSER_MODE_DOM)
   , subscriber (NULL)
   , xPathNameSpaces ()
   , xPathQueryString ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

  Net_ConnectionConfigurations_t*    connectionConfigurations;
  enum Stream_Module_HTMLParser_Mode mode;
  Test_I_ISessionNotify_3_t*         subscriber;
  Stream_HTML_XPathNameSpaces_t      xPathNameSpaces;
  std::string                        xPathQueryString;
};

struct Test_I_IceCastClient_StreamConfiguration_3
 : HTTP_StreamConfiguration
{
  Test_I_IceCastClient_StreamConfiguration_3 ()
   : HTTP_StreamConfiguration ()
  {}
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_IceCastClient_StreamConfiguration_3,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_3> Test_I_IceCastClient_StreamConfiguration_3_t;

struct Test_I_IceCastClient_StreamState_3
 : Test_I_StreamState
{
  Test_I_IceCastClient_StreamState_3 ()
   : Test_I_StreamState ()
   //, sessionData (NULL)
  {}

  //struct Test_I_IceCastClient_SessionData* sessionData;
};

#endif
