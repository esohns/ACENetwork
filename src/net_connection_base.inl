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
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::Net_ConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                    unsigned int statisticCollectionInterval_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 //, configuration_ ()
 , isRegistered_ (false)
 , manager_ (interfaceHandle_in)
 , sessionData_ (NULL)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             true)
 , statisticCollectHandlerID_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  if (statisticCollectionInterval_)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (statisticCollectionInterval_, 0);
    ACE_ASSERT (statisticCollectHandlerID_ == -1);
    ACE_Event_Handler* eh = &statisticCollectHandler_;
    statisticCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (eh,                               // event handler
                                                            NULL,                             // argument
                                                            COMMON_TIME_NOW + interval, // first wakeup time
                                                            interval);                        // interval
    if (statisticCollectHandlerID_ == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule(), continuing\n")));
    //else
    //        ACE_DEBUG ((LM_DEBUG,
    //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
    //                    statisticCollectHandlerID_,
    //                    statisticCollectionInterval_));
  } // end IF

  // initialize configuration/user data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
  UserDataType* user_data_p = NULL;
  if (manager_)
  {
    try
    { // (try to) get (default) configuration/user data from the connection
      // manager
      manager_->get (configuration_,
                     user_data_p);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::get(), continuing\n")));
    }
  } // end IF

  // *TODO*: session data could be a member, then it would just require
  //         initialization...
  try
  {
    ACE_NEW_NORETURN (sessionData_,
                      SessionDataType (user_data_p,
                                       false));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_NORETURN(SessionDataType), continuing\n")));
  }
  if (!sessionData_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate SessionDataType, continuing\n")));
  } // end IF

  // initialize: register with the connection manager, ...
  // *TODO*: find a way to pass role information (acceptor / connector)
  if (!initialize (ROLE_INVALID,
                   configuration_.socketConfiguration))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(), continuing\n")));
}

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  // clean up timer if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    const void* act = NULL;
    if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_,
                                                            &act) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statisticCollectHandlerID_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  statisticCollectHandlerID_));
  } // end IF

  if (manager_ && isRegistered_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("still registered in dtor --> check implementation !\n")));

    finalize ();
  } // end IF
}

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     UserDataType,
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
  // *WARNING*: as the connection has not fully open()ed yet, there is a small
  //            window for races here...
  if (manager_)
  {
    try
    {
      isRegistered_ = manager_->registerc (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), aborting\n")));
    }
    if (!isRegistered_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnectionManager_T::registerc(), aborting\n")));
      return false;
    } // end IF
  } // end IF

  return true;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
void
Net_ConnectionBase_T<ConfigurationType,
                     UserDataType,
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
    // avoid loops (see dtor)
    isRegistered_ = false;

    // (try to) de-register with the connection manager...
    // *WARNING*: as registration happens BEFORE the connection has open()ed,
    //            there is a small window for races here...
    try
    {
      manager_->deregister (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
    }
  } // end IF
}

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     ITransportLayerType,
                     StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
