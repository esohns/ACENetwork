/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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
#include "ace/OS.h"

#include "common_timer_manager.h"

#include "net_macros.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::Net_ConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                    unsigned int statisticsCollectionInterval_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 //, configuration_ ()
 , isRegistered_ (false)
 , manager_ (interfaceHandle_in)
 //, sessionData_ ()
 , statCollectionInterval_ (statisticsCollectionInterval_in)
 , statCollectHandler_ (ACTION_COLLECT,
                        this,
                        true)
 , statCollectHandlerID_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  if (statCollectionInterval_)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (statCollectionInterval_, 0);
    ACE_ASSERT (statCollectHandlerID_ == -1);
    ACE_Event_Handler* eh = &statCollectHandler_;
    statCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (eh,                               // event handler
                                                            NULL,                             // argument
                                                            COMMON_TIME_POLICY () + interval, // first wakeup time
                                                            interval);                        // interval
    if (statCollectHandlerID_ == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule(), continuing\n")));
    //else
    //        ACE_DEBUG ((LM_DEBUG,
    //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
    //                    statCollectHandlerID_,
    //                    statCollectionInterval_));
  } // end IF

  // init user data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
  //ACE_OS::memset (&sessionData_, 0, sizeof (sessionData_));
  if (manager_)
  {
    try
    { // (try to) get user data from the connection manager
      manager_->getData (configuration_,
                         sessionData_);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager::getData(), continuing\n")));
    }
  } // end IF

  // initialize: register with the connection manager, ...
  // *TODO*: find a way to pass role information (acceptor / connector)
  if (!initialize (ROLE_INVALID,
                   configuration_.socketConfiguration))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(), continuing\n")));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  // clean up timer if necessary
  if (statCollectHandlerID_ != -1)
  {
    const void* act = NULL;
    if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statCollectHandlerID_,
                                                            &act) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statCollectHandlerID_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  statCollectHandlerID_));
  } // end IF

  finalize ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::initialize (Net_ClientServerRole_t role_in,
                                                          const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  ACE_UNUSED_ARG (role_in);
  ACE_UNUSED_ARG (configuration_in);

  // sanity check(s)
  ACE_ASSERT (!isRegistered_);
  if (!manager_)
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("no connection manager, returning\n")));
    return true;
  } // end IF

  // (try to) register with the connection manager...
  // *NOTE*: as the connection has not fully open()ed, there is a small window
  //         for races here...
  if (manager_)
  {
    try
    {
      isRegistered_ = manager_->registerConnection (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager::registerConnection(), aborting\n")));
    }
    if (!isRegistered_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnectionManager::registerConnection(), aborting\n")));
      return false;
    } // end IF
  } // end IF

  return true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
void
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::finalize"));

  if (!manager_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("no connection manager, returning\n")));
    return;
  } // end IF

  if (isRegistered_)
  {
    // (try to) de-register with the connection manager...
    // *WARNING*: as registration happens BEFORE the connection has open()ed,
    //            there is a small window for races here...
    try
    {
      manager_->deregisterConnection (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
    }
    isRegistered_ = false;
  } // end IF
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
