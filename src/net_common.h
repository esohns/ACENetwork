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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include <set>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <string>
#endif // ACE_WIN32 || ACE_WIN64
#include <vector>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "guiddef.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Basic_Types.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

//#include "net_iconnection.h"
#include "net_statistic.h"

// forward declarations
template <typename AddressType,
          typename StateType,
          typename StatisticContainerType>
class Net_IConnection_T;

/////////////////////////////////////////

enum Net_LinkLayerType
{
  NET_LINKLAYER_802_3   = 0x01, // i.e. CSMA/CD, aka 'Ethernet'
  NET_LINKLAYER_802_11  = 0x02, // i.e. Wireless LAN aka 'WLAN' (EU; US: 'WiFi')
  NET_LINKLAYER_PPP     = 0x04,
  NET_LINKLAYER_FDDI    = 0x08,
  NET_LINKLAYER_ATM     = 0x10,
  ////////////////////////////////////////
  NET_LINKLAYER_MAX,
  NET_LINKLAYER_INVALID = -1,
};
//enum Net_LinkLayerType& operator++ (enum Net_LinkLayerType& type_in)
//{
////#if MY_ENUMS_ARE_CONTIGUOUS && I_DO_NOT_WORRY_ABOUT_OVERFLOW
////  return type_in = static_cast<enum Net_LinkLayerType> (++static_cast<int>(enum Net_LinkLayerType));
////#else
//  switch (type_in)
//  {
//    case NET_LINKLAYER_802_3: return type_in = NET_LINKLAYER_802_11;
//    case NET_LINKLAYER_802_11: return type_in = NET_LINKLAYER_PPP;
//    case NET_LINKLAYER_PPP: return type_in = NET_LINKLAYER_FDDI;
//    case NET_LINKLAYER_FDDI: return type_in = NET_LINKLAYER_ATM;
//    case NET_LINKLAYER_ATM: 
//    default:
//      return type_in = NET_LINKLAYER_MAX;
//  }
//  
//  return type_in = NET_LINKLAYER_INVALID; // some compilers might warn otherwise
////#endif
//}
//enum Net_LinkLayerType operator++ (enum Net_LinkLayerType& type_in, int)
//{
//  enum Net_LinkLayerType tmp (type_in);
//  ++type_in;
//  return tmp;
//}

enum Net_NetworkLayerType
{
  NET_NETWORKLAYER_IP_UNICAST   = 0x0001, // i.e. "routable" IP
  NET_NETWORKLAYER_IP_BROADCAST = 0x0002,
  NET_NETWORKLAYER_IP_MULTICAST = 0x0004,
  ////////////////////////////////////////
  NET_NETWORKLAYER_MAX,
  NET_NETWORKLAYER_INVALID      = -1,
};

enum Net_TransportLayerType
{
  NET_TRANSPORTLAYER_INVALID = -1,
  NET_TRANSPORTLAYER_IP_CAST = 0,  // 'pseudo' (LAN-only, no flow control)
  NET_TRANSPORTLAYER_NETLINK,      // 'pseudo' ((Linux-)host only, no flow control) kernel<->user space protocol
  NET_TRANSPORTLAYER_TCP,
  NET_TRANSPORTLAYER_UDP,
  NET_TRANSPORTLAYER_SSL,
  ////////////////////////////////////////
  NET_TRANSPORTLAYER_MAX
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: prior to Vista, Win32 used 'indexes' integer identifiers
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
typedef std::vector<struct _GUID> Net_InterfaceIdentifiers_t;
#else
typedef std::vector<std::string> Net_InterfaceIdentifiers_t;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
#else
typedef std::vector<std::string> Net_InterfaceIdentifiers_t;
#endif // ACE_WIN32 || ACE_WIN64
typedef Net_InterfaceIdentifiers_t::iterator Net_InterfacesIdentifiersIterator_t;

enum Net_ClientServerRole
{
  NET_ROLE_INVALID = -1,
  NET_ROLE_CLIENT  = 0,
  NET_ROLE_SERVER,
  ////////////////////////////////////////
  NET_ROLE_MAX
};

//enum Net_Stream_ControlMessageType : int
//{
//  NET_STREAM_CONTROL_MESSAGE_MASK = STREAM_CONTROL_MESSAGE_USER_MASK,
//  ////////////////////////////////////////
//  NET_STREAM_CONTROL_MESSAGE_MAX,
//  NET_STREAM_CONTROL_MESSAGE_INVALID
//};

//enum Net_Stream_SessionMessageType : int
//{
//  NET_STREAM_SESSION_MESSAGE_MASK = STREAM_SESSION_MESSAGE_USER_MASK,
//  // *** notification ***
//  NET_STREAM_SESSION_MESSAGE_CLOSE,
//  ////////////////////////////////////////
//  NET_STREAM_SESSION_MESSAGE_MAX,
//  NET_STREAM_SESSION_MESSAGE_INVALID
//};

//enum Net_Stream_ControlType : int
//{
//  NET_STREAM_CONTROL_MASK = STREAM_CONTROL_USER_MASK,
//  ////////////////////////////////////////
//  NET_STREAM_CONTROL_MAX,
//  NET_STREAM_CONTROL_INVALID
//};

//enum Net_Connection_AbortStrategy : int
enum Net_Connection_AbortStrategy
{
  NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST = 0,
  NET_CONNECTION_ABORT_STRATEGY_RECENT_MOST,
  ////////////////////////////////////////
  NET_CONNECTION_ABORT_STRATEGY_MAX,
  NET_CONNECTION_ABORT_STRATEGY_INVALID
};

// *NOTE*: this extends ACE_Svc_Handler_Close (see Svc_Handler.h)
//enum Net_Connection_CloseReason : int
enum Net_Connection_CloseReason
{
  NET_CONNECTION_CLOSE_REASON_INVALID        = -1,
  ////////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_INITIALIZATION = 0x02,
  NET_CONNECTION_CLOSE_REASON_USER_ABORT,
  ////////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_MAX
};

//enum Net_Connection_Status : int
enum Net_Connection_Status
{
  NET_CONNECTION_STATUS_INVALID              = -1,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_INITIALIZING         = 0,
  NET_CONNECTION_STATUS_INITIALIZATION_FAILED,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_OK,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_CLOSED,      // (local) close ()
  NET_CONNECTION_STATUS_PEER_CLOSED, // connection closed by the peer
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_MAX
};

struct Net_UserData
{
  Net_UserData ()
   : userData (NULL)
  {}

  void* userData;
};

typedef ACE_HANDLE Net_ConnectionId_t;

struct Net_ConnectionState
{
  Net_ConnectionState ()
   : closed (false)
   , handle (ACE_INVALID_HANDLE)
   , lastCollectionTimestamp (ACE_Time_Value::zero)
   , lock ()
   , statistic ()
   , status (NET_CONNECTION_STATUS_INVALID)
   , userData (NULL)
  {}

  bool                       closed; // handle_close() has been called ?
  ACE_HANDLE                 handle;
  ACE_Time_Value             lastCollectionTimestamp;
  ACE_SYNCH_MUTEX            lock;
  Net_Statistic_t            statistic;
  enum Net_Connection_Status status;

  struct Net_UserData*       userData;
};

struct Net_StreamConnectionState
 : Net_ConnectionState
{
  Net_StreamConnectionState ()
   : Net_ConnectionState ()
   , statistic ()
  {}

  Net_StreamStatistic_t statistic;
};

typedef std::set<Net_ConnectionId_t> Net_ConnectionIds_t;
typedef Net_ConnectionIds_t::iterator Net_ConnectionIdsIterator_t;

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Net_StreamConnectionState,
                          Net_StreamStatistic_t> Net_IINETConnection_t;

#endif
