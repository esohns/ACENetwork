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

#include "net_stream.h"

#include <string>

#include "ace/Log_Msg.h"

#include "net_defines.h"
#include "net_macros.h"

Net_Stream::Net_Stream ()
 : inherited ()
 , socketHandler_ (std::string("SocketHandler"),
                   NULL)
 , headerParser_ (std::string("HeaderParser"),
                  NULL)
 , protocolHandler_ (std::string("ProtocolHandler"),
                     NULL)
 , runtimeStatistic_ (std::string("RuntimeStatistic"),
                      NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::Net_Stream"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
  inherited::availableModules_.push_front (&socketHandler_);
  inherited::availableModules_.push_front (&headerParser_);
  inherited::availableModules_.push_front (&runtimeStatistic_);
  inherited::availableModules_.push_front (&protocolHandler_);
  //inherited::availableModules_.insert_tail (&socketHandler_);
  //inherited::availableModules_.insert_tail (&headerParser_);
  //inherited::availableModules_.insert_tail (&runtimeStatistic_);
  //inherited::availableModules_.insert_tail (&protocolHandler_);

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

Net_Stream::~Net_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::~Net_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Net_Stream::initialize (const Net_StreamConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.protocolConfiguration);
  ACE_ASSERT (!inherited::isInitialized_);

  // allocate a new session state, reset stream
  if (!inherited::initialize ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize notification strategy (if any)
  // ------------------------------------
  // - push the final module onto the stream (if any)
  // ------------------------------------
  // - initialize modules
  // - push them onto the stream (tail-first) !
  // ------------------------------------

  inherited::sessionData_->sessionID = configuration_in.sessionID;

  int result = -1;
  inherited::MODULE_T* module_p = NULL;
  if (configuration_in.notificationStrategy)
  {
    module_p = inherited::head ();
    if (!module_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module found, aborting\n")));
      return false;
    } // end IF
    inherited::TASK_T* task_p = module_p->reader ();
    if (!task_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module reader task found, aborting\n")));
      return false;
    } // end IF
    inherited::QUEUE_T* queue_p = task_p->msg_queue ();
    if (!queue_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module reader task queue found, aborting\n")));
      return false;
    } // end IF
    queue_p->notification_strategy (configuration_in.notificationStrategy);
  } // end IF
//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  if (configuration_in.module)
  {
    inherited::IMODULE_T* module_2 =
      dynamic_cast<inherited::IMODULE_T*> (configuration_in.module);
    if (!module_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<Stream_IModule_T> failed, aborting\n")));
      return false;
    } // end IF
    if (!module_2->initialize (configuration_in.moduleConfiguration_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_IModule_T::initialize, aborting\n")));
      return false;
    } // end IF
    result = inherited::push (configuration_in.module);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (configuration_in.module->name ())));
      return false;
    } // end IF
  } // end IF

  // ---------------------------------------------------------------------------

  // ******************* Protocol Handler ************************
  protocolHandler_.initialize (configuration_in.moduleConfiguration_2);
  Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
  protocolHandler_impl =
      dynamic_cast<Net_Module_ProtocolHandler*> (protocolHandler_.writer ());
  if (!protocolHandler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, aborting\n")));
    return false;
  } // end IF
  if (!protocolHandler_impl->initialize (configuration_in.messageAllocator,
                                         configuration_in.protocolConfiguration->peerPingInterval,
                                         configuration_in.protocolConfiguration->pingAutoAnswer,
                                         configuration_in.protocolConfiguration->printPongMessages)) // print ('.') for received "pong"s...
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (protocolHandler_.name ())));
    return false;
  } // end IF
  result = inherited::push (&protocolHandler_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (protocolHandler_.name ())));
    return false;
  } // end IF

  // ******************* Runtime Statistics ************************
  runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl =
    dynamic_cast<Net_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_RuntimeStatistic> failed, aborting\n")));
    return false;
  } // end IF
  if (!runtimeStatistic_impl->initialize (configuration_in.statisticReportingInterval, // reporting interval (seconds)
                                          configuration_in.printFinalReport,           // print final report ?
                                          configuration_in.messageAllocator))          // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF
  result = inherited::push (&runtimeStatistic_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF

  // ******************* Header Parser ************************
  headerParser_.initialize (*configuration_in.moduleConfiguration);
  Net_Module_HeaderParser* headerParser_impl = NULL;
  headerParser_impl =
    dynamic_cast<Net_Module_HeaderParser*> (headerParser_.writer ());
  if (!headerParser_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_HeaderParser> failed, aborting\n")));
    return false;
  } // end IF
  if (!headerParser_impl->initialize ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (headerParser_.name ())));
    return false;
  } // end IF
  result = inherited::push (&headerParser_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (headerParser_.name ())));
    return false;
  } // end IF

  // ******************* Socket Handler ************************
  socketHandler_.initialize (configuration_in.moduleConfiguration_2);
  Net_Module_SocketHandler* socketHandler_impl = NULL;
  socketHandler_impl =
    dynamic_cast<Net_Module_SocketHandler*> (socketHandler_.writer ());
  if (!socketHandler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_SocketHandler> failed, aborting\n")));
    return false;
  } // end IF
  if (!socketHandler_impl->initialize (configuration_in.moduleHandlerConfiguration_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (socketHandler_.name ())));
    return false;
  } // end IF

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along
  socketHandler_.arg (&const_cast<Net_StreamConfiguration&> (configuration_in).userData);
  result = inherited::push (&socketHandler_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (socketHandler_.name ())));
    return false;
  } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;
}

void
Net_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::ping"));

  Net_Module_ProtocolHandler* protocolHandler_impl_p = NULL;
  protocolHandler_impl_p =
    dynamic_cast<Net_Module_ProtocolHandler*> (protocolHandler_.writer ());
  if (!protocolHandler_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, returning\n")));
    return;
  } // end IF

  // delegate to this module...
  protocolHandler_impl_p->handleTimeout (NULL);
}

bool
Net_Stream::collect (Net_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Net_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  if (inherited::sessionData_->lock)
  {
    result = inherited::sessionData_->lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  inherited::sessionData_->currentStatistic.timestamp = COMMON_TIME_NOW;

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
    inherited::sessionData_->currentStatistic = data_out;

  if (inherited::sessionData_->lock)
  {
    result = inherited::sessionData_->lock->release ();
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
