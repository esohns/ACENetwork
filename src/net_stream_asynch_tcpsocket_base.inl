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

#include "stream_common.h"

#include "net_macros.h"

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::Net_StreamAsynchTCPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                     unsigned int statisticCollectionInterval_in)
 : inherited ()
 , inherited2 ()
 , inherited3 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::Net_StreamAsynchTCPSocketBase_T"));

}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::~Net_StreamAsynchTCPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::~Net_StreamAsynchTCPSocketBase_T"));

  // step1: remove enqueued module (if any)
  if (inherited3::configuration_.streamConfiguration.module)
  {
    Common_Module_t* module_p =
      stream_.find (inherited3::configuration_.streamConfiguration.module->name ());
    if (module_p)
    {
      int result =
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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::open (ACE_HANDLE handle_in,
                                                          ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::open"));

  int result = -1;
  unsigned int session_id = 0;

  // step1: tweak socket, initialize I/O, ...
  inherited::open (handle_in, messageBlock_in);

  // step2: initialize/start stream
  // step2a: connect stream head message queue with a notification pipe/queue ?
  if (!inherited3::configuration_.streamConfiguration.useThreadPerConnection)
    inherited3::configuration_.streamConfiguration.notificationStrategy = this;
  // step2b: initialize final module (if any)
  if (inherited3::configuration_.streamConfiguration.module)
  {
//    Common_Module_t* module_p = NULL;
//    module_p =
//      dynamic_cast<Common_Module_t*> (inherited3::configuration_.streamConfiguration.module);
//    if (!module_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
//                  ACE_TEXT (inherited3::configuration_.streamConfiguration.module->name ())));
//      goto close;
//    } // end IF
    Stream_IModule_t* imodule_p = NULL;
    // need a downcast...
    imodule_p =
        dynamic_cast<Stream_IModule_t*> (inherited3::configuration_.streamConfiguration.module);
    if (!imodule_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                  ACE_TEXT (inherited3::configuration_.streamConfiguration.module->name ())));
      goto close;
    } // end IF
    Common_Module_t* clone_p = NULL;
    try
    {
      clone_p = imodule_p->clone ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (inherited3::configuration_.streamConfiguration.module->name ())));
      goto close;
    }
    if (!clone_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (inherited3::configuration_.streamConfiguration.module->name ())));
      goto close;
    }
    inherited3::configuration_.streamConfiguration.module = clone_p;
    inherited3::configuration_.streamConfiguration.deleteModule = true;
  } // end IF

#if defined (_MSC_VER)
  session_id =
    reinterpret_cast<unsigned int> (handle_in); // (== socket handle)
#else
  session_id =
    static_cast<unsigned int> (handle_in); // (== socket handle)
#endif
  // *TODO*: this clearly is a design glitch
  if (!stream_.initialize (session_id,
                           inherited3::configuration_.streamConfiguration,
                           inherited3::configuration_.protocolConfiguration,
                           inherited3::configuration_.streamSessionData))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto close;
  } // end IF
  //stream_.dump_state ();
  // *NOTE*: as soon as this returns, data starts arriving at handle_output()[/msg_queue()]
  stream_.start ();
  if (!stream_.isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto close;
  } // end IF

  // step3: start reading (need to pass any data ?)
  if (messageBlock_in.length () == 0)
   inherited::initiate_read_stream ();
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
    ACE_Asynch_Read_Stream_Result_Impl* fake_result_p =
        inherited::proactor ()->create_asynch_read_stream_result (inherited::proxy (),                  // handler proxy
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
    // invoke ourselves (see handle_read_stream)
    fake_result_p->complete (duplicate_p->length (), // bytes read
                             1,                      // success
                             NULL,                   // ACT
                             0);                     // error

    // clean up
    delete fake_result_p;
  } // end ELSE

  // step4: register with the connection manager (if any)
  if (!inherited3::registerc ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto close;
  } // end IF

  return;

close:
  result = handle_close (handle_in,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::handle_output (ACE_HANDLE handle_in)
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
  // *NOTE*: this is a fire-and-forget API for message_block...
//  if (inherited::outputStream_.write (*buffer_,               // data
  result =
    inherited::outputStream_.write (*message_block_p,                     // data
                                    message_block_p->size (),             // bytes to write
                                    NULL,                                 // ACT
                                    0,                                    // priority
                                    COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::write(%u): \"%m\", aborting\n"),
//               buffer_->size ()));
               message_block_p->size ()));

    // clean up
    message_block_p->release ();

    return -1;
  } // end IF

  return 0;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::handle_close (ACE_HANDLE handle_in,
                                                                  ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;

  // step1: wait for all workers within the stream (if any)
  if (stream_.isRunning ())
  {
    stream_.stop ();
    stream_.waitForCompletion ();
  } // end IF

  // step2: purge any pending notifications ?
  // *WARNING: do this here, while still holding on to the current write buffer
  if (!inherited3::configuration_.streamConfiguration.useThreadPerConnection)
  {
    Stream_Iterator_t iterator (stream_);
    const Common_Module_t* module_p = NULL;
    result = iterator.next (module_p);
    if (result == 1)
    {
      ACE_ASSERT (module_p);
      Common_Task_t* task_p =
          const_cast<Common_Module_t*> (module_p)->writer ();
      ACE_ASSERT (task_p);
      result = task_p->msg_queue ()->flush ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_MessageQueue::flush(): \"%m\", continuing\n")));
    } // end IF
  } // end IF

  // step3: invoke base-class maintenance
  result = inherited::handle_close (inherited::handle (),
                                    mask_in);
  if (result == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::handle_close(): \"%m\", continuing\n")));

  // step4: deregister with the connection manager (if any)
  inherited3::deregister ();

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  inherited3::decrease ();

  return result;
}

//template <typename ConfigurationType,
//          typename UserDataType,
//          typename SessionDataType,
//          typename ITransportLayerType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename SocketHandlerType>
//void
//Net_StreamAsynchTCPSocketBase_T<ConfigurationType,
//                                UserDataType,
//                                SessionDataType,
//                                ITransportLayerType,
//                                StatisticContainerType,
//                                StreamType,
//                                SocketHandlerType>::act (const void* act_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::act"));
//
//  //inherited2::configuration_ = *reinterpret_cast<ConfigurationType*> (const_cast<void*> (act_in));
//}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
bool
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::collect (StatisticContainerType& data_out)
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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::report () const
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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited::handle ();
  // *NOTE*: may 'delete this'
  result = handle_close (handle,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
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
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::info (ACE_HANDLE& handle_out,
                                                          AddressType& localSAP_out,
                                                          AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::info"));

  handle_out = inherited::handle ();
  localSAP_out = inherited::localSAP_;
  remoteSAP_out = inherited::remoteSAP_;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
unsigned int
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return *static_cast<unsigned int*> (inherited::handle ());
#else
  return static_cast<unsigned int> (inherited::handle ());
#endif
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::dump_state () const
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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketHandlerType>
void
Net_StreamAsynchTCPSocketBase_T<AddressType,
                                SocketConfigurationType,
                                ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                StatisticContainerType,
                                StreamType,
                                SocketHandlerType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
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
    if ((error != ECONNRESET) &&
        (error != EPIPE)      &&
        (error != EBADF)      && // local close (), happens on Linux
        (error != 64)) // *TODO*: EHOSTDOWN (- 10000), happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                  result_in.handle (),
                  error, ACE_TEXT (ACE_OS::strerror (error))));
  } // end IF

  switch (result_in.bytes_transferred ())
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&
          (error != EBADF)      && // local close (), happens on Linux
          (error != 64)) // *TODO*: EHOSTDOWN (- 10000), happens on Win32
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                    result_in.handle (),
                    error, ACE_TEXT (ACE_OS::strerror (error))));

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
      inherited::initiate_read_stream ();

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();

  result = handle_close (inherited::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));
}
