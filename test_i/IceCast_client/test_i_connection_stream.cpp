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

#include "stream_misc_defines.h"

#include "stream_net_http_defines.h"

#include "net_macros.h"

// #include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_common_modules.h"
#include "test_i_m3u_module_parser.h"
#include "test_i_module_httpget.h"
#include "test_i_module_httpparser.h"

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
                                             ACE_TEXT_ALWAYS_CHAR ("Marshal")),
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
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::Test_I_ConnectionStream_2"));

}

bool
Test_I_ConnectionStream_2::load (Stream_ILayout* layout_in,
                                 bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::load"));

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  deleteModules_out = true;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_2_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR ("Marshal")),
                  false);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPGet_2_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (MPG123_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_MP3_Decoder_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // MPG123_SUPPORT

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;
  ACE_NEW_RETURN (module_p,
                  Test_I_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
  //branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
#if defined (PROJECTM_SUPPORT)
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DECODE_NAME));
#endif // PROJECTM_SUPPORT
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_LibAVResampler_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_I_WASAPIOut_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_Module (this,
                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  ++index_i;

  ACE_NEW_RETURN (module_p,
                  Test_I_WAV_Encoder_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_FileSink_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

#if defined (PROJECTM_SUPPORT)
  ++index_i;

#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_U_MicVisualize_DirectShow_LibAVResampler_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING)),
                  false);
#elif defined (SOX_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_U_MicVisualize_DirectShow_SoXResampler_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR ("SoX_Resampler_2")),
                  false);
#endif // FFMPEG_SUPPORT || SOX_SUPPORT
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_U_MicVisualize_DirectShow_Vis_ProjectM_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_PROJECTM_DEFAULT_NAME_STRING)),
                    false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_p, index_i);
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

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_IceCastClient_SessionData_2* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_SessionManager_2* session_manager_p =
    Test_I_SessionManager_2::SINGLETON_T::instance ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  media_type_s.majortype = MEDIATYPE_Audio;
  media_type_s.subtype = MEDIASUBTYPE_PCM;
  media_type_s.bFixedSizeSamples = TRUE;
  media_type_s.bTemporalCompression = FALSE;
  media_type_s.formattype = FORMAT_WaveFormatEx;
  media_type_s.cbFormat = sizeof (struct tWAVEFORMATEX);
  media_type_s.pbFormat =
    reinterpret_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tWAVEFORMATEX)));
  ACE_ASSERT (media_type_s.pbFormat);
  ACE_OS::memset (media_type_s.pbFormat, 0, sizeof (struct tWAVEFORMATEX));
  struct tWAVEFORMATEX* wave_format_ex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
  wave_format_ex_p->wFormatTag = WAVE_FORMAT_PCM;
  wave_format_ex_p->nChannels = 2;
  wave_format_ex_p->nSamplesPerSec = 44100;
  wave_format_ex_p->nAvgBytesPerSec = 176400;
  wave_format_ex_p->nBlockAlign = 4;
  wave_format_ex_p->wBitsPerSample = 16;
  wave_format_ex_p->cbSize = 0;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

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
    &const_cast<struct Test_I_IceCastClient_SessionData_2&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_front (media_type_s);

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
