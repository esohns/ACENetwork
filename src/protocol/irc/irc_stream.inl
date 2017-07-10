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
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
IRC_Stream_T<StreamStateType,
             ConfigurationType,
             StatisticContainerType,
             ModuleHandlerConfigurationType,
             SessionDataType,
             SessionDataContainerType,
             ControlMessageType,
             DataMessageType,
             SessionMessageType,
             UserDataType>::IRC_Stream_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::IRC_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
IRC_Stream_T<StreamStateType,
             ConfigurationType,
             StatisticContainerType,
             ModuleHandlerConfigurationType,
             SessionDataType,
             SessionDataContainerType,
             ControlMessageType,
             DataMessageType,
             SessionMessageType,
             UserDataType>::~IRC_Stream_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::~IRC_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

//template <typename StreamStateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename UserDataType>
//bool
//IRC_Stream_T<StreamStateType,
//             ConfigurationType,
//             StatisticContainerType,
//             ModuleHandlerConfigurationType,
//             SessionDataType,
//             SessionDataContainerType,
//             ControlMessageType,
//             DataMessageType,
//             SessionMessageType,
//             UserDataType>::load (Stream_ModuleList_t& modules_out,
//                                  bool& deleteModules_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::load"));
//
//  deleteModules_out = false;
//
//  modules_out.push_back (&runtimeStatistic_);
//  modules_out.push_back (&parser_);
//  modules_out.push_back (&marshal_);
//
//  return true;
//}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
bool
IRC_Stream_T<StreamStateType,
             ConfigurationType,
             StatisticContainerType,
             ModuleHandlerConfigurationType,
             SessionDataType,
             SessionDataContainerType,
             ControlMessageType,
             DataMessageType,
             SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
             UserDataType>::initialize (const CONFIGURATION_T& configuration_in)
#else
             UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  SessionDataType* session_data_p = NULL;
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  BISECTOR_T* bisector_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_irc_stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !
  session_data_p =
      &const_cast<SessionDataType&> (inherited::sessionData_->get ());
  inherited::state_.currentSessionData = session_data_p;
  session_data_p->sessionID = configuration_in.configuration_.sessionID;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------

  // ******************* Marshal ************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("Marshal")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_irc_stream_name_string_),
                ACE_TEXT ("Marshal")));
    goto error;
  } // end IF

  bisector_impl_p = dynamic_cast<BISECTOR_T*> (module_p->writer ());
  if (!bisector_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<IRC_Module_Bisector_T> failed, aborting\n"),
                ACE_TEXT (stream_irc_stream_name_string_)));
    goto error;
  } // end IF
  bisector_impl_p->set (&(inherited::state_));

  // enqueue the module
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_irc_stream_name_string_)));
      goto error;
    } // end IF

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return false;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
bool
IRC_Stream_T<StreamStateType,
             ConfigurationType,
             StatisticContainerType,
             ModuleHandlerConfigurationType,
             SessionDataType,
             SessionDataContainerType,
             ControlMessageType,
             DataMessageType,
             SessionMessageType,
             UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  STATISTIC_WRITER_T* statistic_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_StatisticReport_WriterTask_T> failed, aborting\n")));
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

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result_2)
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

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
IRC_Stream_T<StreamStateType,
             ConfigurationType,
             StatisticContainerType,
             ModuleHandlerConfigurationType,
             SessionDataType,
             SessionDataContainerType,
             ControlMessageType,
             DataMessageType,
             SessionMessageType,
             UserDataType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

//template <typename StreamStateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename UserDataType>
//void
//IRC_Stream_T<StreamStateType,
//             ConfigurationType,
//             StatisticContainerType,
//             ModuleHandlerConfigurationType,
//             SessionDataType,
//             SessionDataContainerType,
//             ControlMessageType,
//             DataMessageType,
//             SessionMessageType,
//             UserDataType>::ping ()
//{
//  NETWORK_TRACE (ACE_TEXT ("IRC_Stream_T::ping"));
//
//  // delegate to the head module, skip over ACE_Stream_Head...
//  typename inherited::MODULE_T* module_p = inherited::head ();
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("no head module found, returning\n")));
//    return;
//  } // end IF
//  module_p = module_p->next ();
//  if (!module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("no head module found, returning\n")));
//    return;
//  } // end IF
//
//  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
//  if (module_p == inherited::tail ())
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));
//    return;
//  } // end IF
//
//  typename inherited::ISTREAM_CONTROL_T* control_impl = NULL;
//  control_impl =
//      dynamic_cast<typename inherited::ISTREAM_CONTROL_T*> (module_p->reader ());
//  if (!control_impl)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
//                module_p->name ()));
//    return;
//  } // end IF
//
//  // *TODO*
//  try {
////    control_impl->stop ();
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Stream_IStreamControl::stop (module: \"%s\"), returning\n"),
//                module_p->name ()));
//    return;
//  }
//}
