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

#include "ace/Log_Msg.h"

#include "stream_misc_defines.h"

#include "stream_net_http_defines.h"

#include "stream_vis_defines.h"

#include "net_macros.h"

#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_m3u_module_parser.h"
#include "test_i_common_modules.h"
#include "test_i_module_httpget.h"

Test_I_ConnectionStream::Test_I_ConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::Test_I_ConnectionStream"));

}

bool
Test_I_ConnectionStream::load (Stream_ILayout* layout_in,
                               bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::load"));

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR ("Marshal")),
                  false);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;
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
                 Test_I_Module_HTTPGet_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                 false);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_Module_Dump_Module (this,
  //                                           ACE_TEXT_ALWAYS_CHAR ("Dump")),
  //                false);
  //layout_in->append (module_p, NULL, 0);

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

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
//  struct Test_I_WebTV_SessionData* session_data_p = NULL;
//  inherited::CONFIGURATION_T::ITERATOR_T iterator;

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
  ACE_ASSERT (inherited::sessionData_);
//  session_data_p =
//    &const_cast<struct Test_I_WebTV_SessionData&> (inherited::sessionData_->getR ());
//  iterator =
//      const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
//  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

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

Test_I_ConnectionStream_3::Test_I_ConnectionStream_3 ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_3::Test_I_ConnectionStream_3"));

}

bool
Test_I_ConnectionStream_3::load (Stream_ILayout* layout_in,
                                 bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_3::load"));

  bool result = inherited::load (layout_in,
                                 deleteModules_out);
  ACE_ASSERT (result);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_3_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR ("Marshal")),
                  false);
  layout_in->append (module_p, NULL, 0);

  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
  //                false);
  //layout_in->append (module_p, NULL, 0);

  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_QueueTarget_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);

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

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  //Test_I_WebTV_SessionData_2* session_data_p = NULL;
  //inherited::CONFIGURATION_T::ITERATOR_T iterator;
  //Stream_Module_t* module_p = NULL;
//  Test_I_Net_Writer_t* netIO_impl_p = NULL;
  //Test_I_HTTPParser* parser_impl_p = NULL;

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
  //ACE_ASSERT (inherited::sessionData_);
  //session_data_p =
  //  &const_cast<Test_I_WebTV_SessionData_2&> (inherited::sessionData_->getR ());
  //iterator =
  //    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  //ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;
  //session_data_p->formats.push_front (configuration_in.configuration_->mediaType);

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // ******************* Net IO ************************
  //module_p =
  //  const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("Marshal")));
  //if (!module_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
  //              ACE_TEXT (stream_name_string_),
  //              ACE_TEXT ("Marshal")));
  //  goto failed;
  //} // end IF
  //parser_impl_p = dynamic_cast<Test_I_HTTPParser*> (module_p->writer ());
  //if (!parser_impl_p)
  //{
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s/%s: dynamic_cast<Test_I_HTTPParser> failed, aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (module_p->name ())));
//    goto failed;

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
