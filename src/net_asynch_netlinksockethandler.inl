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
#include "ace/OS_Memory.h"
#include "ace/Proactor.h"

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType>
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::Net_AsynchNetlinkSocketHandler_T ()
 : inherited ()
 , inherited2 ()
 , inherited3 (NULL,                          // event handler handle
               ACE_Event_Handler::WRITE_MASK) // mask
 , counter_ (0) // initial count
 , inputStream_ ()
 , outputStream_ ()
// , localSAP_ ()
// , remoteSAP_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::Net_AsynchNetlinkSocketHandler_T"));

}

template <typename ConfigurationType>
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::~Net_AsynchNetlinkSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::~Net_AsynchNetlinkSocketHandler_T"));

}

template <typename ConfigurationType>
void
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::open (ACE_HANDLE handle_in,
                                                           ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::open"));

//  int result = -1;

  // step0: initialize baseclass
  inherited2::handle (handle_in);

//  // step1: open netlink socket
//  ACE_Netlink_Addr local_SAP;
//  local_SAP.set (ACE_OS::getpid (), 0);
//  result = socket_.open (local_SAP,                  // local SAP
//                         ACE_PROTOCOL_FAMILY_NETLINK, // protocol family
//                         NETLINK_GENERIC);            // protocol
//  if (result == -1)
//  {
//    ACE_TCHAR buffer[BUFSIZ];
//    ACE_OS::memset (buffer, 0, sizeof (buffer));
//    std::string local_address;
//    // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
////    if (local_SAP.addr_to_string (buffer,
////                                  sizeof (buffer)) == -1)
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
//    local_address = buffer;
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_SOCK_Netlink::open(\"%s\"): \"%m\", returning\n"),
//                ACE_TEXT (local_address.c_str ())));
//    return;
//  } // end IF

  // step1: tweak socket
  if (inherited::configuration_.socketConfiguration->bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (handle_in,
                                            SO_RCVBUF,
                                            inherited::configuration_.socketConfiguration->bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%u) (handle was: %d), aborting\n"),
                  inherited::configuration_.socketConfiguration->bufferSize,
                  handle_in));

      // clean up
      handle_close (handle_in,
                    ACE_Event_Handler::ALL_EVENTS_MASK);

      return;
    } // end IF
  if (!Net_Common_Tools::setKeepAlive (handle_in,
                                       NET_SOCKET_DEFAULT_TCP_KEEPALIVE))
  {
    int error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                  (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false")),
                  handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
  if (!Net_Common_Tools::setLinger (handle_in,
                                    inherited::configuration_.socketConfiguration->linger,
                                    -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                  (inherited::configuration_.socketConfiguration->linger ? ACE_TEXT ("true")
                                                                         : ACE_TEXT ("false")),
                  handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF

  // step2: init i/o streams
  inherited2::proactor (ACE_Proactor::instance ());
  if (inputStream_.open (*this,
                         handle_in,
                         NULL,
                         inherited2::proactor ()) == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to init input stream (handle was: %d), aborting\n"),
                handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
  if (outputStream_.open (*this,
                          handle_in,
                          NULL,
                          inherited2::proactor ()) == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to init output stream (handle was: %d), aborting\n"),
                handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
}

//void
//Net_AsynchNetlinkSocketHandler_T::addresses(const ACE_Netlink_Addr& remoteAddress_in,
//                                          const ACE_Netlink_Addr& localAddress_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::addresses"));

//  localSAP_ = localAddress_in;
//  remoteSAP_ = remoteAddress_in;
//}

template <typename ConfigurationType>
int
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                   ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);
  ACE_UNUSED_ARG (mask_in);

  int result = inputStream_.cancel ();
//  if ((result != 0) && (result != 1))
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));
  int result_2 = outputStream_.cancel ();
//  if ((result_2 != 0) && (result_2 != 1))
  if (result_2 == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));

//  return ((((result != 0) && (result != 1)) ||
//           ((result_2 != 0) && (result_2 != 1))) ? -1
//                                                 : 0);
  return ((result == -1) || (result_2 == -1) ? -1
                                             : 0);
}

template <typename ConfigurationType>
int
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::notify"));

  int result = -1;
  try {
    result = handle_output (inherited2::handle ());
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchNetlinkSocketHandler_T::handle_output(): \"%m\", continuing\n")));
    result = -1;
  }
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler_T::handle_output(): \"%m\", aborting\n")));

    int result_2 = handle_close (inherited2::handle (),
                                 ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler_T::handle_close(): \"%m\", continuing\n")));
  } // end IF

  return result;
}
template <typename ConfigurationType>
int
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::notify (ACE_Event_Handler* handler_in,
                                                             ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::notify"));

  ACE_UNUSED_ARG (handler_in);
  ACE_UNUSED_ARG (mask_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
ACE_Message_Block*
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::allocateMessage"));

  // init return value(s)
  ACE_Message_Block* message_out = NULL;

  if (inherited::configuration_.messageAllocator)
    message_out =
        static_cast<ACE_Message_Block*> (inherited::configuration_.messageAllocator->malloc (requestedSize_in));
  else
    ACE_NEW_NORETURN (message_out,
                      ACE_Message_Block (requestedSize_in,
                                         ACE_Message_Block::MB_DATA,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                         ACE_Time_Value::zero,
                                         ACE_Time_Value::max_time,
                                         NULL,
                                         NULL));
  if (!message_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return message_out;
}

template <typename ConfigurationType>
void
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::handle_write_dgram"));

  int result = -1;
  bool close = false;
  size_t bytes_transferred = result_in.bytes_transferred ();
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset (by peer) ? --> not an error
    error = result_in.error ();
    if ((error != ECONNRESET) &&
        (error != EPIPE)      &&
        (error != EBADF)) // 9 happens on Linux (local close())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));

    close = true;
  } // end IF

  switch (bytes_transferred)
  {
    case UINT32_MAX: // -1
    case 0:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&
          (error != EBADF)) // 9 happens on Linux (local close())
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE_TEXT (ACE_OS::strerror (result_in.error ()))));

      close = true;

      break;
    }
    // *** GOOD CASES ***
    default:
    {
      // short write ?
      if (result_in.bytes_to_write () != bytes_transferred)
      {
        // *TODO*: handle short writes (more) gracefully
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, aborting\n"),
                    result_in.handle (),
                    bytes_transferred,
                    result_in.bytes_to_write ()));

        close = true;
      } // end IF

      break;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ()->release ();

  if (close)
  {
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler_T::handle_close(): \"%m\", continuing\n")));
  } // end IF

  counter_.decrease ();
}

template <typename ConfigurationType>
void
Net_AsynchNetlinkSocketHandler_T<ConfigurationType>::initiate_read_dgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler_T::initiate_read_dgram"));

  int result = -1;
  size_t bytes_to_read = 0;

  // step1: allocate a data buffer
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_.PDUSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler_T::allocateMessage(%u), aborting\n"),
                inherited::configuration_.PDUSize));
    goto close;
  } // end IF

  // start (asynchronous) read...
  bytes_to_read = message_block_p->size ();
  result = inputStream_.recv (message_block_p,                      // buffer
                              bytes_to_read,                        // buffer size
                              0,                                    // flags
                              ACE_PROTOCOL_FAMILY_NETLINK,          // protocol family
                              NULL,                                 // ACT
                              0,                                    // priority
                              COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::recv(%u): \"%m\", aborting\n"),
                message_block_p->size ()));

    // clean up
    message_block_p->release ();

    goto close;
  } // end IF

  return;

close:
  result = handle_close (inherited2::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler_T::handle_close(), continuing\n")));
}
