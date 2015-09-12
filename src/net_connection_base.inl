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

#include "common_timer_manager_common.h"

#include "net_macros.h"

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::Net_ConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                          unsigned int statisticCollectionInterval_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , configuration_ ()
 , state_ ()
 , isRegistered_ (false)
 , manager_ (interfaceHandle_in)
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
    ACE_Event_Handler* handler_p = &statisticCollectHandler_;
    Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
    ACE_ASSERT (timer_manager_p);
    statisticCollectHandlerID_ =
      timer_manager_p->schedule_timer (handler_p,                  // event handler
                                       NULL,                       // argument
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (statisticCollectHandlerID_ == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(), continuing\n")));
//    else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                  statisticCollectHandlerID_,
//                  statisticCollectionInterval_));
  } // end IF

  // initialize configuration/user data
  if (manager_)
  {
    try
    { // (try to) get (default) configuration/user data from the connection
      // manager
      // *TODO*: remove type inference
      manager_->get (configuration_,
                     state_.userData);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::get(), continuing\n")));
    }
  } // end IF

//  // register with the connection manager, if any
//  if (!registerc ())
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  int result = -1;

  // clean up timer, if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    const void* act_p = NULL;
    Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
    ACE_ASSERT (timer_manager_p);
    result = timer_manager_p->cancel_timer (statisticCollectHandlerID_,
                                            &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statisticCollectHandlerID_));
//    else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
//                  statisticCollectHandlerID_));
  } // end IF

//  // deregister with the connection manager, if any
//  deregister ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
bool
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
#if defined (__GNUG__)
                     UserDataType>::registerc (ICONNECTION_T* connection_in)
#else
                     UserDataType>::registerc ()
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::registerc"));

  // sanity check(s)
  ACE_ASSERT (!isRegistered_);

  // (try to) register with the connection manager...
  // *WARNING*: as the connection has not fully open()ed yet, there is a small
  //            window for races here...
  if (manager_)
  {
    try
    {
#if defined (__GNUG__)
      // *WORKAROUND*: see header
      isRegistered_ = manager_->registerc (connection_in ? connection_in
                                                         : this);
#else
      isRegistered_ = manager_->registerc (this);
#endif
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), continuing\n")));
      isRegistered_ = false;
    }
    if (!isRegistered_)
    {
      // *NOTE*: most probable reason: maximum number of connections has been
      //         reached
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("failed to Net_IConnectionManager_T::registerc(), aborting\n")));
      return false;
    } // end IF

    int result = -1;
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    std::string local_address;
    AddressType local_SAP, remote_SAP;
    try
    {
#if defined (__GNUG__)
      // *WORKAROUND*: see header
      ICONNECTION_T* connection_p = (connection_in ? connection_in
                                                   : this);
      ACE_ASSERT (connection_p);
      connection_p->info (handle,
                          local_SAP,
                          remote_SAP);
#else
      this->info (handle,
                  local_SAP,
                  remote_SAP);
#endif
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::info(), aborting\n")));
      return false;
    }
    result = local_SAP.addr_to_string (buffer,
                                       sizeof (buffer),
                                       1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Addr::addr_to_string(): \"%m\", continuing\n")));
    local_address = buffer;
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = remote_SAP.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Addr::addr_to_string(): \"%m\", continuing\n")));

    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("registered connection [0x%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
                this, reinterpret_cast<unsigned int> (handle),
                ACE_TEXT (local_address.c_str ()),
                ACE_TEXT (buffer),
                manager_->numConnections ()));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("registered connection [0x%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
                this, handle,
                ACE_TEXT (local_address.c_str ()),
                ACE_TEXT (buffer),
                manager_->numConnections ()));
#endif
  } // end IF

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::deregister ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::deregister"));

  if (manager_ && isRegistered_)
  {
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_INET_Addr local_SAP, remote_SAP;
    try
    {
      this->info (handle,
                  local_SAP,
                  remote_SAP);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::info(), continuing\n")));
    }

    OWN_TYPE_T* this_p = this;
    unsigned int num_connections = manager_->numConnections () - 1;
    // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
                this_p, reinterpret_cast<unsigned int> (handle),
                num_connections));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
                this_p, handle,
                num_connections));
#endif

    // (try to) de-register with the connection manager...
    isRegistered_ = false;
    // *IMPORTANT NOTE*: may delete 'this'
    try
    {
      manager_->deregister (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::deregister(), continuing\n")));
    }
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
const ConfigurationType&
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::get"));

  return configuration_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
bool
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  // *NOTE*: when using a connection manager, the (default) configuration is
  //         retrieved in the ctor
  configuration_ = configuration_in;

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
const StateType&
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::state"));

  return state_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_Connection_Status
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     StreamType,
                     UserDataType>::status () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::status"));

  // *TODO*: remove type inference
  return state_.status;
}
