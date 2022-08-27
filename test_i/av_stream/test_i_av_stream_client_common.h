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

#ifndef TEST_I_AVSTREAM_CLIENT_COMMON_H
#define TEST_I_AVSTREAM_CLIENT_COMMON_H

#include <map>
#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "OAIdl.h"
#include "control.h"
#include "evr.h"
#include "mfapi.h"
#include "strmif.h"
//#include "sdkddkver.h"
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "wx/window.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_statistic_handler.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_ffmpeg_common.h"
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_common.h"
#include "test_i_configuration.h"

#include "test_i_av_stream_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_network.h"
#include "test_i_av_stream_client_eventhandler.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;

struct Test_I_AVStream_Client_ConnectionConfiguration;
struct Test_I_AVStream_Client_DirectShow_ConnectionState;
struct Test_I_AVStream_Client_DirectShow_StreamConfiguration;
struct Test_I_AVStream_Client_ConnectionConfiguration;
struct Test_I_AVStream_Client_MediaFoundation_ConnectionState;
struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration;
#else
struct Test_I_AVStream_Client_ALSA_V4L_ConnectionState;
struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
template <typename ConfigurationType>
class Test_I_AVStream_Client_SignalHandler_T;
template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_AVStream_Client_EventHandler_T;
extern const char stream_name_string_[];
extern const char stream_name_string_2[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Client_DirectShow_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                        struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                        struct Test_I_AVStream_Client_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Client_DirectShow_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                   struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                   struct Test_I_AVStream_Client_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
  {}

  Test_I_AVStream_Client_DirectShow_StreamSessionData& operator+= (const Test_I_AVStream_Client_DirectShow_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_DirectShow_StreamSessionData,
                                  struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                  struct Test_I_AVStream_Client_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
};
typedef Stream_SessionData_T<Test_I_AVStream_Client_DirectShow_StreamSessionData> Test_I_AVStream_Client_DirectShow_StreamSessionData_t;

class Test_I_AVStream_Client_MediaFoundation_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                        struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                        struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Client_MediaFoundation_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                   struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                   struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
  {}

  Test_I_AVStream_Client_MediaFoundation_StreamSessionData& operator+= (const Test_I_AVStream_Client_MediaFoundation_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_MediaFoundation_StreamSessionData,
                                  struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat,
                                  struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
};
typedef Stream_SessionData_T<Test_I_AVStream_Client_MediaFoundation_StreamSessionData> Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t;
#else
class Test_I_AVStream_Client_ALSA_V4L_StreamSessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                        struct Stream_MediaFramework_ALSA_V4L_Format,
                                        struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_AVStream_Client_ALSA_V4L_StreamSessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                   struct Stream_MediaFramework_ALSA_V4L_Format,
                                   struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
  {}

  Test_I_AVStream_Client_ALSA_V4L_StreamSessionData& operator+= (const Test_I_AVStream_Client_ALSA_V4L_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_AVStream_ALSA_V4L_StreamSessionData,
                                  struct Stream_MediaFramework_ALSA_V4L_Format,
                                  struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
};
typedef Stream_SessionData_T<Test_I_AVStream_Client_ALSA_V4L_StreamSessionData> Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_Statistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_StreamState;
struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_DirectShow_Message;
class Test_I_AVStream_Client_DirectShow_SessionMessage;

struct Test_I_AVStream_Client_MediaFoundation_StreamState;
struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_MediaFoundation_Message;
class Test_I_AVStream_Client_MediaFoundation_SessionMessage;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_DirectShow_StreamState,
                      struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_DirectShow_Message,
                      Test_I_AVStream_Client_DirectShow_SessionMessage> Test_I_AVStream_Client_DirectShow_Audio_StreamBase_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_2,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_DirectShow_StreamState,
                      struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_DirectShow_Message,
                      Test_I_AVStream_Client_DirectShow_SessionMessage> Test_I_AVStream_Client_DirectShow_Video_StreamBase_t;

typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                      struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_MediaFoundation_Message,
                      Test_I_AVStream_Client_MediaFoundation_SessionMessage> Test_I_AVStream_Client_MediaFoundation_Audio_StreamBase_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_2,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                      struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_MediaFoundation_Message,
                      Test_I_AVStream_Client_MediaFoundation_SessionMessage> Test_I_AVStream_Client_MediaFoundation_Video_StreamBase_t;
#else
struct Test_I_AVStream_Client_ALSA_V4L_StreamState;
struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration;
class Test_I_AVStream_Client_ALSA_V4L_Message;
class Test_I_AVStream_Client_ALSA_V4L_SessionMessage;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                      struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_ALSA_V4L_Message,
                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage> Test_I_AVStream_Client_ALSA_StreamBase_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_2,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                      struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_AVStream_Client_ALSA_V4L_Message,
                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage> Test_I_AVStream_Client_V4L_StreamBase_t;
//typedef Stream_Base_T<ACE_MT_SYNCH,
//                      Common_TimePolicy_t,
//                      stream_name_string_3,
//                      enum Stream_ControlType,
//                      enum Stream_SessionMessageType,
//                      enum Stream_StateMachine_ControlState,
//                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
//                      struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
//                      struct Stream_Statistic,
//                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
//                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
//                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
//                      Stream_ControlMessage_t,
//                      Test_I_AVStream_Client_ALSA_V4L_Message,
//                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage> Test_I_AVStream_Client_Net_StreamBase_t;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_AVStream_Client_DirectShow_Message,
                                    Test_I_AVStream_Client_DirectShow_SessionMessage> Test_I_AVStream_Client_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_AVStream_Client_DirectShow_ISessionNotify_t*> Test_I_AVStream_Client_DirectShow_Subscribers_t;
typedef Test_I_AVStream_Client_DirectShow_Subscribers_t::iterator Test_I_AVStream_Client_DirectShow_SubscribersIterator_t;
struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration
 : Test_I_AVStream_ModuleHandlerConfiguration
{
  Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_AVStream_ModuleHandlerConfiguration ()
   , builder (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   //, sourceFormat ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , windowController (NULL)
   , windowController2 (NULL)
#endif // GUI_SUPPORT
  {
    //finishOnDisconnect = true;

    //mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration operator= (const struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration& rhs_in)
  {
    Test_I_AVStream_ModuleHandlerConfiguration::operator= (rhs_in);

    if (builder)
    {
      builder->Release (); builder = NULL;
    } // end IF
    if (rhs_in.builder)
    {
      rhs_in.builder->AddRef ();
      builder = rhs_in.builder;
    } // end IF
    if (connection)
    {
      connection->decrease (); connection = NULL;
    } // end IF
    if (rhs_in.connection)
    {
      rhs_in.connection->increase ();
      connection = rhs_in.connection;
    } // end IF
    connectionConfigurations = rhs_in.connectionConfigurations;
    connectionManager = rhs_in.connectionManager;
    filterConfiguration = rhs_in.filterConfiguration;
    filterCLSID = rhs_in.filterCLSID;
    Stream_MediaFramework_DirectShow_Tools::free (outputFormat);
    struct _AMMediaType* media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (rhs_in.outputFormat);
    if (!media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
      return *this;
    } // end IF
    outputFormat = *media_type_p;
    delete (media_type_p); media_type_p = NULL;
    push = rhs_in.push;
    //Stream_MediaFramework_DirectShow_Tools::free (sourceFormat);
    //media_type_p =
    //    Stream_MediaFramework_DirectShow_Tools::copy (rhs_in.sourceFormat);
    //if (!media_type_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
    //  return *this;
    //} // end IF
    //sourceFormat = *media_type_p;
    //CoTaskMemFree (media_type_p); media_type_p = NULL;
    streamConfiguration = rhs_in.streamConfiguration;
    subscriber = rhs_in.subscriber;
    subscribers = rhs_in.subscribers;
    if (windowController)
    {
      windowController->Release (); windowController = NULL;
    } // end IF
    if (rhs_in.windowController)
    {
      rhs_in.windowController->AddRef ();
      windowController = rhs_in.windowController;
    } // end IF
    if (windowController2)
    {
      windowController2->Release (); windowController2 = NULL;
    } // end IF
    if (rhs_in.windowController2)
    {
      rhs_in.windowController2->AddRef ();
      windowController2 = rhs_in.windowController2;
    } // end IF

    return *this;
  }

  IGraphBuilder*                                                builder;
  Net_IINETConnection_t*                                        connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                               connectionConfigurations;
  Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t*     connectionManager; // TCP IO module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                                         contextId;
#endif // GTK_USE
#endif // GUI_SUPPORT
  struct Test_I_AVStream_Client_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                         filterCLSID;
  struct _AMMediaType                                           outputFormat; // display module
  bool                                                          push;
  Test_I_AVStream_Client_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_I_AVStream_Client_DirectShow_ISessionNotify_t*           subscriber;
  Test_I_AVStream_Client_DirectShow_Subscribers_t*              subscribers;
#if defined (GUI_SUPPORT)
  IVideoWindow*                                                 windowController; // visualization module
  IMFVideoDisplayControl*                                       windowController2; // visualization module (EVR)
#endif // GUI_SUPPORT
};

typedef Stream_ISessionDataNotify_T<Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_AVStream_Client_MediaFoundation_Message,
                                    Test_I_AVStream_Client_MediaFoundation_SessionMessage> Test_I_AVStream_Client_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_I_AVStream_Client_MediaFoundation_ISessionNotify_t*> Test_I_AVStream_Client_MediaFoundation_Subscribers_t;
typedef Test_I_AVStream_Client_MediaFoundation_Subscribers_t::iterator Test_I_AVStream_Client_MediaFoundation_SubscribersIterator_t;
struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_AVStream_ModuleHandlerConfiguration
{
  Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_AVStream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , mediaSource (NULL)
   , outputFormat (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
   //, sourceFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
  {
    //finishOnDisconnect = true;

    HRESULT result = MFCreateMediaType (&outputFormat);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    ACE_ASSERT (outputFormat);
  }

  struct tagRECT                                            area;
  Net_IINETConnection_t*                                    connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                           connectionConfigurations;
  Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t*     connectionManager; // TCP IO module
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx*                                         mediaSource;
#else
  IMFMediaSource*                                           mediaSource;
#endif // _WIN32_WINNT_WIN8
  IMFMediaType*                                             outputFormat; // display module
  TOPOID                                                    sampleGrabberNodeId;
  IMFMediaSession*                                          session;
  //IMFMediaType*                                             sourceFormat;
  Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t*      streamConfiguration;
  Test_I_AVStream_Client_MediaFoundation_ISessionNotify_t*           subscriber;
  Test_I_AVStream_Client_MediaFoundation_Subscribers_t*              subscribers;
  IMFVideoDisplayControl*                                   windowController;
};
#else
typedef Stream_ISessionDataNotify_T<Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_AVStream_Client_ALSA_V4L_Message,
                                    Test_I_AVStream_Client_ALSA_V4L_SessionMessage> Test_I_AVStream_Client_ALSA_V4L_ISessionNotify_t;
typedef std::list<Test_I_AVStream_Client_ALSA_V4L_ISessionNotify_t*> Test_I_AVStream_Client_ALSA_V4L_Subscribers_t;
typedef Test_I_AVStream_Client_ALSA_V4L_Subscribers_t::iterator Test_I_AVStream_Client_ALSA_V4L_SubscribersIterator_t;
struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration;
struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration
 : Test_I_AVStream_ModuleHandlerConfiguration
{
  Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration ()
   : Test_I_AVStream_ModuleHandlerConfiguration ()
   , ALSAConfiguration (NULL)
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , outputFormat ()
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , window (NULL)
#endif // GUI_SUPPORT
  {}

  struct Stream_MediaFramework_ALSA_Configuration*        ALSAConfiguration;
  __u32                                                   buffers; // v4l device buffers
  Net_IINETConnection_t*                                  connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                         connectionConfigurations;
  Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t* connectionManager; // TCP IO module
  enum v4l2_memory                                        method; // v4l2 camera source
  struct Stream_MediaFramework_FFMPEG_VideoMediaType      outputFormat; // display module
  ACE_Time_Value                                          statisticCollectionInterval;
  // *TODO*: remove this ASAP
  Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t*  streamConfiguration;
  Test_I_AVStream_Client_ALSA_V4L_ISessionNotify_t*       subscriber;
  Test_I_AVStream_Client_ALSA_V4L_Subscribers_t*          subscribers;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                                              window;
#elif defined (WXWIDGETS_USE)
  wxWindow*                                               window;
#elif defined (QT_USE)
  XID                                                     window;
#else
  void*                                                   window;
#endif // GTK_USE || WXWIDGETS_USE || QT_USE
#endif // GUI_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_AVStream_Client_DirectShow_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
//   , statisticReportingInterval (0)
   , audioStream (NULL)
   , videoStream (NULL)
  {}

  Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t* connectionManager;
//  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_AVStream_Client_DirectShow_Audio_StreamBase_t*     audioStream;
  Test_I_AVStream_Client_DirectShow_Video_StreamBase_t*     videoStream;
};
typedef Test_I_AVStream_Client_SignalHandler_T<struct Test_I_AVStream_Client_DirectShow_SignalHandlerConfiguration> Test_I_AVStream_Client_DirectShow_SignalHandler_t;
struct Test_I_AVStream_Client_MediaFoundation_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_AVStream_Client_MediaFoundation_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , audioStream (NULL)
   , videoStream (NULL)
  {}

  Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t* connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_AVStream_Client_MediaFoundation_Audio_StreamBase_t*     audioStream;
  Test_I_AVStream_Client_MediaFoundation_Video_StreamBase_t*     videoStream;
};
typedef Test_I_AVStream_Client_SignalHandler_T<struct Test_I_AVStream_Client_MediaFoundation_SignalHandlerConfiguration> Test_I_AVStream_Client_MediaFoundation_SignalHandler_t;
#else
struct Test_I_AVStream_Client_ALSA_V4L_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_AVStream_Client_ALSA_V4L_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , audioStream (NULL)
   , videoStream (NULL)
   , UDPStream (NULL)
  {}

  Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t* connectionManager;
  Test_I_AVStream_Client_ALSA_StreamBase_t*               audioStream;
  Test_I_AVStream_Client_V4L_StreamBase_t*                videoStream;
  Stream_IStreamControlBase*                              UDPStream;
};
typedef Test_I_AVStream_Client_SignalHandler_T<struct Test_I_AVStream_Client_ALSA_V4L_SignalHandlerConfiguration> Test_I_AVStream_Client_ALSA_V4L_SignalHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  Test_I_AVStream_Client_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
   , graphLayout ()
   , module_2 (NULL)
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_DirectShow_AudioVideoFormat format;
  Stream_MediaFramework_DirectShow_Graph_t                 graphLayout;
  Stream_Module_t*                                         module_2;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                               struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration> Test_I_AVStream_Client_DirectShow_StreamConfiguration_t;
typedef Test_I_AVStream_Client_DirectShow_StreamConfiguration_t::ITERATOR_T Test_I_AVStream_Client_DirectShow_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_AVStream_Client_DirectShow_StreamConfiguration_t> Test_I_AVStream_Client_DirectShow_StreamConfigurations_t;
typedef Test_I_AVStream_Client_DirectShow_StreamConfigurations_t::iterator Test_I_AVStream_Client_DirectShow_StreamConfigurationsIterator_t;

struct Test_I_MediaFoundationConfiguration;
struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  Test_I_AVStream_Client_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
   , module_2 (NULL)
   , mediaFoundationConfiguration (NULL)
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat format;
  Stream_Module_t*                                              module_2;

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration*                   mediaFoundationConfiguration;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                               struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration> Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t;
typedef Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t::ITERATOR_T Test_I_AVStream_Client_MediaFoundation_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t> Test_I_AVStream_Client_MediaFoundation_StreamConfigurations_t;
typedef Test_I_AVStream_Client_MediaFoundation_StreamConfigurations_t::iterator Test_I_AVStream_Client_MediaFoundation_StreamConfigurationsIterator_t;
#else
struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration
 : Stream_Configuration
{
  Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
   , module_2 (NULL)
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_ALSA_V4L_Format format;
  Stream_Module_t*                             module_2;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                               struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration> Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t;
typedef Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t::ITERATOR_T Test_I_AVStream_Client_ALSA_V4L_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t> Test_I_AVStream_Client_ALSA_V4L_StreamConfigurations_t;
typedef Test_I_AVStream_Client_ALSA_V4L_StreamConfigurations_t::iterator Test_I_AVStream_Client_ALSA_V4L_StreamConfigurationsIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_StreamState
 : Stream_State
{
  Test_I_AVStream_Client_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_AVStream_Client_DirectShow_StreamSessionData* sessionData;
};

struct Test_I_AVStream_Client_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_AVStream_Client_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_AVStream_Client_MediaFoundation_StreamSessionData* sessionData;
};
#else
struct Test_I_AVStream_Client_ALSA_V4L_StreamState
 : Stream_State
{
  Test_I_AVStream_Client_ALSA_V4L_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_AVStream_Client_ALSA_V4L_StreamSessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_Configuration
 : Test_I_AVStream_Configuration
{
  Test_I_AVStream_Client_DirectShow_Configuration ()
   : Test_I_AVStream_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** signal data **********************************
  struct Test_I_AVStream_Client_DirectShow_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                             connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_AVStream_Client_DirectShow_StreamConfigurations_t            streamConfigurations;
};

struct Test_I_AVStream_Client_MediaFoundation_Configuration
 : Test_I_AVStream_Configuration
{
  Test_I_AVStream_Client_MediaFoundation_Configuration ()
   : Test_I_AVStream_Configuration ()
   , mediaFoundationConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_AVStream_Client_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                                  connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_AVStream_Client_MediaFoundation_StreamConfigurations_t            streamConfigurations;
};
#else
struct Test_I_AVStream_Client_ALSA_V4L_Configuration
 : Test_I_AVStream_Configuration
{
  Test_I_AVStream_Client_ALSA_V4L_Configuration ()
   : Test_I_AVStream_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** signal data **********************************
  struct Test_I_AVStream_Client_ALSA_V4L_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                                    connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_AVStream_Client_ALSA_V4L_StreamConfigurations_t            streamConfigurations;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_AVStream_Client_DirectShow_Message,
                                          Test_I_AVStream_Client_DirectShow_SessionMessage> Test_I_AVStream_Client_DirectShow_MessageAllocator_t;

struct Test_I_AVStream_Client_DirectShow_UI_CBData;
typedef Test_I_AVStream_Client_EventHandler_T<Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                              enum Stream_SessionMessageType,
                                              Test_I_AVStream_Client_DirectShow_Message,
                                              Test_I_AVStream_Client_DirectShow_SessionMessage,
                                              struct Test_I_AVStream_Client_DirectShow_UI_CBData> Test_I_AVStream_Client_DirectShow_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_AVStream_Client_DirectShow_ISessionNotify_t> Test_I_AVStream_Client_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_AVStream_Client_MediaFoundation_Message,
                                          Test_I_AVStream_Client_MediaFoundation_SessionMessage> Test_I_AVStream_Client_MediaFoundation_MessageAllocator_t;

struct Test_I_AVStream_Client_MediaFoundation_UI_CBData;
typedef Test_I_AVStream_Client_EventHandler_T<Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                              enum Stream_SessionMessageType,
                                              Test_I_AVStream_Client_MediaFoundation_Message,
                                              Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                              struct Test_I_AVStream_Client_MediaFoundation_UI_CBData> Test_I_AVStream_Client_MediaFoundation_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_AVStream_Client_MediaFoundation_ISessionNotify_t> Test_I_AVStream_Client_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_AVStream_Client_ALSA_V4L_Message,
                                          Test_I_AVStream_Client_ALSA_V4L_SessionMessage> Test_I_AVStream_Client_ALSA_V4L_MessageAllocator_t;

struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData;
typedef Test_I_AVStream_Client_EventHandler_T<Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                              enum Stream_SessionMessageType,
                                              Test_I_AVStream_Client_ALSA_V4L_Message,
                                              Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                              struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData> Test_I_AVStream_Client_ALSA_V4L_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_AVStream_Client_ALSA_V4L_ISessionNotify_t> Test_I_AVStream_Client_ALSA_V4L_ISubscribe_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_UI_CBData
 : Test_I_AVStream_UI_CBData
{
  Test_I_AVStream_Client_DirectShow_UI_CBData ()
   : Test_I_AVStream_UI_CBData ()
   , configuration (NULL)
   , audioStream (NULL)
   , videoStream (NULL)
   , subscribers ()
   , subscribersLock ()
   , streamConfiguration (NULL)
   , UDPStream (NULL)
  {}

  struct Test_I_AVStream_Client_DirectShow_Configuration* configuration;
  Test_I_AVStream_Client_DirectShow_Audio_StreamBase_t*   audioStream;
  Test_I_AVStream_Client_DirectShow_Video_StreamBase_t*   videoStream;
  Test_I_AVStream_Client_DirectShow_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                               subscribersLock;
  IAMStreamConfig*                                        streamConfiguration;
  Test_I_AVStream_Client_DirectShow_Video_StreamBase_t*   UDPStream;
};

struct Test_I_AVStream_Client_MediaFoundation_UI_CBData
 : Test_I_AVStream_UI_CBData
{
  Test_I_AVStream_Client_MediaFoundation_UI_CBData ()
   : Test_I_AVStream_UI_CBData ()
   , configuration (NULL)
   , audioStream (NULL)
   , videoStream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {}

  struct Test_I_AVStream_Client_MediaFoundation_Configuration* configuration;
  Test_I_AVStream_Client_MediaFoundation_Audio_StreamBase_t*   audioStream;
  Test_I_AVStream_Client_MediaFoundation_Video_StreamBase_t*   videoStream;
  Test_I_AVStream_Client_MediaFoundation_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                                    subscribersLock;
  Test_I_AVStream_Client_MediaFoundation_Video_StreamBase_t*   UDPStream;
};
#else
struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData
 : Test_I_AVStream_UI_CBData
{
  Test_I_AVStream_Client_ALSA_V4L_UI_CBData ()
   : Test_I_AVStream_UI_CBData ()
   , configuration (NULL)
   , fileDescriptor (-1)
   , audioStream (NULL)
   , videoStream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {}

  struct Test_I_AVStream_Client_ALSA_V4L_Configuration* configuration;
  int                                                   fileDescriptor; // (capture) device file descriptor
  Test_I_AVStream_Client_ALSA_StreamBase_t*             audioStream;
  Test_I_AVStream_Client_V4L_StreamBase_t*              videoStream;
  Test_I_AVStream_Client_ALSA_V4L_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                             subscribersLock;
  Test_I_AVStream_Client_V4L_StreamBase_t*              UDPStream;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_AVStream_Client_DirectShow_ThreadData
 : Test_I_AVStream_ThreadData
{
  Test_I_AVStream_Client_DirectShow_ThreadData ()
   : Test_I_AVStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_AVStream_Client_DirectShow_UI_CBData* CBData;
};

struct Test_I_AVStream_Client_MediaFoundation_ThreadData
 : Test_I_AVStream_ThreadData
{
  Test_I_AVStream_Client_MediaFoundation_ThreadData ()
   : Test_I_AVStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_AVStream_Client_MediaFoundation_UI_CBData* CBData;
};
#else
struct Test_I_AVStream_Client_ALSA_V4L_ThreadData
 : Test_I_AVStream_ThreadData
{
  Test_I_AVStream_Client_ALSA_V4L_ThreadData ()
   : Test_I_AVStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#endif
