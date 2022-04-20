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

#ifndef TEST_I_WEBTV_STREAM_COMMON_H
#define TEST_I_WEBTV_STREAM_COMMON_H

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

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_parser_m3u_iparser.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

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

// forward declarations
class Stream_IAllocator;
class Test_I_Message;
class Test_I_SessionMessage;
class Test_I_SessionMessage_2;
//struct Test_I_WebTV_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //Test_I_WebTV_ConnectionConfiguration_t,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;

struct HTTP_Record;
struct Test_I_WebTV_MessageData
 : HTTP_Record
{
  Test_I_WebTV_MessageData ()
   : HTTP_Record ()
   , M3UPlaylist (NULL)
  {};
  ~Test_I_WebTV_MessageData ()
  {
    //if (M3UPlaylist)
    //  delete M3UPlaylist;
  };
  inline void operator+= (Test_I_WebTV_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  struct M3U_Playlist* M3UPlaylist;
};

struct Test_I_WebTV_SessionData
 : Test_I_StreamSessionData
{
  Test_I_WebTV_SessionData ()
   : Test_I_StreamSessionData ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
  {}

  struct Test_I_WebTV_SessionData& operator= (const struct Test_I_WebTV_SessionData& rhs_in)
  {
    Test_I_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                                address;
  Test_I_IConnection_t*                        connection;
  enum Stream_Decoder_CompressionFormatType    format; // HTTP parser module
  std::string                                  targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_WebTV_SessionData> Test_I_WebTV_SessionData_t;

typedef Stream_ISessionDataNotify_T<struct Test_I_WebTV_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::const_iterator Test_I_SubscribersIterator_t;

struct Test_I_WebTV_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_WebTV_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , window (NULL)
#endif // GTK_USE
#endif // GUI_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
    inbound = true;
  }

  Net_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_ISessionNotify_t*        subscriber;
  Test_I_Subscribers_t*           subscribers;
  std::string                     targetFileName; // dump module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                      window;
#endif // GTK_USE
#endif // GUI_SUPPORT
};

struct Test_I_WebTV_StreamConfiguration
 : HTTP_StreamConfiguration
{
  Test_I_WebTV_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
  {}
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_WebTV_StreamConfiguration,
                               struct Test_I_WebTV_ModuleHandlerConfiguration> Test_I_WebTV_StreamConfiguration_t;

struct Test_I_WebTV_StreamState
 : Test_I_StreamState
{
  Test_I_WebTV_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_I_WebTV_SessionData* sessionData;
};

//////////////////////////////////////////

struct Test_I_WebTV_StreamState_2;
class Test_I_WebTV_SessionData_2
 : public Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (FFMPEG_SUPPORT)
                                        struct Stream_MediaFramework_FFMPEG_MediaType,
#endif // FFMPEG_SUPPORT
                                        struct Test_I_WebTV_StreamState_2,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_WebTV_SessionData_2 ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (FFMPEG_SUPPORT)
                                   struct Stream_MediaFramework_FFMPEG_MediaType,
#endif // FFMPEG_SUPPORT
                                   struct Test_I_WebTV_StreamState_2,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
  {}

  Test_I_WebTV_SessionData_2& operator= (const Test_I_WebTV_SessionData_2& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
#if defined (FFMPEG_SUPPORT)
                                  struct Stream_MediaFramework_FFMPEG_MediaType,
#endif // FFMPEG_SUPPORT
                                  struct Test_I_WebTV_StreamState_2,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                             address;
  Test_I_IConnection_t*                     connection;
  enum Stream_Decoder_CompressionFormatType format; // HTTP parser module
  std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<Test_I_WebTV_SessionData_2> Test_I_WebTV_SessionData_2_t;

typedef Stream_ISessionDataNotify_T<Test_I_WebTV_SessionData_2,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2> Test_I_ISessionNotify_2_t;
typedef std::list<Test_I_ISessionNotify_2_t*> Test_I_Subscribers_2_t;
typedef Test_I_Subscribers_2_t::const_iterator Test_I_SubscribersIterator_2_t;

struct Test_I_WebTV_ModuleHandlerConfiguration_2
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_WebTV_ModuleHandlerConfiguration_2 ()
   : HTTP_ModuleHandlerConfiguration ()
   , ALSAConfiguration (NULL)
#if defined (FFMPEG_SUPPORT)
   , codecId (AV_CODEC_ID_NONE)
#endif // FFMPEG_SUPPORT
   , connectionConfigurations (NULL)
   , delayConfiguration (NULL)
   , deviceIdentifier ()
   , display ()
   , program (1)
   , audioStreamType (15) // AAC
   , videoStreamType (27) // H264
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
#if defined (FFMPEG_SUPPORT)
   , outputFormat ()
#endif // FFMPEG_SUPPORT
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , window (NULL)
#endif // GTK_USE
#endif // GUI_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
    inbound = true;
  }

  struct Stream_MediaFramework_ALSA_Configuration* ALSAConfiguration;
#if defined (FFMPEG_SUPPORT)
  enum AVCodecID                                   codecId;
#endif // FFMPEG_SUPPORT
  Net_ConnectionConfigurations_t*                  connectionConfigurations;
  struct Stream_Miscellaneous_DelayConfiguration*  delayConfiguration;
  struct Stream_Device_Identifier                  deviceIdentifier;
  struct Common_UI_DisplayDevice                   display;
  unsigned int                                     program;                  // MPEG TS decoder module
  unsigned int                                     audioStreamType;          // MPEG TS decoder module
  unsigned int                                     videoStreamType;          // MPEG TS decoder module
  Test_I_ISessionNotify_2_t*                       subscriber;
  Test_I_Subscribers_2_t*                          subscribers;
  std::string                                      targetFileName; // dump module
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType    outputFormat;
#endif // FFMPEG_SUPPORT
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                                       window;
#endif // GTK_USE
#endif // GUI_SUPPORT
};

struct Test_I_WebTV_StreamConfiguration_2
 : HTTP_StreamConfiguration
{
  Test_I_WebTV_StreamConfiguration_2 ()
   : HTTP_StreamConfiguration ()
   , mediaType ()
  {}

  struct Stream_MediaFramework_FFMPEG_MediaType mediaType;
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_WebTV_StreamConfiguration_2,
                               struct Test_I_WebTV_ModuleHandlerConfiguration_2> Test_I_WebTV_StreamConfiguration_2_t;

struct Test_I_WebTV_StreamState_2
 : Test_I_StreamState
{
  Test_I_WebTV_StreamState_2 ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  Test_I_WebTV_SessionData_2* sessionData;
};

#endif
