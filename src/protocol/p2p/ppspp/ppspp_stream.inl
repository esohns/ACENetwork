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

#include <ace/Log_Msg.h>

#include "net_macros.h"

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::HTTP_Stream_T (const std::string& name_in)
 : inherited (name_in)
 , marshal_ (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
             NULL,
             false)
 //, parser_ (ACE_TEXT_ALWAYS_CHAR ("Parser"),
 //           NULL,
 //           false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
//, handler_ (ACE_TEXT_ALWAYS_CHAR ("Handler"),
//            NULL,
//            false)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::HTTP_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::~HTTP_Stream_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::~HTTP_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::load (Stream_ModuleList_t& modules_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::load"));

  modules_out.push_back (&runtimeStatistic_);
  //modules_out.push_back (&parser_);
  modules_out.push_back (&marshal_);
  //modules_out.push_back (&handler_);

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  session_data_r.sessionID = configuration_in.sessionID;

  int result = -1;
  typename inherited::MODULE_T* module_p = NULL;
  //if (configuration_in.notificationStrategy)
  //{
  //  module_p = inherited::head ();
  //  if (!module_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module found, aborting\n")));
  //    return false;
  //  } // end IF
  //  typename inherited::TASK_T* task_p = module_p->reader ();
  //  if (!task_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module reader task found, aborting\n")));
  //    return false;
  //  } // end IF
  //  typename inherited::QUEUE_T* queue_p = task_p->msg_queue ();
  //  if (!queue_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module reader task queue found, aborting\n")));
  //    return false;
  //  } // end IF
  //  queue_p->notification_strategy (configuration_in.notificationStrategy);
  //} // end IF

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------
  if (configuration_in.module)
  {
    // *TODO*: (at least part of) this procedure belongs in libACEStream
    //         --> remove type inferences
    typename inherited::IMODULE_T* module_2 =
      dynamic_cast<typename inherited::IMODULE_T*> (configuration_in.module);
    if (!module_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    if (!module_2->initialize (*configuration_in.moduleConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    Stream_Task_t* task_p = configuration_in.module->writer ();
    ACE_ASSERT (task_p);
    typename inherited::IMODULEHANDLER_T* module_handler_p =
        dynamic_cast<typename inherited::IMODULEHANDLER_T*> (task_p);
    if (!module_handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    if (!module_handler_p->initialize (*configuration_in.moduleHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module handler, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    //result = inherited::push (configuration_in.module);
    //if (result == -1)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
    //              configuration_in.module->name ()));
    //  return false;
    //} // end IF
  } // end IF

  // ---------------------------------------------------------------------------

  //   // ******************* Handler ************************
  //   IRC_Module_Handler* handler_impl = NULL;
  //   handler_impl = dynamic_cast<IRC_Module_Handler*> (handler_.writer ());
  //   if (!handler_impl)
  //   {
  //     ACE_DEBUG ((LM_ERROR,
  //                 ACE_TEXT ("dynamic_cast<IRC_Module_Handler> failed, aborting\n")));
  //     return false;
  //   } // end IF
  //   if (!handler_impl->initialize (configuration_in.messageAllocator,
  //                                  (configuration_in.clientPingInterval ? false // servers shouldn't receive "pings" in the first place
  //                                                                       : NET_DEF_PING_PONG), // auto-answer "ping" as a client ?...
  //                                  (configuration_in.clientPingInterval == 0))) // clients print ('.') dots for received "pings"...
  //   {
  //     ACE_DEBUG ((LM_ERROR,
  //                 ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //                 handler_.name ()));
  //     return false;
  //   } // end IF
  //
  //   // enqueue the module...
  //   if (inherited::push (&handler_))
  //   {
  //     ACE_DEBUG ((LM_ERROR,
  //                 ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
  //                 handler_.name ()));
  //     return false;
  //   } // end IF

  // ******************* Runtime Statistics ************************
  //STATISTIC_WRITER_T* runtimeStatistic_impl_p =
  //  dynamic_cast<STATISTIC_WRITER_T*> (runtimeStatistic_.writer ());
  //if (!runtimeStatistic_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_T> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval,
  //                                          configuration_in.messageAllocator))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              runtimeStatistic_.name ()));
  //  return false;
  //} // end IF

  //// enqueue the module...
  //result = inherited::push (&runtimeStatistic_);
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
  //              runtimeStatistic_.name ()));
  //  return false;
  //} // end IF

  //// ******************* Parser ************************
  //PARSER_T* parser_impl_p = NULL;
  //parser_impl_p =
  //  dynamic_cast<PARSER_T*> (parser_.writer ());
  //if (!parser_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<IRC_Module_Parser_T*> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!parser_impl_p->initialize (configuration_in.messageAllocator,                            // message allocator
  //                                configuration_in.moduleHandlerConfiguration_2.crunchMessages, // "crunch" messages ?
  //                                configuration_in.moduleHandlerConfiguration_2.traceScanning,  // debug scanner ?
  //                                configuration_in.moduleHandlerConfiguration_2.traceParsing))  // debug parser ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              parser_.name ()));
  //  return false;
  //} // end IF

  //// enqueue the module...
  //result = inherited::push (&parser_);
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
  //              parser_.name ()));
  //  return false;
  //} // end IF

  // ******************* Marshal ************************
  PARSER_T* parser_impl_p = dynamic_cast<PARSER_T*> (marshal_.writer ());
  if (!parser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<HTTP_Module_Parser_T*> failed, aborting\n")));
    return false;
  } // end IF
  if (!parser_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                marshal_.name ()));
    return false;
  } // end IF
  if (!parser_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                marshal_.name ()));
    return false;
  } // end IF

  // enqueue the module...
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  marshal_.arg (inherited::sessionData_);
  //result = inherited::push (&marshal_);
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
  //              marshal_.name ()));
  //  return false;
  //} // end IF

  // set (session) message allocator
  // *TODO*: clean this up ! --> sanity check
  ACE_ASSERT (configuration_in.messageAllocator);
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;
//   inherited::dump_state();

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::collect"));

  STATISTIC_WRITER_T* runtimeStatistic_impl_p = NULL;
  runtimeStatistic_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module
  return runtimeStatistic_impl_p->collect (data_out);
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::report"));

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
