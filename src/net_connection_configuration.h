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

#ifndef NET_CONNECTION_CONFIGURATION_H
#define NET_CONNECTION_CONFIGURATION_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "Ks.h"
#include "WinSock2.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <map>
#include <string>

#if defined (SSL_SUPPORT)
#include "openssl/ssl.h"
#endif // SSL_SUPPORT

#include "ace/Basic_Types.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"

#include "common_configuration.h"
#include "common_event_common.h"

#include "common_timer_common.h"

#include "net_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "net_common_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "net_defines.h"
#if defined (NETLINK_SUPPORT)
#include "net_netlink_address.h"
#endif // NETLINK_SUPPORT

// forward declarations
class Stream_IAllocator;
struct Net_UserData;

struct Net_SocketConfigurationBase
{
  Net_SocketConfigurationBase ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
   , interfaceIdentifier (GUID_NULL)
#else
   , interfaceIdentifier ()
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
#else
   , interfaceIdentifier (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET))
#endif // ACE_WIN32 || ACE_WIN64
   , linger (NET_SOCKET_DEFAULT_LINGER)
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
   , reuseAddress (NET_SOCKET_DEFAULT_REUSEADDR)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , reusePort (NET_SOCKET_DEFAULT_REUSEPORT)
#endif // ACE_WIN32 || ACE_WIN64
   , domain (PF_INET)
   , type (SOCK_STREAM)
   , protocol (IPPROTO_TCP)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    interfaceIdentifier = Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_3);
#else
    interfaceIdentifier = Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
#endif // ACE_WIN32 || ACE_WIN64
  }

  int          bufferSize; // socket buffer size (I/O)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interfaceIdentifier; // NIC-
#else
  std::string  interfaceIdentifier; // NIC-
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string  interfaceIdentifier; // NIC-
#endif // ACE_WIN32 || ACE_WIN64
  bool         linger;
  bool         useLoopBackDevice; // (if any)
  bool         reuseAddress;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool         reusePort;
#endif // ACE_WIN32 || ACE_WIN64

  int          domain;   // socket(3) parameter
  int          type;     // socket(3) parameter
  int          protocol; // socket(3) parameter
};

template <enum Net_TransportLayerType TransportLayerType_e>
class Net_SocketConfiguration_T
 : public Net_SocketConfigurationBase // *NOTE*: POD
{
  typedef Net_SocketConfigurationBase inherited;

 public:
  Net_SocketConfiguration_T ()
   : inherited ()
  { // *IMPORTANT NOTE*: only the template specializations are currently
    //                   supported
    ACE_ASSERT (false);
    ACE_NOTSUP;
    ACE_NOTREACHED (return;)
  }
  //inline virtual ~Net_SocketConfiguration_T () {}
};

//////////////////////////////////////////

template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP>
 : public Net_SocketConfigurationBase
{
  typedef Net_SocketConfigurationBase inherited;

 public:
  Net_SocketConfiguration_T ()
   : inherited ()
   , address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
#if defined (SSL_SUPPORT)
   , hostname ()
   , method (NULL)
   , minimalVersion (0)
   , maximalVersion (0)
#endif // SSL_SUPPORT
  {
    int result = -1;

    if (unlikely (useLoopBackDevice))
    {
      result = address.set (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                            static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                            1,
                            0);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
    } // end IF
  }
  //inline virtual ~Net_SocketConfiguration_T () {}

  ACE_INET_Addr     address;  // listening/peer-
#if defined (SSL_SUPPORT) // *TODO*: move these somewhere else
  std::string       hostname; // peer- (support TLS SNI)
  const SSL_METHOD* method;
  long              minimalVersion; // minimal protocol-
  long              maximalVersion; // maximal protocol-
#endif // SSL_SUPPORT
};

template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP>
 : public Net_SocketConfigurationBase
{
  typedef Net_SocketConfigurationBase inherited;

 public:
  Net_SocketConfiguration_T ()
   : inherited ()
   , connect (NET_SOCKET_DEFAULT_UDP_CONNECT)
   , listenAddress (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , peerAddress (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                  static_cast<ACE_UINT32> (INADDR_ANY))
   , sourcePort (0)
   , writeOnly (false) // *TODO*: remove ASAP
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
    //                SO_LINGER
    linger = false;
#endif // ACE_WIN32 || ACE_WIN64
    type = SOCK_DGRAM;
    protocol = IPPROTO_UDP;

    int result = -1;

    if (unlikely (useLoopBackDevice))
    {
      result =
        listenAddress.set (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                           static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                           1,
                           0);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
      result =
        peerAddress.set (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                         static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                         1,
                         0);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
    } // end IF

    if (unlikely (writeOnly))
      listenAddress.reset ();
  }
  //inline virtual ~Net_SocketConfiguration_T () {}

  // *IMPORTANT NOTE*: set this for asynchronous event dispatch; the socket
  //                   needs to be associated with the peer address, as the data
  //                   dispatch happens out of context
  bool          connect;
  ACE_INET_Addr listenAddress;
  ACE_INET_Addr peerAddress;
  ACE_UINT16    sourcePort; // specify a specific source port (outbound)
  bool          writeOnly; // *TODO*: remove ASAP
};

#if defined (NETLINK_SUPPORT)
template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_NETLINK>
 : public Net_SocketConfigurationBase
{
  typedef Net_SocketConfigurationBase inherited;

 public:
  Net_SocketConfiguration_T ()
   : inherited ()
   , address ()
   , protocol (NET_PROTOCOL_DEFAULT_NETLINK)
  {}
  //inline virtual ~Net_SocketConfiguration_T () {}

  Net_Netlink_Addr address;
  int              protocol;
};
#endif // NETLINK_SUPPORT

//#define NET_SOCKET_CONFIGURATION_TCP_CAST(X) static_cast<Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP>* > (X)
//#define NET_SOCKET_CONFIGURATION_UDP_CAST(X) static_cast<Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP>* > (X)

//////////////////////////////////////////

struct Net_ConnectionConfigurationBase
{
  Net_ConnectionConfigurationBase ()
   : allocatorConfiguration (NULL)
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , generateUniqueIOModuleNames (false)
   , messageAllocator (NULL)
   , statisticCollectionInterval (0,
                                  NET_STATISTIC_DEFAULT_COLLECTION_INTERVAL_MS * 1000)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S,
                                 0)
   , timerManager (NULL)
   , useThreadPerConnection (false)
  {}

  struct Common_AllocatorConfiguration* allocatorConfiguration;
  enum Common_EventDispatchType         dispatch;
  bool                                  generateUniqueIOModuleNames; // stream-
  Stream_IAllocator*                    messageAllocator;
  ACE_Time_Value                        statisticCollectionInterval; // [ACE_Time_Value::zero: off]
  ACE_Time_Value                        statisticReportingInterval; // [ACE_Time_Value::zero: off]
  Common_ITimerCB_t*                    timerManager;
  bool                                  useThreadPerConnection;
};

template <enum Net_TransportLayerType TransportLayerType_e>
class Net_ConnectionConfiguration_T
 : public Net_ConnectionConfigurationBase
{
  typedef Net_ConnectionConfigurationBase inherited;

 public:
  Net_ConnectionConfiguration_T ()
   : inherited ()
   , socketConfiguration ()
  {}

  // convenient types
  typedef Net_SocketConfiguration_T<TransportLayerType_e> SOCKET_CONFIGURATION_T;

  SOCKET_CONFIGURATION_T socketConfiguration;
};

#define NET_CONFIGURATION_TCP_CAST(X) static_cast<Net_ConnectionConfiguration_T<NET_TRANSPORTLAYER_TCP>* > (X)
#define NET_CONFIGURATION_UDP_CAST(X) static_cast<Net_ConnectionConfiguration_T<NET_TRANSPORTLAYER_UDP>* > (X)

//////////////////////////////////////////

template <typename StreamConfigurationType, // *NOTE*: connection- (&& implements Stream_Configuration_T)
          enum Net_TransportLayerType TransportLayerType_e>
class Net_StreamConnectionConfiguration_T
 : public Net_ConnectionConfiguration_T<TransportLayerType_e>
{
  typedef Net_ConnectionConfiguration_T<TransportLayerType_e> inherited;

 public:
  Net_StreamConnectionConfiguration_T ()
   : inherited ()
   , delayRead (false)
   , streamConfiguration (NULL)
  {}

  // convenient types
  typedef StreamConfigurationType STREAM_CONFIGURATION_T;

  // *IMPORTANT NOTE*: this delays setting up the receiving procedure
  //                   (i.e. registering with the reactor, starting asynch
  //                   receive) so that 'downstream' can 'link' before data
  //                   starts arriving. This is useful when the server starts
  //                   to send data immediately on a successful connect by the
  //                   client: no data is lost
  // *NOTE*: that Stream_Module_Net_IO_Stream_T automatically calls
  //         initiate_read() in its onLink() implementation
  bool                     delayRead;
  StreamConfigurationType* streamConfiguration;
};

//////////////////////////////////////////

#if defined (NETLINK_SUPPORT)
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_NETLINK> Net_NetlinkSocketConfiguration_t;
#endif // NETLINK_SUPPORT
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP> Net_TCPSocketConfiguration_t;
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP> Net_UDPSocketConfiguration_t;

#if defined (NETLINK_SUPPORT)
typedef Net_ConnectionConfiguration_T<NET_TRANSPORTLAYER_NETLINK> Net_NetlinkConnectionConfiguration_t;
#endif // NETLINK_SUPPORT
typedef Net_ConnectionConfiguration_T<NET_TRANSPORTLAYER_TCP> Net_TCPConnectionConfiguration_t;
typedef Net_ConnectionConfiguration_T<NET_TRANSPORTLAYER_UDP> Net_UDPConnectionConfiguration_t;

typedef std::map<std::string,
                 struct Net_ConnectionConfigurationBase*> Net_ConnectionConfigurations_t;
typedef Net_ConnectionConfigurations_t::iterator Net_ConnectionConfigurationsIterator_t;

// include template definition
#include "net_connection_configuration.inl"

#endif
