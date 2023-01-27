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

#include "test_i_av_stream_client_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_AVStream_Client_DirectShow_Audio_Stream::Test_I_AVStream_Client_DirectShow_Audio_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Audio_Stream::Test_I_AVStream_Client_DirectShow_Audio_Stream"));

}

bool
Test_I_AVStream_Client_DirectShow_Audio_Stream::load (Stream_ILayout* layout_inout,
                                                      bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Audio_Stream::load"));

  // *TODO*: remove type inference
//  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
//      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator != inherited::configuration_->end ());
  Stream_Module_t* module_p = NULL;
//  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
//  unsigned int index_i = 0;

  // *TODO*: remove type inference
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_DirectShow_WASAPIIn_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_DirectShow_Audio_Tagger_Module (this,
                                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

//#if defined (GUI_SUPPORT)
//  if ((*iterator).second.second->window)
//  {
//    ACE_NEW_RETURN (module_p,
//                    Test_I_AVStream_Client_ALSA_V4L_Distributor_Module (this,
//                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
//                    false);
//    layout_inout->append (module_p, NULL, 0);
//    branch_p = module_p;
//    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
//    Stream_IDistributorModule* idistributor_p =
//        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
//    ACE_ASSERT (idistributor_p);
//    idistributor_p->initialize (inherited::configuration_->configuration_->branches);
//  } // end IF

  ACE_ASSERT (inherited::configuration_->configuration_->module_2);
  layout_inout->append (inherited::configuration_->configuration_->module_2, NULL, 0);

//  if ((*iterator).second.second->window)
//  {
//    module_p = NULL;
//    ACE_NEW_RETURN (module_p,
//                   Test_I_AVStream_Client_ALSA_V4L_Resize_Module (this,
//                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
//                   false);
//    layout_inout->append (module_p, branch_p, index_i);
//    module_p = NULL;
//#if defined (GTK_USE)
//    ACE_NEW_RETURN (module_p,
//                   Test_I_AVStream_Client_ALSA_V4L_Display_Module (this,
//                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING)),
//                   false);
//    layout_inout->append (module_p, branch_p, index_i);
//    module_p = NULL;
//#endif // GTK_USE
//#endif // GUI_SUPPORT
//  } // end IF

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Client_DirectShow_Audio_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Audio_Stream::initialize"));

  //  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Stream_MediaFramework_DirectShow_AudioVideoFormat format_s;

     // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
               ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  Test_I_AVStream_Client_DirectShow_StreamSessionData& session_data_r =
      const_cast<Test_I_AVStream_Client_DirectShow_StreamSessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_data_r.formats.empty ());
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
                                                format_s);
  session_data_r.formats.push_back (format_s);
  session_data_r.stream = this;

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                 ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

     // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
        setup_pipeline;

  return false;
}
#else
Test_I_AVStream_Client_ALSA_Stream::Test_I_AVStream_Client_ALSA_Stream ()
    : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_ALSA_Stream::Test_I_AVStream_Client_ALSA_Stream"));

}

bool
Test_I_AVStream_Client_ALSA_Stream::load (Stream_ILayout* layout_inout,
                                          bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_ALSA_Stream::load"));

  // *TODO*: remove type inference
//  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
//      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator != inherited::configuration_->end ());
  Stream_Module_t* module_p = NULL;
//  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
//  unsigned int index_i = 0;

  // *TODO*: remove type inference
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_ALSA_V4L_MicSource_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_ALSA_V4L_Audio_Tagger_Module (this,
                                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

//#if defined (GUI_SUPPORT)
//  if ((*iterator).second.second->window)
//  {
//    ACE_NEW_RETURN (module_p,
//                    Test_I_AVStream_Client_ALSA_V4L_Distributor_Module (this,
//                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
//                    false);
//    layout_inout->append (module_p, NULL, 0);
//    branch_p = module_p;
//    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
//    Stream_IDistributorModule* idistributor_p =
//        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
//    ACE_ASSERT (idistributor_p);
//    idistributor_p->initialize (inherited::configuration_->configuration_->branches);
//  } // end IF

  ACE_ASSERT (inherited::configuration_->configuration_->module_2);
  layout_inout->append (inherited::configuration_->configuration_->module_2, NULL, 0);

//  if ((*iterator).second.second->window)
//  {
//    module_p = NULL;
//    ACE_NEW_RETURN (module_p,
//                   Test_I_AVStream_Client_ALSA_V4L_Resize_Module (this,
//                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
//                   false);
//    layout_inout->append (module_p, branch_p, index_i);
//    module_p = NULL;
//#if defined (GTK_USE)
//    ACE_NEW_RETURN (module_p,
//                   Test_I_AVStream_Client_ALSA_V4L_Display_Module (this,
//                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING)),
//                   false);
//    layout_inout->append (module_p, branch_p, index_i);
//    module_p = NULL;
//#endif // GTK_USE
//#endif // GUI_SUPPORT
//  } // end IF

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Client_ALSA_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_ALSA_Stream::initialize"));

  //  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;

     // allocate a new session state, reset stream
  const_cast<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
               ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  Test_I_AVStream_Client_ALSA_V4L_StreamSessionData& session_data_r =
      const_cast<Test_I_AVStream_Client_ALSA_V4L_StreamSessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
//  typename Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t::ITERATOR_T iterator =
//      const_cast<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_data_r.formats.empty ());
  session_data_r.formats.push_back (configuration_in.configuration_->format);
  session_data_r.stream = this;

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                 ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

     // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
        setup_pipeline;

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
