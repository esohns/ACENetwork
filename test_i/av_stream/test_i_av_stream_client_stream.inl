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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "qedit.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_common.h"
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_defines.h"
#include "stream_misc_defines.h"
#include "stream_stat_defines.h"
#include "stream_vis_defines.h"

#include "test_i_common_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_AVStream_Client_DirectShow_Stream_T<ConnectionManagerType,
                                           ConnectorType>::Test_I_AVStream_Client_DirectShow_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Stream_T::Test_I_AVStream_Client_DirectShow_Stream_T"));

}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_DirectShow_Stream_T<ConnectionManagerType,
                                           ConnectorType>::load (Stream_ILayout* layout_out,
                                                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Stream_T::load"));

  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_2 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_2 != inherited::configuration_->end ());
  Stream_Module_t* module_p = NULL;
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_DirectShow_CamSource_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
  layout_out->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Client_DirectShow_Converter_Module (this,
  //                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
  //                false);
  //layout_out->append (module_p, NULL, 0);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_DirectShow_Video_Tagger_Module (this,
                                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_out->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR (MODULE_NET_TARGET_DEFAULT_NAME_STRING)),
                  false);
  layout_out->append (module_p, NULL, 0);
  module_p = NULL;

  if ((*iterator_2).second.second->window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_DirectShow_Distributor_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    layout_out->append (module_p, NULL, 0);
    branch_p = module_p;
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
    Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (branches_a);
  } // end IF

  ACE_ASSERT (inherited::configuration_->configuration_->module_2);
  layout_out->append (inherited::configuration_->configuration_->module_2, NULL, 0);

  if ((*iterator_2).second.second->window)
  {
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_DirectShow_Display_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING)),
                    false);
    layout_out->append (module_p, branch_p, index_i);
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //else
//  //{
//  //  ACE_NEW_RETURN (module_p,
//  //                  Test_I_AVStream_Client_DirectShow_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_NULL_MODULE_NAME)),
//  //                  false);
//  //  modules_out.push_back (module_p);
//  //} // end ELSE
//#endif

  delete_out = true;

  return true;
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_DirectShow_Stream_T<ConnectionManagerType,
                                           ConnectorType>::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_DirectShow_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_I_AVStream_Client_DirectShow_StreamSessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator, iterator_2;
  Test_I_Stream_DirectShow_CamSource* source_impl_p = NULL;
  Stream_Module_t* module_p = NULL;
  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  bool release_direct3DDevice = false;
  IDirect3DDeviceManager9* direct3D_manager_p = NULL;
  UINT reset_token = 0;
  //struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  bool release_builder = false;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;
  struct _AMMediaType media_type_s;
  struct Stream_MediaFramework_DirectShow_AudioVideoFormat format_s;

  iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  iterator_2 =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (iterator_2 != configuration_in.end ());

  // ---------------------------------------------------------------------------
  // step1: set up directshow filter graph
  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED     |
                              COINIT_DISABLE_OLE1DDE   |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if ((*iterator).second.second->builder)
  {
    // *NOTE*: Stream_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_MediaFramework_DirectShow_Tools::reset ((*iterator).second.second->builder,
                                                        CLSID_VideoInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    if (!Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation ((*iterator).second.second->builder,
                                                                       STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L,
                                                                       buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);
    goto continue_;
  } // end IF
  else
    release_builder = true;

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.second->deviceIdentifier,
                                                        CLSID_VideoInputDeviceCategory,
                                                        (*iterator).second.second->builder,
                                                        buffer_negotiation_p,
                                                        stream_config_p,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.identifier._string)));
    release_builder = false;
    goto error;
  } // end IF
  ACE_ASSERT ((*iterator).second.second->builder);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);
  stream_config_p->Release (); stream_config_p = NULL;

continue_:
  if (!Stream_Device_DirectShow_Tools::setCaptureFormat ((*iterator).second.second->builder,
                                                         CLSID_VideoInputDeviceCategory,
                                                         configuration_in.configuration_->format.video))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // sanity check(s)
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);
  //if (!(*iterator).second.second->direct3DConfiguration->handle)
  //{
  //  Stream_MediaFramework_DirectDraw_Tools::initialize (false);
  //  if (!Stream_MediaFramework_DirectDraw_Tools::getDevice (*(*iterator).second.second->direct3DConfiguration,
  //                                                          direct3D_manager_p,
  //                                                          reset_token))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectDraw_Tools::getDevice(), aborting\n"),
  //                ACE_TEXT (stream_name_string_)));
  //    goto error;
  //  } // end IF
  //  ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);
  //  ACE_ASSERT (direct3D_manager_p);
  //  ACE_ASSERT (reset_token);
  //  direct3D_manager_p->Release (); direct3D_manager_p = NULL;
  //  release_direct3DDevice = true;
  //} // end IF
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);

  if (!Stream_Module_Decoder_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                            configuration_in.configuration_->format.video,
                                                            (*iterator).second.second->outputFormat,
                                                            (*iterator).second.second->window,
                                                            (*iterator).second.second->builder,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererGraph(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                         &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release (); filter_p = NULL;

  result_2 = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release (); isample_grabber_p = NULL;

  ACE_ASSERT (buffer_negotiation_p);
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer =
    configuration_in.configuration_->allocatorConfiguration->defaultBufferSize;
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result_2 =
      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result_2)) // E_UNEXPECTED: 0x8000FFFF --> graph already connected
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    goto error;
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::connect ((*iterator).second.second->builder,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                          STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
  {
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting...\n"),
                ACE_TEXT (stream_name_string_)));
#endif // _DEBUG
    if (!Stream_MediaFramework_DirectShow_Tools::connectFirst ((*iterator).second.second->builder,
                                                               STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L));

#if defined (_DEBUG)
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result_2 =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result_2)) // E_FAIL (0x80004005)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s/%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    //goto error;
  } // end IF
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                ACE_TEXT (stream_name_string_),
                allocator_properties.cBuffers,
                allocator_properties.cbBuffer,
                allocator_properties.cbAlign,
                allocator_properties.cbPrefix));
#endif // _DEBUG
  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  result_2 =
    (*iterator).second.second->builder->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  // ---------------------------------------------------------------------------
  // step2: update stream module configuration(s)
  (*iterator_2).second.second->builder = (*iterator).second.second->builder;
  (*iterator_2).second.second->builder->AddRef ();

  // ---------------------------------------------------------------------------
  // step3: allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<Test_I_AVStream_Client_DirectShow_StreamSessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  //(*iterator).second.second->direct3DConfiguration->handle->AddRef ();
  //session_data_p->direct3DDevice =
  //  (*iterator).second.second->direct3DConfiguration->handle;
  //session_data_p->resetToken = reset_token;
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------
  // step4: initialize module(s)

  // ******************* Camera Source ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  source_impl_p =
    dynamic_cast<Test_I_Stream_DirectShow_CamSource*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_Stream_DirectShow_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  // ---------------------------------------------------------------------------
  // step5: update session data
  ACE_ASSERT (session_data_p->formats.empty ());
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
                                                format_s);
  session_data_p->formats.push_back (format_s);
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  if (!Stream_MediaFramework_DirectShow_Tools::getOutputFormat ((*iterator).second.second->builder,
                                                                STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                                media_type_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L)));
    goto error;
  } // end IF
  ACE_OS::memset (&format_s.audio, 0, sizeof (struct _AMMediaType));
  ACE_OS::memset (&format_s.video, 0, sizeof (struct _AMMediaType));
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
                                                format_s);
  Stream_MediaFramework_DirectShow_Tools::free (format_s.video);
  format_s.video = media_type_s;
  session_data_p->formats.push_back (format_s);
  //ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::matchMediaType (*session_data_p->outputFormat, *(*iterator).second.second->outputFormat));
  session_data_p->stream = this;

  // ---------------------------------------------------------------------------
  // step6: initialize head module
  //source_impl_p->setP (&(inherited::state_));
  ////fileReader_impl_p->reset ();
  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //module_p->arg (inherited::sessionData_);

  // step7: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_r.fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if (release_builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF
  if (release_direct3DDevice)
  {
    (*iterator).second.second->direct3DConfiguration->handle->Release (); (*iterator).second.second->direct3DConfiguration->handle = NULL;
  } // end IF
  if (session_data_p)
  {
    if (session_data_p->direct3DDevice)
    {
      session_data_p->direct3DDevice->Release (); session_data_p->direct3DDevice = NULL;
    } // end IF
    Stream_MediaFramework_DirectShow_Tools::free (session_data_p->formats);
    session_data_p->resetToken = 0;
  } // end IF
  if (isample_grabber_p)
    isample_grabber_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

//////////////////////////////////////////

template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::Test_I_AVStream_Client_MediaFoundation_Stream_T ()
 : inherited ()
 , inherited2 ()
 , mediaSession_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::Test_I_AVStream_Client_MediaFoundation_Stream_T"));

}

template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::~Test_I_AVStream_Client_MediaFoundation_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::~Test_I_AVStream_Client_MediaFoundation_Stream_T"));

  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::start"));

  // sanity check(s)
  ACE_ASSERT (mediaSession_);

  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = mediaSession_->Start (&GUID_s,      // time format
                                         &property_s); // start position
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

    // clean up
    PropVariantClear (&property_s);

    return;
  } // end IF
  PropVariantClear (&property_s);

  IMFAsyncCallback* callback_p = this;
  result = mediaSession_->BeginGetEvent (callback_p, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF

  inherited::start ();
}

template <typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::stop (bool waitForCompletion_in,
                                                                      bool recureUpstream_in,
                                                                      bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::stop"));

  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

  inherited::stop (waitForCompletion_in,
                   recureUpstream_in,
                   highPriority_in);
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::load (Stream_ILayout* layout_inout,
                                                                      bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::load"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_MediaFoundation_CamSource_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_AVStream_Client_MediaFoundation_StatisticReport_Module (this,
  //                                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR (MODULE_NET_TARGET_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  if ((*iterator).second.second->window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_MediaFoundation_Display_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                    false);
    layout_inout->append (module_p, NULL, 0);
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //else
//  //{
//  //  ACE_NEW_RETURN (module_p,
//  //                  Test_I_AVStream_Client_MediaFoundation_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_NULL_MODULE_NAME)),
//  //                  false);
//  //  modules_out.push_back (module_p);
//  //} // end ELSE
//#endif
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                ConnectorType>::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_MediaFoundation_Stream_T::initialize"));

  // allocate a new session state, reset stream
  bool setup_pipeline =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline;
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  bool reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Test_I_AVStream_Client_MediaFoundation_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  Test_I_AVStream_Client_MediaFoundation_StreamSessionData& session_data_r =
    const_cast<Test_I_AVStream_Client_MediaFoundation_StreamSessionData&> (inherited::sessionData_->getR ());
  bool input_mediatype_was_null = false;
  IMFMediaType* media_type_p = NULL;
  struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat format_s;

  // ---------------------------------------------------------------------------
  // sanity check(s)
  //ACE_ASSERT (configuration_in.moduleConfiguration);

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  Test_I_Stream_MediaFoundation_CamSource* source_impl_p = NULL;
  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());

  // ******************* Camera Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  source_impl_p =
    dynamic_cast<Test_I_Stream_MediaFoundation_CamSource*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_Stream_MediaFoundation_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result = E_FAIL;
  IMFTopology* topology_p = NULL;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED     |
                            COINIT_DISABLE_OLE1DDE   |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    //return false;
  } // end IF
  else
    COM_initialized = true;

  UINT32 item_count = 0;
  ULONG reference_count = 0;

  if (!configuration_in.configuration_->format.video)
  {
    result = MFCreateMediaType (&configuration_in.configuration_->format.video);
    ACE_ASSERT (SUCCEEDED (result));
    //result =
    //  configuration_in.configuration->format->SetGUID (MF_MT_SUBTYPE,
    //                                                   MFVideoFormat_RGB24);
    //ACE_ASSERT (SUCCEEDED (result));
    result =
      MFSetAttributeSize (configuration_in.configuration_->format.video,
                          MF_MT_FRAME_RATE,
                          STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE, 1);
    ACE_ASSERT (SUCCEEDED (result));
    result =
        MFSetAttributeSize (configuration_in.configuration_->format.video,
                            MF_MT_FRAME_SIZE,
                            STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH,
                            STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      configuration_in.configuration_->format.video->SetUINT32 (MF_MT_INTERLACE_MODE,
                                                                MFVideoInterlace_Progressive);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      MFSetAttributeRatio (configuration_in.configuration_->format.video,
                           MF_MT_PIXEL_ASPECT_RATIO,
                           1, 1);
    ACE_ASSERT (SUCCEEDED (result));
    input_mediatype_was_null = true;
  } // end IF
  ACE_ASSERT (configuration_in.configuration_->format.video);
  if (!Stream_Module_Decoder_Tools::loadVideoRendererTopology ((*iterator).second.second->deviceIdentifier,
                                                               configuration_in.configuration_->format.video,
                                                               source_impl_p,
                                                               (*iterator).second.second->window,
                                                               (*iterator).second.second->sampleGrabberNodeId,
                                                               session_data_r.rendererNodeId,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

  // set default capture media type ?
//  result = media_type_p->GetCount (&item_count);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    goto error;
//  } // end IF
//  if (!item_count)
//  {
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
//    IMFMediaSourceEx* media_source_p = NULL;
//#else
//    IMFMediaSource* media_source_p = NULL;
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
//    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (topology_p,
//                                                                      media_source_p))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n"),
//                  ACE_TEXT (stream_name_string_)));
//      goto error;
//    } // end IF
//  } // end IF

#if defined (_DEBUG)
  if (!input_mediatype_was_null)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: capture format: %s\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (configuration_in.configuration_->format.video, true).c_str ())));
#endif // _DEBUG

  ACE_ASSERT (session_data_r.formats.empty ());
  Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format,
                                                     format_s);
  session_data_r.formats.push_back (format_s);

  media_type_p = NULL;
  if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                     (*iterator).second.second->sampleGrabberNodeId,
                                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);

  format_s.audio = NULL; format_s.video = NULL;
  Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format,
                                                     format_s);
  format_s.video->Release (); format_s.video = NULL;
  format_s.video = media_type_p;
  session_data_r.formats.push_back (format_s);
  media_type_p = NULL;
  session_data_r.stream = this;

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

  // -------------------------------------------------------------

  //source_impl_p->setP (&(inherited::state_));
  ////fileReader_impl_p->reset ();
  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_AVStream_Client_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // *TODO*: remove type inferences
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->mediaFoundationConfiguration->mediaSession =
    mediaSession_;
  if (!inherited2::initialize (*configuration_in.configuration_->mediaFoundationConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_MediaFoundation_Callback_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  inherited::isInitialized_ = true;

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
  {
#if defined (_DEBUG)
    Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);
#endif // _DEBUG
    topology_p->Release ();
  } // end IF
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release (); session_data_r.direct3DDevice = NULL;
  } // end IF
  Stream_MediaFramework_MediaFoundation_Tools::free (session_data_r.formats);
  session_data_r.direct3DManagerResetToken = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (session_data_r.session)
  {
    session_data_r.session->Release (); session_data_r.session = NULL;
  } // end IF
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (COM_initialized)
    CoUninitialize ();

  return false;
}
#else
template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_AVStream_Client_V4L_Stream_T<ConnectionManagerType,
                                    ConnectorType>::Test_I_AVStream_Client_V4L_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Stream_T::Test_I_AVStream_Client_V4L_Stream_T"));

}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_V4L_Stream_T<ConnectionManagerType,
                                    ConnectorType>::load (Stream_ILayout* layout_inout,
                                                          bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Stream_T::load"));

  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  Stream_Module_t* module_p = NULL;
  Stream_Branches_t branches_a;
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;

  // *TODO*: remove type inference
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_ALSA_V4L_CamSource_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_ALSA_V4L_Converter_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_AVStream_Client_ALSA_V4L_Video_Tagger_Module (this,
                                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR (MODULE_NET_TARGET_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (GUI_SUPPORT)
  if ((*iterator).second.second->window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_ALSA_V4L_Distributor_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    layout_inout->append (module_p, NULL, 0);
    branch_p = module_p;
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
    Stream_IDistributorModule* idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (branches_a);
  } // end IF

  ACE_ASSERT (inherited::configuration_->configuration_->module_2);
  layout_inout->append (inherited::configuration_->configuration_->module_2, NULL, 0);

  if ((*iterator).second.second->window)
  {
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_ALSA_V4L_Resize_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING)),
                    false);
    layout_inout->append (module_p, branch_p, index_i);
    module_p = NULL;
#if defined (GTK_USE)
    ACE_NEW_RETURN (module_p,
                    Test_I_AVStream_Client_ALSA_V4L_Display_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING)),
                    false);
    layout_inout->append (module_p, branch_p, index_i);
    module_p = NULL;
#endif // GTK_USE
  } // end IF
#endif // GUI_SUPPORT

  delete_out = true;

  return true;
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_AVStream_Client_V4L_Stream_T<ConnectionManagerType,
                                    ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Client_V4L_Stream_T::initialize"));

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

  // ---------------------------------------------------------------------------
  // sanity check(s)
//  ACE_ASSERT (configuration_in.moduleConfiguration);

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

//  Test_I_AVStream_Client_V4L_CamSource* source_impl_p = NULL;

  // ******************* Camera Source ************************
//  typename inherited::MODULE_T* module_p =
//    const_cast<typename inherited::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING)));
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
//                ACE_TEXT (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING)));
//    return false;
//  } // end IF
//  source_impl_p =
//    dynamic_cast<Test_I_AVStream_Client_V4L_CamSource*> (module_p->writer ());
//  if (!source_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Test_I_AVStream_Client_V4L_CamSource> failed, aborting\n")));
//    return false;
//  } // end IF
//  source_impl_p->setP (&(inherited::state_));
//  //fileReader_impl_p->reset ();
//  // *NOTE*: push()ing the module will open() it
//  //         --> set the argument that is passed along (head module expects a
//  //             handle to the session data)
//  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_AVStream_Client_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

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
