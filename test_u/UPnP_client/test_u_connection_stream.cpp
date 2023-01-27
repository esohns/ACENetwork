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

#include "stream_html_defines.h"

#include "stream_misc_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "http_defines.h"

#include "ssdp_defines.h"

#include "test_u_message.h"
#include "test_u_session_message.h"
#include "test_u_common.h"
#include "test_u_common_modules.h"

Test_U_InboundSSDPConnectionStream::Test_U_InboundSSDPConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundSSDPConnectionStream::Test_U_InboundSSDPConnectionStream"));

}

bool
Test_U_InboundSSDPConnectionStream::load (Stream_ILayout* layout_inout,
                                          bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_InboundSSDPConnectionStream::load"));

  Stream_Module_t* module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_SSDP_Net_IO_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_SSDP_Marshal_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_NET_MARSHAL_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                UPnP_Client_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                UPnP_Client_Module_Dump_Module (this,
  //                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DUMP_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;

  deleteModules_out = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

Test_U_OutboundSSDPConnectionStream::Test_U_OutboundSSDPConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundSSDPConnectionStream::Test_U_OutboundSSDPConnectionStream"));

}

bool
Test_U_OutboundSSDPConnectionStream::load (Stream_ILayout* layout_inout,
                                           bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_OutboundSSDPConnectionStream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_SSDP_Net_IO_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_SSDP_Streamer_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (SSDP_DEFAULT_MODULE_STREAMER_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                UPnP_Client_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

Test_U_HTTP_ConnectionStream::Test_U_HTTP_ConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_HTTP_ConnectionStream::Test_U_HTTP_ConnectionStream"));

}

bool
Test_U_HTTP_ConnectionStream::load (Stream_ILayout* layout_inout,
                                    bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_HTTP_ConnectionStream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_HTTP_Net_IO_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_Module_HTTP_Marshal_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_NET_MARSHAL_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                UPnP_Client_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_XMLParser_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (MODULE_XML_PARSER_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  UPnP_Client_XPathQuery_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_XPATH_QUERY_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  return true;
}

bool
Test_U_HTTP_ConnectionStream::initialize (const inherited::CONFIGURATION_T& configuration_in,
                                          ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_HTTP_ConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct UPnP_Client_SessionData* session_data_p = NULL;
  //inherited::CONFIGURATION_T::ITERATOR_T iterator;

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

  // configure session data
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<struct UPnP_Client_SessionData&> (inherited::sessionData_->getR ());
  //iterator =
  //  const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  //ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
  session_data_p->parserContext = configuration_in.configuration_->parserContext;

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
