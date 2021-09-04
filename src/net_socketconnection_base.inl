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
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"

#include "common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::Net_SocketConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , configuration_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::Net_SocketConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::~Net_SocketConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::~Net_SocketConnectionBase_T"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  // wait for any worker(s)
  // *TODO*: remove type inference
  if (inherited::CONNECTION_BASE_T::configuration_->streamConfiguration.useThreadPerConnection)
  {
    result = ACE_Task_Base::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::send"));

  ACE_ASSERT (false);
  ACE_NOTSUP

  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::set (Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::set"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  ITRANSPORTLAYER_T* itransportlayer_p = this;
  ACE_ASSERT (itransportlayer_p);

  if (!itransportlayer_p->initialize (this->dispatch (),
                                      role_in,
                                      *configuration_.socketConfiguration))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ITransportLayer_T::initialize(), continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::open"));

  // sanity check(s)
  ACE_ASSERT (!configuration_);

  // step0: initialize this connection
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  switch (this->role ())
  {
    case NET_ROLE_CLIENT:
    {
      ICONNECTOR_T* iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      configuration_ = &(iconnector_p->get ());
      break;
    }
    case NET_ROLE_SERVER:
    {
      ILISTENER_T* ilistener_p = static_cast<ILISTENER_T*> (arg_in);
      ACE_ASSERT (ilistener_p);
      configuration_ = &(ilistener_p->get ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown role (was: %d), aborting\n"),
                  this->role ()));
      return -1;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->userData);
  ConfigurationType* configuration_p =
    configuration_->userData->configuration;
  ACE_ASSERT (configuration_p);
  if (!inherited::CONNECTION_BASE_T::initialize (*configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  // *TODO*: remove type inference
  int result = inherited::open (configuration_p);
  if (result == -1)
  {
    // *NOTE*: this can happen when the connection handle is still registered
    //         with the reactor (i.e. the reactor is still processing events on
    //         a file descriptor that has been closed and is now being reused by
    //         the system)
    // *NOTE*: more likely, this happened because the (select) reactor is out of
    //         "free" (read) slots
    int error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);

    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to HandlerType::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF

//  // step2: start a worker ?
//  if (inherited::configuration_.streamConfiguration.useThreadPerConnection)
//  {
//    ACE_thread_t thread_ids[1];
//    thread_ids[0] = 0;
//    ACE_hthread_t thread_handles[1];
//    thread_handles[0] = 0;
//    char thread_name[BUFSIZ];
//    ACE_OS::memset (thread_name, 0, sizeof (thread_name));
//    ACE_OS::strcpy (thread_name,
//                    ACE_TEXT_ALWAYS_CHAR (NET_CONNECTION_HANDLER_THREAD_NAME));
//    const char* thread_names[1];
//    thread_names[0] = thread_name;
//    result = inherited::activate ((THR_NEW_LWP      |
//                                   THR_JOINABLE     |
//                                   THR_INHERIT_SCHED),                    // flags
//                                  1,                                      // # threads
//                                  0,                                      // force spawning
//                                  ACE_DEFAULT_THREAD_PRIORITY,            // priority
//                                  NET_CONNECTION_HANDLER_THREAD_GROUP_ID, // group id
//                                  NULL,                                   // corresp. task --> use 'this'
//                                  thread_handles,                         // thread handle(s)
//                                  NULL,                                   // thread stack(s)
//                                  NULL,                                   // thread stack size(s)
//                                  thread_ids,                             // thread id(s)
//                                  thread_names);                          // thread name(s)
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", aborting\n")));
//      return -1;
//    } // end IF
//  } // end IF

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::close"));

  int result = -1;

  // [*NOTE*: hereby override the default behavior of a ACE_Svc_Handler,
  // which would call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default
  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector when connect() fails (e.g. connection refused)
  // - by the connector/acceptor when open() fails (e.g. too many connections !)

  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - connector when connect() fails (e.g. connection refused)
    // - acceptor/connector when initialization fails (i.e. open() returned -1
    //   due to e.g. too many connections)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (result)
      {
        // --> worker thread --> NOP
//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     ACE_TEXT (inherited::name ())));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));
        result = 0;

        break;
      } // end IF

      // *WARNING*: falls through !
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. cannot connect, too many connections, ...)
    // *NOTE*: this (eventually) calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      result = inherited::close (arg_in);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to HandlerType::close(%u): \"%m\", continuing\n"),
                    arg_in));
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_SocketConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           UserDataType>::open (ACE_HANDLE handle_in,
                                                ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::open"));

  ACE_UNUSED_ARG (handle_in);
  ACE_UNUSED_ARG (messageBlock_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return);
}

/////////////////////////////////////////

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchSocketConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 UserDataType>::Net_AsynchSocketConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , configuration_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::Net_AsynchSocketConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_AsynchSocketConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 UserDataType>::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::act"));

  // sanity check(s)
  ACE_ASSERT (!configuration_);

  // initialize this connection
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  switch (this->role ())
  {
    case NET_ROLE_CLIENT:
    {
      const ICONNECTOR_T* iconnector_p =
        static_cast<const ICONNECTOR_T*> (act_in);
      ACE_ASSERT (iconnector_p);
      configuration_ = &iconnector_p->get ();
      break;
    }
    case NET_ROLE_SERVER:
    {
      const ILISTENER_T* ilistener_p = static_cast<const ILISTENER_T*> (act_in);
      ACE_ASSERT (ilistener_p);
      configuration_ = &ilistener_p->get ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown role (was: %d), returning\n"),
                  this->role ()));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->userData);
  ConfigurationType* configuration_p =
    configuration_->userData->configuration;
  ACE_ASSERT (configuration_p);
  if (!inherited::CONNECTION_BASE_T::initialize (*configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(): \"%m\", returning\n")));
    return;
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_AsynchSocketConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::send"));

  ACE_ASSERT (false);
  ACE_NOTSUP

  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_AsynchSocketConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 UserDataType>::set (Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::set"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  ITRANSPORTLAYER_T* itransportlayer_p = this;
  ACE_ASSERT (itransportlayer_p);
  if (!itransportlayer_p->initialize (this->dispatch (),
                                      role_in,
                                      *configuration_.socketConfiguration))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ITransportLayer_T::initialize(), continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_AsynchSocketConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::open"));

  ACE_UNUSED_ARG (arg_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1)

#if defined (_MSC_VER)
  ACE_NOTREACHED (return;)
#endif // _MSC_VER
}
