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

#include "net_sock_connector.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#if defined (SSL_SUPPORT)
#include "ace/Countdown_Time.h"
#include "ace/Time_Value.h"
#endif // SSL_SUPPORT

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

Net_SOCK_Connector::Net_SOCK_Connector ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Connector::Net_SOCK_Connector"));

}

int
Net_SOCK_Connector::connect (ACE_SOCK_Stream& stream_in,
                             const ACE_Addr& remoteAddress_in,
                             const ACE_Time_Value* timeout_in,
                             const ACE_Addr& localAddress_in,
                             int reuseAddr_in,
                             int flags_in,
                             int permissions_in,
                             int protocol_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Connector::connect"));

  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (permissions_in);

  // *NOTE*: essentially follows functionality of ACE_SOCK_Connector,
  //         overriding shared_connect_start()
  int result =
    inherited::shared_open (stream_in,
                            remoteAddress_in.get_type (),
                            protocol_in,
                            reuseAddr_in);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Connector::shared_open(0x%@): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Connector::shared_open(%d): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF

  result = shared_connect_start (stream_in,
                                 timeout_in,
                                 localAddress_in,
                                 remoteAddress_in);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SOCK_Connector::shared_connect_start(0x%@): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SOCK_Connector::shared_connect_start(%d): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF

  result =
    ACE_OS::connect (stream_in.get_handle (),
                     reinterpret_cast<sockaddr*> (remoteAddress_in.get_addr ()),
                     remoteAddress_in.get_size ());
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::connect(0x%@): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::connect(%d): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF

  result = inherited::shared_connect_finish (stream_in,
                                             timeout_in,
                                             result);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Connector::shared_connect_finish(0x%@): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Connector::shared_connect_finish(%d): \"%m\", aborting\n"),
                stream_in.get_handle ()));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF

  return result;
}

int
Net_SOCK_Connector::shared_connect_start (ACE_SOCK_Stream& stream_in,
                                          const ACE_Time_Value* timeout_in,
                                          const ACE_Addr& localAddress_in,
                                          const ACE_Addr& remoteAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Connector::shared_connect_start"));

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  const ACE_INET_Addr* inet_addr_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_HANDLE handle = stream_in.get_handle ();

  // sanity check(s)
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);

  if (localAddress_in != ACE_Addr::sap_any)
  {
    result =
      ACE_OS::bind (handle,
                    reinterpret_cast<struct sockaddr*> (localAddress_in.get_addr ()),
                    localAddress_in.get_size ());
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::bind(0x%@): \"%m\", aborting\n"),
                  handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::bind(%d): \"%m\", aborting\n"),
                  handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto close;
    } // end IF
  } // end IF

  // Enable non-blocking, if required.
  if (timeout_in)
  {
    result = stream_in.enable (ACE_NONBLOCK);
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Stream::enable(ACE_NONBLOCK) (handle was: 0x%@): \"%m\", continuing\n"),
                  handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Stream::enable(ACE_NONBLOCK) (handle was: %d): \"%m\", continuing\n"),
                  handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto close;
    } // end IF
  } // end IF

  ///////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  // enable SIO_LOOPBACK_FAST_PATH on Win32 ?
  if (likely (remoteAddress_in.get_type () == AF_INET))
  { // *TODO*: use static_cast<> and assert here
    inet_addr_p = dynamic_cast<const ACE_INET_Addr*> (&remoteAddress_in);
    if (unlikely (!inet_addr_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<ACE_INET_Addr>(0x%@), aborting\n"),
                  &remoteAddress_in));
      goto close;
    } // end IF
    if (inet_addr_p->is_loopback () &&
        NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH)
      if (unlikely (!Net_Common_Tools::setLoopBackFastPath (handle)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setLoopBackFastPath(0x%@): \"%m\", aborting\n"),
                    handle));
        goto close;
      } // end IF
  } // end IF
#endif // _WIN32_WINNT_WIN8
#endif // ACE_WIN32 || ACE_WIN64

  return 0;

close:
  // Save/restore errno.
  ACE_Errno_Guard error (errno);

  result = stream_in.close ();
  if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Stream::close(0x%@): \"%m\", continuing\n"),
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Stream::close(%d): \"%m\", continuing\n"),
                handle));
#endif // ACE_WIN32 || ACE_WIN64

  return -1;
}

//////////////////////////////////////////

#if defined (SSL_SUPPORT)
Net_SOCK_SSL_Connector::Net_SOCK_SSL_Connector ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_SSL_Connector::Net_SOCK_SSL_Connector"));

}

int
Net_SOCK_SSL_Connector::connect (ACE_SSL_SOCK_Stream& stream_in,
                                 const ACE_Addr& remoteAddress_in,
                                 const ACE_Time_Value* timeout_in,
                                 const ACE_Addr& localAddress_in,
                                 int reuseAddr_in,
                                 int flags_in,
                                 int permissions_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_SSL_Connector::connect"));

  // Take into account the time to complete the basic TCP handshake
  // and the SSL handshake.
  ACE_Time_Value time_copy;
  ACE_Countdown_Time countdown (&time_copy);
  if (timeout_in != 0)
    {
      time_copy += *timeout_in;
      countdown.start ();
    }

  int result =
    inherited::connector_.connect (stream_in.peer (),
                                   remoteAddress_in,
                                   timeout_in,
                                   localAddress_in,
                                   reuseAddr_in,
                                   flags_in,
                                   permissions_in);

  int error = 0;
  if (result == -1)
    error = errno;  // Save us some TSS accesses.

  // Obtain the handle from the underlying SOCK_Stream and set it in
  // the SSL_SOCK_Stream.  Note that the case where a connection is in
  // progress is also handled.  In that case, the handle must also be
  // set in the SSL_SOCK_Stream so that the correct handle is returned
  // when performing non-blocking connect()s.
  if (stream_in.get_handle () == ACE_INVALID_HANDLE
      && (result == 0
          || (result == -1 && (error == EWOULDBLOCK
                               || error == EINPROGRESS))))
    stream_in.set_handle (stream_in.peer ().get_handle ());

  if (result == -1)
    return result;

  // If using a timeout, update the countdown timer to reflect the time
  // spent on the connect itself, then pass the remaining time to
  // ssl_connect to bound the time on the handshake.
  if (timeout_in != 0)
    {
      countdown.update ();
      timeout_in = &time_copy;
    }

  result = this->ssl_connect (stream_in, timeout_in);

  if (result == -1)
    stream_in.close ();

  return result;
}
#endif // SSL_SUPPORT
