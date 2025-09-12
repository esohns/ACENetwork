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

#include "test_i_av_stream_server_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"

#include "test_i_av_stream_server_session_message.h"
#include "test_i_common_modules.h"
#include "test_i_av_stream_client_stream.h"

#include "test_i_av_stream_server_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_AVStream_Server_DirectShow_TCPStream::Test_I_AVStream_Server_DirectShow_TCPStream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_TCPStream::Test_I_AVStream_Server_DirectShow_TCPStream"));

}

Test_I_AVStream_Server_DirectShow_TCPStream::~Test_I_AVStream_Server_DirectShow_TCPStream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_TCPStream::~Test_I_AVStream_Server_DirectShow_TCPStream"));

  inherited::shutdown ();
}

bool
Test_I_AVStream_Server_DirectShow_TCPStream::load (Stream_ILayout* layout_in,
                                                   bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_TCPStream::load"));

  if (!inherited::load (layout_in,
                        delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (delete_out);

  Stream_Module_t* module_p = NULL;
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Parser_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Splitter_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_AUDIO_NAME_STRING));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_VIDEO_NAME_STRING));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_WASAPIOut_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);

  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Server_DirectShow_StatisticReport_Module (this,
  //                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ++index_i;

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Defragment_Module (this,
                                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Resize_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);

#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_GTK_Display_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING)),
                  false);
#elif defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Display_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Direct3D_Display_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING)),
                  false);
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_p, index_i);

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_DirectShow_TCPStream::initialize (const CONFIGURATION_T& configuration_in,
                                                         ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_TCPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  struct Stream_MediaFramework_DirectShow_AudioVideoFormat format_s;
  Test_I_Server_DirectShow_SessionManager_t* session_manager_p =
    Test_I_Server_DirectShow_SessionManager_t::SINGLETON_T::instance ();

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
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_DirectShow_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_DirectShow_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_r.lock = &(inherited::sessionDataLock_);
  //inherited::state_.sessionData = &session_data_r;

  // ---------------------------------------------------------------------------

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("source format: %s\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (configuration_in.configuration_->format, true).c_str ())));

  //log_file_name =
  //  Common_File_Tools::getLogDirectory (std::string (),
  //                                      0);
  //log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  //log_file_name += STREAM_DEV_DIRECTSHOW_LOGFILE_NAME;
  //Stream_Module_Device_DirectShow_Tools::debug (graphBuilder_,
  //                                              log_file_name);

  // ---------------------------------------------------------------------------

  ACE_ASSERT (session_data_r.formats.empty ());
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
                                                format_s);
  session_data_r.formats.push_back (format_s);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!session_data_r.formats.empty ())
    session_data_r.formats.clear ();

  return false;
}

void
Test_I_AVStream_Server_DirectShow_TCPStream::setFormat (IGraphBuilder* builder_in,
                                                        const std::wstring& sourceFilterName_in,
                                                        const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_TCPStream::setFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!sourceFilterName_in.empty ());

  IBaseFilter* filter_p = NULL;
  IPin* pin_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (sourceFilterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF

  Common_IInitialize_T<struct _AMMediaType>* iinitialize_p =
    dynamic_cast<Common_IInitialize_T<struct _AMMediaType>*> (pin_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                pin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (!iinitialize_p->initialize (mediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  pin_p->Release (); pin_p = NULL;

  return;

error:
  if (filter_p)
    filter_p->Release ();
  if (pin_p)
    pin_p->Release ();
}

//////////////////////////////////////////

Test_I_AVStream_Server_DirectShow_UDPStream::Test_I_AVStream_Server_DirectShow_UDPStream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_UDPStream::Test_I_AVStream_Server_DirectShow_UDPStream"));

}

Test_I_AVStream_Server_DirectShow_UDPStream::~Test_I_AVStream_Server_DirectShow_UDPStream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_UDPStream::~Test_I_AVStream_Server_DirectShow_UDPStream"));

  inherited::shutdown ();
}

bool
Test_I_AVStream_Server_DirectShow_UDPStream::load (Stream_ILayout* layout_in,
                                                   bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_UDPStream::load"));

  if (!inherited::load (layout_in,
                        delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (delete_out);

  Stream_Module_t* module_p = NULL;
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Parser_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Splitter_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_AUDIO_NAME_STRING));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_VIDEO_NAME_STRING));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_WASAPIOut_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);

  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Server_DirectShow_StatisticReport_Module (this,
  //                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ++index_i;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_DirectShow_Display_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Direct3D_Display_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING)),
                  false);
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_p, index_i);

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_DirectShow_UDPStream::initialize (const CONFIGURATION_T& configuration_in,
                                                         ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_UDPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  struct Stream_MediaFramework_DirectShow_AudioVideoFormat format_s;
  Test_I_Server_DirectShow_SessionManager_t* session_manager_p =
    Test_I_Server_DirectShow_SessionManager_t::SINGLETON_T::instance ();

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
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_DirectShow_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_DirectShow_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_r.lock = &(inherited::sessionDataLock_);
  //inherited::state_.sessionData = &session_data_r;

  // ---------------------------------------------------------------------------

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("source format: %s\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (configuration_in.configuration_->format, true).c_str ())));

  //log_file_name =
  //  Common_File_Tools::getLogDirectory (std::string (),
  //                                      0);
  //log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  //log_file_name += STREAM_DEV_DIRECTSHOW_LOGFILE_NAME;
  //Stream_Module_Device_DirectShow_Tools::debug (graphBuilder_,
  //                                              log_file_name);

  // ---------------------------------------------------------------------------

  ACE_ASSERT (session_data_r.formats.empty ());
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
                                                format_s);
  session_data_r.formats.push_back (format_s);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!session_data_r.formats.empty ())
    session_data_r.formats.clear ();

  return false;
}

void
Test_I_AVStream_Server_DirectShow_UDPStream::setFormat (IGraphBuilder* builder_in,
                                               const std::wstring& sourceFilterName_in,
                                               const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_DirectShow_UDPStream::setFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!sourceFilterName_in.empty ());

  IBaseFilter* filter_p = NULL;
  IPin* pin_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (sourceFilterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF

  Common_IInitialize_T<struct _AMMediaType>* iinitialize_p =
    dynamic_cast<Common_IInitialize_T<struct _AMMediaType>*> (pin_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                pin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (!iinitialize_p->initialize (mediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  pin_p->Release (); pin_p = NULL;

  return;

error:
  if (filter_p)
    filter_p->Release ();
  if (pin_p)
    pin_p->Release ();
}

//////////////////////////////////////////

Test_I_AVStream_Server_MediaFoundation_TCPStream::Test_I_AVStream_Server_MediaFoundation_TCPStream ()
 : inherited ()
 , mediaSession_ (NULL)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_TCPStream::Test_I_AVStream_Server_MediaFoundation_TCPStream"));

}

Test_I_AVStream_Server_MediaFoundation_TCPStream::~Test_I_AVStream_Server_MediaFoundation_TCPStream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_TCPStream::~Test_I_AVStream_Server_MediaFoundation_TCPStream"));

  inherited::shutdown ();
}

bool
Test_I_AVStream_Server_MediaFoundation_TCPStream::load (Stream_ILayout* layout_in,
                                               bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_TCPStream::load"));

    if (!inherited::load (layout_in,
                          delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  ACE_ASSERT (delete_out);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_MediaFoundation_Parser_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_MediaFoundation_Splitter_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Server_MediaFoundation_StatisticReport_Module (this,
  //                                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_MediaFoundation_Display_Module (this,
                                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_MediaFoundation_TCPStream::initialize (const CONFIGURATION_T& configuration_in,
                                                              ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_TCPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  std::string url_string = ACE_TEXT_ALWAYS_CHAR (AVSTREAM_TARGET_DEFAULT_SCHEME_HANDLER_URL);
  //url_string += ACE_TEXT_ALWAYS_CHAR ("//test");
  Test_I_Server_MediaFoundation_SessionManager_t* session_manager_p =
    Test_I_Server_MediaFoundation_SessionManager_t::SINGLETON_T::instance ();

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
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_MediaFoundation_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_MediaFoundation_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_r.lock = &(inherited::sessionDataLock_);
  //inherited::state_.sessionData = &session_data_r;
  ACE_ASSERT ((*iterator).second.second->sourceFormat);
  ACE_ASSERT (!session_data_r.sourceFormat);

  session_data_r.sourceFormat =
    Stream_MediaFramework_MediaFoundation_Tools::copy ((*iterator).second.second->sourceFormat);
  if (!session_data_r.sourceFormat)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // ---------------------------------------------------------------------------

  bool graph_loaded = false;
  bool COM_initialized = false;
  IMFTopology* topology_p = NULL;
  UINT32 item_count = 0;
  ULONG reference_count = 0;
  HRESULT result_2 = CoInitializeEx (NULL,
                                     (COINIT_MULTITHREADED     |
                                      COINIT_DISABLE_OLE1DDE   |
                                      COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

  // sanity check(s)
  ACE_ASSERT ((*iterator).second.second->sourceFormat);

  bool release_source = false;
  IMFTopologyNode* topology_node_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  IMFMediaType* media_type_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFActivate* activate_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

  result_2 = MFCreateTopology (&topology_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result_2 = topology_p->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                    MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = topology_p->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = topology_p->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                    MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = topology_p->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result_2));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result_2 = topology_p->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                    TRUE);
  ACE_ASSERT (SUCCEEDED (result_2));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result_2 = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                   &topology_node_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result_2 = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result_2));

  result_2 =
    Stream_MediaFramework_MediaFoundation_MediaSource_t::CreateInstance (NULL,
                                                                         IID_IMFMediaSource,
                                                                         reinterpret_cast<void**> (&media_source_p));
  ACE_ASSERT (SUCCEEDED (result_2));
  ACE_ASSERT (media_source_p);
  result_2 = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                          media_source_p);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    media_source_p->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  result_2 = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                          stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result_2));
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  result_2 = topology_p->AddNode (topology_node_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result_2));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added source node (id: %q)...\n"),
              node_id));
  topology_node_p->Release (); topology_node_p = NULL;

  if (!Stream_Module_Decoder_Tools::loadTargetRendererTopology (url_string,
                                                                (*iterator).second.second->sourceFormat,
                                                                (*iterator).second.second->window,
                                                                session_data_r.rendererNodeId,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadTargetRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: source format: \"%s\"\n"),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString ((*iterator).second.second->sourceFormat).c_str ()),
              ACE_TEXT (stream_name_string_)));
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
  if ((*iterator).second.second->session)
  {
    reference_count = (*iterator).second.second->session->AddRef ();
    mediaSession_ = (*iterator).second.second->session;

    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 true,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);

  if (!(*iterator).second.second->session)
  {
    reference_count = mediaSession_->AddRef ();
    (*iterator).second.second->session = mediaSession_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release (); session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.sourceFormat)
  {
    session_data_r.sourceFormat->Release (); session_data_r.sourceFormat = NULL;
  } // end IF
  session_data_r.direct3DManagerResetToken = 0;

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

//////////////////////////////////////////

Test_I_AVStream_Server_MediaFoundation_UDPStream::Test_I_AVStream_Server_MediaFoundation_UDPStream ()
 : inherited ()
 , mediaSession_ (NULL)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_UDPStream::Test_I_AVStream_Server_MediaFoundation_UDPStream"));

}

Test_I_AVStream_Server_MediaFoundation_UDPStream::~Test_I_AVStream_Server_MediaFoundation_UDPStream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_UDPStream::~Test_I_AVStream_Server_MediaFoundation_UDPStream"));

  inherited::shutdown ();
}

bool
Test_I_AVStream_Server_MediaFoundation_UDPStream::load (Stream_ILayout* layout_in,
                                                        bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_UDPStream::load"));

    if (!inherited::load (layout_in,
                          delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  ACE_ASSERT (delete_out);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_MediaFoundation_Parser_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Server_MediaFoundation_StatisticReport_Module (this,
  //                                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_MediaFoundation_Display_Module (this,
                                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_MediaFoundation_UDPStream::initialize (const CONFIGURATION_T& configuration_in,
                                                     ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_MediaFoundation_UDPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  std::string url_string = ACE_TEXT_ALWAYS_CHAR (AVSTREAM_TARGET_DEFAULT_SCHEME_HANDLER_URL);
  Test_I_Server_MediaFoundation_SessionManager_t* session_manager_p =
    Test_I_Server_MediaFoundation_SessionManager_t::SINGLETON_T::instance ();

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
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_MediaFoundation_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_MediaFoundation_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_r.lock = &(inherited::sessionDataLock_);
  //inherited::state_.sessionData = &session_data_r;
  ACE_ASSERT ((*iterator).second.second->sourceFormat);
  ACE_ASSERT (!session_data_r.sourceFormat);

  session_data_r.sourceFormat =
    Stream_MediaFramework_MediaFoundation_Tools::copy ((*iterator).second.second->sourceFormat);
  if (!session_data_r.sourceFormat)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // ---------------------------------------------------------------------------

  bool graph_loaded = false;
  bool COM_initialized = false;
  IMFTopology* topology_p = NULL;
  UINT32 item_count = 0;
  ULONG reference_count = 0;
  HRESULT result_2 = CoInitializeEx (NULL,
                                     (COINIT_MULTITHREADED     |
                                      COINIT_DISABLE_OLE1DDE   |
                                      COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

  // sanity check(s)
  ACE_ASSERT ((*iterator).second.second->sourceFormat);

  if (!Stream_Module_Decoder_Tools::loadTargetRendererTopology (url_string,
                                                                (*iterator).second.second->sourceFormat,
                                                                NULL,
                                                                //(*iterator).second.second->window,
                                                                session_data_r.rendererNodeId,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadTargetRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: source format: \"%s\"\n"),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString ((*iterator).second.second->sourceFormat).c_str ()),
              ACE_TEXT (stream_name_string_)));
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
  if ((*iterator).second.second->session)
  {
    reference_count = (*iterator).second.second->session->AddRef ();
    mediaSession_ = (*iterator).second.second->session;

    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);

  if (!(*iterator).second.second->session)
  {
    reference_count = mediaSession_->AddRef ();
    (*iterator).second.second->session = mediaSession_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release (); session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.sourceFormat)
  {
    session_data_r.sourceFormat->Release (); session_data_r.sourceFormat = NULL;
  } // end IF
  session_data_r.direct3DManagerResetToken = 0;

  if (COM_initialized)
    CoUninitialize ();

  return false;
}
#else
Test_I_AVStream_Server_TCPStream::Test_I_AVStream_Server_TCPStream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_TCPStream::Test_I_AVStream_Server_TCPStream"));

}

bool
Test_I_AVStream_Server_TCPStream::load (Stream_ILayout* layout_in,
                                        bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_TCPStream::load"));

  // initialize return value(s)
  delete_out = true;

  if (!inherited::load (layout_in,
                        delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n")));
    return false;
  } // end IF

  Stream_Branches_t branches_a;
  Stream_Module_t* module_p = NULL;
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;

  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Parser_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Splitter_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_AUDIO_NAME_STRING));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_VIDEO_NAME_STRING));
  Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Defragment_Module (this,
                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                 Test_I_AVStream_Server_Target_ALSA_Module (this,
                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                 false);
  layout_in->append (module_p, branch_p, index_i);

  ++index_i;
  module_p = NULL;
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Defragment_Module (this,
                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Resize_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Display_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
#endif // GTK_USE
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_TCPStream::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
                                              ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_TCPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_Server_SessionManager_t* session_manager_p =
    Test_I_Server_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  // session_data_r.lock = &(inherited::sessionDataLock_);
  // inherited::state_.sessionData = &session_data_r;
  ACE_ASSERT (session_data_r.formats.empty ());
  session_data_r.formats.push_back (configuration_in.configuration_->format);
  //  session_data_r.sessionId = (*iterator).second.second->sessionId;
  session_data_r.targetFileName = (*iterator).second.second->targetFileName;

  //  configuration_in.moduleConfiguration.streamState = &state_;
//  configuration_p->stateMachineLock = &inherited::state_.stateMachineLock;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}

void
Test_I_AVStream_Server_TCPStream::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_TCPStream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

//////////////////////////////////////////

Test_I_AVStream_Server_UDPStream::Test_I_AVStream_Server_UDPStream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_UDPStream::Test_I_AVStream_Server_UDPStream"));

}

bool
Test_I_AVStream_Server_UDPStream::load (Stream_ILayout* layout_in,
                                        bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_UDPStream::load"));

//  // initialize return value(s)
//  modules_out.clear ();
//  delete_out = false;
  ACE_ASSERT (delete_out);

  Stream_Module_t* module_p = NULL;

  if (!inherited::load (layout_in,
                        delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n")));
    return false;
  } // end IF

  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Splitter_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //Test_I_AVStream_Server_Module_AVIDecoder_Module            decoder_;
//  ACE_NEW_RETURN (module_p,
//                  Test_I_AVStream_Server_StatisticReport_Module (this,
//                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
//                  false);
//  layout_in->append (module_p, NULL, 0);
//  module_p = NULL;
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Server_Display_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
#endif // GTK_USE
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Test_I_AVStream_Server_UDPStream::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
                                              ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_UDPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_Server_SessionManager_t* session_manager_p =
    Test_I_Server_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;

  Test_I_AVStream_Server_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Server_StreamSessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  // session_data_r.lock = &(inherited::sessionDataLock_);
  // inherited::state_.sessionData = &session_data_r;
  ACE_ASSERT (session_data_r.formats.empty ());
  session_data_r.formats.push_back (configuration_in.configuration_->format);
  //  session_data_r.sessionId = (*iterator).second.second->sessionId;
  session_data_r.targetFileName = (*iterator).second.second->targetFileName;

  //  configuration_in.moduleConfiguration.streamState = &state_;
//  configuration_p->stateMachineLock = &inherited::state_.stateMachineLock;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}

void
Test_I_AVStream_Server_UDPStream::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_UDPStream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

#endif
