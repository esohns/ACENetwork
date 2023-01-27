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

#include "stream_macros.h"

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::Test_I_HTTPGet_Stream_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("SourceStream"),
              true)
 , netSource_ (ACE_TEXT_ALWAYS_CHAR ("NetSource"),
               NULL,
               false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
 , HTTPGet_ (ACE_TEXT_ALWAYS_CHAR ("HTTPGet"),
             NULL,
             false)
 , HTMLParser_ (ACE_TEXT_ALWAYS_CHAR ("HTMLParser"),
                NULL,
                false)
 //, HTMLWriter_ (ACE_TEXT_ALWAYS_CHAR ("HTMLWriter"),
 //               NULL,
 //               false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::Test_I_HTTPGet_Stream_T"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
  inherited::availableModules_.push_front (&netSource_);
  inherited::availableModules_.push_front (&runtimeStatistic_);
  inherited::availableModules_.push_front (&HTTPGet_);
  inherited::availableModules_.push_front (&HTMLParser_);
  //inherited::availableModules_.push_front (&HTMLWriter_);

  // *TODO* fix ACE bug: modules should initialize their "next" member to NULL
  //inherited::MODULE_T* module_p = NULL;
  //for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  //     iterator.next (module_p);
  //     iterator.advance ())
  //  module_p->next (NULL);
  for (inherited::MODULE_CONTAINER_ITERATOR_T iterator = inherited::availableModules_.begin ();
       iterator != inherited::availableModules_.end ();
       iterator++)
     (*iterator)->next (NULL);
}

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::~Test_I_HTTPGet_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::~Test_I_HTTPGet_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

template <typename ConnectorType>
void
Test_I_HTTPGet_Stream_T<ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::initialize (const Test_I_Stream_Configuration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
      configuration_in.moduleHandlerConfiguration->targetFileName;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ---------------------------------------------------------------------------

  //Test_I_Stream_Module_HTMLWriter* HTMLWriter_impl_p = NULL;
  Test_I_Stream_Module_HTMLParser* HTMLParser_impl_p = NULL;
  Test_I_Stream_Module_HTTPGet* HTTPGet_impl_p = NULL;
  Test_I_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p = NULL;
  SOURCE_WRITER_T* netSource_impl_p = NULL;

  // ******************* HTML Parser ************************
  HTMLParser_.initialize (*configuration_in.moduleConfiguration);
  HTMLParser_impl_p = dynamic_cast<Test_I_Stream_Module_HTMLParser*> (HTMLParser_.writer ());
  if (!HTMLParser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_HTMLParser> failed, aborting\n")));
    goto failed;
  } // end IF
  // *TODO*: remove type inferences
  if (!HTMLParser_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                HTMLParser_.name ()));
    goto failed;
  } // end IF
//  result = inherited::push (&HTMLParser_);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                HTMLParser_.name ()));
//    goto failed;
//  } // end IF

  // ******************* HTTP Get ************************
  HTTPGet_.initialize (*configuration_in.moduleConfiguration);
  HTTPGet_impl_p =
    dynamic_cast<Test_I_Stream_Module_HTTPGet*> (HTTPGet_.writer ());
  if (!HTTPGet_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_HTTPGet> failed, aborting\n")));
    goto failed;
  } // end IF
  // *TODO*: remove type inferences
  if (!HTTPGet_impl_p->initialize (configuration_in.messageAllocator,
                                   configuration_in.moduleHandlerConfiguration->URL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                HTTPGet_.name ()));
    goto failed;
  } // end IF
//  result = inherited::push (&HTTPGet_);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                HTTPGet_.name ()));
//    goto failed;
//  } // end IF

  // ******************* Runtime Statistics ************************
  runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  runtimeStatistic_impl_p =
      dynamic_cast<Test_I_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_RuntimeStatistic> failed, aborting\n")));
    goto failed;
  } // end IF
  if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval (seconds)
                                            configuration_in.printFinalReport,           // print final report ?
                                            configuration_in.messageAllocator))          // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                runtimeStatistic_.name ()));
    goto failed;
  } // end IF
//  result = inherited::push (&runtimeStatistic_);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                runtimeStatistic_.name ()));
//    goto failed;
//  } // end IF

  // ******************* Net Source ************************
  netSource_.initialize (*configuration_in.moduleConfiguration);
  netSource_impl_p = dynamic_cast<SOURCE_WRITER_T*> (netSource_.writer ());
  if (!netSource_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_Net_Source_T> failed, aborting\n")));
    goto failed;
  } // end IF
  if (!netSource_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                netSource_.name ()));
    goto failed;
  } // end IF
  if (!netSource_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                netSource_.name ()));
    goto failed;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  netSource_.arg (inherited::sessionData_);
//  result = inherited::push (&netSource_);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                netSource_.name ()));
//    goto failed;
//  } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());

  Test_I_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_I_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_Statistic_WriterTask_t> failed, aborting\n")));
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

  session_data_r.currentStatistic.timestamp = COMMON_TIME_NOW;

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

template <typename ConnectorType>
void
Test_I_HTTPGet_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::report"));

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
  ACE_NOTSUP

  ACE_NOTREACHED (return;)
}
