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
SMTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::SMTP_Stream_T ()
 : inherited ()
 , marshal_ (this,
             ACE_TEXT_ALWAYS_CHAR ("Marshal"))
 //, parser_ (ACE_TEXT_ALWAYS_CHAR ("Parser"),
 //           NULL,
 //           false)
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR ("StatisticReport"))
//, handler_ (ACE_TEXT_ALWAYS_CHAR ("Handler"),
//            NULL,
//            false)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Stream_T::SMTP_Stream_T"));

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
SMTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::load (Stream_ILayout* layout_inout,
                                         bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Stream_T::load"));

  deleteModules_out = false;

  layout_inout->append (&statistic_, NULL, 0);
  //modules_out.push_back (&parser_);
  layout_inout->append (&marshal_, NULL, 0);
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
SMTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              StatisticHandlerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);
  //ACE_ASSERT (configuration_in.configuration_->messageAllocator);

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
      const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  //session_data_r.sessionId = configuration_in.configuration_->sessionId;

//  int result = -1;
  typename inherited::MODULE_T* module_p = NULL;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  // ******************* Marshal ************************
  PARSER_T* parser_impl_p = dynamic_cast<PARSER_T*> (marshal_.writer ());
  if (!parser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<SMTP_Module_Parser_T*> failed, aborting\n")));
    return false;
  } // end IF
  parser_impl_p->setP (&(inherited::state_));

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
  //inherited::allocator_ = configuration_in.configuration_.messageAllocator;

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
SMTP_Stream_T<StreamStateType,
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
  NETWORK_TRACE (ACE_TEXT ("SMTP_Stream_T::collect"));

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
SMTP_Stream_T<StreamStateType,
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
  NETWORK_TRACE (ACE_TEXT ("SMTP_Stream_T::report"));

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

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
