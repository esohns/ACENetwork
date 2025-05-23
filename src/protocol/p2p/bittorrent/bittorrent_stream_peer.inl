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

#include "bittorrent_defines.h"

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
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
BitTorrent_PeerStream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        TimerManagerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionConfigurationType,
                        ConnectionStateType,
                        HandlerConfigurationType,
                        SessionStateType,
                        ConnectionManagerType,
                        UserDataType>::BitTorrent_PeerStream_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStream_T::BitTorrent_PeerStream_T"));

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
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
BitTorrent_PeerStream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        TimerManagerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionConfigurationType,
                        ConnectionStateType,
                        HandlerConfigurationType,
                        SessionStateType,
                        ConnectionManagerType,
                        UserDataType>::load (Stream_ILayout* layout_inout,
                                             bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStream_T::load"));

  // initialize return value(s)
  deleteModules_out = true;

  inherited::load (layout_inout,
                   deleteModules_out);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  MODULE_MARSHAL_T (this,
                                    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  MODULE_STATISTIC_T (this,
                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  //ACE_NEW_RETURN (module_p,
  //                MODULE_HANDLER_T (this,
  //                                  ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //  module_p = NULL;

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
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
BitTorrent_PeerStream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        TimerManagerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionConfigurationType,
                        ConnectionStateType,
                        HandlerConfigurationType,
                        SessionStateType,
                        ConnectionManagerType,
                        UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
                                                   ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  SessionDataType* session_data_p = NULL;

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
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  ACE_ASSERT (session_data_p);
  session_data_p->stream = this;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  inherited::isInitialized_ = false;

  return false;
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
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
BitTorrent_PeerStream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        TimerManagerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionConfigurationType,
                        ConnectionStateType,
                        HandlerConfigurationType,
                        SessionStateType,
                        ConnectionManagerType,
                        UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStream_T::collect"));

  typename inherited::MODULE_T* module_p = NULL;
  module_p =
      const_cast<typename inherited::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: module \"%s\" not found, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  STATISTIC_WRITER_T* statisticReport_impl_p = NULL;
  statisticReport_impl_p =
      dynamic_cast<STATISTIC_WRITER_T*> (module_p->writer ());
  if (!statisticReport_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  return statisticReport_impl_p->collect (data_out);
}
