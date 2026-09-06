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
#include "amvideo.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_html_defines.h"

#include "stream_misc_defines.h"

#include "stream_net_http_defines.h"

#include "net_macros.h"

#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_module_httpget.h"
#include "test_i_session_message.h"

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

  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  bool save_to_file_b = !(*iterator).second.second->targetFileName.empty ();

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

  ACE_NEW_RETURN (module_p,
                  Test_I_Module_HTTPGet_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (FFMPEG_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_AudioDecoder_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // FFMPEG_SUPPORT

  ACE_NEW_RETURN (module_p,
                  Test_I_Audio_Tagger_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  if (save_to_file_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_QueueTarget_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR ("QueueTarget_2")),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF

  ACE_NEW_RETURN (module_p,
                  Test_I_QueueTarget_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

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
  struct Test_I_URLStreamLoad_SessionData* session_data_p = NULL;
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
    &const_cast<struct Test_I_URLStreamLoad_SessionData&> (session_manager_p->getR (inherited::id_));
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

Test_I_ConnectionStream_1b::Test_I_ConnectionStream_1b ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_1b::Test_I_ConnectionStream_1b"));

}

bool
Test_I_ConnectionStream_1b::load (Stream_ILayout* layout_in,
                                  bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream_1b::load"));

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

  ACE_NEW_RETURN (module_p,
                  Test_I_Module_HTTPGet_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (FFMPEG_SUPPORT)
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
#else
#error "no supported video decoder, aborting"
#endif // FFMPEG_SUPPORT
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Video_Tagger_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_QueueTarget_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  deleteModules_out = true;

  return true;
}

bool
Test_I_ConnectionStream_1b::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                        ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_1b::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_URLStreamLoad_SessionData* session_data_p = NULL;
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
                ACE_TEXT (stream_name_string_1b)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct Test_I_URLStreamLoad_SessionData&> (session_manager_p->getR (inherited::id_));
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
                  ACE_TEXT (stream_name_string_1b)));
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
                ACE_TEXT (stream_name_string_1b)));

  return false;
}
