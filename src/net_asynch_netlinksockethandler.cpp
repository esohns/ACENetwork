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
#include "stdafx.h"

#include "net_asynch_netlinksockethandler.h"

#include "ace/OS.h"
#include "ace/OS_Memory.h"
#include "ace/Proactor.h"

#include "stream_defines.h"

#include "net_defines.h"
#include "net_common_tools.h"
#include "net_macros.h"

Net_AsynchNetlinkSocketHandler::Net_AsynchNetlinkSocketHandler ()
 //: inherited ()
 //, inherited2(0,    // initial count
 //             true) // delete on zero ?
 : /*inherited2 ()
 ,*/ inherited3 (NULL,                          // event handler handle
                 ACE_Event_Handler::WRITE_MASK) // mask
// , inputStream_ ()
// , outputStream_ ()
// , localSAP_ ()
// , remoteSAP_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::Net_AsynchNetlinkSocketHandler"));

}

Net_AsynchNetlinkSocketHandler::~Net_AsynchNetlinkSocketHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::~Net_AsynchNetlinkSocketHandler"));

}

void
Net_AsynchNetlinkSocketHandler::open (ACE_HANDLE handle_in,
                                      ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::open"));

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
  if (inherited::configuration_.socketConfiguration.bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (handle_in,
                                            SO_RCVBUF,
                                            inherited::configuration_.socketConfiguration.bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%u) (handle was: %d), aborting\n"),
                  inherited::configuration_.socketConfiguration.bufferSize,
                  handle_in));

      // clean up
      handle_close (handle_in,
                    ACE_Event_Handler::ALL_EVENTS_MASK);

      return;
    } // end IF
  if (!Net_Common_Tools::setKeepAlive (handle_in,
                                       NET_DEFAULT_SOCKET_TCP_KEEPALIVE))
  {
    int error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                  (NET_DEFAULT_SOCKET_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false")),
                  handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
  if (!Net_Common_Tools::setLinger (handle_in,
                                    NET_DEFAULT_SOCKET_LINGER))
  {
    int error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s) (handle was: %d), aborting\n"),
                  ((NET_DEFAULT_SOCKET_LINGER > 0) ? ACE_TEXT ("true")
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
//Net_AsynchNetlinkSocketHandler::addresses(const ACE_Netlink_Addr& remoteAddress_in,
//                                          const ACE_Netlink_Addr& localAddress_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::addresses"));

//  localSAP_ = localAddress_in;
//  remoteSAP_ = remoteAddress_in;
//}

int
Net_AsynchNetlinkSocketHandler::handle_close(ACE_HANDLE handle_in,
                                             ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::handle_close"));

  ACE_UNUSED_ARG (mask_in);

  int result = -1;

  // clean up
  result = inputStream_.cancel ();
//  if (result == -1)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::cancel(): \"%m\", continuing\n")));
  result = outputStream_.cancel ();
//  if (result == -1)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::cancel(): \"%m\", continuing\n")));

  if (handle_in != ACE_INVALID_HANDLE)
  {
    result = ACE_OS::closesocket (handle_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle_in));
  } // end IF

  return result;
}

int
Net_AsynchNetlinkSocketHandler::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::notify"));

  int result = -1;
  try
  {
    result = handle_output (inherited2::handle ());
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchNetlinkSocketHandler::handle_output(), aborting")));
  }
  if (result == -1)
    handle_close (inherited2::handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK);

  return result;
}
int
Net_AsynchNetlinkSocketHandler::notify (ACE_Event_Handler* handler_in,
                                        ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::notify"));

  ACE_UNUSED_ARG (handler_in);
  ACE_UNUSED_ARG (mask_in);

  // *NOTE*: should NEVER be reached !
  ACE_ASSERT (false);

#if defined (_MSC_VER)
  return -1;
#else
  ACE_NOTREACHED (return -1;)
#endif
}

ACE_Message_Block*
Net_AsynchNetlinkSocketHandler::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::allocateMessage"));

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

void
Net_AsynchNetlinkSocketHandler::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::handle_write_dgram"));

  int result = -1;
  bool close = false;
  size_t bytes_transferred = result_in.bytes_transferred ();

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection reset (by peer) ? --> not an error
    if ((result_in.error () != ECONNRESET) &&
        (result_in.error () != EPIPE))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (result_in.error ()))));

    close = true;
  } // end IF

  switch (bytes_transferred)
  {
    case -1:
    case 0:
    {
      // connection reset (by peer) ? --> not an error
      if ((result_in.error() != ECONNRESET) &&
          (result_in.error() != EPIPE))
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
        // *TODO*: handle short writes more gracefully
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, aborting"),
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
    handle_close (inherited2::handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK);
}

void
Net_AsynchNetlinkSocketHandler::initiate_read_dgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::initiate_read_dgram"));

  int result = -1;
  ACE_Message_Block* message_p = NULL;

  // allocate a data buffer
  message_p = allocateMessage (inherited::configuration_.bufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchNetlinkSocketHandler::allocateMessage(%u), aborting\n"),
                inherited::configuration_.bufferSize));

    // clean up
    handle_close (inherited2::handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK);
  } // end IF

  // start (asynch) read...
  size_t bytes_to_read = message_p->size (); // why oh why...
  result = inputStream_.recv (message_p,                   // buffer
                              bytes_to_read,               // buffer size
                              0,                           // flags
                              ACE_PROTOCOL_FAMILY_NETLINK, // protocol family
                              NULL,                        // ACT
                              0,                           // priority
                              ACE_SIGRTMIN);               // signal
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::recv(%u): \"%m\", aborting\n"),
                message_p->size ()));

    // clean up
    message_p->release ();
    handle_close (inherited2::handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK);
  } // end IF
}

//void
//Net_AsynchNetlinkSocketHandler::abort ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkSocketHandler::abort"));

//  handle_close (inherited::handle (),
//                ACE_Event_Handler::ALL_EVENTS_MASK);
//}
