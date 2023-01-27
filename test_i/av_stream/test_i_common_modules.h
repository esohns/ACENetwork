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

#ifndef TEST_I_COMMON_MODULES_H
#define TEST_I_COMMON_MODULES_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_lib_tagger.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#include "stream_dev_mic_source_wasapi.h"
#include "stream_dev_target_wasapi.h"

#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#include "stream_lib_directshow_target.h"
#include "stream_lib_mediafoundation_target.h"

#if defined (GUI_SUPPORT)
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"

#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#else
#include "stream_dev_cam_source_v4l.h"
#include "stream_dev_mic_source_alsa.h"
#include "stream_dev_target_alsa.h"

#include "stream_lib_v4l_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_pixbuf.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"

#include "stream_dec_libav_decoder.h"

#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT

#include "stream_misc_distributor.h"
#include "stream_misc_splitter.h"

#include "stream_net_io.h"

//#include "stream_stat_statistic_report.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"

#include "test_i_module_eventhandler.h"
#include "test_i_module_parser.h"
#include "test_i_module_splitter.h"
#include "test_i_module_streamer.h"

#include "test_i_av_stream_client_common.h"
#include "test_i_av_stream_client_message.h"
#include "test_i_av_stream_client_session_message.h"

#include "test_i_av_stream_server_common.h"
#include "test_i_av_stream_server_message.h"
#include "test_i_av_stream_server_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// forward declarations
struct Test_I_AVStream_Server_DirectShow_FilterConfiguration;
struct Test_I_AVStream_Server_DirectShow_PinConfiguration;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_DirectShow_Message,
                               Test_I_AVStream_Client_DirectShow_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_AVStream_Client_DirectShow_Message,
                                Test_I_AVStream_Client_DirectShow_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_DirectShow_TaskBaseAsynch_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_MediaFoundation_Message,
                               Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_MediaFoundation_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_AVStream_Client_MediaFoundation_Message,
                                Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_MediaFoundation_TaskBaseAsynch_t;

// source
typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_I_AVStream_Client_DirectShow_Message,
                                       Test_I_AVStream_Client_DirectShow_SessionMessage,
                                       struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_I_AVStream_Client_DirectShow_StreamState,
                                       Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                       Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                       struct Stream_Statistic,
                                       Common_Timer_Manager_t,
                                       struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_WASAPIIn;
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Server_DirectShow_Message,
                                   Test_I_AVStream_Server_DirectShow_SessionMessage,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Server_DirectShow_WASAPIOut;

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_I_AVStream_Client_DirectShow_Message,
                                           Test_I_AVStream_Client_DirectShow_SessionMessage,
                                           struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_I_AVStream_Client_DirectShow_StreamState,
                                           Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                           Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                           struct Stream_Statistic,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct Stream_MediaFramework_DirectShow_AudioVideoFormat,
                                           false> Test_I_Stream_DirectShow_CamSource;
typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Test_I_AVStream_Client_MediaFoundation_Message,
                                                Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                                struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                                Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                                Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                                struct Stream_Statistic,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData,
                                                struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_Stream_MediaFoundation_CamSource;
#else
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_ALSA_V4L_Message,
                               Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_ALSA_V4L_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_AVStream_Client_ALSA_V4L_Message,
                                Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_ALSA_V4L_TaskBaseAsynch_t;

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_CamSource;
typedef Stream_Dev_Mic_Source_ALSA_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t> Test_I_AVStream_Client_ALSA_V4L_MicSource;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_DirectShow_Message,
                               Test_I_AVStream_Client_DirectShow_SessionMessage,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_Audio_Tagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_DirectShow_Message,
                               Test_I_AVStream_Client_DirectShow_SessionMessage,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_Video_Tagger;

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_MediaFoundation_Message,
                               Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_AVStream_Client_MediaFoundation_Audio_Tagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_MediaFoundation_Message,
                               Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_AVStream_Client_MediaFoundation_Video_Tagger;
#else
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_ALSA_V4L_Message,
                               Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Client_ALSA_V4L_Audio_Tagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Client_ALSA_V4L_Message,
                               Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Client_ALSA_V4L_Video_Tagger;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_DirectShow_Message,
                                     Test_I_AVStream_Client_DirectShow_SessionMessage,
                                     struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_DirectShow_StreamState,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Module_TCPWriter_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_DirectShow_Message,
                                     Test_I_AVStream_Client_DirectShow_SessionMessage,
                                     struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Stream_State,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Module_TCPReader_t;
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_DirectShow_Message,
                                     Test_I_AVStream_Client_DirectShow_SessionMessage,
                                     struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_DirectShow_StreamState,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Module_UDPWriter_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_DirectShow_Message,
                                     Test_I_AVStream_Client_DirectShow_SessionMessage,
                                     struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Stream_State,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Module_UDPReader_t;

typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Module_TCPWriter_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Module_TCPReader_t;
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Module_UDPWriter_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Module_UDPReader_t;
#else
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_TCP_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_TCP_Reader_t;

typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_UDP_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_AVStream_Client_ALSA_V4L_UDPConnectionManager_t,
                                     struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_UDP_Reader_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_AVStream_Parser_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_AVStream_Server_DirectShow_Message,
                                 Test_I_AVStream_Server_DirectShow_SessionMessage> Test_I_AVStream_Server_DirectShow_Parser;
typedef Test_I_AVStream_Parser_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_AVStream_Server_MediaFoundation_Message,
                                 Test_I_AVStream_Server_MediaFoundation_SessionMessage> Test_I_AVStream_Server_MediaFoundation_Parser;

typedef Test_I_AVStream_Splitter_T<ACE_MT_SYNCH,
                                   struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Server_DirectShow_Message,
                                   Test_I_AVStream_Server_DirectShow_SessionMessage,
                                   Test_I_AVStream_Client_DirectShow_StreamSessionData_t> Test_I_AVStream_Server_DirectShow_Splitter;
typedef Test_I_AVStream_Splitter_T<ACE_MT_SYNCH,
                                   struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Server_MediaFoundation_Message,
                                   Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                   Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Server_MediaFoundation_Splitter;

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_AVStream_Client_DirectShow_Message,
                                                      Test_I_AVStream_Client_DirectShow_SessionMessage,
                                                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t> Test_I_AVStream_Client_DirectShow_Distributor;

typedef Test_I_AVStream_Streamer_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Client_DirectShow_Message,
                                   Test_I_AVStream_Client_DirectShow_SessionMessage,
                                   struct Stream_UserData> Test_I_AVStream_Client_DirectShow_Streamer;
typedef Test_I_AVStream_Streamer_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Client_MediaFoundation_Message,
                                   Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                   struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_Streamer;

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_AVStream_Client_MediaFoundation_Message,
                                                      Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Client_MediaFoundation_Distributor;

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_DirectShow_Message,
//                                                      Test_I_AVStream_Client_DirectShow_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
//                                                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t> Test_I_AVStream_Client_DirectShow_Module_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_DirectShow_Message,
//                                                      Test_I_AVStream_Client_DirectShow_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_DirectShow_StreamSessionData,
//                                                      Test_I_AVStream_Client_DirectShow_StreamSessionData_t> Test_I_AVStream_Client_DirectShow_Module_Statistic_WriterTask_t;
//
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_MediaFoundation_Message,
//                                                      Test_I_AVStream_Client_MediaFoundation_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
//                                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Client_MediaFoundation_Module_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_MediaFoundation_Message,
//                                                      Test_I_AVStream_Client_MediaFoundation_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
//                                                      Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Client_MediaFoundation_Module_Statistic_WriterTask_t;
#else
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_AVStream_Server_Message,
                               Test_I_AVStream_Server_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_Server_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_AVStream_Server_Message,
                                Test_I_AVStream_Server_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_Server_TaskBaseAsynch_t;

typedef Test_I_AVStream_Parser_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_AVStream_Server_Message,
                                 Test_I_AVStream_Server_SessionMessage> Test_I_AVStream_Server_Parser;

typedef Test_I_AVStream_Splitter_T<ACE_MT_SYNCH,
                                   struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Server_Message,
                                   Test_I_AVStream_Server_SessionMessage,
                                   Test_I_AVStream_Server_StreamSessionData_t> Test_I_AVStream_Server_Splitter;

typedef Test_I_AVStream_Streamer_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_AVStream_Client_ALSA_V4L_Message,
                                   Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                   struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_Streamer;

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t> Test_I_AVStream_Client_ALSA_V4L_Distributor;

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_ALSA_V4L_Message,
//                                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
//                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t> Test_I_AVStream_Client_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Client_ALSA_V4L_Message,
//                                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
//                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t> Test_I_AVStream_Client_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Vis_DirectShow_Target_Direct3D_T<ACE_MT_SYNCH,
//                                                Common_TimePolicy_t,
//                                                struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
//                                                Stream_ControlMessage_t,
//                                                Test_I_AVStream_Client_DirectShow_Stream_Message,
//                                                Test_I_AVStream_Client_DirectShow_SessionMessage,
//                                                Test_I_AVStream_Client_DirectShow_SessionData,
//                                                Test_I_AVStream_Client_DirectShow_SessionData_t> Test_I_AVStream_Client_DirectShow_Direct3D_Display;

struct Test_I_AVStream_Client_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Test_I_AVStream_Client_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   //, format (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  // *TODO*: specify this as part of the network protocol header/handshake
  //struct _AMMediaType*                                           format; // handle
  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_I_AVStream_Client_DirectShow_Message,
                                                         struct Test_I_AVStream_Client_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_I_AVStream_Client_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_I_AVStream_Client_DirectShow_Message,
                                                                struct Test_I_AVStream_Client_DirectShow_FilterConfiguration,
                                                                struct Test_I_AVStream_Client_DirectShow_FilterPinConfiguration> Test_I_AVStream_Client_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_AVStream_Client_DirectShow_Message,
                                       Test_I_AVStream_Client_DirectShow_SessionMessage,
                                       Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                       Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                       struct Test_I_AVStream_Client_DirectShow_FilterConfiguration,
                                       struct Test_I_AVStream_Client_DirectShow_PinConfiguration,
                                       Test_I_AVStream_Client_DirectShowFilter_t,
                                       struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_Display;
typedef Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_MT_SYNCH,
                                                     Common_TimePolicy_t,
                                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                                     Stream_ControlMessage_t,
                                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                                     struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_AVStream_Client_MediaFoundation_Display;
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t> Test_I_AVStream_Client_ALSA_V4L_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_AVStream_Client_ALSA_V4L_Message,
                                                      Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                                      Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t> Test_I_AVStream_Client_ALSA_V4L_Distributor_Writer_t;

typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_AVStream_Client_ALSA_V4L_Message,
                                       Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                       Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Test_I_AVStream_Client_ALSA_V4L_Display;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_LibAVConverter_T<Test_I_DirectShow_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_Converter;
typedef Stream_Decoder_LibAVConverter_T<Test_I_MediaFoundation_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_AVStream_Client_MediaFoundation_Converter;

typedef Stream_Visualization_LibAVResize_T<Test_I_DirectShow_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Client_DirectShow_Resize;
typedef Stream_Visualization_LibAVResize_T<Test_I_MediaFoundation_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Test_I_AVStream_Client_MediaFoundation_Resize;
#else
typedef Stream_Decoder_LibAVConverter_T<Test_I_ALSA_V4L_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Client_ALSA_V4L_Converter;

typedef Stream_Visualization_LibAVResize_T<Test_I_ALSA_V4L_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Client_ALSA_V4L_Resize;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_DirectShow_Message,
                                            Test_I_AVStream_Client_DirectShow_SessionMessage,
                                            Test_I_AVStream_Client_DirectShow_StreamSessionData,
                                            Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Client_DirectShow_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_MediaFoundation_Message,
                                            Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                            Test_I_AVStream_Client_MediaFoundation_StreamSessionData,
                                            Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Client_MediaFoundation_EventHandler;
#else
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Client_ALSA_V4L_Message,
                                            Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                            Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,
                                            Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Client_ALSA_V4L_Module_EventHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// target
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_DirectShow_Message,
//                                                      Test_I_AVStream_Server_DirectShow_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_DirectShow_StreamSessionData,
//                                                      Test_I_AVStream_Server_DirectShow_StreamSessionData_t> Test_I_AVStream_Server_DirectShow_Module_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_DirectShow_Message,
//                                                      Test_I_AVStream_Server_DirectShow_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_DirectShow_StreamSessionData,
//                                                      Test_I_AVStream_Server_DirectShow_StreamSessionData_t> Test_I_AVStream_Server_DirectShow_Module_Statistic_WriterTask_t;
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_MediaFoundation_Message,
//                                                      Test_I_AVStream_Server_MediaFoundation_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
//                                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Server_MediaFoundation_Module_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_MediaFoundation_Message,
//                                                      Test_I_AVStream_Server_MediaFoundation_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
//                                                      Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Server_MediaFoundation_Module_Statistic_WriterTask_t;
#else
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_Message,
//                                                      Test_I_AVStream_Server_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_StreamSessionData,
//                                                      Test_I_AVStream_Server_StreamSessionData_t> Test_I_AVStream_Server_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_AVStream_Server_Message,
//                                                      Test_I_AVStream_Server_SessionMessage,
//                                                      Test_I_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_AVStream_Server_StreamSessionData,
//                                                      Test_I_AVStream_Server_StreamSessionData_t> Test_I_AVStream_Server_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Server_DirectShow_Message,
                                     Test_I_AVStream_Server_DirectShow_SessionMessage,
                                     Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                     Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                     struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Server_Direct3D_Display;

typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_I_AVStream_Server_DirectShow_Message,
                                                         struct Test_I_AVStream_Server_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_I_AVStream_Server_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_I_AVStream_Server_DirectShow_Message,
                                                                struct Test_I_AVStream_Server_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_I_AVStream_Server_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_AVStream_Server_DirectShow_Message,
                                       Test_I_AVStream_Server_DirectShow_SessionMessage,
                                       Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                       Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                       struct Test_I_AVStream_Server_DirectShow_FilterConfiguration,
                                       struct Test_I_AVStream_Server_DirectShow_PinConfiguration,
                                       Test_I_AVStream_Server_DirectShowFilter_t,
                                       struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Test_I_AVStream_Server_DirectShow_Display;
typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Server_MediaFoundation_Message,
                                            Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Server_MediaFoundation_Display;
//typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Stream_ControlMessage_t,
//                                            Test_I_AVStream_Server_MediaFoundation_Message,
//                                            Test_I_AVStream_Server_MediaFoundation_SessionMessage,
//                                            Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
//                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t> Test_I_AVStream_Server_Stream_DisplayNull;
#else
#if defined (GUI_SUPPORT)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Visualization_LibAVResize_T<Test_I_Server_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Server_Resize;
#endif // FFMPEG_SUPPORT
#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_AVStream_Server_Message,
                                       Test_I_AVStream_Server_SessionMessage,
                                       Test_I_AVStream_Server_StreamSessionData_t,
                                       struct Stream_MediaFramework_ALSA_V4L_Format> Test_I_AVStream_Server_Display;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_AVStream_Server_Message,
                                 Test_I_AVStream_Server_SessionMessage,
                                 Test_I_AVStream_Server_StreamSessionData> Test_I_AVStream_Server_Target_ALSA;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Server_DirectShow_Message,
                                            Test_I_AVStream_Server_DirectShow_SessionMessage,
                                            Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                            Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Server_DirectShow_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Server_MediaFoundation_Message,
                                            Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                            Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Server_MediaFoundation_EventHandler;
#else
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_AVStream_Server_Message,
                                            Test_I_AVStream_Server_SessionMessage,
                                            Test_I_AVStream_Server_StreamSessionData,
                                            Test_I_AVStream_Server_StreamSessionData_t,
                                            struct Stream_UserData> Test_I_AVStream_Server_Module_EventHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                      // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wasapi_module_name_string,
                              Stream_INotify_t,                                                    // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_WASAPIIn);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                      // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                                    // stream notification interface type
                              Test_I_AVStream_Server_DirectShow_WASAPIOut);                        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Stream_DirectShow_CamSource);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Stream_MediaFoundation_CamSource);                        // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_CamSource);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                // session data type
                             enum Stream_SessionMessageType,                                    // session event type
                             struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                             libacestream_default_dev_mic_source_alsa_module_name_string,
                             Stream_INotify_t,                                                  // stream notification interface type
                             Test_I_AVStream_Client_ALSA_V4L_MicSource);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_Audio_Tagger);           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_Video_Tagger);           // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_Audio_Tagger);           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_Video_Tagger);           // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Audio_Tagger);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Video_Tagger);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                             // session event type
                          struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                           // stream notification interface type
                          Test_I_AVStream_Client_DirectShow_Module_TCPReader_t,               // reader type
                          Test_I_AVStream_Client_DirectShow_Module_TCPWriter_t,               // writer type
                          Test_I_AVStream_Client_DirectShow_Module_TCP_IO);                    // name
DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                             // session event type
                          struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                           // stream notification interface type
                          Test_I_AVStream_Client_DirectShow_Module_UDPReader_t,               // reader type
                          Test_I_AVStream_Client_DirectShow_Module_UDPWriter_t,               // writer type
                          Test_I_AVStream_Client_DirectShow_Module_UDP_IO);                    // name

DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_I_AVStream_Client_MediaFoundation_Module_TCPReader_t,               // reader type
                          Test_I_AVStream_Client_MediaFoundation_Module_TCPWriter_t,               // writer type
                          Test_I_AVStream_Client_MediaFoundation_Module_TCP_IO);                    // name
DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_I_AVStream_Client_MediaFoundation_Module_UDPReader_t,               // reader type
                          Test_I_AVStream_Client_MediaFoundation_Module_UDPWriter_t,               // writer type
                          Test_I_AVStream_Client_MediaFoundation_Module_UDP_IO);                    // name
#else
DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_I_AVStream_Client_ALSA_V4L_TCP_Reader_t,                      // reader type
                          Test_I_AVStream_Client_ALSA_V4L_TCP_Writer_t,                      // writer type
                          Test_I_AVStream_Client_ALSA_V4L_TCP_IO);                           // name

DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_I_AVStream_Client_ALSA_V4L_UDP_Reader_t,                      // reader type
                          Test_I_AVStream_Client_ALSA_V4L_UDP_Writer_t,                      // writer type
                          Test_I_AVStream_Client_ALSA_V4L_UDP_IO);                           // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                      // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_parser_module_name_string,
                              Stream_INotify_t,                                                    // stream notification interface type
                              Test_I_AVStream_Server_DirectShow_Parser);                           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_MediaFoundation_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                           // session event type
                              struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_parser_module_name_string,
                              Stream_INotify_t,                                                         // stream notification interface type
                              Test_I_AVStream_Server_MediaFoundation_Parser);                           // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                      // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                                    // stream notification interface type
                              Test_I_AVStream_Server_DirectShow_Splitter);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_MediaFoundation_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                           // session event type
                              struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                                         // stream notification interface type
                              Test_I_AVStream_Server_MediaFoundation_Splitter);                         // writer type

DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_DirectShow_StreamSessionData,                 // session data type
                          enum Stream_SessionMessageType,                                      // session event type
                          struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                                    // stream notification interface type
                          Test_I_AVStream_Client_DirectShow_Distributor::READER_TASK_T,        // reader type
                          Test_I_AVStream_Client_DirectShow_Distributor,                       // writer type
                          Test_I_AVStream_Client_DirectShow_Distributor);                      // name

DATASTREAM_MODULE_DUPLEX_A (Test_I_AVStream_Client_DirectShow_StreamSessionData,                 // session data type
                            enum Stream_SessionMessageType,                                      // session event type
                            struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                            libacestream_default_misc_splitter_module_name_string,
                            Stream_INotify_t,                                                    // stream notification interface type
                            Test_I_AVStream_Client_DirectShow_Streamer::READER_TASK_T,           // reader type
                            Test_I_AVStream_Client_DirectShow_Streamer,                          // writer type
                            Test_I_AVStream_Client_DirectShow_Streamer);                         // name
DATASTREAM_MODULE_DUPLEX_A (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                 // session data type
                            enum Stream_SessionMessageType,                                           // session event type
                            struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                            libacestream_default_misc_splitter_module_name_string,
                            Stream_INotify_t,                                                         // stream notification interface type
                            Test_I_AVStream_Client_MediaFoundation_Streamer::READER_TASK_T,           // reader type
                            Test_I_AVStream_Client_MediaFoundation_Streamer,                          // writer type
                            Test_I_AVStream_Client_MediaFoundation_Streamer);                         // name

DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                 // session data type
                          enum Stream_SessionMessageType,                                           // session event type
                          struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                                         // stream notification interface type
                          Test_I_AVStream_Client_MediaFoundation_Distributor::READER_TASK_T,        // reader type
                          Test_I_AVStream_Client_MediaFoundation_Distributor,                       // writer type
                          Test_I_AVStream_Client_MediaFoundation_Distributor);                      // name

//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                             // session event type
//                          struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                           // stream notification interface type
//                          Test_I_AVStream_Client_DirectShow_Module_Statistic_ReaderTask_t,     // reader type
//                          Test_I_AVStream_Client_DirectShow_Module_Statistic_WriterTask_t,     // writer type
//                          Test_I_AVStream_Client_DirectShow_StatisticReport);                  // name
//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                                  // session event type
//                          struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                                // stream notification interface type
//                          Test_I_AVStream_Client_MediaFoundation_Module_Statistic_ReaderTask_t,     // reader type
//                          Test_I_AVStream_Client_MediaFoundation_Module_Statistic_WriterTask_t,     // writer type
//                          Test_I_AVStream_Client_MediaFoundation_StatisticReport);                  // name
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_AVStream_Server_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_parser_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_AVStream_Server_Parser);                           // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_AVStream_Server_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_AVStream_Server_Splitter);                         // writer type

DATASTREAM_MODULE_DUPLEX_A (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                 // session data type
                            enum Stream_SessionMessageType,                                    // session event type
                            struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                            libacestream_default_misc_splitter_module_name_string,
                            Stream_INotify_t,                                                  // stream notification interface type
                            Test_I_AVStream_Client_ALSA_V4L_Streamer::READER_TASK_T,           // reader type
                            Test_I_AVStream_Client_ALSA_V4L_Streamer,                          // writer type
                            Test_I_AVStream_Client_ALSA_V4L_Streamer);                         // name

DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                 // session data type
                          enum Stream_SessionMessageType,                               // session event type
                          struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                             // stream notification interface type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor::READER_TASK_T,        // reader type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor,                       // writer type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor);                      // name

//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                       // session event type
//                          struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                     // stream notification interface type
//                          Test_I_AVStream_Client_Statistic_ReaderTask_t,                 // reader type
//                          Test_I_AVStream_Client_Statistic_WriterTask_t,                 // writer type
//                          Test_I_AVStream_Client_ALSA_V4L_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_Display);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_Display);                          // writer type
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                          // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,    // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                       // stream notification interface type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor_Reader_t,                 // reader type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor_Writer_t,                 // writer type
                          Test_I_AVStream_Client_ALSA_V4L_Distributor);                         // module name prefix

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                  // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Display);                            // writer type
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                       // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_Converter);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_Converter);                        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                       // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_Resize);                           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_Resize);                           // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                             // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Converter);                              // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                  // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Resize);                            // writer type
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_DirectShow_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Client_DirectShow_EventHandler);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_MediaFoundation_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Client_MediaFoundation_EventHandler);                     // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Client_ALSA_V4L_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_I_AVStream_Client_ALSA_V4L_Module_EventHandler);              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Server_DirectShow_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                             // session event type
//                          struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                           // stream notification interface type
//                          Test_I_AVStream_Server_DirectShow_Module_Statistic_ReaderTask_t,     // reader type
//                          Test_I_AVStream_Server_DirectShow_Module_Statistic_WriterTask_t,     // writer type
//                          Test_I_AVStream_Server_DirectShow_StatisticReport);                  // name
//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Server_MediaFoundation_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                                  // session event type
//                          struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                                // stream notification interface type
//                          Test_I_AVStream_Server_MediaFoundation_Module_Statistic_ReaderTask_t,     // reader type
//                          Test_I_AVStream_Server_MediaFoundation_Module_Statistic_WriterTask_t,     // writer type
//                          Test_I_AVStream_Server_MediaFoundation_StatisticReport);                  // name
#else
//DATASTREAM_MODULE_DUPLEX (Test_I_AVStream_Server_StreamSessionData,                // session data type
//                          enum Stream_SessionMessageType,                  // session event type
//                          struct Test_I_AVStream_Server_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                // stream notification interface type
//                          Test_I_AVStream_Server_Statistic_ReaderTask_t,            // reader type
//                          Test_I_AVStream_Server_Statistic_WriterTask_t,            // writer type
//                          Test_I_AVStream_Server_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                 // session data type
                              enum Stream_SessionMessageType,                                      // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                                    // stream notification interface type
                              Test_I_AVStream_Server_Direct3D_Display);                            // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Server_DirectShow_Display);                          // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_MediaFoundation_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Server_MediaFoundation_Display);                          // writer type
#else
#if defined (GUI_SUPPORT)
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                  // session data type
                             enum Stream_SessionMessageType,                         // session event type
                             struct Test_I_AVStream_Server_ModuleHandlerConfiguration,   // module handler configuration type
                             libacestream_default_vis_libav_resize_module_name_string,
                             Stream_INotify_t,                                       // stream notification interface type
                             Test_I_AVStream_Server_Resize);                             // writer type
#endif // FFMPEG_SUPPORT
#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                       // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_AVStream_Server_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_AVStream_Server_Display);                                 // writer type
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                // session data type
                             enum Stream_SessionMessageType,                           // session event type
                             struct Test_I_AVStream_Server_ModuleHandlerConfiguration, // module handler configuration type
                             libacestream_default_dev_target_alsa_module_name_string,
                             Stream_INotify_t,                                         // stream notification interface type
                             Test_I_AVStream_Server_Target_ALSA);                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_DirectShow_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_AVStream_Server_DirectShow_EventHandler);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_MediaFoundation_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AVStream_Server_MediaFoundation_EventHandler);                     // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_AVStream_Server_StreamSessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_AVStream_Server_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              Test_I_AVStream_Server_Module_EventHandler);              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
