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

#include <bitset>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ace/Date_Time.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnection.h"

#include "FILE_Stream.h"
#include "IOStream_alt_T.h"
//#include "IRC_client_configuration.h"
//#include "IRC_client_defines.h"

// forward declarations
//class IRC_Client_IIRCControl;
//class IRC_Client_Stream;
struct IRC_Client_Configuration;
struct IRC_Client_CursesState;
struct IRC_Client_StreamModuleConfiguration;

// see (RFC1459 section 4.2.3.1)
//            b - set a ban mask to keep users out
//            i - invite-only channel flag
//            k - set a channel key (password)
//            l - set the user limit to channel
//            m - moderated channel
//            n - no messages to channel from clients on the outside
//            o - give/take channel operator privileges
//            p - private channel flag
//            s - secret channel flag
//            t - topic settable by channel operator only flag
//            v - give/take the ability to speak on a moderated channel
// see (RFC2812 section 4.2 and following)
//            a - anonymous channel ('&!'-channels only)
//            i - invite-only channel flag
//            k - set a channel key (password)
//            l - set the user limit to channel
//            m - moderated channel
//            n - no messages to channel from clients on the outside
//            p - private channel flag
//            q - quiet channel (server use only)
//            r - server reop flag ('!'-channels only)
//            s - secret channel flag
//            t - topic settable by channel operator only flag
// *NOTE*: see also: https://www.alien.net.au/irc/chanmodes.html
enum IRC_Client_ChannelMode
{
  CHANNELMODE_ANONYMOUS = 0,    // 'a'
  CHANNELMODE_BAN,              // 'b'
  CHANNELMODE_INVITEONLY,       // 'i'
  CHANNELMODE_PASSWORD,         // 'k'
  CHANNELMODE_USERLIMIT,        // 'l'
  CHANNELMODE_MODERATED,        // 'm'
  CHANNELMODE_BLOCKFOREIGNMSGS, // 'n'
  CHANNELMODE_OPERATOR,         // 'o'
  CHANNELMODE_PRIVATE,          // 'p'
  CHANNELMODE_QUIET,            // 'q'
  CHANNELMODE_REOP,             // 'r'
  CHANNELMODE_SECRET,           // 's'
  CHANNELMODE_RESTRICTEDTOPIC,  // 't'
  CHANNELMODE_VOICE,            // 'v'
  ///////////////////////////////////////
  CHANNELMODE_MAX,
  CHANNELMODE_INVALID
};
// *NOTE*: --> CHANNELMODE_MAX
typedef std::bitset<14> IRC_Client_ChannelModes_t;

// see (RFC1459 section 4.2.3.2)
//            i - marks a users as invisible
//            s - marks a user for receipt of server notices
//            w - user receives wallops
//            o - operator flag
// see (RFC2812 section 3.1.5)
//            a - user is flagged as away (server-to-server only)
//            i - marks a users as invisible
//            w - user receives wallops
//            r - restricted user connection
//            o - operator flag
//            O - local operator flag
//            s - marks a user for receipt of server notices
// *NOTE*: see also: https://www.alien.net.au/irc/usermodes.html
enum IRC_Client_UserMode
{
  USERMODE_AWAY = 0,           // 'a'
  USERMODE_INVISIBLE,          // 'i'
  USERMODE_OPERATOR,           // 'o'
  USERMODE_LOCALOPERATOR,      // 'O'
  USERMODE_RESTRICTEDCONN,     // 'r'
  USERMODE_RECVNOTICES,        // 's'
  USERMODE_RECVWALLOPS,        // 'w'
  ///////////////////////////////////////
  USERMODE_MAX,
  USERMODE_INVALID
};
// *NOTE*: --> USERMODE_MAX
typedef std::bitset<7> IRC_Client_UserModes_t;

// phonebook
typedef std::set<std::string> IRC_Client_Networks_t;
typedef IRC_Client_Networks_t::const_iterator IRC_Client_NetworksIterator_t;
typedef std::pair<unsigned short, unsigned short> IRC_Client_PortRange_t;
typedef std::vector<IRC_Client_PortRange_t> IRC_Client_PortRanges_t;
typedef IRC_Client_PortRanges_t::const_iterator IRC_Client_PortRangesIterator_t;
struct IRC_Client_ConnectionEntry
{
  inline IRC_Client_ConnectionEntry ()
   : hostName ()
   , listeningPorts ()
   , network ()
  {};

  std::string             hostName;
  IRC_Client_PortRanges_t listeningPorts;
  std::string             network;
};
typedef std::multimap<std::string,
                      IRC_Client_ConnectionEntry> IRC_Client_Servers_t;
typedef IRC_Client_Servers_t::const_iterator IRC_Client_ServersIterator_t;
struct IRC_Client_PhoneBook
{
  inline IRC_Client_PhoneBook ()
   : networks ()
   , servers ()
   , timestamp ()
  {};

  // *NOTE*: this member is redundant (but still useful ?)...
  IRC_Client_Networks_t networks;
  IRC_Client_Servers_t  servers;
  ACE_Date_Time         timestamp;
};

typedef std::deque<std::string> IRC_Client_MessageQueue_t;
typedef IRC_Client_MessageQueue_t::iterator IRC_Client_MessageQueueIterator_t;
typedef IRC_Client_MessageQueue_t::reverse_iterator IRC_Client_MessageQueueReverseIterator_t;

//  ACE_IOStream<ACE_FILE_Stream> output_;
typedef ACE_IOStream_alt_T<ACE_FILE_Stream> IRC_Client_IOStream_t;

struct IRC_Client_InputThreadData
{
  inline IRC_Client_InputThreadData ()
   : configuration (NULL)
   , cursesState (NULL)
   , groupID (-1)
   , moduleConfiguration (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  IRC_Client_Configuration*             configuration;
  IRC_Client_CursesState*               cursesState;
  int                                   groupID;
  IRC_Client_StreamModuleConfiguration* moduleConfiguration;
  bool                                  useReactor;
};

enum IRC_Client_CharacterEncoding
{
  CHARACTERENCODING_INVALID = -1,
  CHARACTERENCODING_ASCII = 0,
  CHARACTERENCODING_LOCALE, // *NOTE*: may lead to problems if clients do not
                            //         share the same configuration
  CHARACTERENCODING_UTF8,
  ///////////////////////////////////////
  CHARACTERENCODING_MAX
};

#endif
