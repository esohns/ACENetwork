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

#include <ace/Synch.h>
#include "test_u_connection_stream.h"

#include <ace/Log_Msg.h>

#include "net_macros.h"

#include "test_u_message.h"
#include "test_u_session_message.h"
#include "test_u_common_modules.h"

Test_U_InboundConnectionStream::Test_U_InboundConnectionStream (const std::string& name_in)
 : inherited (name_in)
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
Test_U_InboundConnectionStream::load (Stream_ModuleList_t& modules_out,
                                      bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->moduleHandlerConfiguration);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Dump_Module (ACE_TEXT_ALWAYS_CHAR ("Dump"),
                                             NULL,
                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_DHCPDiscover_Module (ACE_TEXT_ALWAYS_CHAR ("DHCPDiscover"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_StatisticReport_Module (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                        NULL,
                                                        false),
                  false);
  modules_out.push_back (module_p);
//  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_Module_Parser_Module (ACE_TEXT_ALWAYS_CHAR ("Parser"),
//                                               NULL,
//                                               false),
//                  false);
//  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Marshal_Module (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                NULL,
                                                false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Net_IO_Module (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
                                               NULL,
                                               false),
                  false);
  modules_out.push_back (module_p);

  return true;
}

bool
Test_U_InboundConnectionStream::initialize (const Test_U_StreamConfiguration& configuration_in,
                                            bool setupPipeline_in,
                                            bool resetSessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  Test_U_DHCPClient_SessionData& session_data_r =
    const_cast<Test_U_DHCPClient_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID = configuration_in.sessionID;
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ---------------------------------------------------------------------------

  Test_U_Module_Net_Writer_t* netIO_impl_p = NULL;

  // ******************* Net IO ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("NetIO")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("NetIO")));
    return false;
  } // end IF
  //netIO_.initialize (*configuration_in.moduleConfiguration);
  netIO_impl_p =
      dynamic_cast<Test_U_Module_Net_Writer_t*> (module_p->writer ());
  if (!netIO_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_Net_Writer_t> failed, aborting\n")));
    goto failed;
  } // end IF
  netIO_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}

bool
Test_U_InboundConnectionStream::collect (DHCP_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_U_DHCPClient_SessionData& session_data_r =
    const_cast<Test_U_DHCPClient_SessionData&> (inherited::sessionData_->get ());

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  Test_U_Module_StatisticReport_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_U_Module_StatisticReport_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_StatisticReport_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

    // synch access
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
Test_U_InboundConnectionStream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::report"));

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

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_InboundConnectionStream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_InboundConnectionStream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

////////////////////////////////////////////////////////////////////////////////

Test_U_OutboundConnectionStream::Test_U_OutboundConnectionStream (const std::string& name_in)
 : inherited (name_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::Test_U_OutboundConnectionStream"));

}

Test_U_OutboundConnectionStream::~Test_U_OutboundConnectionStream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::~Test_U_OutboundConnectionStream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_OutboundConnectionStream::load (Stream_ModuleList_t& modules_out,
                                       bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_StatisticReport_Module (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                        NULL,
                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Streamer_Module (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                 NULL,
                                                 false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Net_IO_Module (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
                                               NULL,
                                               false),
                  false);
  modules_out.push_back (module_p);

  return true;
}

bool
Test_U_OutboundConnectionStream::initialize (const Test_U_StreamConfiguration& configuration_in,
                                             bool setupPipeline_in,
                                             bool resetSessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  Test_U_DHCPClient_SessionData& session_data_r =
    const_cast<Test_U_DHCPClient_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ---------------------------------------------------------------------------

  Test_U_Module_Net_Writer_t* netIO_impl_p = NULL;

  // ******************* Net IO ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("NetIO")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("NetIO")));
    return false;
  } // end IF
  //netIO_.initialize (*configuration_in.moduleConfiguration);
  netIO_impl_p = dynamic_cast<Test_U_Module_Net_Writer_t*> (module_p->writer ());
  if (!netIO_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_Net_Writer_t> failed, aborting\n")));
    goto failed;
  } // end IF
  netIO_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}

bool
Test_U_OutboundConnectionStream::collect (DHCP_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_U_DHCPClient_SessionData& session_data_r =
    const_cast<Test_U_DHCPClient_SessionData&> (inherited::sessionData_->get ());

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  Test_U_Module_StatisticReport_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_U_Module_StatisticReport_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_StatisticReport_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
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

  // delegate to the statistics module...
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
Test_U_OutboundConnectionStream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::report"));

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

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_OutboundConnectionStream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_OutboundConnectionStream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
