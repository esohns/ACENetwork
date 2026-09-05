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
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_audio_decoder.h"
#include "stream_dec_libav_decoder.h"
#include "stream_dec_libav_hw_decoder.h"
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_target_wasapi.h"
#else
#include "stream_dev_target_alsa.h"

#if defined (LIBPIPEWIRE_SUPPORT)
#include "stream_dev_target_pipewire.h"
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_lib_tagger.h"

#include "stream_misc_delay.h"
#include "stream_misc_injector.h"
#include "stream_misc_media_splitter.h"
#include "stream_misc_messagehandler.h"
#include "stream_misc_queue_source.h"
#include "stream_misc_queue_target.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#endif // GTK_SUPPORT

#include "http_common.h"
#include "http_module_parser.h"
#include "http_module_streamer.h"
#if defined (REFLEX_USE)
#include "http_reflex_parser_driver.h"
#elif defined (ANTLR_USE)
#undef emit
#undef METHOD
#include "http_antlr_parser_driver.h"
#else
#include "http_parser_driver.h"
#endif // REFLEX_USE || ANTLR_USE

#include "test_i_common.h"

#include "test_i_url_stream_load_stream_common.h"

// forward declarations
class Test_I_SessionMessage;
class Test_I_Message;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_URLStreamLoad_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_SessionManager_t;

// declare module(s)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_Message,
                                Test_I_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_TaskBaseAsynch_t;

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage> Test_I_HTTPStreamer;
#if defined (REFLEX_USE)
typedef HTTP_ReflexParserDriver_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  Test_I_SessionMessage> HTTP_ParserDriver_t;
#elif defined (ANTLR_USE)
typedef HTTP_ANTLRParserDriver_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 Test_I_SessionMessage> HTTP_ParserDriver_t;
#else
typedef HTTP_ParserDriver_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            Test_I_SessionMessage> HTTP_ParserDriver_t;
#endif // REFLEX_USE || ANTLR_USE
typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_I_Message,
                             Test_I_SessionMessage,
                             HTTP_ParserDriver_t> Test_I_HTTPParser;
DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
                         enum Stream_SessionMessageType,                         // session event type
                         struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                         libacenetwork_protocol_default_http_parser_module_name_string,
                         Stream_INotify_t,                                       // stream notification interface type
                         Test_I_HTTPStreamer,                                    // reader type
                         Test_I_HTTPParser,                                      // writer type
                         Test_I_HTTPMarshal);                                    // name

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                       // stream notification interface type
                          Test_I_StatisticReport_ReaderTask_t,                    // reader type
                          Test_I_StatisticReport_WriterTask_t,                    // writer type
                          Test_I_StatisticReport);                                // name

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_UserData> Test_I_Audio_Tagger;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Audio_Tagger);                                   // writer type

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_UserData> Test_I_Video_Tagger;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Video_Tagger);                                   // writer type

typedef Stream_Module_Injector_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage,
                                 struct Stream_UserData> Test_I_Audio_Injector;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_injector_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Audio_Injector);                                 // writer type

typedef Stream_Module_QueueWriter_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage,
                                    struct Stream_UserData> Test_I_QueueTarget;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                 // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_queue_sink_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_QueueTarget);                                     // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage,
                                       struct Test_I_URLStreamLoad_SessionData,
                                       struct Stream_UserData> Test_I_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_MessageHandler);                                      // writer type

//////////////////////////////////////////

typedef Stream_Module_QueueReader_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage,
                                    struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_URLStreamLoad_StreamState,
                                    struct Stream_Statistic,
                                    Test_I_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Test_I_QueueSource;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_misc_queue_source_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_QueueSource);                                         // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                       // stream notification interface type
                          Test_I_StatisticReport_ReaderTask_t,                    // reader type
                          Test_I_StatisticReport_WriterTask_t,                    // writer type
                          Test_I_StatisticReport);                                // name

typedef Stream_Miscellaneous_MediaSplitter_T<ACE_MT_SYNCH,
                                             struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_I_Message,
                                             Test_I_SessionMessage,
                                             Test_I_URLStreamLoad_SessionData_t> Test_I_Splitter_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                     // session data type
                          enum Stream_SessionMessageType,                              // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,      // module handler configuration type
                          libacestream_default_misc_media_splitter_module_name_string,
                          Stream_INotify_t,                                            // stream notification interface type
                          Test_I_Splitter_Writer_t::READER_TASK_T,                     // reader type
                          Test_I_Splitter_Writer_t,                                    // writer type
                          Test_I_Splitter);                                            // module name prefix

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_SessionMessage,
                                                      Test_I_URLStreamLoad_SessionData_t> Test_I_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_I_Distributor_Writer_t::READER_TASK_T,               // reader type
                          Test_I_Distributor_Writer_t,                              // writer type
                          Test_I_Distributor);                                      // module name prefix

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVAudioDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage,
                                           Test_I_URLStreamLoad_SessionData_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_AudioDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,          // module handler configuration type
                              libacestream_default_dec_libav_audio_decoder_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_AudioDecoder);                                            // writer type

typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message,
                                      Test_I_SessionMessage,
                                      Test_I_URLStreamLoad_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_VideoDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_VideoDecoder);                                      // writer type

typedef Stream_LibAV_HW_Decoder_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_I_Message,
                                  Test_I_SessionMessage,
                                  Test_I_URLStreamLoad_SessionData_t,
                                  struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_VideoHWDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_dec_libav_hw_decoder_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Test_I_VideoHWDecoder);                                       // writer type

typedef Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_VideoConverter;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_VideoConverter);                                      // writer type

typedef Stream_Visualization_LibAVResize_T<Test_I_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_VideoResize;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_VideoResize);                                      // writer type
#endif // FFMPEG_SUPPORT

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_I_Message,
                              Test_I_SessionMessage,
                              struct Stream_MediaFramework_FFMPEG_MediaType,
                              struct Stream_UserData> Test_I_VideoDelay;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_VideoDelay);                                     // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Message,
                                   Test_I_SessionMessage,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_WASAPIOut;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_WASAPIOut);                                         // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message,
                                      Test_I_SessionMessage,
                                      struct Test_I_URLStreamLoad_SessionData,
                                      Test_I_URLStreamLoad_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_GTK_Cairo;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_URLStreamLoad_SessionData,                // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_GTK_Cairo);                                      // writer type
#endif // GTK_SUPPORT

#endif
