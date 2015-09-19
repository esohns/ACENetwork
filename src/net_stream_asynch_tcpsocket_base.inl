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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/Stream.h"

#include "stream_common.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchTCPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  unsigned int statisticCollectionInterval_in)
 : inherited ()
 , inherited2 ()
 , inherited3 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::Net_StreamAsynchTCPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::~Net_StreamAsynchTCPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::~Net_StreamAsynchTCPSocketBase_T"));

  int result = -1;

  // step1: remove enqueued module (if any)
  // *TODO*: remove type inferences
  if (inherited3::configuration_.streamConfiguration.module)
  {
    Stream_Module_t* module_p =
      stream_.find (inherited3::configuration_.streamConfiguration.module->name ());
    if (module_p)
    {
      result =
        stream_.remove (inherited3::configuration_.streamConfiguration.module->name (),
                        ACE_Module_Base::M_DELETE_NONE);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    inherited3::configuration_.streamConfiguration.module->name ()));
    } // end IF
    if (inherited3::configuration_.streamConfiguration.deleteModule)
      delete inherited3::configuration_.streamConfiguration.module;
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::open (ACE_HANDLE handle_in,
                                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::open"));

  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step1: tweak socket, initialize I/O, ...
  if (!inherited::initialize (inherited3::configuration_.socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::initialize(), aborting\n")));
    goto close;
  } // end IF
  inherited::open (handle_in, messageBlock_in);

  // step2: initialize/start stream
  // step2a: connect the stream head message queue with this handler ?
  if (!inherited3::configuration_.streamConfiguration.useThreadPerConnection)
    inherited3::configuration_.streamConfiguration.notificationStrategy = this;

  if (inherited3::configuration_.streamConfiguration.module)
  {
    // step2b: clone final module (if any) ?
    if (inherited3::configuration_.streamConfiguration.cloneModule)
    {
      IMODULE_T* imodule_p = NULL;
      // need a downcast...
      imodule_p =
        dynamic_cast<IMODULE_T*> (inherited3::configuration_.streamConfiguration.module);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                    inherited3::configuration_.streamConfiguration.module->name ()));
        goto close;
      } // end IF
      Stream_Module_t* clone_p = NULL;
      try
      {
        clone_p = imodule_p->clone ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IModule_T::clone(), aborting\n"),
                    inherited3::configuration_.streamConfiguration.module->name ()));
        goto close;
      }
      if (!clone_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IModule_T::clone(), aborting\n"),
                    inherited3::configuration_.streamConfiguration.module->name ()));
        goto close;
      }
      inherited3::configuration_.streamConfiguration.module = clone_p;
      inherited3::configuration_.streamConfiguration.deleteModule = true;
    } // end IF

    // *TODO*: step2b: initialize final module (if any)
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited3::configuration_.streamConfiguration.sessionID =
    reinterpret_cast<unsigned int> (handle_in); // (== socket handle)
#else
  inherited3::configuration_.streamConfiguration.sessionID =
    static_cast<unsigned int> (handle_in); // (== socket handle)
#endif
  if (!stream_.initialize (inherited3::configuration_.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto close;
  } // end IF
  session_data_p = &stream_.sessionData ();
  const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
      &const_cast<StateType&> (inherited3::state ());
  //stream_.dump_state ();
  // *NOTE*: as soon as this returns, data starts arriving at
  //         handle_output()[/msg_queue()]
  stream_.start ();
  if (!stream_.isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto close;
  } // end IF

  // step3: start reading (need to pass any data ?)
  if (messageBlock_in.length () == 0)
  {
    bool result = inherited::initiate_read_stream ();
    if (!result)
    {
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENXIO)               && // happens on Win32
          (error != EFAULT)              && // *TODO*: happens on Win32
          (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
          (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
      if (error)
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::initiate_read_stream(): \"%m\", aborting\n")));
      goto close;
    } // end IF
  } // end IF
  else
  {
    ACE_Message_Block* duplicate_p = messageBlock_in.duplicate ();
    if (!duplicate_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto close;
    } // end IF
    // fake a result to emulate regular behavior...
    ACE_Proactor* proactor_p = inherited::proactor ();
    ACE_ASSERT (proactor_p);
    ACE_Asynch_Read_Stream_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_stream_result (inherited::proxy (),                  // handler proxy
                                                      handle_in,                            // socket handle
                                                      *duplicate_p,                         // buffer
                                                      duplicate_p->size (),                 // (max) bytes to read
                                                      NULL,                                 // ACT
                                                      ACE_INVALID_HANDLE,                   // event
                                                      0,                                    // priority
                                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
    if (!fake_result_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_stream_result: \"%m\", aborting\n")));
      goto close;
    } // end IF
    size_t bytes_transferred = duplicate_p->length ();
    // <complete> for Accept would have already moved the <wr_ptr>
    // forward; update it to the beginning position
    duplicate_p->wr_ptr (duplicate_p->wr_ptr () - bytes_transferred);
    // invoke ourselves (see handle_read_stream())
    fake_result_p->complete (duplicate_p->length (), // bytes read
                             1,                      // success
                             NULL,                   // ACT
                             0);                     // error

    // clean up
    delete fake_result_p;
  } // end ELSE

  // step4: register with the connection manager (if any)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!inherited3::registerc ())
#else
  if (!inherited3::registerc (this))
#endif
  {
    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto close;
  } // end IF
  this->decrease (); // 'float' the connection

  return;

close:
  close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

  // *NOTE*: should 'delete this'
//  inherited3::decrease ();
  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
//  if (buffer_ == NULL)
//  {
  // send next data chunk from the stream...
  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  // been notified
//  result = stream_.get (buffer_, &ACE_Time_Value::zero);
  result = stream_.get (message_block_p,
                        &const_cast<ACE_Time_Value&> (ACE_Time_Value::zero));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // start (asynchronous) write
  inherited::counter_.increase ();
  // *NOTE*: this is a fire-and-forget API for message_block
  int error = 0;
send:
  //  if (inherited::outputStream_.write (*buffer_,               // data
  result =
      inherited::outputStream_.write (*message_block_p,                     // data
                                      message_block_p->length (),           // bytes to write
                                      NULL,                                 // ACT
                                      0,                                    // priority
                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
  if (result == -1)
  {
    error = ACE_OS::last_error ();
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) goto send; // 11: happens on Linux
    if ((error != ENOTSOCK)   && // 10038: happens on Win32
        (error != ECONNRESET) && // 10054: happens on Win32
        (error != ENOTCONN))     // 10057: happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Stream::write(%u): \"%m\", aborting\n"),
//               buffer_->size ()));
                  message_block_p->size ()));

    // clean up
    message_block_p->release ();
    inherited::counter_.decrease ();

    return -1;
  } // end IF

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;

  // step1: wait for all workers within the stream (if any)
  stream_.stop (true); // <-- wait for completion

  // step2: purge any pending notifications ?
  // *WARNING: do this here, while still holding on to the current write buffer
  if (!inherited3::configuration_.streamConfiguration.useThreadPerConnection)
  {
    Stream_Iterator_t iterator (stream_);
    const Stream_Module_t* module_p = NULL;
    result = iterator.next (module_p);
    if (result == 1)
    {
      ACE_ASSERT (module_p);
      Stream_Task_t* task_p =
          const_cast<Stream_Module_t*> (module_p)->reader ();
      ACE_ASSERT (task_p);
      result = task_p->flush (ACE_Task_Flags::ACE_FLUSHALL);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task_T::flush(): \"%m\", continuing\n")));
      if (result > 0)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("flushed %d messages...\n")));
    } // end IF
  } // end IF

  // step3: invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (result == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));

  // step4: deregister with the connection manager (if any)
  if (inherited3::isRegistered_)
    inherited3::deregister ();

  return result;
}

//template <typename ConfigurationType,
//          typename UserDataType,
//          typename StateType,
//          typename ITransportLayerType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename HandlerType>
//void
//Net_StreamAsynchTCPSocketBase_T<ConfigurationType,
//                                UserDataType,
//                                StateType,
//                                ITransportLayerType,
//                                StatisticContainerType,
//                                StreamType,
//                                HandlerType>::act (const void* act_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::act"));
//
//  //inherited2::configuration_ = *reinterpret_cast<ConfigurationType*> (const_cast<void*> (act_in));
//}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                       AddressType& localSAP_out,
                                                                       AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::info"));

  handle_out = inherited::handle ();
  localSAP_out = inherited::localSAP_;
  remoteSAP_out = inherited::remoteSAP_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
unsigned int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<unsigned int> (inherited::handle ());
#else
  return static_cast<unsigned int> (inherited::handle ());
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::notification"));

  return this;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  // *TODO*: find a better way to do this
  //inherited3::close ();
  inherited3* iconnection_p = this;
  iconnection_p->close ();

  return 0;
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
//                                ModuleConfigurationType>::close (u_long arg_in)
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

//  ACE_UNUSED_ARG (arg_in);
  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited::handle ();
  // *NOTE*: may 'delete this'
  result = handle_close (handle,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", aborting\n")));
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));

  //  step2: release the socket handle
  if (handle != ACE_INVALID_HANDLE)
  {
    result = ACE_OS::closesocket (handle);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle));
//    inherited::handle (ACE_INVALID_HANDLE);
  } // end IF

//  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::waitForCompletion ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  //        --> all data has been dispatched (here: to the proactor/kernel)
  stream_.flush ();

  // step2: wait for the asynchronous operations to complete
  inherited::counter_.wait (0);

  // *TODO*: different platforms may implement methods by which successful
  //         placing of the data onto the wire can be established
  //         (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  ACE_HANDLE handle = inherited::handle ();
  bool no_delay = Net_Common_Tools::getNoDelay (handle);
  Net_Common_Tools::setNoDelay (handle, true);
  Net_Common_Tools::setNoDelay (handle, no_delay);
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::collect"));

  try
  {
    return stream_.collect (data_out);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::report"));

  try
  {
    return stream_.report ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  int result = local_inet_address.addr_to_string (buffer,
                                                  sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string peer_address;
  result = peer_inet_address.addr_to_string (buffer,
                                             sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    peer_address = buffer;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (peer_address.c_str ())));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_read_stream"));

  int result = -1;
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset (by peer) ? --> not an error
    error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ERROR_NETNAME_DELETED)   &&  // happens on Win32
        (error != ERROR_OPERATION_ABORTED) &&  // local close(), happens in Win32
        (error != ERROR_CONNECTION_ABORTED))   // local close(), happens on Win32
#else
    if ((error != ECONNRESET) &&
        (error != EPIPE)      &&
        (error != EBADF)) // local close(), happens on Linux
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
  } // end IF

  switch (result_in.bytes_transferred ())
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ERROR_NETNAME_DELETED) &&  // happens on Win32
          (error != ERROR_CONNECTION_ABORTED)) // local close(), happens on Win32
#else
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&
          (error != EBADF)) // local close(), happens on Linux
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));

      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_));

      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes...\n"),
//                   result.handle (),
//                   result.bytes_transferred ()));

      // push the buffer onto the stream for processing
      result = stream_.put (&result_in.message_block (), NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));
        break;
      } // end IF

      // start next read
      bool result_2 = inherited::initiate_read_stream ();
      if (!result_2)
      {
        int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if ((error != ENXIO)               && // happens on Win32
            (error != EFAULT)              && // *TODO*: happens on Win32
            (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
            (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
        if (error)
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::initiate_read_stream(): \"%m\", aborting\n")));
        goto close;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();

close:
  result = handle_close (inherited::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));
}

//template <typename HandlerType,
//          typename AddressType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType,
//          typename ModuleConfigurationType,
//          typename ModuleHandlerConfigurationType>
//void
//Net_StreamAsynchTCPSocketBase_T<HandlerType,
//                                AddressType,
//                                ConfigurationType,
//                                StateType,
//                                StatisticContainerType,
//                                StreamType,
//                                UserDataType,
//                                ModuleConfigurationType,
//                                ModuleHandlerConfigurationType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_write_stream"));
//
//  inherited::handle_write_stream (result_in);
//  inherited::counter_.decrease ();
//}
