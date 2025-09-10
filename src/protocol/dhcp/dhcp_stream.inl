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
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionManagerType,
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
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionManagerType,
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
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->get ());
  session_data_r.sessionID = configuration_in.configuration_.sessionID;

//  int result = -1;
  typename inherited::MODULE_T* module_p = NULL;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------

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
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionManagerType,
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
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
DHCP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionManagerType,
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
