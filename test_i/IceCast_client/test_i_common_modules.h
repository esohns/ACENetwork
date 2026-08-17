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

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"
#include "stream_dec_libav_filter.h"
#include "stream_dec_libav_hw_decoder.h"
#endif // FFMPEG_SUPPORT
#if defined (MPG123_SUPPORT)
#include "stream_dec_mp3_decoder.h"
#endif // MPG123_SUPPORT
#if defined (LIBXML2_SUPPORT)
#include "stream_module_htmlparser.h"
#include "stream_module_xpathquery.h"
#endif // LIBXML2_SUPPORT
#if defined (OPUS_SUPPORT)
#include "stream_dec_opus_decoder.h"
#endif // OPUS_SUPPORT
#if defined (VORBIS_SUPPORT)
#include "stream_dec_vorbis_decoder.h"
#endif // VORBIS_SUPPORT
#if defined (WEBM_SUPPORT)
//#include "stream_dec_libwebm_demuxer.h"
#include "stream_dec_libwebm_demuxer_2.h"
#endif // WEBM_SUPPORT
#include "stream_dec_wav_encoder.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_target_wasapi.h"
#else
#include "stream_dev_target_alsa.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_file_sink.h"

#include "stream_lib_tagger.h"

#include "stream_misc_defragment.h"
#include "stream_misc_delay.h"
#include "stream_misc_distributor.h"
#include "stream_misc_injector.h"
#include "stream_misc_media_splitter.h"
#include "stream_misc_messagehandler.h"
#include "stream_misc_queue_target.h"

#include "stream_stat_statistic_report.h"

#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_SUPPORT
#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#if defined (PROJECTM_SUPPORT)
#include "stream_vis_projectm.h"
#endif // PROJECTM_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#else
#if defined (WAYLAND_SUPPORT)
#include "stream_vis_wayland_window.h"
#endif // WAYLAND_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "http_common.h"
#include "http_network.h"

#include "test_i_common.h"

#include "test_i_icecast_client_stream_common.h"

// forward declarations
class Test_I_SessionMessage;
class Test_I_Message;

// declare module(s)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_IceCastClient_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                       // stream notification interface type
                          Test_I_StatisticReport_ReaderTask_t,                    // reader type
                          Test_I_StatisticReport_WriterTask_t,                    // writer type
                          Test_I_StatisticReport);                                // name

typedef Stream_Module_Defragment_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Message,
                                   Test_I_SessionMessage> Test_I_Defragment;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,                 // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_defragment_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_Defragment);                                      // writer type

//////////////////////////////////////////

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage_2,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_ReaderTask_2_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage_2,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_WriterTask_2_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData_2,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                       // stream notification interface type
                          Test_I_StatisticReport_ReaderTask_2_t,                    // reader type
                          Test_I_StatisticReport_WriterTask_2_t,                    // writer type
                          Test_I_StatisticReport_2);                                // name

#if defined (MPG123_SUPPORT)
typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2,
                                    Test_I_IceCastClient_SessionData_2_t> Test_I_MP3_Decoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MP3_Decoder);                                      // writer type
#endif // MPG123_SUPPORT

#if defined (OPUS_SUPPORT)
typedef Stream_Decoder_OpusDecoder_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     Test_I_IceCastClient_SessionData_2_t> Test_I_Opus_Decoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_opus_decoder_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Opus_Decoder);                                     // writer type
#endif // OPUS_SUPPORT

#if defined (VORBIS_SUPPORT)
typedef Stream_Decoder_VorbisDecoder_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage_2,
                                       Test_I_IceCastClient_SessionData_2_t> Test_I_Vorbis_Decoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                  // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,   // module handler configuration type
                              libacestream_default_dec_vorbis_decoder_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Vorbis_Decoder);                                     // writer type
#endif // VORBIS_SUPPORT

#if defined (WEBM_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_LibWebM_2_Demuxer_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage_2,
                                           struct _AMMediaType> Test_I_WebM_Demuxer;
#else
typedef Stream_Decoder_LibWebM_2_Demuxer_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage_2,
                                           struct Stream_MediaFramework_ALSA_MediaType> Test_I_WebM_Demuxer;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_libwebm_2_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_WebM_Demuxer);                                     // writer type
#endif // WEBM_SUPPORT

typedef Stream_Miscellaneous_MediaSplitter_T<ACE_MT_SYNCH,
                                             struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                             Stream_ControlMessage_t,
                                             Test_I_Message,
                                             Test_I_SessionMessage_2,
                                             Test_I_IceCastClient_SessionData_2_t> Test_I_MediaSplitter;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                   // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,    // module handler configuration type
                              libacestream_default_misc_media_splitter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_MediaSplitter);                                       // writer type

// video
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     Test_I_IceCastClient_SessionData_2_t,
                                     struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                 // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,  // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVDecoder);                                      // writer type

typedef Stream_LibAV_HW_Decoder_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                  Stream_ControlMessage_t,
                                  Test_I_Message,
                                  Test_I_SessionMessage_2,
                                  Test_I_IceCastClient_SessionData_2_t,
                                  struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAV_HW_Decoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                    // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,     // module handler configuration type
                              libacestream_default_dec_libav_hw_decoder_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Test_I_LibAV_HW_Decoder);                                     // writer type
#endif // FFMPEG_SUPPORT

#if defined (FFMPEG_SUPPORT)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_TaskBaseSynch_t;

typedef Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVConvert;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                   // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,    // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_LibAVConvert);                                        // writer type

typedef Stream_Visualization_LibAVResize_T<Test_I_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVResize;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                 // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,  // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVResize);                                       // writer type
#endif // FFMPEG_SUPPORT

// audio
typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage_2,
                                                      Test_I_IceCastClient_SessionData_2_t> Test_I_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage_2,
                                                      Test_I_IceCastClient_SessionData_2_t> Test_I_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData_2,                // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_I_Distributor_Reader_t,                              // reader task
                          Test_I_Distributor_Writer_t,                              // writer task
                          Test_I_Distributor);                                      // name

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVFilter_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     Test_I_IceCastClient_SessionData_2_t,
                                     struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVResampler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_libav_filter_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_LibAVResampler);                                   // writer type
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                   Stream_ControlMessage_t,
                                   Test_I_Message,
                                   Test_I_SessionMessage_2,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct _AMMediaType> Test_I_WASAPIOut;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                 // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,  // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_WASAPIOut);                                         // writer type

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                              Stream_ControlMessage_t,
                              Test_I_Message,
                              Test_I_SessionMessage_2,
                              struct _AMMediaType,
                              struct Stream_UserData> Test_I_Delay;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Delay);                                            // writer type

#if defined (GTK_SUPPORT)
typedef Common_Math_FFT_T<float, FFT_ALGORITHM_UNKNOWN> Common_Math_FFT_t;
typedef Common_ISetP_T<GdkWindow> Common_ISetP_t;
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                          Stream_ControlMessage_t,
                                                          Test_I_Message,
                                                          Test_I_SessionMessage_2,
                                                          struct Test_I_IceCastClient_SessionData_2,
                                                          Test_I_IceCastClient_SessionData_2_t,
                                                          Common_Timer_Manager_t,
                                                          struct _AMMediaType,
                                                          float,
                                                          FFT_ALGORITHM_UNKNOWN> Test_I_Vis_SpectrumAnalyzer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                     // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,      // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_Vis_SpectrumAnalyzer);                                  // writer type
#endif // GTK_SUPPORT

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2,
                                    Test_I_IceCastClient_SessionData_2_t,
                                    struct Test_I_IceCastClient_SessionData_2,
                                    struct _AMMediaType,
                                    struct Stream_UserData> Test_I_WAV_Encoder;

typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     Test_I_IceCastClient_SessionData_2,
                                     Test_I_IceCastClient_SessionData_2_t,
                                     struct _AMMediaType> Test_I_Direct3d;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Direct3d);                                         // writer type
#else
typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage_2,
                                 struct Test_I_IceCastClient_SessionData_2> Test_I_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dev_target_alsa_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_ALSA);                                             // writer type

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                              Stream_ControlMessage_t,
                              Test_I_Message,
                              Test_I_SessionMessage_2,
                              struct Stream_MediaFramework_ALSA_MediaType,
                              struct Stream_UserData> Test_I_Delay;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Delay);                                            // writer type

#if defined (GTK_SUPPORT)
typedef Common_Math_FFT_T<float, FFT_ALGORITHM_UNKNOWN> Common_Math_FFT_t;
typedef Common_ISetP_T<GdkWindow> Common_ISetP_t;
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                                          Stream_ControlMessage_t,
                                                          Test_I_Message,
                                                          Test_I_SessionMessage_2,
                                                          struct Test_I_IceCastClient_SessionData_2,
                                                          Test_I_IceCastClient_SessionData_2_t,
                                                          Common_Timer_Manager_t,
                                                          struct Stream_MediaFramework_ALSA_MediaType,
                                                          float,
                                                          FFT_ALGORITHM_UNKNOWN> Test_I_Vis_SpectrumAnalyzer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                     // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,      // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_Vis_SpectrumAnalyzer);                                  // writer type
#endif // GTK_SUPPORT

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2,
                                    Test_I_IceCastClient_SessionData_2_t,
                                    struct Test_I_IceCastClient_SessionData_2,
                                    struct Stream_MediaFramework_ALSA_MediaType,
                                    struct Stream_UserData> Test_I_WAV_Encoder;

#if defined (WAYLAND_SUPPORT)
typedef Stream_Module_Vis_Wayland_Window_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage_2,
                                           Test_I_IceCastClient_SessionData_2_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Test_I_Wayland;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                  // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,   // module handler configuration type
                              libacestream_default_vis_wayland_window_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Wayland);                                            // writer type
#endif // WAYLAND_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_WAV_Encoder);                                      // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                   Stream_ControlMessage_t,
                                   Test_I_Message,
                                   Test_I_SessionMessage_2> Test_I_FileSink;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2, // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Test_I_FileSink);                          // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (PROJECTM_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Vis_ProjectM_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     struct _AMMediaType> Test_I_Vis_ProjectM;
#else
typedef Stream_Module_Vis_ProjectM_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     struct Stream_MediaFramework_ALSA_MediaType> Test_I_Vis_ProjectM;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_vis_projectm_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Vis_ProjectM);                                     // writer type
#endif // PROJECTM_SUPPORT

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage,
                                       struct Test_I_IceCastClient_SessionData,
                                       struct Stream_UserData> Test_I_Event_Handler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Event_Handler);                                       // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage_2,
                                       struct Test_I_IceCastClient_SessionData_2,
                                       struct Stream_UserData> Test_I_Event_Handler_2;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                   // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Event_Handler_2);                                     // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_IceCastClient_ModuleHandlerConfiguration_3,
                                       Stream_ControlMessage_t,
                                       Test_I_Message_3,
                                       Test_I_SessionMessage,
                                       struct Test_I_IceCastClient_SessionData,
                                       struct Stream_UserData> Test_I_Event_Handler_3;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_3,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Event_Handler_3);                                     // writer type

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_UserData> Test_I_AudioTagger;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_AudioTagger);                                      // writer type

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_UserData> Test_I_VideoTagger;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_VideoTagger);                                      // writer type

typedef Stream_Module_QueueWriter_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage_2,
                                    struct Stream_UserData> Test_I_QueueTarget;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_misc_queue_sink_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_QueueTarget);                                      // writer type

typedef Stream_Module_Injector_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage_2,
                                 struct Stream_UserData> Test_I_Video_Injector;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_misc_injector_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Video_Injector);                                   // writer type

//////////////////////////////////////////

typedef Stream_Module_HTMLParser_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration_3,
                                   Stream_ControlMessage_t,
                                   Test_I_Message_3,
                                   Test_I_SessionMessage,
                                   Test_I_IceCastClient_SessionData_t,
                                   struct Test_I_IceCastClient_SessionData,
                                   struct Test_I_SAXParserContext> Test_I_HTML_Parser;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_3, // module handler configuration type
                              libacestream_default_html_parser_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_HTML_Parser);                                      // writer type

typedef Stream_Module_XPathQuery_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration_3,
                                   Stream_ControlMessage_t,
                                   Test_I_Message_3,
                                   Test_I_SessionMessage,
                                   Test_I_IceCastClient_SessionData_t,
                                   struct Test_I_IceCastClient_SessionData> Test_I_XPath_Query;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_3, // module handler configuration type
                              libacestream_default_xpath_query_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_XPath_Query);                                      // writer type

#endif
