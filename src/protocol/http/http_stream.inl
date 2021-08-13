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

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "http_defines.h"

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename UserDataType>
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType,
              ConnectionManagerType,
              UserDataType>::HTTP_Stream_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::HTTP_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType,
              ConnectionManagerType,
              UserDataType>::load (Stream_ILayout* layout_out,
                                   bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::load"));

  // initialize return value(s)
  deleteModules_out = true;

  inherited::load (layout_out,
                   deleteModules_out);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  MODULE_MARSHAL_T (this,
                                    ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_out->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  MODULE_STATISTIC_T (this,
                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  layout_out->append (module_p, NULL, 0);

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
HTTP_Stream_T<StreamStateType,
              ConfigurationType,
              StatisticContainerType,
              TimerManagerType,
              ModuleHandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType,
              ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              UserDataType>::initialize (const CONFIGURATION_T& configuration_in,
#else
              UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
#endif
                                         ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  //  int result = -1;
//  SessionDataType* session_data_p = NULL;
  typename inherited::MODULE_T* module_p = NULL;
  typename inherited::WRITER_T* writer_impl_p = NULL;

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);

//  session_data_p =
//    &const_cast<SessionDataType&> (inherited::sessionData_->getR ());

  // ---------------------------------------------------------------------------

  // ******************* Marshal ************************
  module_p =
      const_cast<typename inherited::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  writer_impl_p = dynamic_cast<typename inherited::WRITER_T*> (module_p->writer ());
  if (!writer_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_Net_IOWriter_T*> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  writer_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
