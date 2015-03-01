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

#include "ace/OS.h"
#include "ace/Log_Msg.h"

#include "net_macros.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::Net_ConnectionBase_T ()
 : inherited (1,    // initial count
              true) // delete on zero ?
 , manager_ (NULL)
 //, configuration_ ()
 , sessionData_ ()
 , isRegistered_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  // init user data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
  //ACE_OS::memset (&sessionData_, 0, sizeof (sessionData_));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::Net_ConnectionBase_T (Net_IConnectionManager_t* interfaceHandle_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , manager_ (interfaceHandle_in)
 //, configuration_ ()
 , sessionData_ ()
 , isRegistered_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

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
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename StatisticsContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::initialize (const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

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
          typename TransportLayerType,
          typename StatisticsContainerType>
void
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::finalize"));

  // sanity check(s)
  if (!manager_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("no connection manager, returning\n")));

    return;
  } // end IF

  if (isRegistered_)
  {
    // (try to) de-register with the connection manager...
    // *WARNING*: as we register BEFORE the connection has fully opened, there
    // is a small window for races...
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
          typename TransportLayerType,
          typename StatisticsContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     TransportLayerType,
                     StatisticsContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
