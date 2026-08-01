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
#include "stream_dec_libav_filter.h"
#endif // FFMPEG_SUPPORT
#include "stream_dec_mp3_decoder.h"
#include "stream_dec_wav_encoder.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_target_wasapi.h"
#else
#include "stream_dev_target_alsa.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_file_sink.h"

#include "stream_misc_defragment.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#include "stream_module_source_http_get.h"

#include "stream_vis_gtk_pixbuf.h"

#include "http_common.h"
#include "http_module_parser.h"
#include "http_module_streamer.h"
#include "http_network.h"
#include "http_parser_driver.h"

#include "test_i_common.h"

#include "test_i_icecast_client_stream_common.h"

// forward declarations
class Test_I_SessionMessage;
class Test_I_Message;

// declare module(s)
typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage> Test_I_HTTPStreamer;
typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct Test_I_IceCastClient_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_I_Message,
                             Test_I_SessionMessage,
                             HTTP_ParserDriver_T<ACE_MT_SYNCH,
                                                 Common_TimePolicy_t,
                                                 Test_I_SessionMessage> > Test_I_HTTPParser;
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData,                // session data type
                         enum Stream_SessionMessageType,                         // session event type
                         struct Test_I_IceCastClient_ModuleHandlerConfiguration, // module handler configuration type
                         libacenetwork_protocol_default_http_parser_module_name_string,
                         Stream_INotify_t,                                       // stream notification interface type
                         Test_I_HTTPStreamer,                                    // reader type
                         Test_I_HTTPParser,                                      // writer type
                         Test_I_HTTPMarshal);                                    // name

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

//typedef Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            struct Test_I_IceCastClient_ModuleHandlerConfiguration,
//                                            Stream_ControlMessage_t,
//                                            Test_I_Message,
//                                            Test_I_SessionMessage> Test_I_HTTPGet;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData,   // session data type
//                              enum Stream_SessionMessageType,            // session event type
//                              struct Test_I_IceCastClient_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_net_http_get_module_name_string,
//                              Stream_INotify_t,                          // stream notification interface type
//                              Test_I_HTTPGet);                           // writer type

//////////////////////////////////////////

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_2> Test_I_HTTPStreamer_2;
typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                             Stream_ControlMessage_t,
                             Test_I_Message,
                             Test_I_SessionMessage_2,
                             HTTP_ParserDriver_T<ACE_MT_SYNCH,
                                                 Common_TimePolicy_t,
                                                 Test_I_SessionMessage_2> > Test_I_HTTPParser_2;
DATASTREAM_MODULE_DUPLEX (struct Test_I_IceCastClient_SessionData_2,                // session data type
                         enum Stream_SessionMessageType,                         // session event type
                         struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                         libacenetwork_protocol_default_http_parser_module_name_string,
                         Stream_INotify_t,                                       // stream notification interface type
                         Test_I_HTTPStreamer_2,                                    // reader type
                         Test_I_HTTPParser_2,                                      // writer type
                         Test_I_HTTPMarshal_2);                                    // name

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

typedef Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                            Stream_ControlMessage_t,
                                            Test_I_Message,
                                            Test_I_SessionMessage_2> Test_I_HTTPGet_2;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_net_http_get_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_HTTPGet_2);                                        // writer type

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
                              libacestream_default_net_http_get_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MP3_Decoder);                                      // writer type
#endif // MPG123_SUPPORT

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVFilter_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_2,
                                     Test_I_IceCastClient_SessionData_2_t,
                                     struct _AMMediaType> Test_I_LibAVResampler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_libav_filter_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_LibAVResampler);                                   // writer type
#endif // FFMPEG_SUPPORT

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
#else
typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage_2,
                                 Test_I_IceCastClient_SessionData_2_t> Test_I_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dev_target_alsa_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_ALSA);                                             // writer type

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
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2,                // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_WAV_Encoder);                                      // writer type

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_IceCastClient_ModuleHandlerConfiguration_2,
                                   Stream_ControlMessage_t,
                                   Test_I_Message,
                                   Test_I_SessionMessage_2> Test_I_FileSink;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_IceCastClient_SessionData_2, // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct Test_I_IceCastClient_ModuleHandlerConfiguration_2, // module handler configuration type
                              libacestream_default_net_http_get_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Test_I_FileSink);                          // writer type

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

#endif
