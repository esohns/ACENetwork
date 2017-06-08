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

#ifndef IRC_COMMON_H
#define IRC_COMMON_H

#include <bitset>
#include <string>

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "net_common.h"

#include "irc_icontrol.h"

// forward declarations
struct IRC_Configuration;
//struct IRC_UserData;

//typedef IRC_IControl_T<IRC_IStreamNotify_t> IRC_IControl_t;

enum IRC_CharacterEncoding
{
  IRC_CHARACTERENCODING_INVALID = -1,
  IRC_CHARACTERENCODING_ASCII   = 0,
  IRC_CHARACTERENCODING_LOCALE, // *NOTE*: may lead to problems if peers do not
                                //         share the same configuration
  IRC_CHARACTERENCODING_UTF8,
  ////////////////////////////////////////
  IRC_CHARACTERENCODING_MAX
};

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
enum IRC_ChannelMode
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
  ////////////////////////////////////////
  CHANNELMODE_MAX,
  CHANNELMODE_INVALID
};
// *NOTE*: --> CHANNELMODE_MAX
typedef std::bitset<14> IRC_ChannelModes_t;

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
enum IRC_UserMode
{
  USERMODE_AWAY = 0,       // 'a'
  USERMODE_INVISIBLE,      // 'i'
  USERMODE_OPERATOR,       // 'o'
  USERMODE_LOCALOPERATOR,  // 'O'
  USERMODE_RESTRICTEDCONN, // 'r'
  USERMODE_RECVNOTICES,    // 's'
  USERMODE_RECVWALLOPS,    // 'w'
  ////////////////////////////////////////
  USERMODE_MAX,
  USERMODE_INVALID
};
// *NOTE*: --> USERMODE_MAX
typedef std::bitset<7> IRC_UserModes_t;

struct IRC_LoginOptions
{
  inline IRC_LoginOptions ()
   : password ()
   , nickname ()
   , user ()
   , channel ()
  {};

  std::string password;
  std::string nickname;
  struct User
  {
    inline User ()
     : userName ()
     , hostName ()
     , serverName ()
     , realName ()
    {};

    std::string userName;
    struct Hostname
    {
      inline Hostname ()
       : discriminator (INVALID)
       , string (NULL)
      {};

      enum
      {
        INVALID = -1,
        MODE    = 0,
        STRING,
      } discriminator;
      union
      {
        // *NOTE*: "modern" connects (see RFC2812 Section 3.1.3)
        unsigned char mode;
        // *NOTE*: "traditional" connects (see RFC1459 Section 4.1.3)
        std::string*  string;
      };
    } hostName;
    std::string serverName;
    std::string realName;
  } user;
  std::string channel;
};

typedef Stream_Statistic IRC_RuntimeStatistic_t;

#endif
