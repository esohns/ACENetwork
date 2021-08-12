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

#include "ace/Asynch_IO_Impl.h"
#include "ace/Event_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"

#include "common_tools.h"

#include "stream_common.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::Net_StreamAsynchUDPSocketBase_T ()
 : inherited ()
 , inherited2 ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::open (ACE_HANDLE handle_in,
                                                     ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::open"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
#if defined (ACE_LINUX)
  bool handle_privileges = false;
#endif // ACE_LINUX
  bool handle_socket = false;
  bool decrease_reference_count = false;
  const ConfigurationType& configuration_r = this->getR ();

  // step0: initialize base-class
  if (unlikely (!inherited::initialize (configuration_r.socketHandlerConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketHandlerBase::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // step2b: tweak socket, initialize I/O
  inherited::open (ACE_INVALID_HANDLE,
                   messageBlock_in);

  // step4: start reading ?
  if (likely (!configuration_r.writeOnly))
  {
    if (likely (messageBlock_in.length () == 0))
      inherited::initiate_read_dgram ();
    else
    {
      // forward data argument
      ACE_Message_Block* message_block_p = messageBlock_in.duplicate ();
      if (!message_block_p)
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
        goto error;
      } // end IF
      // fake a result to emulate regular behavior
      ACE_Proactor* proactor_p = inherited::proactor ();
      ACE_ASSERT (proactor_p);
      ACE_Asynch_Read_Dgram_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_dgram_result (inherited::proxy (),                  // handler proxy
                                                     inherited2::get_handle (),            // socket handle
                                                     message_block_p,                      // buffer
                                                     message_block_p->size (),             // #bytes to read
                                                     0,                                    // flags
                                                     PF_INET,                              // protocol family
                                                     NULL,                                 // asynchronous completion token
                                                     ACE_INVALID_HANDLE,                   // event
                                                     0,                                    // priority
                                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
      if (!fake_result_p)
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_dgram_result: \"%m\", aborting\n")));
        goto error;
      } // end IF
      size_t bytes_transferred = message_block_p->length ();
      // <complete> for Accept would have already moved the <wr_ptr>
      // forward; update it to the beginning position
      message_block_p->wr_ptr (message_block_p->wr_ptr () - bytes_transferred);
      // invoke 'this' (see handle_read_stream())
      fake_result_p->complete (message_block_p->length (), // bytes read
                               1,                          // success
                               NULL,                       // asynchronous completion token
                               0);                         // error

      // clean up
      delete fake_result_p;
    } // end ELSE
    // *NOTE*: registered with the proactor at this point
    //         --> data may start arriving at handle_input ()
  } // end IF

  // step5: 'float' the connection
  // *TODO*: this looks wrong...
  if (unlikely (decrease_reference_count))
  {
    this->decrease ();
    //ACE_ASSERT (this->count () == 1); // connection manager (+ stream module(s))
  } // end IF
  else
  {
    //ACE_ASSERT (this->count () == 2); // connection manager + read operation
    //                                     (+ stream module(s))
  } // end ELSE

  return;

error:
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::switchUser (static_cast<uid_t> (-1));
#endif

  if (handle_socket)
  {
    result = inherited::handle_close (inherited2::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  inherited2::get_handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  inherited2::get_handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF

  // *TODO*: this looks wrong...
  if (decrease_reference_count)
    this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ACE_Message_Block* message_block_p, *message_block_2, *message_block_3 = NULL;
  unsigned int length = 0;
  size_t bytes_sent = 0;
  ssize_t result_2 = 0;

  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  //                   been notified
  result = inherited2::getq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // fragment the data ?
  message_block_2 = message_block_p;
  do
  {
    length = message_block_2->length ();
    if (length > inherited::PDUSize_)
    {
      message_block_3 = message_block_2->duplicate ();
      if (unlikely (!message_block_3))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
        message_block_p->release (); message_block_p = NULL;
        return -1;
      } // end IF
      message_block_2->cont (message_block_3);

      message_block_2->length (inherited::PDUSize_);
      message_block_3->rd_ptr (inherited::PDUSize_);

      message_block_2 = message_block_3;
      continue;
    } // end IF

    message_block_2 = message_block_2->cont ();
  } while (message_block_2);

  // start (asynchronous) write(s)
  do
  {
    length = 0;
    message_block_2 = message_block_p;
    while (length < inherited::PDUSize_)
    {
      length += message_block_2->length ();
      message_block_2 = message_block_2->cont ();
      if (!message_block_2)
        break;
    } // end WHILE
    if (message_block_2)
    {
      message_block_3 = message_block_p;
      while (message_block_3->cont () != message_block_2)
        message_block_3 = message_block_3->cont ();
      message_block_3->cont (NULL);
    } // end IF

    this->increase ();
    inherited::counter_.increase ();
send:
    // *NOTE*: this is a fire-and-forget API for message_block_p
    result_2 =
      inherited::outputStream_.send (message_block_p,                      // data
                                     bytes_sent,                           // #bytes sent
                                     0,                                    // flags
                                     inherited::address_,                  // peer address
                                     NULL,                                 // asynchronous completion token
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
    if (unlikely (result_2 == -1))
    {
      int error = ACE_OS::last_error ();
      // *WARNING*: this could fail on multi-threaded proactors
      if (error == EAGAIN)
        goto send; // 11: happens on Linux

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                  message_block_p->total_length ()));
      message_block_p->release (); message_block_p = NULL;
      this->decrease ();
      inherited::counter_.decrease ();
      return -1;
    } // end IF
    message_block_p = message_block_2;
  } while (message_block_p);

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                             ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_close"));

  int result = -1;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data
  //                       and wait for all workers within the stream
  //                   [--> cancel any pending asynchronous operations]

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling any pending write operations

  // step2: invoke base-class maintenance
  // *NOTE*: if the connection has been close()d locally, the socket is already
  //         closed at this point
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#endif

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::info (ACE_HANDLE& handle_out,
                                                     AddressType& localSAP_out,
                                                     AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited2::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  result = inherited::get_local_addr (localSAP_out);
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: Win32: handle is not a socket (i.e. socket already closed)
#else
    if (error != EBADF)    //     9: Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  if (likely (inherited::writeOnly_))
    remoteSAP_out = inherited::address_;
  else
  {
    result = inherited::get_remote_addr (remoteSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENOTSOCK) && // 10038: Win32: socket already closed
          (error != ENOTCONN))   // 10057: Win32: not connected
#else
      if ((error != EBADF) &&  //   9: Linux: socket already closed
          (error != ENOTCONN)) // 107: Linux: not connected
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_remote_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end ELSE
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_ConnectionId_t
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ());
#else
  return static_cast<Net_ConnectionId_t> (inherited::handle ());
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  ISOCKET_CONNECTION_T* isocket_connection_p = this;
  try {
    isocket_connection_p->close ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISocketConnection_T::close(): \"%m\", aborting\n")));
    return -1;
  }

  return 0;
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited2::get_handle ();

  // *TODO*: remove type inference
  if (unlikely (inherited::writeOnly_))
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
  else
    result = inherited::handle_close (handle,
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(): \"%m\", continuing\n")));

  //  step2: release the socket handle
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    result = inherited2::close ();
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("0x%@: failed to SocketType::close(): \"%m\", continuing\n"),
                  reinterpret_cast<size_t> (handle)));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%d: failed to SocketType::close(): \"%m\", continuing\n"),
                  handle));
#endif
    //inherited2::set_handle (handle); // debugging purposes only !
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::waitForCompletion"));

  ACE_UNUSED_ARG (waitForThreads_in);
  
  int result = -1;
  ACE_Time_Value one_second (1, 0);

  // sanity check(s)
  ACE_ASSERT (inherited2::msg_queue_);

  // step1: wait for the queue to flush
  while (!inherited2::msg_queue_->is_empty ())
  {
    result = ACE_OS::sleep (one_second);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
  } // end WHILE

  // step2: wait for any scheduled asynchronous operations to complete
  inherited::counter_.wait (0);
  // --> all data has been dispatched to the kernel (socket)

  // *TODO*: different platforms may implement methods by which successful
  //         placing of the data onto the wire can be established
  //         (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  ACE_HANDLE handle = inherited::handle ();
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [0x%@]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_read_dgram"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // sanity check
  result = result_in.success ();
  if (unlikely (result == 0))
  {
    // connection closed/reset ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
        (error != ECONNRESET))                // 10054: happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (0x%@): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif
  } // end IF

  switch (static_cast<int> (result_in.bytes_transferred ()))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
      if ((error != EBADF)                   && // 9    : local close (), happens on Linux
          (error != EPIPE)                   && // 32
          (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
          (error != ECONNRESET))                // 10054: happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to read from input stream (0x%@): \"%s\", aborting\n"),
                      result_in.handle (),
                      ACE::sock_error (static_cast<int> (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes\n"),
//                   result_in.handle (),
//                   result_in.bytes_transferred ()));

      // enqueue the buffer for processing
      result = inherited2::putq (result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));
        break;
      } // end IF

      // start next read
      if (unlikely (!inherited::initiate_read_dgram ()))
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
                      ACE_TEXT ("failed to HandlerType::initiate_read_dgram(): \"%m\", aborting\n")));
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  message_block_p = result_in.message_block ();
  if (message_block_p)
    message_block_p->release ();

  result = handle_close (result_in.handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                result_in.handle (),
                ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                result_in.handle (),
                ACE_Event_Handler::ALL_EVENTS_MASK));
#endif

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_write_dgram"));

  inherited::handle_write_dgram (result_in);
  this->decrease ();
}

//////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::Net_StreamAsynchUDPSocketBase_T ()
 : inherited ()
 , inherited2 ()
 , inherited3 ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::open (ACE_HANDLE handle_in,
                                                     ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::open"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  bool handle_socket = false;
  const ConfigurationType& configuration_r = this->getR ();
  struct Net_NetlinkSocketConfiguration* socket_configuration_p = NULL;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_r.socketHandlerConfiguration.socketConfiguration);

  socket_configuration_p =
    dynamic_cast<struct Net_NetlinkSocketConfiguration*> (configuration_r.socketHandlerConfiguration.socketConfiguration);

  // sanity check(s)
  ACE_ASSERT (socket_configuration_p);

  // step1: open socket
  result =
      inherited::open (socket_configuration_p->address,   // (local) SAP
                       ACE_PROTOCOL_FAMILY_NETLINK,       // protocol family
                       socket_configuration_p->protocol); // protocol
                       // NETLINK_USERSOCK);                                             // protocol
                       //NETLINK_GENERIC);                                              // protocol
  if (unlikely (result == -1))
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    // *TODO*: find a replacement for Net_Netlink_Addr::addr_to_string
    if (socket_configuration_p->address.addr_to_string (buffer,
                                                        sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketType::open(%s): \"%m\", aborting\n"),
                buffer));
    goto error;
  } // end IF

  // step2a: initialize base-class
  if (unlikely (!inherited::initialize (configuration_r.socketHandlerConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketHandlerBase::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  // step2b: tweak socket, initialize I/O
  inherited::open (inherited2::get_handle (),
                   messageBlock_in);

//  // step5: register with the connection manager (if any) ?
//  if (!inherited4::isRegistered_)
//  {
//#if defined (__GNUG__)
//    if (!inherited4::registerc (this))
//#else
//    if (!inherited4::registerc ())
//#endif
//    {
//      // *NOTE*: perhaps max# connections has been reached
//      //ACE_DEBUG ((LM_ERROR,
//      //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
//      goto error;
//    } // end IF
//  } // end IF
//  handle_manager = true;

  // step4: start reading (need to pass any data ?)
  if (likely (messageBlock_in.length () == 0))
    inherited::initiate_read_dgram ();
  else
  {
    // forward data argument
    ACE_Message_Block* message_block_p = messageBlock_in.duplicate ();
    if (!message_block_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    // fake a result to emulate regular behavior
    ACE_Proactor* proactor_p = inherited::proactor ();
    ACE_ASSERT (proactor_p);
    ACE_Asynch_Read_Dgram_Result_Impl* fake_result_p =
      proactor_p->create_asynch_read_dgram_result (inherited::proxy (),                  // handler proxy
                                                   inherited2::get_handle (),            // socket handle
                                                   message_block_p,                      // buffer
                                                   message_block_p->size (),             // #bytes to read
                                                   0,                                    // flags
                                                   PF_NETLINK,                           // protocol family
                                                   NULL,                                 // asynchronous completion token
                                                   ACE_INVALID_HANDLE,                   // event
                                                   0,                                    // priority
                                                   COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
    if (!fake_result_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_dgram_result: \"%m\", aborting\n")));
      goto error;
    } // end IF
    size_t bytes_transferred = message_block_p->length ();
    // <complete> for Accept would have already moved the <wr_ptr>
    // forward; update it to the beginning position
    message_block_p->wr_ptr (message_block_p->wr_ptr () - bytes_transferred);
    // invoke ourselves (see handle_read_stream)
    fake_result_p->complete (message_block_p->length (), // bytes read
                             1,                          // success
                             NULL,                       // asynchronous completion token
                             0);                         // error

    // clean up
    delete fake_result_p;
  } // end ELSE
  // *NOTE*: registered with the proactor at this point
  //         --> data may start arriving at handle_input ()

  return;

error:
  if (handle_socket)
  {
    result = inherited::handle_close (inherited2::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  inherited2::get_handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  inherited2::get_handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  Net_Netlink_Addr peer_address;
  size_t bytes_sent = 0;
  ssize_t result_2 = -1;

  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  //                   been notified
  result = inherited3::getq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // start (asynchronous) write
  // *NOTE*: this is a fire-and-forget API for message_block_p
  peer_address.set (0, 0); // send to kernel
  result_2 =
      inherited::outputStream_.send (message_block_p,                      // data
                                     bytes_sent,                           // #bytes sent
                                     0,                                    // flags
                                     peer_address,                         // peer address
                                     NULL,                                 // asynchronous completion token
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                message_block_p->total_length ()));

    // clean up
    message_block_p->release ();
    this->decrease ();
    inherited::counter_.decrease ();

    return -1;
  } // end IF

  return 0;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                             ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_close"));

  int result = -1;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data
  //                       and wait for all workers within the stream
  //                   [--> cancel any pending asynchronous operations]

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling any pending write operations

  // step3: invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  this->decrease ();

  return result;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::info (ACE_HANDLE& handle_out,
                                                     Net_Netlink_Addr& localSAP_out,
                                                     Net_Netlink_Addr& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited2::get_handle ();
//  result = localSAP_out.set (static_cast<u_short> (0),
//                             static_cast<ACE_UINT32> (INADDR_NONE));
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set(0, %d): \"%m\", continuing\n"),
//                INADDR_NONE));
  localSAP_out = ACE_Addr::sap_any;
  if (likely (!inherited::writeOnly_))
  {
    result = inherited2::get_local_addr (localSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (error != EBADF) // 9: Linux: socket already closed
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_SOCK_Netlink::get_local_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end IF
  remoteSAP_out = inherited::address_;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  ISOCKET_CONNECTION_T* isocket_connection_p = this;
  try {
    isocket_connection_p->close ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISocketConnection_T::close(): \"%m\", aborting\n")));
    return -1;
  }

  return 0;
}
template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited::handle ();
  // *NOTE*: may 'delete this'
  result = handle_close (handle,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(): \"%m\", continuing\n")));

  //  step2: release the socket handle
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    int result_2 = ACE_OS::closesocket (handle);
    if (result_2 == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                  handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle));
#endif
  } // end IF
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::waitForCompletion"));

  ACE_UNUSED_ARG (waitForThreads_in);
  
  int result = -1;
  ACE_Time_Value one_second (1, 0);

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  // step1: wait for the queue to flush
  while (!inherited::msg_queue_->is_empty ())
  {
    result = ACE_OS::sleep (one_second);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
  } // end WHILE

  // step2: wait for the asynchronous operations to complete
  inherited::counter_.wait (0);
  // --> all data has been dispatched to the kernel (socket)

  // *TODO*: different platforms may implement methods by which successful
  //         placing of the data onto the wire can be established
  //         (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  ACE_HANDLE handle = inherited::handle ();
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address_string;
  result = local_address.addr_to_string (buffer,
                                         sizeof (buffer),
                                         1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address_string = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_address.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              buffer));
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_read_dgram"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // sanity check
  result = result_in.success ();
  if (unlikely (result == 0))
  {
    // connection closed/reset ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
        (error != ECONNRESET))                // 10054: happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                  result_in.handle (),
                  error, ACE_TEXT (ACE_OS::strerror (error))));
  } // end IF

  switch (result_in.bytes_transferred ())
  {
    case UINT32_MAX:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
      if ((error != EBADF)                   && // 9    : local close (), happens on Linux
          (error != EPIPE)                   && // 32
          (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
          (error != ECONNRESET))                // 10054: happens on Win32
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                    result_in.handle (),
                    error, ACE_TEXT (ACE_OS::strerror (error))));
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes\n"),
//                   result_in.handle (),
//                   result_in.bytes_transferred ()));

      // push the buffer onto our stream for processing
      result = inherited3::putq (result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));
        break;
      } // end IF

      // start next read
      if (unlikely (!inherited::initiate_read_dgram ()))
      {
        int error = ACE_OS::last_error ();
        if (error)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HandlerType::initiate_read_dgram(): \"%m\", aborting\n")));
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  message_block_p = result_in.message_block ();
  if (message_block_p)
    message_block_p->release ();

  result = handle_close (result_in.handle (),
    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                result_in.handle (),
                ACE_Event_Handler::ALL_EVENTS_MASK));

  this->decrease ();
}
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
