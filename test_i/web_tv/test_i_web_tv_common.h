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

#ifndef TEST_I_WEBTV_COMMON_H
#define TEST_I_WEBTV_COMMON_H

#include <list>

#include "ace/Date_Time.h"

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "http_common.h"

#include "test_i_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "test_i_timeouthandler.h"

#include "test_i_web_tv_stream_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;
class Test_I_SessionMessage_2;
class Test_I_Stream;

typedef std::list<std::string> Test_I_WebTV_ChannelSegmentURLs_t;
typedef Test_I_WebTV_ChannelSegmentURLs_t::iterator Test_I_WebTV_ChannelSegmentURLsIterator_t;
typedef Test_I_WebTV_ChannelSegmentURLs_t::const_iterator Test_I_WebTV_ChannelSegmentURLsConstIterator_t;
struct Test_I_WebTV_ChannelSegment
{
  ACE_Time_Value                    start;
  ACE_Time_Value                    end;
  unsigned int                      length; // s
  Test_I_WebTV_ChannelSegmentURLs_t URLs;
};

struct Test_I_WebTV_ChannelResolution
{
  unsigned int              frameRate;
  Common_Image_Resolution_t resolution;
  std::string               URI; // relative to channel baseURI
};
typedef std::vector<struct Test_I_WebTV_ChannelResolution> Test_I_WebTV_ChannelResolutions_t;
typedef Test_I_WebTV_ChannelResolutions_t::iterator Test_I_WebTV_ChannelResolutionsIterator_t;
typedef Test_I_WebTV_ChannelResolutions_t::const_iterator Test_I_WebTV_ChannelResolutionsConstIterator_t;

// *NOTE*: the main program URL 'mainURL' contains information about the
//         available streams
struct Test_I_WebTV_ChannelConfiguration
{
  std::string                        baseURI; // sub-URI (if any) of the individual streams
  unsigned int                       indexPositions; // i.e. for computing leading "0"s
  std::string                        mainURL; // program-
  std::string                        name;
  Test_I_WebTV_ChannelResolutions_t  resolutions;
  struct Test_I_WebTV_ChannelSegment segment;
};
typedef std::map<unsigned int, struct Test_I_WebTV_ChannelConfiguration> Test_I_WebTV_ChannelConfigurations_t;
typedef Test_I_WebTV_ChannelConfigurations_t::iterator Test_I_WebTV_ChannelConfigurationsIterator_t;
typedef Test_I_WebTV_ChannelConfigurations_t::const_iterator Test_I_WebTV_ChannelConfigurationsConstIterator_t;

struct Test_I_WebTV_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_WebTV_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , connectionConfigurations ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
   , streamConfiguration_3 ()
  {}

  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t       connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_WebTV_StreamConfiguration_t   streamConfiguration;
  Test_I_WebTV_StreamConfiguration_2_t streamConfiguration_2;
  Test_I_WebTV_StreamConfiguration_2_t streamConfiguration_3; // input
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

typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_WebTV_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_WebTV_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#else
#endif // GTK_USE || WXWIDGETS_USE
//   , statistic ()
  {}
};

struct Test_I_WebTV_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  Test_I_WebTV_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE || WXWIDGETS_USE
   , channels (NULL)
   , currentChannel (0)
   , currentStream (0)
   , dispatch (NULL)
   , handle (ACE_INVALID_HANDLE)
   , progressData ()
   , stream (NULL)
   , subscribers ()
   , timeoutHandler (NULL)
   , videoUpdateEventSourceId (0)
  {}

  struct Test_I_WebTV_Configuration*    configuration;
  Test_I_WebTV_ChannelConfigurations_t* channels;
  unsigned int                          currentChannel;
  unsigned int                          currentStream;
  Common_IDispatch*                     dispatch;
  ACE_HANDLE                            handle; // connection-
  struct Test_I_WebTV_UI_ProgressData   progressData;
  Test_I_Stream*                        stream; // input-
  Test_I_Subscribers_t                  subscribers;
  Test_I_TimeoutHandler*                timeoutHandler;
  guint                                 videoUpdateEventSourceId;
};
#endif // GUI_SUPPORT

#endif
