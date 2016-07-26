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

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_common_modules.h"
#include "test_u_module_headerparser.h"
#include "test_u_module_protocolhandler.h"

Net_Stream::Net_Stream (const std::string& name_in)
 : inherited (name_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::Net_Stream"));

}

Net_Stream::~Net_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::~Net_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Net_Stream::load (Stream_ModuleList_t& modules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::load"));

  // initialize return value(s)
  for (Stream_ModuleListIterator_t iterator = modules_out.begin ();
       iterator != modules_out.end ();
       iterator++)
    delete *iterator;
  modules_out.clear ();

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->moduleHandlerConfiguration);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Net_Module_ProtocolHandler_Module (ACE_TEXT_ALWAYS_CHAR ("ProtocolHandler"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Net_Module_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                      NULL,
                                                      false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Net_Module_HeaderParser_Module (ACE_TEXT_ALWAYS_CHAR ("HeaderParser"),
                                                  NULL,
                                                  false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Net_Module_SocketHandler_Module (ACE_TEXT_ALWAYS_CHAR ("SocketHandler"),
                                                   NULL,
                                                   false),
                  false);
  modules_out.push_back (module_p);

  return true;
}

bool
Net_Stream::initialize (const Net_StreamConfiguration& configuration_in,
                        bool setupPipeline_in,
                        bool resetSessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.protocolConfiguration);
  ACE_ASSERT (!inherited::isInitialized_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  Net_StreamSessionData& session_data_r =
      const_cast<Net_StreamSessionData&> (inherited::sessionData_->get ());
  session_data_r.sessionID = configuration_in.sessionID;

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* Socket Handler ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("SocketHandler")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("SocketHandler")));
    return false;
  } // end IF
  //  socketHandler_.initialize (configuration_in.moduleConfiguration_2);
  Net_Module_SocketHandler* socketHandler_impl_p =
    dynamic_cast<Net_Module_SocketHandler*> (module_p->writer ());
  if (!socketHandler_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_SocketHandler> failed, aborting\n")));
    return false;
  } // end IF
//  if (!socketHandler_impl_p->initialize (configuration_in.moduleHandlerConfiguration_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
//                socketHandler_.name ()));
//    return false;
//  } // end IF
  if (!socketHandler_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                module_p->name ()));
    return false;
  } // end IF
  socketHandler_impl_p->initialize ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup (configuration_in.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;
}

void
Net_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::ping"));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("ProtocolHandler")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, returning\n"),
                ACE_TEXT ("ProtocolHandler")));
    return;
  } // end IF
  Net_Module_ProtocolHandler* protocolHandler_impl_p = NULL;
  protocolHandler_impl_p =
    dynamic_cast<Net_Module_ProtocolHandler*> (module_p->writer ());
  if (!protocolHandler_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, returning\n")));
    return;
  } // end IF

  // delegate to this module
  protocolHandler_impl_p->handleTimeout (NULL);
}

bool
Net_Stream::collect (Net_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Net_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Net_StreamSessionData& session_data_r =
      const_cast<Net_StreamSessionData&> (inherited::sessionData_->get ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistics module
  bool result_2 = false;
  try {
    result_2 = runtimeStatistic_impl->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}

void
Net_Stream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::report"));

//   Net_Module_Statistic_ReaderTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report ());

  // just a dummy
  ACE_ASSERT (false);

  ACE_NOTREACHED (return;)
}
