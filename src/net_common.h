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
#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <string>
#endif // ACE_WIN32 || ACE_WIN64
#include <vector>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#endif // ACE_WIN32 || ACE_WIN64

#if defined (NETLINK_SUPPORT)
#include "ace/Netlink_Addr.h"
#endif // NETLINK_SUPPORT
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"

// forward declarations
template <ACE_SYNCH_DECL>
class Net_IConnectionManagerBase_T;
template <typename ConfigurationType>
class Net_ITransportLayer_T;
class Net_Common_Tools;

enum Net_LinkLayerType
{
  NET_LINKLAYER_802_3   = 0x01, // i.e. CSMA/CD, aka 'Ethernet'
  NET_LINKLAYER_802_11  = 0x02, // i.e. Wireless LAN aka 'WLAN' (EU; US: 'WiFi')
  NET_LINKLAYER_PPP     = 0x04,
  NET_LINKLAYER_FDDI    = 0x10,
  NET_LINKLAYER_ATM     = 0x11,
  ////////////////////////////////////////
  NET_LINKLAYER_MAX,
  NET_LINKLAYER_INVALID = -1,
};

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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
typedef std::vector<struct _GUID> Net_InterfaceIdentifiers_t;
#else
typedef std::vector<std::string> Net_InterfaceIdentifiers_t;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  NET_CONNECTION_STATUS_OK                   = 0,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_INITIALIZATION_FAILED,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_CLOSED,      // (local) close ()
  NET_CONNECTION_STATUS_PEER_CLOSED, // connection closed by the peer
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_MAX
};

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
class Net_Netlink_Addr
 : public ACE_Netlink_Addr
{
  typedef ACE_Netlink_Addr inherited;

 public:
  Net_Netlink_Addr ()
   : inherited ()
  {}
  Net_Netlink_Addr (const sockaddr_nl* address_in,
                    int length_in)
   : inherited (address_in,
                length_in)
  {}
  inline virtual ~Net_Netlink_Addr () {}

  inline Net_Netlink_Addr& operator= (const ACE_Addr& rhs) { *this = rhs; return *this; }

  // *NOTE*: (currently) implemented in net_configuration.cpp
  virtual int addr_to_string (ACE_TCHAR[],    // buffer
                              size_t,         // size
                              int = 1) const; // ipaddr_format
  inline bool is_any (void) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }

  inline void reset (void) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

struct Net_Statistic
{
  Net_Statistic ()
    : sentBytes (0.0F)
    , receivedBytes (0.0F)
    , timeStamp (ACE_Time_Value::zero)
    , previousBytes (0.0F)
    , previousTimeStamp (ACE_Time_Value::zero)
  {}
  struct Net_Statistic operator+= (const struct Net_Statistic& rhs_in)
  {
    sentBytes += rhs_in.sentBytes;
    receivedBytes += rhs_in.receivedBytes;

    timeStamp = rhs_in.timeStamp;
    previousBytes += rhs_in.previousBytes;
    previousTimeStamp = rhs_in.previousTimeStamp;

    return *this;
  }

  float          sentBytes;
  float          receivedBytes;

  // statistic and speed calculations
  ACE_Time_Value timeStamp;
  float          previousBytes; // total-
  ACE_Time_Value previousTimeStamp;
};
typedef struct Net_Statistic Net_Statistic_t;
typedef Common_IStatistic_T<Net_Statistic_t> Net_IStatisticHandler_t;
typedef Common_StatisticHandler_T<Net_Statistic_t> Net_StatisticHandler_t;

struct Net_StreamStatistic
 : Net_Statistic
{
  Net_StreamStatistic ()
   : Net_Statistic ()
   , streamStatistic ()
  {}
  struct Net_StreamStatistic operator+= (const struct Net_StreamStatistic& rhs_in)
  {
    Net_Statistic::operator+= (rhs_in);

    streamStatistic += rhs_in.streamStatistic;

    return *this;
  }

  struct Stream_Statistic streamStatistic;
};
typedef struct Net_StreamStatistic Net_StreamStatistic_t;
typedef Common_IStatistic_T<Net_StreamStatistic_t> Net_IStreamStatisticHandler_t;
typedef Common_StatisticHandler_T<Net_StreamStatistic_t> Net_StreamStatisticHandler_t;

struct Net_UserData
{
  Net_UserData ()
   : userData (NULL)
  {}

  void* userData;
};

typedef unsigned int Net_ConnectionId_t;

struct Net_ConnectionState
{
  Net_ConnectionState ()
   : handle (ACE_INVALID_HANDLE)
   , lastCollectionTimestamp (ACE_Time_Value::zero)
   , lock ()
   , statistic ()
   , status (NET_CONNECTION_STATUS_INVALID)
   , userData (NULL)
  {}

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

typedef Net_IConnectionManagerBase_T<ACE_MT_SYNCH> Net_IConnectionManagerBase_t;

#endif
