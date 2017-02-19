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

#include <ace/Log_Msg.h>
#include <ace/SOCK_Stream.h>
#include <ace/Svc_Handler.h>

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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::Net_StreamConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , configuration_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::Net_StreamConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::~Net_StreamConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::~Net_StreamConnectionBase_T"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->socketHandlerConfiguration);

  // wait for any worker(s)
  // *TODO*: remove type inference
  if (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketHandlerConfiguration->useThreadPerConnection))
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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::set (Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::set"));

  // sanity check(s)
  SocketConfigurationType socket_configuration;
  //// *TODO*: remove type inference
  //if (configuration_.socketConfiguration)
  //  socket_configuration = *configuration_.socketConfiguration;

  ITRANSPORTLAYER_T* itransportlayer_p = this;
  ACE_ASSERT (itransportlayer_p);

  if (!itransportlayer_p->initialize (this->dispatch (),
                                      role_in,
                                      socket_configuration))
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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::send"));

  int result = -1;

  Stream_Module_t* module_p, *module_2 = NULL;
  Stream_Task_t* task_p = NULL;
  // *NOTE*: feed the data into the stream at the 'top' of the outbound stream
  //         (which is the module just above the stream tail)
  result = inherited::stream_.top (module_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", returning\n")));
    goto clean_up;
  } // end IF
  ACE_ASSERT (module_p);
  module_2 = inherited::stream_.tail ();
  ACE_ASSERT (module_2);
  while (module_p->next () != module_2)
    module_p = module_p->next ();
  ACE_ASSERT (module_p);
  //Stream_Task_t* task_p = module_p->writer ();
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  //result = task_p->reply (message_inout, NULL);
  result = task_p->put_next (message_inout, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", returning\n")));
    goto clean_up;
  } // end IF

  return;

clean_up:
  message_inout->release ();
  message_inout = NULL;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           //UserDataType>::get () const
                           UserDataType>::wait (StreamStatusType state_in,
                                                const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = inherited::stream_.top (top_module_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (!istatemachine_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::waitForIdleState () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::waitForIdleState"));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::stream_.find (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                ACE_TEXT (STREAM_MODULE_HEAD_NAME)));
    return;
  } // end IF
  Stream_Task_t* task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  Stream_IMessageQueue* imessage_queue_p =
    dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue ());
  if (!imessage_queue_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Stream_IMessageQueue*>(0x%@), returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                module_p->name (),
                task_p->msg_queue ()));
    return;
  } // end IF
  try {
    imessage_queue_p->waitForIdleState ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IMessageQueue::waitForIdleState(), returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                module_p->name ()));
    return;
  }
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::open"));

  // step0: initialize this connection
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  const HandlerConfigurationType* handler_configuration_p = NULL;
  switch (this->role ())
  {
    case NET_ROLE_CLIENT:
    {
      ICONNECTOR_T* iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      handler_configuration_p = &(iconnector_p->get ());
      break;
    }
    case NET_ROLE_SERVER:
    {
      ILISTENER_T* ilistener_p = static_cast<ILISTENER_T*> (arg_in);
      ACE_ASSERT (ilistener_p);
      handler_configuration_p = &(ilistener_p->get ());
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
  ACE_ASSERT (handler_configuration_p);
  // *TODO*: remove type inference
  ACE_ASSERT (handler_configuration_p->userData);
  ConfigurationType* configuration_p =
    handler_configuration_p->userData->connectionConfiguration;
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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::close"));

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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::open (ACE_HANDLE handle_in,
                                                ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::open"));

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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::Net_AsynchStreamConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , configuration_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::Net_AsynchStreamConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::~Net_AsynchStreamConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::~Net_AsynchStreamConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::act"));

  // initialize this connection

  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  const HandlerConfigurationType* handler_configuration_p = NULL;
  switch (this->role ())
  {
    case NET_ROLE_CLIENT:
    {
      const ICONNECTOR_T* iconnector_p =
        static_cast<const ICONNECTOR_T*> (act_in);
      ACE_ASSERT (iconnector_p);
      handler_configuration_p = &iconnector_p->get ();
      break;
    }
    case NET_ROLE_SERVER:
    {
      const ILISTENER_T* ilistener_p = static_cast<const ILISTENER_T*> (act_in);
      ACE_ASSERT (ilistener_p);
      handler_configuration_p = &ilistener_p->get ();
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
  ACE_ASSERT (handler_configuration_p);
  // *TODO*: remove type inference
  ACE_ASSERT (handler_configuration_p->connectionConfiguration);
  ConfigurationType* configuration_p =
    handler_configuration_p->connectionConfiguration;
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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::send"));

  int result = -1;

  Stream_Module_t* module_p = NULL;
  Stream_Task_t* task_p = NULL;
  // *NOTE*: feed the data into the stream at the 'top' of the outbound stream
  //         (which is the module just above the stream tail)
  module_p = inherited::stream_.tail ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::tail(): \"%m\", returning\n")));
    goto clean_up;
  } // end IF
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  task_p = task_p->next ();
  ACE_ASSERT (task_p);
  //result = task_p->reply (message_inout, NULL);
  result = task_p->put (message_inout, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", returning\n")));
    goto clean_up;
  } // end IF

  return;

clean_up:
  message_inout->release ();
  message_inout = NULL;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 //UserDataType>::get () const
                                 UserDataType>::wait (StreamStatusType state_in,
                                                      const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = inherited::stream_.top (top_module_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (!istatemachine_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::set (Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::set"));

  ITRANSPORTLAYER_T* itransportlayer_p = this;
  ACE_ASSERT (itransportlayer_p);

  if (!itransportlayer_p->initialize (this->dispatch (),
                                      role_in,
                                      configuration_.socketConfiguration))
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
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
unsigned int
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::flush (bool flushSessionMessages_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::flush"));

  // step1: flush all outbound data
  inherited::stream_.flush (false,                   // flush inbound data ?
                            flushSessionMessages_in, // flush session messages ?
                            false);                  // flush upstream (if any) ?

  // step2: cancel all outstanding asynchronous operations
  int result_2 = inherited::handle_close (ACE_INVALID_HANDLE,
                                          ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(): \"%m\", continuing\n")));

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::waitForIdleState () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForIdleState"));

  // step1: wait for the stream outbound queue to become idle
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::stream_.find (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                ACE_TEXT (STREAM_MODULE_HEAD_NAME)));
    return;
  } // end IF
  Stream_Task_t* task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  Stream_IMessageQueue* imessage_queue_p =
    dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue ());
  if (!imessage_queue_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Stream_IMessageQueue*>(0x%@), returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                module_p->name (),
                task_p->msg_queue ()));
    return;
  } // end IF
  try {
    imessage_queue_p->waitForIdleState ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IMessageQueue::waitForIdleState(), returning\n"),
                ACE_TEXT (inherited::stream_.name ().c_str ()),
                module_p->name ()));
    return;
  }
  // --> stream data has been processed

  // step2: wait for any asynchronous operations to complete
  inherited::counter_.wait (0);
  // --> all data has been dispatched to the kernel (socket)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::open"));

  ACE_UNUSED_ARG (arg_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}
