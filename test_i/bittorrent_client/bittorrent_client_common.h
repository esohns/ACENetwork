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

#ifndef BitTorrent_Client_COMMON_H
#define BitTorrent_Client_COMMON_H

#include <bitset>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <ace/Date_Time.h>
#include <ace/INET_Addr.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_inotify.h"

#include "stream_cachedmessageallocator.h"
#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnector.h"

#include "bittorrent_common.h"
#include "bittorrent_icontrol.h"
#include "bittorrent_stream_common.h"

//#include "FILE_Stream.h"
//#include "IOStream_alt_T.h"

// forward declarations
struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_ModuleHandlerConfiguration;
struct BitTorrent_Client_SessionData;
class BitTorrent_Client_SessionMessage;
struct BitTorrent_Client_SessionState;
struct BitTorrent_Client_SocketHandlerConfiguration;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType>
class BitTorrent_Message_T;
template <typename ControlMessageType,
          typename DataMessageType>
class BitTorrent_SessionMessage_T;
struct BitTorrent_Record;

typedef Stream_INotify_T<Stream_SessionMessageType> BitTorrent_Client_IStreamNotify_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    BitTorrent_Client_SessionData,
                                    Stream_SessionMessageType,
                                    BitTorrent_Message,
                                    BitTorrent_Client_SessionMessage> BitTorrent_Client_ISessionNotify_t;

// phonebook
typedef std::set<std::string> BitTorrent_Client_Networks_t;
typedef BitTorrent_Client_Networks_t::const_iterator BitTorrent_Client_NetworksIterator_t;
typedef std::pair<unsigned short, unsigned short> BitTorrent_Client_PortRange_t;
typedef std::vector<BitTorrent_Client_PortRange_t> BitTorrent_Client_PortRanges_t;
typedef BitTorrent_Client_PortRanges_t::const_iterator BitTorrent_Client_PortRangesIterator_t;
struct BitTorrent_Client_ConnectionEntry
{
  inline BitTorrent_Client_ConnectionEntry ()
   : hostName ()
   , ports ()
   , netWork ()
  {};

  std::string                    hostName;
  BitTorrent_Client_PortRanges_t ports;
  std::string                    netWork;
};
typedef std::vector<BitTorrent_Client_ConnectionEntry> BitTorrent_Client_Connections_t;
typedef BitTorrent_Client_Connections_t::const_iterator BitTorrent_Client_ConnectionsIterator_t;
typedef std::multimap<std::string,
                      BitTorrent_Client_ConnectionEntry> BitTorrent_Client_Servers_t;
typedef BitTorrent_Client_Servers_t::const_iterator BitTorrent_Client_ServersIterator_t;
struct BitTorrent_Client_PhoneBook
{
  inline BitTorrent_Client_PhoneBook ()
   : networks ()
   , servers ()
   , timeStamp ()
  {};

  // *TODO*: this member is redundant (but still useful ?)
  BitTorrent_Client_Networks_t networks;
  BitTorrent_Client_Servers_t  servers;
  ACE_Date_Time                timeStamp;
};

typedef std::deque<std::string> BitTorrent_Client_MessageQueue_t;
typedef BitTorrent_Client_MessageQueue_t::iterator BitTorrent_Client_MessageQueueIterator_t;
typedef BitTorrent_Client_MessageQueue_t::reverse_iterator BitTorrent_Client_MessageQueueReverseIterator_t;

//  ACE_IOStream<ACE_FILE_Stream> output_;
typedef ACE_IOStream_alt_T<ACE_FILE_Stream> BitTorrent_Client_IOStream_t;

struct BitTorrent_Client_InputThreadData
{
  inline BitTorrent_Client_InputThreadData ()
   : configuration (NULL)
   , cursesState (NULL)
   , groupID (-1)
   , moduleHandlerConfiguration (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  BitTorrent_Client_Configuration*              configuration;
  BitTorrent_Client_CursesState*                cursesState;
  int                                           groupID;
  BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
  bool                                          useReactor;
};

struct BitTorrent_Client_UserData
{
  inline BitTorrent_Client_UserData ()
   : configuration (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  BitTorrent_Client_Configuration*              configuration;

  // *TODO*: remove these ASAP
  Stream_ModuleConfiguration*                   moduleConfiguration;
  BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct BitTorrent_Client_SessionData
 : BitTorrent_Stream_SessionData
{
  inline BitTorrent_Client_SessionData ()
   : BitTorrent_Stream_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  BitTorrent_Client_SessionState* connectionState;

  BitTorrent_Client_UserData*     userData;
};

typedef Stream_Statistic BitTorrent_RuntimeStatistic_t;

struct BitTorrent_Client_ConnectionState
 : BitTorrent_ConnectionState
{
  inline BitTorrent_Client_ConnectionState ()
   : BitTorrent_ConnectionState ()
   , configuration (NULL)
   , controller (NULL)
   , userData (NULL)
  {};

  BitTorrent_Client_Configuration* configuration;
  BitTorrent_IControl*             controller;

  BitTorrent_Client_UserData*      userData;
};

// *TODO*: remove this ASAP
struct BitTorrent_Client_SessionState
 : BitTorrent_Client_ConnectionState
{
  inline BitTorrent_Client_SessionState ()
   : BitTorrent_Client_ConnectionState ()
   , away (false)
   , channel ()
   , channelModes ()
   , isFirstMessage (false)
   , nickName ()
   , userModes ()
  {};

  // *TODO*: remove this
  bool                      away;
  std::string               channel;
  BitTorrent_ChannelModes_t channelModes;
  bool                      isFirstMessage;
  std::string               nickName;
  BitTorrent_UserModes_t    userModes;
};

typedef Net_IConnector_T<ACE_INET_Addr,
                         BitTorrent_Client_SocketHandlerConfiguration> BitTorrent_Client_IConnector_t;
struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline BitTorrent_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , connector (NULL)
   , cursesState (NULL)
   , peerAddress ()
  {};

  BitTorrent_Client_IConnector_t* connector;
  BitTorrent_Client_CursesState*  cursesState;
  ACE_INET_Addr                   peerAddress;
};

typedef Stream_CachedMessageAllocator_T<Stream_AllocatorConfiguration,
                                        BitTorrent_Client_ControlMessage_t,
                                        BitTorrent_Message,
                                        BitTorrent_Client_SessionMessage> BitTorrent_Client_MessageAllocator_t;

#endif
