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

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "stream_common.h"

// forward declarations
template <typename ConfigurationType>
class Net_ITransportLayer_T;
struct Net_SocketConfiguration;

enum Net_ClientServerRole
{
  NET_ROLE_INVALID = -1,
  NET_ROLE_CLIENT = 0,
  NET_ROLE_SERVER,
  ////////////////////////////////////////
  NET_ROLE_MAX
};

enum Net_TransportLayerType
{
  NET_TRANSPORTLAYER_INVALID = -1,
  NET_TRANSPORTLAYER_IP_BROADCAST = 0,
  NET_TRANSPORTLAYER_IP_MULTICAST,
  NET_TRANSPORTLAYER_NETLINK,
  NET_TRANSPORTLAYER_TCP,
  NET_TRANSPORTLAYER_UDP,
  ////////////////////////////////////////
  NET_TRANSPORTLAYER_MAX
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

// *NOTE*: this extends ACE_Svc_Handler_Close (see Svc_Handler.h)
enum Net_Connection_CloseReason
{
  NET_CONNECTION_CLOSE_REASON_INVALID = -1,
  ////////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_INITIALIZATION = 0x02,
  NET_CONNECTION_CLOSE_REASON_USER_ABORT,
  ////////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_MAX
};

enum Net_Connection_Status
{
  NET_CONNECTION_STATUS_INVALID = -1,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_OK = 0,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_INITIALIZATION_FAILED,
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_CLOSED,      // (local) close ()
  NET_CONNECTION_STATUS_PEER_CLOSED, // connection closed by the peer
  ////////////////////////////////////////
  NET_CONNECTION_STATUS_MAX
};

typedef Stream_Statistic Net_RuntimeStatistic_t;
typedef Net_ITransportLayer_T<Net_SocketConfiguration> Net_ITransportLayer_t;

struct Net_UserData
{
  inline Net_UserData ()
   : userData (NULL)
  {};

  void* userData;
};

struct Net_ConnectionState
{
  inline Net_ConnectionState ()
   : status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
   , lock ()
   , userData (NULL)
  {};

  Net_Connection_Status  status;

  Net_RuntimeStatistic_t currentStatistic;
  ACE_Time_Value         lastCollectionTimestamp;
  ACE_SYNCH_MUTEX        lock;

  Net_UserData*          userData;
};

#endif
