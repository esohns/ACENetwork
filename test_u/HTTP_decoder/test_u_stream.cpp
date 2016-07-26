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

#include "net_macros.h"

#include "test_u_session_message.h"

Test_U_Stream::Test_U_Stream (const std::string& name_in)
 : inherited (name_in)
 , IO_ (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
        NULL,
        false)
 , dump_ (ACE_TEXT_ALWAYS_CHAR ("FileDump"),
          NULL,
          false)
 , marshal_ (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
             NULL,
             false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
 , fileWriter_ (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                NULL,
                false)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

Test_U_Stream::~Test_U_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::~Test_U_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_Stream::load (Stream_ModuleList_t& modules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d
  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&runtimeStatistic_);
  modules_out.push_back (&marshal_);
  modules_out.push_back (&dump_);
  modules_out.push_back (&IO_);

  return true;
}

bool
Test_U_Stream::initialize (const Test_U_StreamConfiguration& configuration_in,
                           bool setupPipeline_in,
                           bool resetSessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  if (inherited::isInitialized_)
  {
    if (!inherited::finalize ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));
  } // end IF

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
  Test_U_StreamSessionData& session_data_r =
      const_cast<Test_U_StreamSessionData&> (inherited::sessionData_->get ());
  session_data_r.sessionID = configuration_in.sessionID;
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  //session_data_r.targetFileName =
  //  configuration_in.moduleHandlerConfiguration->targetFileName;
  //configuration_in.moduleConfiguration.streamState = &state_;

  // *IMPORTANT NOTE*: a connection data processing stream may be appended
  //                   ('outbound' scenario) or prepended ('inbound' (e.g.
  //                   listener-based) scenario) to another stream. In the first
  //                   case, the net io (head) module behaves in a somewhat
  //                   particular manner, as it may be neither 'active' (run a
  //                   dedicated thread) nor 'passive' (borrow calling thread in
  //                   start()). Instead, it can behave as a regular
  //                   synchronous (i.e. passive) module; this reduces the
  //                   thread-count and generally improves efficiency
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  // *TODO*: remove type inference
  bool reset_configuration = false;
  bool is_active, is_passive;
  if (!configuration_in.moduleHandlerConfiguration->inbound)
  {
    is_active = configuration_in.moduleHandlerConfiguration->active;
    is_passive = configuration_in.moduleHandlerConfiguration->passive;

    configuration_in.moduleHandlerConfiguration->active = false;
    configuration_in.moduleHandlerConfiguration->passive = false;

    reset_configuration = true;
  } // end IF

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleConfiguration);

  READER_T* IOReader_impl_p = NULL;
  WRITER_T* IOWriter_impl_p = NULL;
  std::string buffer;

  const_cast<Test_U_StreamConfiguration&> (configuration_in).moduleHandlerConfiguration->targetFileName =
      buffer;

  // ******************* IO ************************
//  IO_.initialize (*configuration_in.moduleConfiguration);
  IOWriter_impl_p = dynamic_cast<WRITER_T*> (IO_.writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n")));
    goto error;
  } // end IF
//  if (!IOWriter_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOWriter_T, aborting\n"),
//                IO_.name ()));
//    goto error;
//  } // end IF
  if (!IOWriter_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOWriter_T, aborting\n"),
                IO_.name ()));
    goto error;
  } // end IF
  //  IOWriter_impl_p->reset ();
  IOReader_impl_p = dynamic_cast<READER_T*> (IO_.reader ());
  if (!IOReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOReader_T> failed, aborting\n")));
    goto error;
  } // end IF
  if (!IOReader_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOReader_T, aborting\n"),
                IO_.name ()));
    goto error;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  IO_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  result = true;

error:
  if (reset_configuration)
  {
    configuration_in.moduleHandlerConfiguration->active = is_active;
    configuration_in.moduleHandlerConfiguration->passive = is_passive;
  } // end IF

  return result;
}

void
Test_U_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

bool
Test_U_Stream::collect (Net_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Test_U_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_U_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_Statistic_WriterTask_t*> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Test_U_StreamSessionData& session_data_r =
      const_cast<Test_U_StreamSessionData&> (inherited::sessionData_->get ());
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
  try
  {
    result_2 = runtimeStatistic_impl->collect (data_out);
  }
  catch (...)
  {
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
Test_U_Stream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::report"));

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
