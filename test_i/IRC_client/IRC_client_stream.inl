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

#include "ace/Log_Msg.h"

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "irc_defines.h"

#include "IRC_client_common_modules.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_network.h"

template <typename TimerManagerType>
IRC_Client_Stream_T<TimerManagerType>::IRC_Client_Stream_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream_T::IRC_Client_Stream_T"));

}

template <typename TimerManagerType>
bool
IRC_Client_Stream_T<TimerManagerType>::load (Stream_ILayout* layout_in,
                                             bool& delete_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream_T::load"));

  if (!inherited::load (layout_in,
                        delete_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n"),
                ACE_TEXT (libacenetwork_default_irc_stream_name_string)));
    return false;
  } // end IF

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IRC_Client_Module_Marshal_Module (this,
                                                    ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IRC_Client_Module_Parser_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_MODULE_PARSER_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IRC_Client_Module_StatisticReport_Module (this,
                                                            ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IRC_Client_Module_IRCHandler_Module (this,
                                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename TimerManagerType>
bool
IRC_Client_Stream_T<TimerManagerType>::initialize (const IRC_Client_StreamConfiguration_t& configuration_in,
                                                   ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.configuration_);
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct IRC_Client_SessionData* session_data_p = NULL;
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  IRC_Client_Module_Bisector_t* bisector_impl_p = NULL;
  IRC_Client_Connection_Manager_t* connection_manager_p = NULL;
  IRC_Client_IConnection_t* iconnection_p = NULL;

  // allocate a new session state, reset stream
  const_cast<IRC_Client_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (libacenetwork_default_irc_stream_name_string)));
    goto error;
  } // end IF
  const_cast<IRC_Client_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !
  session_data_p =
      &const_cast<struct IRC_Client_SessionData&> (inherited::sessionData_->getR ());
  inherited::state_.sessionData = session_data_p;
  //session_data_p->sessionId = configuration_in.sessionId;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  iconnection_p = connection_manager_p->get (handle_in);
  ACE_ASSERT (iconnection_p);
  session_data_p->sessionState =
    &const_cast<struct IRC_SessionState&> (iconnection_p->state ());

  // ---------------------------------------------------------------------------
  // ******************* Marshal ************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_MODULE_MARSHAL_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (libacenetwork_default_irc_stream_name_string),
                ACE_TEXT (IRC_DEFAULT_MODULE_MARSHAL_NAME_STRING)));
    goto error;
  } // end IF
  bisector_impl_p =
    dynamic_cast<IRC_Client_Module_Bisector_t*> (module_p->writer ());
  if (!bisector_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<IRC_Module_Bisector_T> failed, aborting\n"),
                ACE_TEXT (libacenetwork_default_irc_stream_name_string)));
    goto error;
  } // end IF
  bisector_impl_p->setP (&(inherited::state_));

  // enqueue the module
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (libacenetwork_default_irc_stream_name_string)));
      goto error;
    } // end IF

  inherited::isInitialized_ = true;
  //   inherited::dump_state();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<IRC_Client_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
