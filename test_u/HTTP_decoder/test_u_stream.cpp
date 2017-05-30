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

#include "ace/Synch.h"

//#include "http_scanner.h"

#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "test_u_session_message.h"

Test_U_Stream::Test_U_Stream (const std::string& name_in)
 : inherited (name_in,
              true)
 , IO_ (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
        NULL,
        false)
// , dump_ (ACE_TEXT_ALWAYS_CHAR ("FileDump"),
//          NULL,
//          false)
 , marshal_ (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
             NULL,
             false)
 , statisticReport_ (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
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
Test_U_Stream::load (Stream_ModuleList_t& modules_out,
                     bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d
  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&statisticReport_);
  modules_out.push_back (&marshal_);
//  modules_out.push_back (&dump_);
  modules_out.push_back (&IO_);

  return true;
}

bool
Test_U_Stream::initialize (const struct Test_U_StreamConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  if (inherited::isInitialized_)
  {
    if (!inherited::finalize ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));
  } // end IF

  bool result = false;
  bool setup_pipeline = configuration_in.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<struct Test_U_StreamConfiguration&> (configuration_in).setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  const_cast<struct Test_U_StreamConfiguration&> (configuration_in).setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  struct Test_U_HTTPDecoder_SessionData& session_data_r =
      const_cast<struct Test_U_HTTPDecoder_SessionData&> (inherited::sessionData_->get ());
  session_data_r.sessionID = configuration_in.sessionID;
  // *TODO*: remove type inferences
  //session_data_r.targetFileName =
  //  configuration_in.moduleHandlerConfiguration->targetFileName;

  // *IMPORTANT NOTE*: a connection data processing stream may be appended
  //                   ('outbound' scenario) or prepended ('inbound' (e.g.
  //                   listener-based) scenario) to another stream. In the first
  //                   case, the net io (head) module behaves in a somewhat
  //                   particular manner, as it may be neither 'active' (run a
  //                   dedicated thread) nor 'passive' (borrow calling thread in
  //                   start()). Instead, it can behave as a regular
  //                   synchronous (i.e. passive) module; this reduces the
  //                   thread-count and generally improves efficiency
//  bool reset_configuration = false;
//  enum Stream_HeadModuleConcurrency concurrency_mode;
//  bool is_concurrent;
//  if (!configuration_in.moduleHandlerConfiguration->inbound)
//  {
//    concurrency_mode = configuration_in.moduleHandlerConfiguration->concurrency;
//    is_concurrent = configuration_in.moduleHandlerConfiguration->concurrent;

//    configuration_in.moduleHandlerConfiguration->concurrency =
//      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
//    configuration_in.moduleHandlerConfiguration->concurrent = true;

//    reset_configuration = true;
//  } // end IF

  // ---------------------------------------------------------------------------
  READER_T* IOReader_impl_p = NULL;
  WRITER_T* IOWriter_impl_p = NULL;
  Test_U_ModuleHandlerConfigurationsIterator_t iterator =
      const_cast<struct Test_U_StreamConfiguration&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());

  (*iterator).second->targetFileName.clear ();

  // ******************* IO ************************
//  IO_.initialize (*configuration_in.moduleConfiguration);
  IOWriter_impl_p = dynamic_cast<WRITER_T*> (IO_.writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n")));
    goto error;
  } // end IF
  IOWriter_impl_p->set (&(inherited::state_));

  IOReader_impl_p = dynamic_cast<READER_T*> (IO_.reader ());
  if (!IOReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOReader_T> failed, aborting\n")));
    goto error;
  } // end IF
  if (!IOReader_impl_p->initialize (*(*iterator).second))
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

  if (configuration_in.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  result = true;

error:
  if (reset_setup_pipeline)
    const_cast<struct Test_U_StreamConfiguration&> (configuration_in).setupPipeline =
      setup_pipeline;
//  if (reset_configuration)
//  {
//    configuration_in.moduleHandlerConfiguration->concurrency = concurrency_mode;
//    configuration_in.moduleHandlerConfiguration->concurrent = is_concurrent;
//  } // end IF

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

  Test_U_Module_StatisticReport_WriterTask_t* statisticReport_impl =
    dynamic_cast<Test_U_Module_StatisticReport_WriterTask_t*> (statisticReport_.writer ());
  if (!statisticReport_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_StatisticReport_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF

  // synch access
  struct Test_U_HTTPDecoder_SessionData& session_data_r =
      const_cast<struct Test_U_HTTPDecoder_SessionData&> (inherited::sessionData_->get ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistics module
  bool result_2 = false;
  try {
    result_2 = statisticReport_impl->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
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
