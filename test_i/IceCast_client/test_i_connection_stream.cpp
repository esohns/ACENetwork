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
#include "stdafx.h"

#include "test_i_connection_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmreg.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_dec_defines.h"

#include "stream_html_defines.h"

#include "stream_misc_defines.h"

#include "stream_net_http_defines.h"

#include "net_macros.h"

#include "test_i_session_message.h"
#include "test_i_common_modules.h"
#include "test_i_m3u_module_parser.h"
#include "test_i_module_encoder.h"
#include "test_i_module_httpget.h"
#include "test_i_module_httpparser.h"
#include "test_i_icecast_client_defines.h"

Test_I_ConnectionStream::Test_I_ConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::Test_I_ConnectionStream"));

}

bool
Test_I_ConnectionStream::load (Stream_ILayout* layout_in,
                               bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream::load"));

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Defragment_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_M3U_Module_Parser_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                 Test_I_HTTPGet_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                 false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //ACE_NEW_RETURN (module_p,
  //                Test_I_Module_Dump_Module (this,
  //                                           ACE_TEXT_ALWAYS_CHAR ("Dump")),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  deleteModules_out = true;

  return true;
}

bool
Test_I_ConnectionStream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                     ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_IceCastClient_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct Test_I_IceCastClient_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_p->sessionID = configuration_in.sessionID;
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::STREAM_BASE_T::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

//////////////////////////////////////////

Test_I_ConnectionStream_2::Test_I_ConnectionStream_2 ()
 : inherited ()
#if defined (FFMPEG_SUPPORT)
 , inherited2 ()
#endif // FFMPEG_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::Test_I_ConnectionStream_2"));

}

bool
Test_I_ConnectionStream_2::load (Stream_ILayout* layout_in,
                                 bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::load"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);

  typename inherited::MODULE_T* branch_p = NULL, *branch_2 = NULL, *branch_3 = NULL; // NULL: 'main' branch
  Stream_IDistributorModule *idistributor_p = NULL, *idistributor_2 = NULL, *idistributor_3 = NULL;
  unsigned int index_i = 0, index_2 = 0, index_3 = 0;
  Stream_Branches_t branches_a;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Test_I_SessionManager_2* session_manager_p =
    Test_I_SessionManager_2::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  struct Test_I_IceCastClient_SessionData_2& session_data_r =
    const_cast<struct Test_I_IceCastClient_SessionData_2&> (session_manager_p->getR (inherited::id_));
  bool may_have_video_b = false;

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  deleteModules_out = true;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_2_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                 Test_I_StatisticReport_2_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                 false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPGet_2_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_s.nChannels = 2;
  waveformatex_s.nSamplesPerSec = 44100;
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_s;
  media_type_s.channels = 2;
  media_type_s.rate = 44100;
  // media_type_s.subFormat = SND_PCM_SUBFORMAT_STD;
#endif // ACE_WIN32 || ACE_WIN64

  if (Common_String_Tools::endswith (inherited::configuration_->configuration_->URL,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_OGG_SUFFIX)))
  {
#if defined (VORBIS_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_Vorbis_Decoder_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_VORBIS_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
#endif // VORBIS_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveformatex_s.wBitsPerSample = 32;
#else
    media_type_s.format = SND_PCM_FORMAT_FLOAT;
#endif
  } // end IF
  else if (Common_String_Tools::endswith (inherited::configuration_->configuration_->URL,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_OPUS_SUFFIX)))
  {
#if defined (OPUS_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_Opus_Decoder_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_OPUS_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
#endif // OPUS_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveformatex_s.wBitsPerSample = 32;
    waveformatex_s.nSamplesPerSec = 48000;
#else
    media_type_s.format = SND_PCM_FORMAT_FLOAT;
    media_type_s.rate = 48000;
#endif
  } // end IF
  else if (Common_String_Tools::endswith (inherited::configuration_->configuration_->URL,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_WEBM_SUFFIX)))
  { may_have_video_b = true;
#if defined (WEBM_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_WebM_Demuxer_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_WEBM_DEMUXER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
#endif // WEBM_SUPPORT

    ACE_NEW_RETURN (module_p,
                    Test_I_MediaSplitter_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MEDIASPLITTER_DEFAULT_NAME_STRING)),
                    false);
    branch_p = module_p;
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  //  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
  //#if defined (PROJECTM_SUPPORT)
  //  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DECODE_NAME));
  //#endif // PROJECTM_SUPPORT
    idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (branches_a);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;

    // audio
#if defined (VORBIS_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_Vorbis_Decoder_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_VORBIS_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
#endif // VORBIS_SUPPORT

    ACE_NEW_RETURN (module_p,
                    Test_I_AudioTagger_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ++index_i;

    // video
#if defined (FFMPEG_SUPPORT)
    if ((*iterator).second.second->codecConfiguration->deviceType != AV_HWDEVICE_TYPE_NONE)
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_LibAV_HW_Decoder_Module (this,
                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_HW_DECODER_DEFAULT_NAME_STRING)),
                      false);
    } // end IF
    else
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_LibAVDecoder_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING)),
                      false);
    } // end ELSE
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
#endif // FFMPEG_SUPPORT

    ACE_NEW_RETURN (module_p,
                    Test_I_Distributor_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    branch_3 = module_p;
    branches_a.clear ();
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
    idistributor_3 =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_3);
    idistributor_3->initialize (branches_a);
    layout_in->append (module_p, branch_p, index_i); // 1: video branch
    module_p = NULL;

    // save
    ACE_NEW_RETURN (module_p,
                    Test_I_LibAVConvert_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_VideoTagger_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_QueueTarget_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;

    ++index_3;

    // display
    if (unlikely (!inherited::configuration_->configuration_->displayVideo))
      goto continue_;

    ACE_NEW_RETURN (module_p,
                    Test_I_Delay_Module (this,
                                         ACE_TEXT_ALWAYS_CHAR ("Delay_2")),
                    false);
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;

#if defined (FFMPEG_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_LibAVResize_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_NEW_RETURN (module_p,
                    Test_I_Direct3d_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING)),
                    false);
#else
    ACE_NEW_RETURN (module_p,
                    Test_I_Wayland_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_WAYLAND_WINDOW_DEFAULT_NAME_STRING)),
                    false);
#endif // ACE_WIN32 || ACE_WIN64
    layout_in->append (module_p, branch_3, index_3);
    module_p = NULL;

continue_:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveformatex_s.wBitsPerSample = 32;
    //waveformatex_s.nSamplesPerSec = 48000;
#else
    media_type_s.format = SND_PCM_FORMAT_FLOAT;
    //media_type_s.rate = 48000;
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  { // --> assume mp3 stream
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: \"%s\" apparently not an OGG/Vorbis stream; assuming MP3....\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (inherited::configuration_->configuration_->URL.c_str ())));

#if defined (MPG123_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_MP3_Decoder_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
#endif // MPG123_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
    waveformatex_s.wBitsPerSample = 16;
#else
    media_type_s.format = SND_PCM_FORMAT_S16;
#endif
  } // end ELSE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  waveformatex_s.nBlockAlign =
    (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
  waveformatex_s.nAvgBytesPerSec =
    (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
  if (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                 media_type_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (session_data_r.formats.empty ());
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_final_s;
  inherited2::getMediaType (media_type_s,
                            STREAM_MEDIATYPE_AUDIO,
                            media_type_final_s.audio);
  // *TODO*: cannot set these in advance; must be deduced at runtime and
  //         notified by corresponding 'resize' session message(s)...
  // *TODO*: this needs to be NV12 because the encoder sets the input format
  //         during session initialization (!); not upon arrival of the first
  //         data, when the decoder sends a resize notification
  media_type_final_s.video.format = AV_PIX_FMT_NV12;
  media_type_final_s.video.frameRate =
    { TEST_I_ICECAST_CLIENT_DEFAULT_INPUT_FRAMERATE, 1 };
  media_type_final_s.video.resolution = { 1920, 1080 };
  session_data_r.formats.push_front (media_type_final_s);
#else
  session_data_r.formats.push_front (media_type_s);
#endif // FFMPEG_SUPPORT

  ACE_NEW_RETURN (module_p,
                  Test_I_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  branch_2 = module_p;
  branches_a.clear ();
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
#if defined (PROJECTM_SUPPORT)
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DECODE_NAME));
#endif // PROJECTM_SUPPORT
  idistributor_2 =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_2);
  idistributor_2->initialize (branches_a);
  layout_in->append (module_p, branch_p, 0); // 0: audio branch
  module_p = NULL;

#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_LibAVResampler_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;
#endif // FFMPEG_SUPPORT

  switch (inherited::configuration_->configuration_->renderer)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_WASAPIOut_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
#else
    case STREAM_DEVICE_RENDERER_ALSA:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_ALSA_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case STREAM_DEVICE_RENDERER_PIPEWIRE:
    {
#if defined (LIBPIPEWIRE_SUPPORT)
      ACE_NEW_RETURN (module_p,
                      Test_I_Pipewire_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_PIPEWIRE_DEFAULT_NAME_STRING)),
                      false);
#endif // LIBPIPEWIRE_SUPPORT
      break;
    }
#endif // ACE_WIN32 || ACE_WIN64
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;

  ++index_2;

  ACE_NEW_RETURN (module_p,
                  Test_I_Delay_Module (this,
                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Vis_SpectrumAnalyzer_Module (this,
                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;

  ++index_2;

  if (unlikely (may_have_video_b))
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Video_Injector_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_INJECTOR_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

#if defined (FFMPEG_SUPPORT)
    ACE_NEW_RETURN (module_p,
                    Test_I_Encoder_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_ENCODER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;
#endif // FFMPEG_SUPPORT

    // need to patch output filename *.wav --> *.mp4
    if (Common_String_Tools::endswith (Common_String_Tools::tolower ((*iterator).second.second->targetFileName),
                                       ACE_TEXT_ALWAYS_CHAR (".wav")))
    {
      std::string target_filename_string =
        Common_File_Tools::cropExtension ((*iterator).second.second->targetFileName);
      target_filename_string += ACE_TEXT_ALWAYS_CHAR (".mp4");
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: adjusting target filename \"%s\" to \"%s\"....\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_File_Tools::basename ((*iterator).second.second->targetFileName).c_str ()),
                  ACE_TEXT (Common_File_Tools::basename (target_filename_string).c_str ())));
      (*iterator).second.second->targetFileName = target_filename_string;
    } // end IF
  } // end IF
  else
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_WAV_Encoder_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_NEW_RETURN (module_p,
                    Test_I_FileSink_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  }

#if defined (PROJECTM_SUPPORT)
  ++index_2;

#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_LibAVResampler_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR ("LibAV_Filter_2")),
                  false);
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;
#endif // FFMPEG_SUPPORT

  ACE_NEW_RETURN (module_p,
                  Test_I_Vis_ProjectM_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_PROJECTM_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_2, index_2);
  module_p = NULL;
#endif // PROJECTM_SUPPORT

  return true;
}

bool
Test_I_ConnectionStream_2::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                       ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  // sanity check(s)
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_IceCastClient_SessionData_2* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  Test_I_SessionManager_2* session_manager_p =
    Test_I_SessionManager_2::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct Test_I_IceCastClient_SessionData_2&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  (*iterator).second.second->resize = this;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline = setup_pipeline;
  if (!inherited::STREAM_BASE_T::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

void
Test_I_ConnectionStream_2::resize (const Common_Image_Resolution_t& resolution_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::resize"));

  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  (*iterator).second.second->outputFormat.video.resolution = resolution_in;

  inherited::notify (STREAM_SESSION_MESSAGE_RESIZE,
                     false,
                     true);
}

//////////////////////////////////////////

Test_I_ConnectionStream_3::Test_I_ConnectionStream_3 ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_3::Test_I_ConnectionStream_3"));

}

bool
Test_I_ConnectionStream_3::load (Stream_ILayout* layout_in,
                                 bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream_3::load"));

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_3_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Defragment_3_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                 Test_I_HTTPGet_3_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                 false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_HTML_Parser_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (MODULE_HTML_PARSER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_XPath_Query_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (MODULE_XPATH_QUERY_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //ACE_NEW_RETURN (module_p,
  //                Test_I_Module_Dump_Module (this,
  //                                           ACE_TEXT_ALWAYS_CHAR ("Dump")),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  deleteModules_out = true;

  return true;
}

bool
Test_I_ConnectionStream_3::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                       ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_3::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_IceCastClient_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct Test_I_IceCastClient_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline = setup_pipeline;
  if (!inherited::STREAM_BASE_T::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}
