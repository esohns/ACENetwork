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

#ifndef TEST_I_AVSTREAM_COMMON_H
#define TEST_I_AVSTREAM_COMMON_H

#include <limits>
#include <map>
#include <set>
#include <string>

#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "d3d9.h"
#undef GetObject
#include "mfidl.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"

#include "common_parser_common.h"

#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_common.h"
#include "stream_lib_directshow_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_net_common.h"

#include "net_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"
#include "test_i_stream_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "test_i_av_stream_defines.h"

// forward declarations
class Stream_IAllocator;

//typedef int Test_I_HeaderType_t;
//typedef int Test_I_CommandType_t;

enum acestream_av_stream_message_type
{
  AVSTREAM_MESSAGE_AUDIO = 0,
  AVSTREAM_MESSAGE_VIDEO,
  ////////////////////////////////////////
  AVSTREAM_MESSAGE_INVALID,
  AVSTREAM_MESSAGE_MAX
};

#if defined (_MSC_VER)
#pragma pack (push, 1)
#endif // _MSC_VER
struct acestream_av_stream_header
{
  ACE_UINT8  type;
  // *TODO*: support lengths of more than std::numeric_limits<ACE_UINT32>::max() bytes
  //         --> change the length to ACE_UINT64, update scanner.l and regenerate the
  //             scanner
  ACE_UINT32 length;
#if defined (__GNUC__)
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
#if defined (_MSC_VER)
#pragma pack (pop)
#endif // _MSC_VER

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_DirectShow_StreamSessionData
 : Test_I_DirectShow_StreamSessionData
{
  Test_I_AVStream_DirectShow_StreamSessionData ()
   : Test_I_DirectShow_StreamSessionData ()
   , direct3DDevice (NULL)
   , resetToken (0)
   , stream (NULL)
  {}

  // *NOTE*: called on stream link after connecting; 'this' is upstream
  struct Test_I_AVStream_DirectShow_StreamSessionData& operator+= (const struct Test_I_AVStream_DirectShow_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_DirectShow_StreamSessionData::operator+= (rhs_in);

    // sanity check(s)
//    ACE_ASSERT (rhs_in.inputFormat);

//    HRESULT result = S_OK; // *NOTE*: result is modified only when errors occur
//    CMediaType media_type (*(rhs_in.inputFormat), &result);
//    ACE_ASSERT (SUCCEEDED (result));
//    if (media_type.IsPartiallySpecified ())
//      goto continue_; // nothing to do

//    if (inputFormat)
//      Stream_MediaFramework_DirectShow_Tools::delete_ (inputFormat);
//    inputFormat =
//      Stream_MediaFramework_DirectShow_Tools::copy (*(rhs_in.inputFormat));
//    if (!inputFormat)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), continuing\n")));

//continue_:
    return *this;
  }

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*              direct3DDevice;
#else
  IDirect3DDevice9*                direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  UINT                             resetToken; // direct 3D manager 'id'
  ACE_Stream<ACE_MT_SYNCH,
             Common_TimePolicy_t>* stream;
};
//typedef Stream_SessionData_T<Test_I_AVStream_DirectShow_SessionData> Test_I_AVStream_DirectShow_SessionData_t;

struct Test_I_AVStream_MediaFoundation_StreamSessionData
 : Test_I_MediaFoundation_StreamSessionData
{
  Test_I_AVStream_MediaFoundation_StreamSessionData ()
   : Test_I_MediaFoundation_StreamSessionData ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , session (NULL)
   , stream (NULL)
  {}

  struct Test_I_AVStream_MediaFoundation_StreamSessionData& operator+= (const struct Test_I_AVStream_MediaFoundation_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_MediaFoundation_StreamSessionData::operator+= (rhs_in);

    return *this;
  }

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*              direct3DDevice;
#else
  IDirect3DDevice9*                direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  UINT                             direct3DManagerResetToken;
  TOPOID                           rendererNodeId;
  IMFMediaSession*                 session;
  ACE_Stream<ACE_MT_SYNCH,
             Common_TimePolicy_t>* stream;
};
//typedef Stream_SessionData_T<Test_I_AVStream_MediaFoundation_StreamSessionData> Test_I_AVStream_MediaFoundation_StreamSessionData_t;
#else
struct Test_I_AVStream_ALSA_V4L_StreamSessionData
 : Test_I_StreamSessionData
{
  Test_I_AVStream_ALSA_V4L_StreamSessionData ()
   : Test_I_StreamSessionData ()
   , stream (NULL)
  {}

  struct Test_I_AVStream_ALSA_V4L_StreamSessionData& operator+= (const struct Test_I_AVStream_ALSA_V4L_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_StreamSessionData::operator+= (rhs_in);

    return *this;
  }

  ACE_Stream<ACE_MT_SYNCH,
             Common_TimePolicy_t>* stream;
};
//typedef Stream_SessionData_T<Test_I_AVStream_V4L_StreamSessionData> Test_I_AVStream_V4L_StreamSessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
struct Test_I_AVStream_Configuration;
struct Test_I_AVStream_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_AVStream_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , configuration (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , deviceIdentifier ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , display ()
   , fullScreen (false)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , pixelBuffer (NULL)
   , pixelBufferLock (NULL)
#endif // GTK_USE
   , parserConfiguration (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    , window ()
#else
#if defined (GTK_USE)
    , window (NULL)
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    deviceIdentifier.identifier._guid = GUID_NULL;
    deviceIdentifier.identifierDiscriminator = Stream_Device_Identifier::GUID;
#else
    deviceIdentifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct Test_I_AVStream_Configuration*               configuration;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                                contextId;
#endif // GTK_USE
#endif // GUI_SUPPORT
  struct Stream_Device_Identifier                      deviceIdentifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_UI_DisplayDevice                       display;
  bool                                                 fullScreen;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkPixbuf*                                           pixelBuffer;
  ACE_SYNCH_MUTEX*                                     pixelBufferLock;
#endif // GTK_USE
  struct Common_FlexBisonParserConfiguration*          parserConfiguration;                  // parser module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                                 window;
#else
#if defined (GTK_USE)
  GdkWindow*                                           window;
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
};

struct Test_I_AVStream_Configuration
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
  Test_I_AVStream_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , allocatorConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration ()
#endif // ACE_WIN32 || ACE_WIN64
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
  {}

  struct Common_AllocatorConfiguration                allocatorConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  // *************************** protocol data *********************************
  enum Net_TransportLayerType                         protocol;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_AVStream_UI_ProgressData
 : Test_I_UI_ProgressData
{
  Test_I_AVStream_UI_ProgressData ()
   : Test_I_UI_ProgressData ()
   , transferred (0)
  {}

  size_t transferred; // bytes
};

struct Test_I_AVStream_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_AVStream_UI_CBData ()
   : Test_I_UI_CBData ()
   , configuration (NULL)
   , isFirst (true)
#if defined (GTK_USE)
   , pixelBuffer (NULL)
#endif // GTK_USE
   , progressData ()
  {}

  struct Test_I_AVStream_Configuration*  configuration;
  bool                                    isFirst; // first activation ?
#if defined (GTK_USE)
  GdkPixbuf*                              pixelBuffer;
#endif // GTK_USE
  struct Test_I_AVStream_UI_ProgressData progressData;
};

struct Test_I_AVStream_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_AVStream_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_AVStream_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
