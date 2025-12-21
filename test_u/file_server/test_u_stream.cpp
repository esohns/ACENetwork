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

#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_string_tools.h"

#include "stream_dec_defines.h"

#include "stream_file_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_common_modules.h"
#include "test_u_module_headerparser.h"
#include "test_u_module_protocolhandler.h"

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

Test_U_Stream::~Test_U_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::~Test_U_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  std::string extension_string =
    Common_String_Tools::tolower (Common_File_Tools::fileExtension ((*iterator).second.second->fileIdentifier.identifier, false));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_FileReaderH_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  if (extension_string == ACE_TEXT_ALWAYS_CHAR ("ts")     ||
      extension_string == ACE_TEXT_ALWAYS_CHAR ("mpegts") ||
      extension_string == ACE_TEXT_ALWAYS_CHAR ("mts"))
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_MPEG_TS_Decoder_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING)),
                    false);
    layout_inout->append (module_p, NULL, 0);
    module_p = NULL;
  }

  ACE_NEW_RETURN (module_p,
                  Test_U_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_U_Net_Output_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (MODULE_NET_OUTPUT_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                           ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);

  // update configuration
  inherited::MODULE_T* module_p = inherited::head ();
  ACE_ASSERT (module_p);
  inherited::TASK_T* task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
  (*iterator).second.second->outboundQueue =
    dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue ());
  ACE_ASSERT ((*iterator).second.second->outboundQueue);

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct FileServer_SessionData* session_data_p = NULL;
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
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

 session_data_p =
   &const_cast<struct FileServer_SessionData&> (session_manager_p->getR (inherited::id_));
  //session_data_p->sessionID = configuration_in.sessionID;

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}

//////////////////////////////////////////

Test_U_UDPStream::Test_U_UDPStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_UDPStream::Test_U_UDPStream"));

}

Test_U_UDPStream::~Test_U_UDPStream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_UDPStream::~Test_U_UDPStream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_UDPStream::load (Stream_ILayout* layout_inout,
                        bool& delete_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_UDPStream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_FileReaderH_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_MPEG_TS_Decoder_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  if (inherited::configuration_->configuration_->dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    ACE_NEW_RETURN (module_p,
                    Test_U_Module_Net_UDPTarget_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_NET_TARGET_DEFAULT_NAME_STRING)),
                    false);
  else
    ACE_NEW_RETURN (module_p,
                  Test_U_Module_Net_AsynchUDPTarget_Module (this,
                                                            ACE_TEXT_ALWAYS_CHAR (MODULE_NET_TARGET_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}

bool
Test_U_UDPStream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                              ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_UDPStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
  ACE_ASSERT (session_manager_p);

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct FileServer_SessionData* session_data_p = NULL;
  Stream_Module_t* module_p = NULL;
  Test_U_FileReaderH* file_source_impl_p = NULL;

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

  //(*iterator).second.second->stream = this;
  session_data_p =
    &const_cast<struct FileServer_SessionData&> (session_manager_p->getR (inherited::id_));
  //session_data_p->sessionId = configuration_in.sessionId;

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (configuration_in.configuration_->notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
