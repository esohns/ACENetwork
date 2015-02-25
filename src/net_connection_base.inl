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
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
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
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
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
          typename StatisticsContainerType>
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     StatisticsContainerType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  fini ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     StatisticsContainerType>::init (const ACE_INET_Addr& peerAddress_in,
                                                     unsigned short portNumber_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::init"));

  ACE_UNUSED_ARG (peerAddress_in);
  ACE_UNUSED_ARG (portNumber_in);

  // sanity check(s)
  if (!manager_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid connection manager, aborting\n")));
    return false;
  } // end if

  // (try to) register with the connection manager...
  // *warning*: as we register before the connection has fully opened, there
  // is a small window for races...
  try
  {
    isRegistered_ = manager_->registerConnection (this);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager::egisterConnection(), continuing\n")));
  }

  return true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     StatisticsContainerType>::fini ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::fini"));

  // sanity check(s)
  if (!manager_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid connection manager, returning\n")));
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
          typename StatisticsContainerType>
bool
Net_ConnectionBase_T<ConfigurationType,
                     SessionDataType,
                     StatisticsContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
