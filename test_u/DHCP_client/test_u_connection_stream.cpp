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

#include "test_u_connection_stream.h"

#include "ace/Log_Msg.h"

#include "stream_misc_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "dhcp_defines.h"

#include "test_u_message.h"
#include "test_u_session_message.h"
#include "test_u_common.h"
#include "test_u_common_modules.h"

Test_U_InboundConnectionStream::Test_U_InboundConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::Test_U_InboundConnectionStream"));

}

Test_U_InboundConnectionStream::~Test_U_InboundConnectionStream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::~Test_U_InboundConnectionStream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_InboundConnectionStream::load (Stream_ILayout* layout_inout,
                                      bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::load"));

  if (!inherited::load (layout_inout,
                        deleteModules_out))
    return false;

  Stream_Module_t* module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                DHCPClient_Module_Net_IO_Module (this,
  //                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  DHCPClient_Module_Marshal_Module (this,
                                                    ACE_TEXT_ALWAYS_CHAR (MODULE_NET_MARSHAL_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                DHCPClient_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  DHCPClient_Module_DHCPDiscover_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (DHCP_DEFAULT_MODULE_DISCOVER_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  DHCPClient_Module_Dump_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DUMP_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  deleteModules_out = true;

  return true;
}

bool
Test_U_InboundConnectionStream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                            ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct DHCPClient_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();

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
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct DHCPClient_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_p->sessionID = configuration_in.sessionID;
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
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

////////////////////////////////////////////////////////////////////////////////

Test_U_OutboundConnectionStream::Test_U_OutboundConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::Test_U_OutboundConnectionStream"));

}

Test_U_OutboundConnectionStream::~Test_U_OutboundConnectionStream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::~Test_U_OutboundConnectionStream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_OutboundConnectionStream::load (Stream_ILayout* layout_inout,
                                       bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  if (!inherited::load (layout_inout,
                        deleteModules_out))
    return false;

  Stream_Module_t* module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                DHCPClient_Module_Net_IO_Module (this,
  //                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  DHCPClient_Module_Streamer_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR (MODULE_NET_MARSHAL_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                DHCPClient_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;

  return true;
}

bool
Test_U_OutboundConnectionStream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                             ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct DHCPClient_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();

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
    &const_cast<struct DHCPClient_SessionData&> (session_manager_p->getR (inherited::id_));
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

  ACE_ASSERT (inherited::isInitialized_);
  //inherited::dump_state ();

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::STREAM_BASE_T::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}
