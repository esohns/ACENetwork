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

#include "IRC_client_stream.h"

#include <string>

IRC_Client_Stream::IRC_Client_Stream ()
 : inherited ()
 , IRCMarshal_ (ACE_TEXT_ALWAYS_CHAR ("IRCMarshal"),
                NULL)
 , IRCParser_ (ACE_TEXT_ALWAYS_CHAR ("IRCParser"),
               NULL)
 //, IRCHandler_ (ACE_TEXT_ALWAYS_CHAR ("IRCHandler"),
 //               NULL)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::IRC_Client_Stream"));

  // remember the ones we "own"...
  // *TODO*: clean this up
  // *NOTE*: one problem is that we need to explicitly close() all
  // modules which we have NOT enqueued onto the stream (e.g. because init()
  // failed...)
  inherited::availableModules_.push_front (&IRCMarshal_);
  inherited::availableModules_.push_front (&IRCParser_);
  inherited::availableModules_.push_front (&runtimeStatistic_);

  // *TODO*: fix ACE bug: modules should initialize their "next" member to NULL
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

IRC_Client_Stream::~IRC_Client_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::~IRC_Client_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
IRC_Client_Stream::initialize (const IRC_Client_StreamConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !

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

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------
  if (configuration_in.module)
  {
    // *TODO*: (at least part of) this procedure belongs in libACEStream
    //         --> remove type inferences
    inherited::IMODULE_T* module_2 =
      dynamic_cast<inherited::IMODULE_T*> (configuration_in.module);
    if (!module_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                  ACE_TEXT (configuration_in.module->name ())));
      return false;
    } // end IF
    if (!module_2->initialize (configuration_in.moduleConfiguration_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                  ACE_TEXT (configuration_in.module->name ())));
      return false;
    } // end IF
    Stream_Task_t* task_p = configuration_in.module->writer ();
    ACE_ASSERT (task_p);
    inherited::IMODULEHANDLER_T* module_handler_p =
        dynamic_cast<inherited::IMODULEHANDLER_T*> (task_p);
    if (!module_handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
                  ACE_TEXT (configuration_in.module->name ())));
      return false;
    } // end IF
    if (!module_handler_p->initialize (configuration_in.moduleHandlerConfiguration_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module handler, aborting\n"),
                  ACE_TEXT (configuration_in.module->name ())));
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

  // ******************* Runtime Statistics ************************
  IRC_Client_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF
  if (!runtimeStatistic_impl->initialize (configuration_in.statisticReportingInterval,
                                          configuration_in.messageAllocator))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&runtimeStatistic_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF

//   // ******************* IRC Handler ************************
//   IRC_Client_Module_IRCHandler* IRCHandler_impl = NULL;
//   IRCHandler_impl = dynamic_cast<IRC_Client_Module_IRCHandler*> (//                                      myIRCHandler.writer());
//   if (!IRCHandler_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<IRC_Client_Module_IRCHandler) failed> (aborting\n")));
//     return false;
//   } // end IF
//   if (!IRCHandler_impl->init(configuration_in.messageAllocator,
//                              (configuration_in.clientPingInterval ? false // servers shouldn't receive "pings" in the first place
//                                                                   : RPG_NET_DEF_CLIENT_PING_PONG), // auto-answer "ping" as a client ?...
//                              (configuration_in.clientPingInterval == 0))) // clients print ('.') dots for received "pings"...
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
//                myIRCHandler.name()));
//     return false;
//   } // end IF
//
//   // enqueue the module...
//   if (inherited::push(&myIRCHandler))
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
//                myIRCHandler.name()));
//     return false;
//   } // end IF

  // ******************* IRC Parser ************************
  IRC_Client_Module_IRCParser* IRCParser_impl = NULL;
  IRCParser_impl =
    dynamic_cast<IRC_Client_Module_IRCParser*> (IRCParser_.writer ());
  if (!IRCParser_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCParser) failed> (aborting\n")));
    return false;
  } // end IF
  if (!IRCParser_impl->initialize (configuration_in.messageAllocator,                            // message allocator
                                   configuration_in.moduleHandlerConfiguration_2.crunchMessages, // "crunch" messages ?
                                   configuration_in.moduleHandlerConfiguration_2.traceScanning,  // debug scanner ?
                                   configuration_in.moduleHandlerConfiguration_2.traceParsing))  // debug parser ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRCParser_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&IRCParser_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (IRCParser_.name ())));
    return false;
  } // end IF

  // ******************* IRC Marshal ************************
  IRC_Client_Module_IRCSplitter* IRCSplitter_impl = NULL;
  IRCSplitter_impl =
   dynamic_cast<IRC_Client_Module_IRCSplitter*> (IRCMarshal_.writer ());
  if (!IRCSplitter_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCSplitter> failed, aborting\n")));
    return false;
  } // end IF
  if (!IRCSplitter_impl->initialize (configuration_in.moduleHandlerConfiguration_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRCMarshal_.name ())));
    return false;
  } // end IF
  if (!IRCSplitter_impl->initialize (state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRCMarshal_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  IRCMarshal_.arg (inherited::sessionData_);
  if (inherited::push (&IRCMarshal_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (IRCMarshal_.name ())));
    return false;
  } // end IF

  // set (session) message allocator
  // *TODO*: clean this up ! --> sanity check
  ACE_ASSERT (configuration_in.messageAllocator);
  inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;
//   inherited::dump_state();

  return true;
}

bool
IRC_Client_Stream::collect (IRC_Client_RuntimeStatistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::collect"));

  IRC_Client_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl =
    dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t*> (const_cast<IRC_Client_Module_RuntimeStatistic_Module&> (runtimeStatistic_).writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module...
  return (runtimeStatistic_impl->collect (data_out));
}

void
IRC_Client_Stream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::report"));

//   RPG_Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<RPG_Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report());

  // just a dummy
  ACE_ASSERT (false);

  ACE_NOTREACHED (return;)
}

void
IRC_Client_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::ping"));

  // delegate to the head module, skip over ACE_Stream_Head...
  MODULE_T* module_p = inherited::head ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF
  module_p = module_p->next ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module_p == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl = NULL;
  control_impl = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->reader ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  } // end IF

  // *TODO*
  try
  {
//    control_impl->stop ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::stop (module: \"%s\"), returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  }
}
