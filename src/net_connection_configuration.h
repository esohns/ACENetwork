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
#include <Ks.h>
#endif // ACE_WIN32 || ACE_WIN64

#include <map>
#include <string>

#include "ace/Basic_Types.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"

#include "common_configuration.h"

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_configuration.h"

#include "net_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "net_common_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "net_defines.h"
#include "net_iconnectionmanager.h"

// forward declarations
class Stream_IAllocator;
struct Net_UserData;

struct Net_SocketConfigurationBase
{
  Net_SocketConfigurationBase ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
   , interfaceIdentifier (GUID_NULL)
#else
   , interfaceIdentifier ()
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
   , interfaceIdentifier (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET))
#endif // ACE_WIN32 || ACE_WIN64
   , linger (NET_SOCKET_DEFAULT_LINGER)
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , statisticCollectionInterval (0,
                                  NET_STATISTIC_DEFAULT_COLLECTION_INTERVAL_MS * 1000)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S,
                                 0)
   , useThreadPerConnection (false)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //interfaceIdentifier =
    //  Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
#endif // ACE_WIN32 || ACE_WIN64
  }
  inline virtual ~Net_SocketConfigurationBase () {}

  int            bufferSize; // socket buffer size (I/O)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID   interfaceIdentifier; // NIC-
#else
  std::string    interfaceIdentifier; // NIC-
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string    interfaceIdentifier; // NIC-
#endif // ACE_WIN32 || ACE_WIN64
  // *NOTE*: win32 udp sockets do not linger
  bool           linger;
  // *NOTE*: applies to the corresponding protocol, if it has fixed size
  //         datagrams
  // *NOTE*: this is the size of the chunks requested from the kernel
  bool           useLoopBackDevice;   // (if any)

  // *TODO*: move these into Net_SocketConfiguration_T ASAP
  unsigned int   PDUSize; // package data unit size
  ACE_Time_Value statisticCollectionInterval; // [ACE_Time_Value::zero: off]
  ACE_Time_Value statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool           useThreadPerConnection;
};

template <enum Net_TransportLayerType TransportLayerType_e>
class Net_SocketConfiguration_T
 : public Net_SocketConfigurationBase
{
 public:
  Net_SocketConfiguration_T ()
   : Net_SocketConfigurationBase ()
  {}
  inline virtual ~Net_SocketConfiguration_T () {}
};

//////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_NETLINK>
 : public Net_SocketConfigurationBase
{
 public:
  Net_SocketConfiguration_T ()
   : Net_SocketConfigurationBase ()
   , address ()
   , protocol (NET_PROTOCOL_DEFAULT_NETLINK)
  {}
  inline virtual ~Net_SocketConfiguration_T () {}

  Net_Netlink_Addr address;
  int              protocol;
};
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP>
 : public Net_SocketConfigurationBase
{
 public:
  Net_SocketConfiguration_T ()
   : Net_SocketConfigurationBase ()
   , address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
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
  inline virtual ~Net_SocketConfiguration_T () {}

  ACE_INET_Addr address; // listening/peer-
};

template <>
class Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP>
 : public Net_SocketConfigurationBase
{
 public:
  Net_SocketConfiguration_T ()
   : Net_SocketConfigurationBase ()
   , connect (NET_SOCKET_DEFAULT_UDP_CONNECT)
// *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
//                SO_LINGER
   , listenAddress (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , peerAddress (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                  static_cast<ACE_UINT32> (INADDR_ANY))
   , sourcePort (0)
   , writeOnly (false) // *TODO*: remove ASAP
  {
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
  inline virtual ~Net_SocketConfiguration_T () {}

  // *IMPORTANT NOTE*: set this for asynchronous event dispatch; the socket
  //                   needs to be associated with the peer address, as the data
  //                   dispatch happens out of context
  bool          connect;
  ACE_INET_Addr listenAddress;
  ACE_INET_Addr peerAddress;
  ACE_UINT16    sourcePort; // specify a specific source port (outbound)
  bool          writeOnly; // *TODO*: remove ASAP
};

#define NET_SOCKET_CONFIGURATION_TCP_CAST(X) static_cast<Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP>* > (X)
#define NET_SOCKET_CONFIGURATION_UDP_CAST(X) static_cast<Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP>* > (X)

//////////////////////////////////////////

//struct Stream_Configuration;
class Common_ITimer;
template <enum Net_TransportLayerType TransportLayerType_e>
class Net_ConnectionConfigurationBase_T
 : public Net_SocketConfiguration_T<TransportLayerType_e>
{
 public:
  Net_ConnectionConfigurationBase_T ()
   : Net_SocketConfiguration_T<TransportLayerType_e> ()
   , dispatch (NET_EVENT_DEFAULT_DISPATCH)
   , generateUniqueIOModuleNames (false)
   , messageAllocator (NULL)
   , timerManager (NULL)
  {}

  enum Common_EventDispatchType dispatch;
  bool                          generateUniqueIOModuleNames; // stream
  Stream_IAllocator*            messageAllocator;
  Common_ITimerCB_t*            timerManager;
};

template <typename AllocatorConfigurationType,
          typename StreamConfigurationType, // *NOTE*: connection-
          enum Net_TransportLayerType TransportLayerType_e>
class Net_ConnectionConfiguration_T
 : public Net_ConnectionConfigurationBase_T<TransportLayerType_e>
{
  typedef Net_ConnectionConfigurationBase_T<TransportLayerType_e> inherited;

 public:
  Net_ConnectionConfiguration_T ();
  inline virtual ~Net_ConnectionConfiguration_T () {}

  bool initialize (const AllocatorConfigurationType&,
                   const StreamConfigurationType&);

  AllocatorConfigurationType allocatorConfiguration_;
  StreamConfigurationType*   streamConfiguration_;
  bool                       isInitialized_;
};

//////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_NETLINK> Net_NetlinkSocketConfiguration_t;
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_TCP> Net_TCPSocketConfiguration_t;
typedef Net_SocketConfiguration_T<NET_TRANSPORTLAYER_UDP> Net_UDPSocketConfiguration_t;

typedef std::map<std::string,
                 Net_SocketConfigurationBase*> Net_ConnectionConfigurations_t;
typedef Net_ConnectionConfigurations_t::iterator Net_ConnectionConfigurationsIterator_t;

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
typedef Net_ConnectionConfigurationBase_T<NET_TRANSPORTLAYER_NETLINK> Net_NetlinkConnectionConfigurationBase_t;
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
typedef Net_ConnectionConfigurationBase_T<NET_TRANSPORTLAYER_TCP> Net_TCPConnectionConfigurationBase_t;
typedef Net_ConnectionConfigurationBase_T<NET_TRANSPORTLAYER_UDP> Net_UDPConnectionConfigurationBase_t;

//#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
//typedef Net_ITransportLayer_T<Net_NetlinkSocketConfiguration_t> Net_INetlinkTransportLayer_t;
//#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
//typedef Net_ITransportLayer_T<Net_TCPSocketConfiguration_t> Net_ITCPTransportLayer_t;
//typedef Net_ITransportLayer_T<Net_UDPSocketConfiguration_t> Net_IUDPTransportLayer_t;

// include template definition
#include "net_connection_configuration.inl"

#endif
