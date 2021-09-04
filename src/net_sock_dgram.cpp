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

#include "net_sock_dgram.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

Net_SOCK_Dgram::Net_SOCK_Dgram ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Dgram::Net_SOCK_Dgram"));

}

Net_SOCK_Dgram::~Net_SOCK_Dgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Dgram::~Net_SOCK_Dgram"));

  // *NOTE*: the ACE_SOCK_Dgram, ACE_SOCK and ACE_IPC_SAP dtors all fail to
  //         close() the socket handle, causing handle leakage
  //         --> do it here
  // *TODO*: this ought to be notified to the ACE maintainers
  // *NOTE*: ACE_SOCK::close() returns 0 if the handle is ACE_INVALID_HANDLE
  int result = inherited::close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK::close(): \"%m\", continuing\n")));
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
Net_SOCK_Dgram::open (const ACE_Addr& localSAP_in,
                      int protocolFamily_in,
                      int protocol_in,
                      int reuseAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_Dgram::open"));

  // *IMPORTANT NOTE*: this bit has been assembled from ACE_SOCK_Dgram.cpp and
  //                   SOCK.cpp

  if (localSAP_in != ACE_Addr::sap_any)
    protocolFamily_in = localSAP_in.get_type ();
  else if (protocolFamily_in == PF_UNSPEC)
    {
#if defined (ACE_HAS_IPV6)
      protocolFamily_in = ACE::ipv6_enabled () ? PF_INET6 : PF_INET;
#else
      protocolFamily_in = PF_INET;
#endif /* ACE_HAS_IPV6 */
    }

  int one = 1;
  this->set_handle (ACE_OS::socket (protocolFamily_in,
                                    SOCK_DGRAM,
                                    protocol_in));

  if (this->get_handle () == ACE_INVALID_HANDLE)
    return -1;
  else if (protocolFamily_in != PF_UNIX
           && reuseAddress_in
           && this->set_option (SOL_SOCKET,
                                SO_REUSEADDR,
                                &one,
                                sizeof one) == -1)
    {
      this->close ();
      return -1;
    }
  else if (protocolFamily_in != PF_UNIX
           && reuseAddress_in
           && this->set_option (SOL_SOCKET,
                                SO_REUSEPORT,
                                &one,
                                sizeof one) == -1)
    {
      this->close ();
      return -1;
    }

  return this->shared_open (localSAP_in,
                            protocolFamily_in);
}
#endif

//////////////////////////////////////////

Net_SOCK_CODgram::Net_SOCK_CODgram ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_CODgram::Net_SOCK_CODgram"));

}

Net_SOCK_CODgram::~Net_SOCK_CODgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_CODgram::~Net_SOCK_CODgram"));

  // *NOTE*: the ACE_SOCK_CODgram, ACE_SOCK_IO, ACE_SOCK and ACE_IPC_SAP dtors
  //         all fail to close() the socket handle, causing handle leakage
  //         --> do it here
  // *TODO*: this ought to be notified to the ACE maintainers
  // *NOTE*: ACE_SOCK::close() returns 0 if the handle is ACE_INVALID_HANDLE
  int result = inherited::close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK::close(): \"%m\", continuing\n")));
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
Net_SOCK_CODgram::open (const ACE_Addr& remoteSAP_in,
                        const ACE_Addr& localSAP_in,
                        int protocolFamily_in,
                        int protocol_in,
                        int reuseAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SOCK_CODgram::open"));

  // *IMPORTANT NOTE*: this bit has mostly been copy/pasted from
  //                   ACE_SOCK_CODgram.cpp

  // Depending on the addresses passed as described above, figure out what
  // address family to specify for the new socket. If either address is
  // !ACE_Addr::sap_any, use that family. If they don't match, it's an error.
  if (remoteSAP_in != ACE_Addr::sap_any)
    {
      if (localSAP_in == ACE_Addr::sap_any)
        protocolFamily_in = remoteSAP_in.get_type ();
      else
        { // Both specified; family must match
          if (localSAP_in.get_type () != remoteSAP_in.get_type ())
            {
              errno = EAFNOSUPPORT;
              return -1;
            }
          protocolFamily_in = remoteSAP_in.get_type ();
        }
    }
  else
    {
      if (localSAP_in != ACE_Addr::sap_any)
        {
          protocolFamily_in = localSAP_in.get_type ();
        }
    }

  int one = 1;
  this->set_handle (ACE_OS::socket (protocolFamily_in,
                                    SOCK_DGRAM,
                                    protocol_in));

  if (this->get_handle () == ACE_INVALID_HANDLE)
    return -1;
  else if (protocolFamily_in != PF_UNIX
           && reuseAddress_in
           && this->set_option (SOL_SOCKET,
                                SO_REUSEADDR,
                                &one,
                                sizeof one) == -1)
    {
      this->close ();
      return -1;
    }
  else if (protocolFamily_in != PF_UNIX
           && reuseAddress_in
           && this->set_option (SOL_SOCKET,
                                SO_REUSEPORT,
                                &one,
                                sizeof one) == -1)
    {
      this->close ();
      return -1;
    }

  bool error = false;

  if (localSAP_in == ACE_Addr::sap_any && remoteSAP_in == ACE_Addr::sap_any)
  {
    // Assign an arbitrary port number from the transient range!!
    if ((protocolFamily_in == PF_INET
     #if defined (ACE_HAS_IPV6)
         || protocolFamily_in == PF_INET6
     #endif /* ACE_HAS_IPV6 */
         ) && ACE::bind_port (this->get_handle ()) == -1)
      error = true;
  }
  // We are binding just the local address.
  else if (localSAP_in != ACE_Addr::sap_any && remoteSAP_in == ACE_Addr::sap_any)
  {
    if (ACE_OS::bind (this->get_handle (),
                      (sockaddr *) localSAP_in.get_addr (),
                      localSAP_in.get_size ()) == -1)
      error = true;
  }
  // We are connecting to the remote address.
  else if (localSAP_in == ACE_Addr::sap_any && remoteSAP_in != ACE_Addr::sap_any)
  {
    if (ACE_OS::connect (this->get_handle (),
                         (sockaddr *) remoteSAP_in.get_addr (),
                         remoteSAP_in.get_size ()) == -1)
      error = true;
  }
  // We are binding to the local address and connecting to the
  // remote addresses.
  else
  {
    if (ACE_OS::bind (this->get_handle (),
                      (sockaddr *) localSAP_in.get_addr (),
                      localSAP_in.get_size ()) == -1
        || ACE_OS::connect (this->get_handle (),
                            (sockaddr *) remoteSAP_in.get_addr (),
                            remoteSAP_in.get_size ()) == -1)
      error = true;
  }
  if (error)
  {
    this->close ();
    this->set_handle (ACE_INVALID_HANDLE);
  }
  return error ? -1 : 0;
}
#endif // ACE_WIN32 || ACE_WIN64
