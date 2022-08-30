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

#include "net_sock_acceptor.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

Net_SOCK_Acceptor::Net_SOCK_Acceptor ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Acceptor::Net_SOCK_Acceptor"));

}

int
Net_SOCK_Acceptor::open (const ACE_Addr &localAddress_in,
                         int reuseAddr_in,
                         int protocolFamily_in,
                         int backLog_in,
                         int protocol_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Acceptor::open"));

  // *NOTE*: essentially follows functionality of ACE_SOCK_Acceptor
  int result = -1;

  if (localAddress_in != ACE_Addr::sap_any)
    protocolFamily_in = localAddress_in.get_type ();
  else if (protocolFamily_in == PF_UNSPEC)
  {
#if defined (ACE_HAS_IPV6)
    protocolFamily_in = ACE::ipv6_enabled () ? PF_INET6 : PF_INET;
#else
    protocolFamily_in = PF_INET;
#endif /* ACE_HAS_IPV6 */
  } // end ELSE

  result = ACE_SOCK::open (SOCK_STREAM,
                           protocolFamily_in,
                           protocol_in,
                           reuseAddr_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK::open(SOCK_STREAM,%d,%d,%d): \"%m\", aborting\n"),
                protocolFamily_in, protocol_in,
                reuseAddr_in));
    return -1;
  } // end IF

  return shared_open (localAddress_in,
                      protocolFamily_in,
                      backLog_in);
}

int
Net_SOCK_Acceptor::shared_open (const ACE_Addr& localAddress_in,
                                int protocolFamily_in,
                                int backLog_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Acceptor::shared_open"));

  int result = -1;
  ACE_HANDLE handle = this->get_handle ();

#if defined (ACE_HAS_IPV6)
  if (protocolFamily_in == PF_INET6)
  {
    sockaddr_in6 local_inet6_addr;
    ACE_OS::memset (reinterpret_cast<void *> (&local_inet6_addr),
                    0,
                    sizeof (local_inet6_addr));
    if (localAddress_in == ACE_Addr::sap_any)
    {
      local_inet6_addr.sin6_family = AF_INET6;
      local_inet6_addr.sin6_port = 0;
      local_inet6_addr.sin6_addr = in6addr_any;
    }
    else
      local_inet6_addr =
        *reinterpret_cast<sockaddr_in6 *> (localAddress_in.get_addr ());

# if defined (ACE_WIN32)
    // on windows vista and later, Winsock can support dual stack sockets
    // but this must be explicitly set prior to the bind. Since this
    // behavior is the default on *nix platforms, it should be benigh to
    // just do it here. On older platforms the setsockopt will fail, but
    // that should be OK.
    int zero = 0;
    result = ACE_OS::setsockopt (handle,
                                 IPPROTO_IPV6,
                                 IPV6_V6ONLY,
                                 reinterpret_cast<const char*> (&zero),
                                 sizeof (zero));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,IPV6_V6ONLY): \"%m\", aborting\n"),
                  handle));
      goto close;
    } // end IF
# endif /* ACE_WIN32 */
    // We probably don't need a bind_port written here.
    // There are currently no supported OS's that define
    // ACE_LACKS_WILDCARD_BIND.
    result = ACE_OS::bind (handle,
                           reinterpret_cast<sockaddr*> (&local_inet6_addr),
                           sizeof (local_inet6_addr));
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
  else
#endif // ACE_HAS_IPV6
  if (protocolFamily_in == PF_INET)
  {
    sockaddr_in local_inet_addr;
    ACE_OS::memset (reinterpret_cast<void*> (&local_inet_addr),
                    0,
                    sizeof (local_inet_addr));
    if (localAddress_in == ACE_Addr::sap_any)
      local_inet_addr.sin_port = 0;
    else
      local_inet_addr =
        *reinterpret_cast<sockaddr_in*> (localAddress_in.get_addr ());
    if (local_inet_addr.sin_port == 0)
    {
      result = ACE::bind_port (handle,
                                ACE_NTOHL (ACE_UINT32 (local_inet_addr.sin_addr.s_addr)));
      if (unlikely (result == -1))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::bind_port(0x%@): \"%m\", aborting\n"),
                    handle));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::bind_port(%d): \"%m\", aborting\n"),
                    handle));
#endif // ACE_WIN32 || ACE_WIN64
        goto close;
      } // end IF
    } // end IF
    else
    {
      result = ACE_OS::bind (handle,
                             reinterpret_cast<sockaddr*> (&local_inet_addr),
                             sizeof local_inet_addr);
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
    } // end ELSE
  } // end IF
  else
  {
    result =
      ACE_OS::bind (handle,
                    reinterpret_cast<sockaddr*> (localAddress_in.get_addr ()),
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
  } // end ELSE

  ///////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  // enable SIO_LOOPBACK_FAST_PATH on Win32 ?
  if (localAddress_in.get_type () == AF_INET)
  {
    const ACE_INET_Addr* inet_addr_p = NULL;
    inet_addr_p = dynamic_cast<const ACE_INET_Addr*> (&localAddress_in);
    if (unlikely (!inet_addr_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<ACE_INET_Addr>(0x%@), aborting\n"),
                  &localAddress_in));
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

  result = ACE_OS::listen (handle,
                           backLog_in);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(0x%@): \"%m\", aborting\n"),
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(%d): \"%m\", aborting\n"),
                handle));
#endif // ACE_WIN32 || ACE_WIN64
    goto close;
  } // end IF

  return 0;

close:
  ACE_Errno_Guard errno_guard (errno);

  result = inherited::close ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Acceptor::close(): \"%m\", continuing\n")));

  return -1;
}
