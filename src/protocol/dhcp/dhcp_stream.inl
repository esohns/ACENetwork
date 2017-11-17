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

#include "net_macros.h"

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::DHCP_Stream_T ()
 : inherited ()
 , marshal_ (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
             NULL,
             false)
 //, parser_ (ACE_TEXT_ALWAYS_CHAR ("Parser"),
 //           NULL,
 //           false)
 , statistic_ (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
               NULL,
               false)
//, handler_ (ACE_TEXT_ALWAYS_CHAR ("Handler"),
//            NULL,
//            false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::DHCP_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::load (Stream_ModuleList_t& modules_out,
                                         bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::load"));

  deleteModules_out = false;

  modules_out.push_back (&statistic_);
  //modules_out.push_back (&parser_);
  modules_out.push_back (&marshal_);
  //modules_out.push_back (&handler_);

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              SessionMessageType>::initialize (const CONFIGURATION_T& configuration_in)
#else
              SessionMessageType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::initialize"));

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
  session_data_r.sessionID = configuration_in.configuration_.sessionID;

//  int result = -1;
  typename inherited::MODULE_T* module_p = NULL;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------

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

  // ******************* Statistic ************************
  //STATISTIC_WRITER_T* statistic_report_impl_p =
  //  dynamic_cast<STATISTIC_WRITER_T*> (runtimeStatistic_.writer ());
  //if (!statistic_report_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_T> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!statistic_report_impl_p->initialize (configuration_in.statisticReportingInterval,
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
                ACE_TEXT ("dynamic_cast<DHCP_Module_Parser_T*> failed, aborting\n")));
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
  inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;
//   inherited::dump_state();

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::collect"));

  STATISTIC_WRITER_T* statistic_report_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (statistic_.writer ());
  if (!statistic_report_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module
  return statistic_report_impl_p->collect (data_out);
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::report"));

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
//   // delegate to this module
//   return (runtimeStatistic_impl->report());

  // just a dummy
  ACE_ASSERT (false);

  ACE_NOTREACHED (return;)
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::ping"));

  // delegate to the head module, skip over ACE_Stream_Head...
  typename inherited::MODULE_T* module_p = inherited::head ();
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

  typename inherited::ISTREAM_CONTROL_T* control_impl = NULL;
  control_impl =
    dynamic_cast<typename inherited::ISTREAM_CONTROL_T*> (module_p->reader ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  // *TODO*
  try {
//    control_impl->stop ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::stop (module: \"%s\"), returning\n"),
                module_p->name ()));
    return;
  }
}
