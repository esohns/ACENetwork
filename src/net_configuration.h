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

#ifndef NET_CONFIGURATION_H
#define NET_CONFIGURATION_H

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
#include <cguid.h>

#include "net_common_tools.h"
#endif
#include "net_defines.h"
#include "net_iconnectionmanager.h"

// forward declarations
class Stream_IAllocator;
struct Net_UserData;

struct Net_AllocatorConfiguration
 : Common_AllocatorConfiguration
{
  Net_AllocatorConfiguration ()
   : Common_AllocatorConfiguration ()
  {
    defaultBufferSize = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    paddingBytes = NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Net_SocketConfigurationBase
{
  Net_SocketConfigurationBase ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , interfaceIdentifier (GUID_NULL)
#else
   , interfaceIdentifier (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET))
#endif
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    interfaceIdentifier =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
#endif
  };
  inline virtual ~Net_SocketConfigurationBase () {}

  int         bufferSize; // socket buffer size (I/O)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interfaceIdentifier; // NIC-
#else
  std::string  interfaceIdentifier; // NIC-
#endif
  bool         useLoopBackDevice;   // (if any)
};

#if defined (ACE_HAS_NETLINK)
struct Net_NetlinkSocketConfiguration
 : Net_SocketConfigurationBase
{
  Net_NetlinkSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , address ()
   , protocol (NET_PROTOCOL_DEFAULT_NETLINK)
  {};

  Net_Netlink_Addr address;
  int              protocol;
};
#endif

struct Net_TCPSocketConfiguration
 : Net_SocketConfigurationBase
{
  Net_TCPSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , linger (NET_SOCKET_DEFAULT_LINGER)
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
  };

  ACE_INET_Addr address;
  bool          linger;
};

struct Net_UDPSocketConfiguration
 : Net_SocketConfigurationBase
{
  Net_UDPSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , connect (NET_SOCKET_DEFAULT_UDP_CONNECT)
   // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
   //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , linger (NET_SOCKET_DEFAULT_LINGER)
#endif
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
  };

  // *IMPORTANT NOTE*: set this for asynchronous event dispatch; the socket
  //                   needs to be associated with the peer address, as the data
  //                   dispatch happens out of context
  bool          connect;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool          linger;
#endif
  ACE_INET_Addr listenAddress;
  ACE_INET_Addr peerAddress;
  ACE_UINT16    sourcePort; // specify a specific source port (outbound)
  bool          writeOnly; // *TODO*: remove ASAP
};

//typedef std::deque<ACE_INET_Addr> Net_InetAddressStack_t;
//typedef Net_InetAddressStack_t::iterator Net_InetAddressStackIterator_t;

struct Net_ConnectionConfiguration;
struct Net_SocketHandlerConfiguration
{
  Net_SocketHandlerConfiguration ()
   : connectionConfiguration (NULL)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
                                 0)
   , useThreadPerConnection (false)
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Net_ConnectionConfiguration* connectionConfiguration;
  struct Net_SocketConfigurationBase* socketConfiguration;
  ACE_Time_Value                      statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                useThreadPerConnection;

  struct Net_UserData*                userData;
};

struct Stream_Configuration;
class Common_ITimer;
template <typename ConnectionConfigurationType, // derives from Net_ConnectionConfiguration
          typename AllocatorConfigurationType,
          typename StreamConfigurationType>
class Net_StreamConnectionConfiguration_T;
struct Net_ConnectionConfiguration;
typedef Net_StreamConnectionConfiguration_T<struct Net_ConnectionConfiguration,
                                            struct Net_AllocatorConfiguration,
                                            struct Stream_Configuration> Net_ConnectionConfiguration_t;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Net_ConnectionConfiguration_t,
                                 struct Net_ConnectionState,
                                 Net_Statistic_t,
                                 struct Net_UserData> Net_IInetConnectionManager_t;
struct Net_ConnectionConfiguration
{
  Net_ConnectionConfiguration ()
   : connectionManager (NULL)
   , generateUniqueIOModuleNames (false)
   , messageAllocator (NULL)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , socketHandlerConfiguration ()
   , timerManager (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData (NULL)
  {};

  Net_IInetConnectionManager_t*         connectionManager;
  bool                                  generateUniqueIOModuleNames; // stream
  Stream_IAllocator*                    messageAllocator;
  // *NOTE*: applies to the corresponding protocol, if it has fixed size
  //         datagrams; otherwise, this is the size of the individual (opaque)
  //         stream buffers
  unsigned int                          PDUSize; // package data unit size
  struct Net_SocketHandlerConfiguration socketHandlerConfiguration;
  Common_ITimer_t*                      timerManager;
  bool                                  useReactor;

  struct Net_UserData*                  userData;
};
typedef std::map<std::string,
                 Net_ConnectionConfiguration_t> Net_ConnectionConfigurations_t;
typedef Net_ConnectionConfigurations_t::iterator Net_ConnectionConfigurationIterator_t;

template <typename ConnectionConfigurationType, // derives from Net_ConnectionConfiguration
          typename AllocatorConfigurationType,
          typename StreamConfigurationType>
class Net_StreamConnectionConfiguration_T
 : public ConnectionConfigurationType
{
  typedef ConnectionConfigurationType inherited;

 public:
  Net_StreamConnectionConfiguration_T ();
  inline virtual ~Net_StreamConnectionConfiguration_T () {}

  bool initialize (const AllocatorConfigurationType&,
                   const StreamConfigurationType&);

  AllocatorConfigurationType allocatorConfiguration_;
  StreamConfigurationType*   streamConfiguration_;
  bool                       isInitialized_;
};

// include template definition
#include "net_configuration.inl"

struct Net_SessionConfiguration
{
  Net_SessionConfiguration ()
   : parserConfiguration (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  struct Common_ParserConfiguration* parserConfiguration;
  bool                               useReactor;
};

struct Net_ListenerConfiguration
{
  Net_ListenerConfiguration ()
   : addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionConfiguration (NULL)
  {};

  int                                 addressFamily;
  struct Net_ConnectionConfiguration* connectionConfiguration;
};

#endif
