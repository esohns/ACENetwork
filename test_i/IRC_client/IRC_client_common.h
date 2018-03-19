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

#ifndef IRC_CLIENT_COMMON_H
#define IRC_CLIENT_COMMON_H

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ace/Date_Time.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_configuration.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnector.h"

#include "irc_common.h"
#include "irc_configuration.h"
#include "irc_icontrol.h"
//#include "irc_network.h"
#include "irc_stream_common.h"

#include "FILE_Stream.h"
#include "IOStream_alt_T.h"
#include "IRC_client_defines.h"

#include "IRC_client_network.h"

// phonebook
typedef std::set<std::string> IRC_Client_Networks_t;
typedef IRC_Client_Networks_t::const_iterator IRC_Client_NetworksIterator_t;
typedef std::pair<unsigned short, unsigned short> IRC_Client_PortRange_t;
typedef std::vector<IRC_Client_PortRange_t> IRC_Client_PortRanges_t;
typedef IRC_Client_PortRanges_t::const_iterator IRC_Client_PortRangesIterator_t;
struct IRC_Client_ConnectionEntry
{
  IRC_Client_ConnectionEntry ()
   : hostName ()
   , ports ()
   , netWork ()
  {};

  std::string             hostName;
  IRC_Client_PortRanges_t ports;
  std::string             netWork;
};
typedef std::vector<struct IRC_Client_ConnectionEntry> IRC_Client_Connections_t;
typedef IRC_Client_Connections_t::const_iterator IRC_Client_ConnectionsIterator_t;
typedef std::multimap<std::string,
                      struct IRC_Client_ConnectionEntry> IRC_Client_Servers_t;
typedef IRC_Client_Servers_t::const_iterator IRC_Client_ServersIterator_t;
struct IRC_Client_PhoneBook
{
  IRC_Client_PhoneBook ()
   : networks ()
   , servers ()
   , timeStamp ()
  {};

  // *TODO*: this member is redundant (but still useful ?)
  IRC_Client_Networks_t networks;
  IRC_Client_Servers_t  servers;
  ACE_Date_Time         timeStamp;
};

typedef std::deque<std::string> IRC_Client_MessageQueue_t;
typedef IRC_Client_MessageQueue_t::iterator IRC_Client_MessageQueueIterator_t;
typedef IRC_Client_MessageQueue_t::reverse_iterator IRC_Client_MessageQueueReverseIterator_t;

//  ACE_IOStream<ACE_FILE_Stream> output_;
typedef ACE_IOStream_alt_T<ACE_FILE_Stream> IRC_Client_IOStream_t;

struct IRC_Client_Configuration;
struct IRC_Client_CursesState;
struct IRC_Client_ModuleHandlerConfiguration;
struct IRC_Client_InputThreadData
{
  IRC_Client_InputThreadData ()
   : configuration (NULL)
   , cursesState (NULL)
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , groupId (-1)
   , moduleHandlerConfiguration (NULL)
  {};

  struct IRC_Client_Configuration*              configuration;
  struct IRC_Client_CursesState*                cursesState;
  enum Common_EventDispatchType                 dispatch;
  int                                           groupId;
  struct IRC_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct IRC_Client_UserData
{
  IRC_Client_UserData ()
   //: connectionConfiguration (NULL)
   //, moduleConfiguration (NULL)
   //, moduleHandlerConfiguration (NULL)
  {};

  //struct IRC_Client_ConnectionConfiguration*    connectionConfiguration;

  //// *TODO*: remove these ASAP
  //struct Stream_ModuleConfiguration*            moduleConfiguration;
  //struct IRC_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

typedef Net_IConnector_T<ACE_INET_Addr,
                         IRC_Client_ConnectionConfiguration_t> IRC_Client_IConnector_t;
struct IRC_Client_CursesState;
struct IRC_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  IRC_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , connector (NULL)
   , cursesState (NULL)
   , peerAddress ()
  {};

  IRC_Client_IConnector_t*       connector;
  struct IRC_Client_CursesState* cursesState;
  ACE_INET_Addr                  peerAddress;
};

#endif
