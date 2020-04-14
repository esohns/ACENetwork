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

//#include "ace/Synch.h"
//#include "test_u_connection_common.h"
#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "stream_file_defines.h"

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

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_FileReader_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR ("FileSource")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_MPEG_TS_Decoder_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR ("MPEGTSDecoder")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Net_IO_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR ("NetworkIO")),
                  false);
  layout_inout->append (module_p, NULL, 0);

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
  (*iterator).second.second.outboundQueue =
    dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue ());
  ACE_ASSERT ((*iterator).second.second.outboundQueue);

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
//  struct FileServer_SessionData* session_data_p = NULL;
  Test_U_Module_Net_Writer_t* net_io_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
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
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);

//  session_data_p =
//      &const_cast<struct FileServer_SessionData&> (inherited::sessionData_->getR ());
  //session_data_p->sessionID = configuration_in.sessionID;

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* Socket Handler ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("NetworkIO")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ("NetworkIO")));
    goto error;
  } // end IF
  net_io_impl_p =
    dynamic_cast<Test_U_Module_Net_Writer_t*> (module_p->writer ());
  if (!net_io_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  net_io_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (configuration_in.configuration_.notificationStrategy))
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
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
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
  if (inherited::configuration_->configuration_.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
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

  // update configuration
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
  //(*iterator).second.second.stream = this;

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
//  struct FileServer_SessionData* session_data_p = NULL;
  Stream_Module_t* module_p = NULL;
  Test_U_FileReaderH* file_source_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
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
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);

//  session_data_p =
//      &const_cast<struct FileServer_SessionData&> (inherited::sessionData_->getR ());
  //session_data_p->sessionId = configuration_in.sessionId;

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* File Reader ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)));
    goto error;
  } // end IF
  file_source_impl_p =
    dynamic_cast<Test_U_FileReaderH*> (module_p->writer ());
  if (!file_source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_FileReaderH> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  file_source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (configuration_in.configuration_.notificationStrategy))
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
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return false;
}
