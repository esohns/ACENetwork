/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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

#ifndef DHCP_TOOLS_H
#define DHCP_TOOLS_H

#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_SUPPORT)
#define __GNUC_ATOMICS 1
#define HAVE_STDATOMIC_H 1
using namespace std;
extern "C"
{
#include "dhcpctl/dhcpctl.h"
}
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#include "dhcp_codes.h"
#include "dhcp_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_USE)
typedef void (*Net_DHClientCallback_t) (dhcpctl_handle, dhcpctl_status, void*);
#endif // DHCLIENT_USE
#endif // ACE_WIN32 || ACE_WIN64

class DHCP_Tools
{
 public:
  inline virtual ~DHCP_Tools () {}

  // debug info
  static std::string dump (const DHCP_Record&);

  static std::string OpToString (DHCP_Op_t);
  static std::string OptionToString (DHCP_Option_t);
  static std::string MessageTypeToString (DHCP_MessageType_t);

  static DHCP_MessageType_t MessageTypeToType (const std::string&);
  static DHCP_OptionFieldType_t OptionToFieldType (DHCP_Option_t);

  static bool isRequest (const DHCP_Record&);
  static DHCP_MessageType_t type (const DHCP_Record&);

  static unsigned int generateXID ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_USE)
  static bool DHClientOmapiSupport (bool); // toggle

  // *IMPORTANT NOTE*: the following preconditions must be met:
  //                   - a local ISC 'dhclient' process must be running
  //                   - it must have been configured to listen for OMAPI
  //                     (see also: man omapi(3), dhclient.conf(5)) traffic
  // *NOTE*: the final precondition implies an entry 'omapi port [port]' in
  //         /etc/dhclient.conf; this feature is not documented in recent
  //         versions, however
  static bool connectDHClient (const ACE_INET_Addr&,   // address
                               dhcpctl_handle,         // authenticator
                               Net_DHClientCallback_t, // event callback
                               void*,                  // callback user data
                               dhcpctl_handle&);       // return value: connection handle
  static void disconnectDHClient (dhcpctl_handle&); // connection handle

  static bool getInterfaceState (dhcpctl_handle,         // connection handle
                                 const std::string&,     // interface identifier
                                 Net_DHClientCallback_t, // result callback
                                 void*);                 // callback user data
  static bool hasState (dhcpctl_handle,      // interface handle
                        const std::string&); // state

  // *TODO*: apparently, omapi cannot retrieve leases (tried interface
  //         identifier, MAC address); go ahead, try it with omshell
  //         --> parse the dhclient.leases file manually for now
  static bool hasActiveLease (const std::string&,  // FQ dhclient.leases filename
                              const std::string&); // interface identifier
  // *TODO*: this API doesn't work yet
  static bool hasActiveLease (dhcpctl_handle,      // connection handle
                              const std::string&); // interface identifier

  static bool relinquishLease (dhcpctl_handle,      // connection handle
                               const std::string&); // interface identifier
#endif // DHCLIENT_USE
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~DHCP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools (const DHCP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Tools& operator= (const DHCP_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_USE)
  static bool hasDHClientOmapiSupport ();
#endif // DHCLIENT_USE
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
