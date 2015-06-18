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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::Net_ConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                        unsigned int statisticCollectionInterval_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , configuration_ ()
 , isRegistered_ (false)
 , manager_ (interfaceHandle_in)
 , sessionData_ (NULL)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             true)
 , statisticCollectHandlerID_ (-1)
 , status_ (NET_CONNECTION_STATUS_OK)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  if (statisticCollectionInterval_)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (statisticCollectionInterval_, 0);
    ACE_ASSERT (statisticCollectHandlerID_ == -1);
    ACE_Event_Handler* handler_p = &statisticCollectHandler_;
    statisticCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
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
//  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
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

//  // register with the connection manager, if any
//  if (!registerc ())
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), continuing\n")));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  int result = -1;

  // clean up timer, if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (statisticCollectHandlerID_,
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

  // clean up
  delete sessionData_;

//  // deregister with the connection manager, if any
//  deregister ();
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
bool
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::registerc ()
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
      isRegistered_ = manager_->registerc (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), continuing\n")));
      isRegistered_ = false;
    }
    if (!isRegistered_)
    {
      // *NOTE*: perhaps max# connections has been reached
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("failed to Net_IConnectionManager_T::registerc(), aborting\n")));
      return false;
    } // end IF

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    std::string local_address;
    AddressType local_SAP, remote_SAP;
    try
    {
      this->info (handle,
                  local_SAP,
                  remote_SAP);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::info(), aborting\n")));
      return false;
    }
    if (local_SAP.addr_to_string (buffer,
                                  sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Addr::addr_to_string(): \"%m\", continuing\n")));
    local_address = buffer;
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (remote_SAP.addr_to_string (buffer,
                                   sizeof (buffer)) == -1)
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::deregister ()
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

    OWN_TYPE* this_p = this;
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
bool
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_Connection_Status
Net_ConnectionBase_T<AddressType,
                     SocketConfigurationType,
                     ConfigurationType,
                     UserDataType,
                     SessionDataType,
                     StatisticContainerType,
                     StreamType>::status () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::status"));

  return status_;
}
