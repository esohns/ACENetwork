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

#include "test_i_stream.h"

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"

#include "stream_misc_defines.h"

#include "stream_vis_defines.h"

#include "test_i_common_modules.h"

Test_I_AVStream::Test_I_AVStream ()
    : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream::Test_I_AVStream"));

}

bool
Test_I_AVStream::load (Stream_ILayout* layout_in,
                       bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream::load"));

  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  bool save_to_file_b = !(*iterator).second.second->targetFileName.empty ();
  Stream_Branches_t branches_a;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_QueueSource_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

//ACE_NEW_RETURN (module_p,
//                Test_I_StatisticReport_Module (this,
//                                               ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
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
                  Test_I_MPEGTSDecoder_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Defragment_3_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Audio_Injector_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_INJECTOR_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;

  ACE_NEW_RETURN (module_p,
                  Test_I_Splitter_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MEDIASPLITTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);
  module_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_AudioDecoder_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING)),
                  false);
#endif // FFMPEG_SUPPORT
#else
#if defined (FAAD_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_FAADDecoder_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING)),
                  false);
#endif // FAAD_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  if (save_to_file_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Audio_Tagger_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_QueueTarget_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end IF

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
                 ACE_TEXT ("%s: invalid/unknown renderer (was: %d), aborting\n"),
                 ACE_TEXT (stream_name_string_4b),
                 inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  ++index_i;

//continue_:
  if (inherited::configuration_->configuration_->useHardwareDecoder)
    ACE_NEW_RETURN (module_p,
                    Test_I_VideoHWDecoder_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_HW_DECODER_DEFAULT_NAME_STRING)),
                    false);
  else
    ACE_NEW_RETURN (module_p,
                    Test_I_VideoDecoder_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING)),
                    false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Video_Tagger_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

  typename inherited::MODULE_T* branch_2 = NULL; // NULL: 'display|save' branches
  unsigned int index_2 = 0;

  if (save_to_file_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Distributor_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    branch_2 = module_p;
    branches_a.clear ();
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
    Stream_IDistributorModule* idistributor_2 =
        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_2);
    idistributor_2->initialize (branches_a);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_VideoResize_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_VideoDelay_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_GTKCairo_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

    ++index_2;

    ACE_NEW_RETURN (module_p,
                    Test_I_QueueSource_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR ("QueueSource_2")),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_LibAVConverter_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_2, index_2);
    module_p = NULL;

    // ACE_NEW_RETURN (module_p,
    //                 Test_I_Audio_Injector_Module (this,
    //                                               ACE_TEXT_ALWAYS_CHAR ("Injector_2")),
    //                 false);
    // layout_in->append (module_p, branch_2, index_2);
    // module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_AVIEncoder_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_AVI_DEFAULT_NAME_STRING)),
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

    ++index_2;
  } // end IF
  else
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_VideoResize_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_VideoDelay_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_GTKCairo_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end ELSE
  ++index_i;

  deleteModules_out = true;

  return true;
}

bool
Test_I_AVStream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream::initialize"));

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_I_WebTV_SessionData_3* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_SessionManager_3* session_manager_p =
    Test_I_SessionManager_3::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
               ACE_TEXT (stream_name_string_4b)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<Test_I_WebTV_SessionData_3&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_front (configuration_in.configuration_->mediaType);
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // if (!(*iterator).second.second->targetFileName.empty ())
  // {
  //   Stream_Module_t* module_p =
  //     const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("QueueSource_2"),
  //                                                    false,
  //                                                    false));
  //   ACE_ASSERT (module_p);
  //   module_p->arg (inherited::sessionData_);
  // } // end IF

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                 ACE_TEXT (stream_name_string_4b)));
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
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
               ACE_TEXT (stream_name_string_4b)));

  return false;
}

//////////////////////////////////////////

Test_I_AudioStream::Test_I_AudioStream()
    : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AudioStream::Test_I_AudioStream"));

}

bool
Test_I_AudioStream::load (Stream_ILayout* layout_in,
                          bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AudioStream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_QueueSource_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);

//module_p = NULL;
//ACE_NEW_RETURN (module_p,
//                Test_I_StatisticReport_Module (this,
//                                               ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
//                false);
//layout_in->append (module_p, NULL, 0);

  ACE_NEW_RETURN (module_p,
                  Test_I_Audio_Tagger_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);

  ACE_NEW_RETURN (module_p,
                  Test_I_QueueTarget_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);

  deleteModules_out = true;

  return true;
}

bool
Test_I_AudioStream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AudioStream::initialize"));

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_I_WebTV_SessionData_3* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_SessionManager_3* session_manager_p =
    Test_I_SessionManager_3::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
               ACE_TEXT (stream_name_string_4a)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<Test_I_WebTV_SessionData_3&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_front (configuration_in.configuration_->mediaType);
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                 ACE_TEXT (stream_name_string_4a)));
      goto failed;
    } // end IF

  // ---------------------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
        setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_4a)));

  return false;
}
