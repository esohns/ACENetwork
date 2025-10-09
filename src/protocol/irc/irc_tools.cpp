/***************************************************************************
 *   Copyright (C) To009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version To of the License, or     *
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
 *   59 Temple Place - Suite 330, Boston, MA  0To111-1307, USA.             *
 ***************************************************************************/
#include "stdafx.h"

#include "irc_tools.h"

#include <regex>
#include <sstream>

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Log_Msg.h"

#include "common.h"

#include "stream_common.h"
#include "stream_iallocator.h"

#include "net_defines.h"
#include "net_macros.h"

#include "irc_common.h"
#include "irc_configuration.h"
#include "irc_defines.h"

std::string
IRC_Tools::dump (const IRC_Record& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::dump"));

  std::ostringstream converter;
  // see RFC1459
  if (!message_in.prefix_.origin.empty ())
  {
    converter << ACE_TEXT_ALWAYS_CHAR ("PREFIX [origin: \"")
              << message_in.prefix_.origin
              << ACE_TEXT_ALWAYS_CHAR ("\"]");
    if (!message_in.prefix_.user.empty ())
      converter << ACE_TEXT_ALWAYS_CHAR (", [user: \"")
                << message_in.prefix_.user
                << ACE_TEXT_ALWAYS_CHAR ("\"]");
    if (!message_in.prefix_.host.empty ())
      converter << ACE_TEXT_ALWAYS_CHAR (", [host: \"")
                << message_in.prefix_.host
                << ACE_TEXT_ALWAYS_CHAR ("\"]");
    converter << ACE_TEXT_ALWAYS_CHAR (" \\PREFIX") << std::endl;
  } // end IF
  converter << ACE_TEXT_ALWAYS_CHAR ("COMMAND [");
  switch (message_in.command_.discriminator)
  {
    case IRC_Record::Command::STRING:
    {
      converter << message_in.command_.string;
      break;
    }
    case IRC_Record::Command::NUMERIC:
    {
      converter << ACE_TEXT_ALWAYS_CHAR ("\"")
                << IRC_Tools::CommandToString (message_in.command_.numeric)
                << ACE_TEXT_ALWAYS_CHAR ("\" [")
                << message_in.command_.numeric
                << ACE_TEXT_ALWAYS_CHAR ("]");
      break;
    }
    // *NOTE*: command hasn't been set (yet)
    case IRC_Record::Command::INVALID:
    {
      converter << ACE_TEXT_ALWAYS_CHAR ("\"INVALID\"");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command discriminator (was: %d), aborting\n"),
                  message_in.command_.discriminator));
      return std::string ();
    }
  } // end SWITCH
  converter << ACE_TEXT_ALWAYS_CHAR ("] \\COMMAND") << std::endl;
  if (!message_in.parameters_.empty ())
  {
    converter << ACE_TEXT_ALWAYS_CHAR ("PARAMS") << std::endl;
    int i = 1;
    for (std::list<std::string>::const_iterator iterator = message_in.parameters_.begin ();
          iterator != message_in.parameters_.end ();
          iterator++, i++)
    {
      converter << ACE_TEXT_ALWAYS_CHAR ("#")
                << i
                << ACE_TEXT_ALWAYS_CHAR (" : \"")
                << *iterator
                << ACE_TEXT_ALWAYS_CHAR ("\"")
                << std::endl;
    } // end FOR
    converter << ACE_TEXT_ALWAYS_CHAR ("\\PARAMS") << std::endl;
  } // end IF

  return converter.str ();
}

std::string
IRC_Tools::dump (const IRC_UserModes_t& userModes_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::dump"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("+");

  for (int i = 0;
       i < USERMODE_MAX;
       i++)
    if (userModes_in.test (i))
      result += IRC_Tools::UserModeToChar (static_cast<IRC_UserMode> (i));

  return result;
}

std::string
IRC_Tools::dump (const IRC_ChannelModes_t& channelModes_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::dump"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("+");

  for (int i = 0;
       i < CHANNELMODE_MAX;
       i++)
    if (channelModes_in.test (i))
      result += IRC_Tools::ChannelModeToChar (static_cast<IRC_ChannelMode> (i));

  return result;
}

std::string
IRC_Tools::CommandToString (IRC_CommandType_t command_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CommandToString"));

  std::string result; 

  switch (command_in)
  {
    case IRC_Record::PASS:
      result = ACE_TEXT_ALWAYS_CHAR ("PASS"); break;
    case IRC_Record::NICK:
      result = ACE_TEXT_ALWAYS_CHAR ("NICK"); break;
    case IRC_Record::USER:
      result = ACE_TEXT_ALWAYS_CHAR ("USER"); break;
    case IRC_Record::SERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVER"); break;
    case IRC_Record::OPER:
      result = ACE_TEXT_ALWAYS_CHAR ("OPER"); break;
    case IRC_Record::QUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("QUIT"); break;
    case IRC_Record::SQUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("SQUIT"); break;
    case IRC_Record::JOIN:
      result = ACE_TEXT_ALWAYS_CHAR ("JOIN"); break;
    case IRC_Record::PART:
      result = ACE_TEXT_ALWAYS_CHAR ("PART"); break;
    case IRC_Record::MODE:
      result = ACE_TEXT_ALWAYS_CHAR ("MODE"); break;
    case IRC_Record::TOPIC:
      result = ACE_TEXT_ALWAYS_CHAR ("TOPIC"); break;
    case IRC_Record::NAMES:
      result = ACE_TEXT_ALWAYS_CHAR ("NAMES"); break;
    case IRC_Record::LIST:
      result = ACE_TEXT_ALWAYS_CHAR ("LIST"); break;
    case IRC_Record::INVITE:
      result = ACE_TEXT_ALWAYS_CHAR ("INVITE"); break;
    case IRC_Record::KICK:
      result = ACE_TEXT_ALWAYS_CHAR ("KICK"); break;
    case IRC_Record::SVERSION:
      result = ACE_TEXT_ALWAYS_CHAR ("VERSION"); break;
    case IRC_Record::STATS:
      result = ACE_TEXT_ALWAYS_CHAR ("STATS"); break;
    case IRC_Record::LINKS:
      result = ACE_TEXT_ALWAYS_CHAR ("LINKS"); break;
    case IRC_Record::TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("TIME"); break;
    case IRC_Record::CONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("CONNECT"); break;
    case IRC_Record::TRACE:
      result = ACE_TEXT_ALWAYS_CHAR ("TRACE"); break;
    case IRC_Record::ADMIN:
      result = ACE_TEXT_ALWAYS_CHAR ("ADMIN"); break;
    case IRC_Record::INFO:
      result = ACE_TEXT_ALWAYS_CHAR ("INFO"); break;
    case IRC_Record::PRIVMSG:
      result = ACE_TEXT_ALWAYS_CHAR ("PRIVMSG"); break;
    case IRC_Record::NOTICE:
      result = ACE_TEXT_ALWAYS_CHAR ("NOTICE"); break;
    case IRC_Record::WHO:
      result = ACE_TEXT_ALWAYS_CHAR ("WHO"); break;
    case IRC_Record::WHOIS:
      result = ACE_TEXT_ALWAYS_CHAR ("WHOIS"); break;
    case IRC_Record::WHOWAS:
      result = ACE_TEXT_ALWAYS_CHAR ("WHOWAS"); break;
    case IRC_Record::KILL:
      result = ACE_TEXT_ALWAYS_CHAR ("KILL"); break;
    case IRC_Record::PING:
      result = ACE_TEXT_ALWAYS_CHAR ("PING"); break;
    case IRC_Record::PONG:
      result = ACE_TEXT_ALWAYS_CHAR ("PONG"); break;
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler")
    case IRC_Record::__QUIRK__ERROR:
#else
    case IRC_Record::ERROR:
#endif // ACE_WIN32 || ACE_WIN64
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR"); break;
    case IRC_Record::AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("AWAY"); break;
    case IRC_Record::REHASH:
      result = ACE_TEXT_ALWAYS_CHAR ("REHASH"); break;
    case IRC_Record::RESTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RESTART"); break;
    case IRC_Record::SUMMON:
      result = ACE_TEXT_ALWAYS_CHAR ("SUMMON"); break;
    case IRC_Record::USERS:
      result = ACE_TEXT_ALWAYS_CHAR ("USERS"); break;
    case IRC_Record::WALLOPS:
      result = ACE_TEXT_ALWAYS_CHAR ("WALLOPS"); break;
    case IRC_Record::USERHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("USERHOST"); break;
    case IRC_Record::ISON:
      result = ACE_TEXT_ALWAYS_CHAR ("ISON"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown command (was: %d), aborting\n"),
                  command_in));
      break;
    }
  } // end SWITCH

  return result;
}

IRC_CommandType_t
IRC_Tools::CommandToType (const std::string& commandString_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CommandToType"));

  if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("PASS"))
    return IRC_Record::PASS;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("NICK"))
    return IRC_Record::NICK;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("USER"))
    return IRC_Record::USER;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("SERVER"))
    return IRC_Record::SERVER;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("OPER"))
    return IRC_Record::OPER;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("QUIT"))
    return IRC_Record::QUIT;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("SQUIT"))
    return IRC_Record::SQUIT;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("JOIN"))
    return IRC_Record::JOIN;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("PART"))
    return IRC_Record::PART;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("MODE"))
    return IRC_Record::MODE;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("TOPIC"))
    return IRC_Record::TOPIC;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("NAMES"))
    return IRC_Record::NAMES;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("LIST"))
    return IRC_Record::LIST;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("INVITE"))
    return IRC_Record::INVITE;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("KICK"))
    return IRC_Record::KICK;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("VERSION"))
    return IRC_Record::SVERSION;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("STATS"))
    return IRC_Record::STATS;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("LINKS"))
    return IRC_Record::LINKS;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("TIME"))
    return IRC_Record::TIME;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("CONNECT"))
    return IRC_Record::CONNECT;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("TRACE"))
    return IRC_Record::TRACE;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("ADMIN"))
    return IRC_Record::ADMIN;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("INFO"))
    return IRC_Record::INFO;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("PRIVMSG"))
    return IRC_Record::PRIVMSG;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("NOTICE"))
    return IRC_Record::NOTICE;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("WHO"))
    return IRC_Record::WHO;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("WHOIS"))
    return IRC_Record::WHOIS;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("WHOWAS"))
    return IRC_Record::WHOWAS;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("KILL"))
    return IRC_Record::KILL;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("PING"))
    return IRC_Record::PING;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("PONG"))
    return IRC_Record::PONG;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("ERROR"))
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler")
    return IRC_Record::__QUIRK__ERROR;
#else
    return IRC_Record::ERROR;
#endif
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("AWAY"))
    return IRC_Record::AWAY;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("REHASH"))
    return IRC_Record::REHASH;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("RESTART"))
    return IRC_Record::RESTART;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("SUMMON"))
    return IRC_Record::SUMMON;
  else if ((commandString_in) == ACE_TEXT_ALWAYS_CHAR ("USERS"))
    return IRC_Record::USERS;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("WALLOPS"))
    return IRC_Record::WALLOPS;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("USERHOST"))
    return IRC_Record::USERHOST;
  else if (commandString_in == ACE_TEXT_ALWAYS_CHAR ("ISON"))
    return IRC_Record::ISON;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown command (was: \"%s\"), aborting\n"),
                ACE_TEXT (commandString_in.c_str ())));
    return IRC_Record::IRC_COMMANDTYPE_INVALID;
  } // end ELSE
}

unsigned int
IRC_Tools::merge (const std::string& modes_in,
                  IRC_UserModes_t& modes_inout)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::merge"));

  unsigned int result = 0;
  IRC_UserMode mode = USERMODE_INVALID;

  // *NOTE* format is {[+|-]|i|w|s|o}
  bool assign = (modes_in[0] == '+');
  bool toggled;
  std::string::const_iterator iterator = modes_in.begin ();
  for (++iterator;
       iterator != modes_in.end ();
       ++iterator)
  {
    mode = IRC_Tools::CharToUserMode (*iterator);
    if (mode >= USERMODE_MAX)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid user mode (was: \"%c\"), continuing\n"),
                  *iterator));
      continue; // unknown user mode...
    } // end IF

    toggled = (( modes_inout.test (mode) && !assign) ||
               (!modes_inout.test (mode) &&  assign));
    modes_inout.set (mode, assign);
    if (toggled)
      ++result;
  } // end FOR

  return result;
}

unsigned int
IRC_Tools::merge (const std::string& modes_in,
                  IRC_ChannelModes_t& modes_inout)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::merge"));

  unsigned int result = 0;
  IRC_ChannelMode mode = CHANNELMODE_INVALID;

  // *NOTE* format is {[+|-]|o|p|s|i|t|n|m|l|b|v|k}
  bool assign = (modes_in[0] == '+');
  bool toggled;
  std::string::const_iterator iterator = modes_in.begin ();
  for (++iterator;
       iterator != modes_in.end ();
       ++iterator)
  {
    mode = IRC_Tools::CharToChannelMode (*iterator);
    if (mode >= CHANNELMODE_MAX)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid channel mode (was: \"%c\"), continuing\n"),
                  *iterator));
      continue; // unknown channel mode...
    } // end IF

    toggled = (( modes_inout.test (mode) && !assign) ||
               (!modes_inout.test (mode) &&  assign));
    modes_inout.set (mode, assign);
    if (toggled)
      ++result;
  } // end FOR

  return result;
}

char
IRC_Tools::ChannelModeToChar (enum IRC_ChannelMode mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::ChannelModeToChar"));

  switch (mode_in)
  {
    case CHANNELMODE_PASSWORD:
      return 'k';
    case CHANNELMODE_VOICE:
      return 'v';
    case CHANNELMODE_BAN:
      return 'b';
    case CHANNELMODE_USERLIMIT:
      return 'l';
    case CHANNELMODE_MODERATED:
      return 'm';
    case CHANNELMODE_BLOCKFOREIGNMSGS:
      return 'n';
    case CHANNELMODE_RESTRICTEDTOPIC:
      return 't';
    case CHANNELMODE_INVITEONLY:
      return 'i';
    case CHANNELMODE_SECRET:
      return 's';
    case CHANNELMODE_PRIVATE:
      return 'p';
    case CHANNELMODE_OPERATOR:
      return 'o';
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid channel mode (was: \"%s\"), aborting\n"),
                  ACE_TEXT (IRC_Tools::ChannelModeToString (mode_in).c_str ())));
      break;
    }
  } // end SWITCH

  return ' ';
}

char
IRC_Tools::UserModeToChar (enum IRC_UserMode mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::UserModeToChar"));

  switch (mode_in)
  {
    case USERMODE_AWAY:
      return 'a';
    case USERMODE_HIDE_OPER:
      return 'H';
    case USERMODE_INVISIBLE:
      return 'i';
    case USERMODE_OPERATOR:
      return 'o';
    case USERMODE_LOCALOPERATOR:
      return 'O';
    case USERMODE_RESTRICTEDCONN:
      return 'r';
    case USERMODE_RECVNOTICES:
      return 's';
    case USERMODE_RECVWALLOPS:
      return 'w';
    case USERMODE_HOSTHIDING:
      return 'x';
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid user mode (was: \"%s\"), aborting\n"),
                  ACE_TEXT (IRC_Tools::UserModeToString (mode_in).c_str ())));
      break;
    }
  } // end SWITCH

  return ' ';
}

bool
IRC_Tools::isValidChannelName (const std::string& string_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::isValidChannelName"));

  // *NOTE*: RFC1459:
  //   <channel>    ::= ('#' | '&') <chstring>
  //   <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and
  //                     comma (',')>
  return ((string_in.find ('#', 0) == 0) ||
          (string_in.find ('&', 0) == 0));
}

bool
IRC_Tools::parse (const std::string& inputString_in,
                  IRC_CommandType_t& command_out,
                  string_list_t& parameters_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::parse"));

  // initialize return value(s)
  command_out = IRC_Record::CommandType::IRC_COMMANDTYPE_INVALID;
  parameters_out.clear ();

  std::regex regex (ACE_TEXT_ALWAYS_CHAR ("^(?:/)([[:alpha:]]+)(?:[[:space:]]*)(?:(.+)(?:[[:space:]]+))*(.+)?$"));
  std::smatch match_results;
  if (!std::regex_match (inputString_in,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
    return false;
  ACE_ASSERT (match_results[1].matched);
  command_out = IRC_Tools::CharToCommand (match_results[1].str ()[0]);
  for (unsigned int i = 2;
       i < match_results.size ();
       ++i)
    if (match_results[i].matched)
      parameters_out.push_back (match_results[i].str ());

  return true;
}

std::string
IRC_Tools::CommandToString (IRC_NumericCommand_t numeric_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CommandToString"));

  // initialize result
  std::string result = ACE_TEXT ("INVALID/UNKNOWN");

  switch (numeric_in)
  {
    case IRC_Codes::RPL_WELCOME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WELCOME"); break;
    case IRC_Codes::RPL_YOURHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOURHOST"); break;
    case IRC_Codes::RPL_CREATED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CREATED"); break;
    case IRC_Codes::RPL_MYINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MYINFO"); break;
    case IRC_Codes::RPL_PROTOCTL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_PROTOCTL"); break;
    case IRC_Codes::RPL_MAPMORE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MAPMORE"); break;
    case IRC_Codes::RPL_MAPEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MAPEND"); break;
    case IRC_Codes::RPL_SNOMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SNOMASK"); break;
    case IRC_Codes::RPL_STATMEMTOT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATMEMTOT"); break;
    case IRC_Codes::RPL_STATMEM:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATMEM"); break;

    case IRC_Codes::RPL_YOURCOOKIE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOURCOOKIE"); break;

    case IRC_Codes::RPL_YOURID:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOURID"); break;
    case IRC_Codes::RPL_SAVENICK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SAVENICK"); break;

    case IRC_Codes::RPL_ATTEMPTINGJUNC:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ATTEMPTINGJUNC"); break;
    case IRC_Codes::RPL_ATTEMPTINGREROUTE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ATTEMPTINGREROUTE"); break;

    case IRC_Codes::RPL_TRACELINK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACELINK"); break;
    case IRC_Codes::RPL_TRACECONNECTING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACECONNECTING"); break;
    case IRC_Codes::RPL_TRACEHANDSHAKE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEHANDSHAKE"); break;
    case IRC_Codes::RPL_TRACEUNKNOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEUNKNOWN"); break;
    case IRC_Codes::RPL_TRACEOPERATOR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEOPERATOR"); break;
    case IRC_Codes::RPL_TRACEUSER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEUSER"); break;
    case IRC_Codes::RPL_TRACESERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACESERVER"); break;
    case IRC_Codes::RPL_TRACESERVICE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACESERVICE"); break;
    case IRC_Codes::RPL_TRACENEWTYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACENEWTYPE"); break;
    case IRC_Codes::RPL_TRACECLASS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACECLASS"); break;
    case IRC_Codes::RPL_TRACERECONNECT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACERECONNECT"); break;

    case IRC_Codes::RPL_STATSLINKINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSLINKINFO"); break;
    case IRC_Codes::RPL_STATSCOMMANDS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSCOMMANDS"); break;
    case IRC_Codes::RPL_STATSCLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSCLINE"); break;
    case IRC_Codes::RPL_STATSNLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSNLINE"); break;
    case IRC_Codes::RPL_STATSILINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSILINE"); break;
    case IRC_Codes::RPL_STATSKLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSKLINE"); break;
    case IRC_Codes::RPL_STATSQLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSQLINE"); break;
    case IRC_Codes::RPL_STATSYLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSYLINE"); break;
    case IRC_Codes::RPL_ENDOFSTATS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFSTATS"); break;

    case IRC_Codes::RPL_STATSPLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSPLINE"); break;
    case IRC_Codes::RPL_UMODEIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_UMODEIS"); break;
    case IRC_Codes::RPL_SQLINE_NICK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SQLINE_NICK"); break;
    case IRC_Codes::RPL_STATSELINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSELINE"); break;
    case IRC_Codes::RPL_STATSFLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSFLINE"); break;
    case IRC_Codes::RPL_STATSGLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSGLINE"); break;
    case IRC_Codes::RPL_STATSCOUNT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSCOUNT"); break;

    case IRC_Codes::RPL_SERVICEINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SERVICEINFO"); break;
    case IRC_Codes::RPL_ENDOFSERVICES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFSERVICES"); break;
    case IRC_Codes::RPL_SERVICE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SERVICE"); break;
    case IRC_Codes::RPL_SERVLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SERVLIST"); break;
    case IRC_Codes::RPL_SERVLISTEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SERVLISTEND"); break;
    case IRC_Codes::RPL_STATSVERBOSE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSVERBOSE"); break;
    case IRC_Codes::RPL_STATSENGINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSENGINE"); break;
    case IRC_Codes::RPL_STATSIAUTH:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSIAUTH"); break;

    case IRC_Codes::RPL_STATSVLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSVLINE"); break;
    case IRC_Codes::RPL_STATSLLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSLLINE"); break;
    case IRC_Codes::RPL_STATSUPTIME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSUPTIME"); break;
    case IRC_Codes::RPL_STATSOLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSOLINE"); break;
    case IRC_Codes::RPL_STATSHLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSHLINE"); break;
    case IRC_Codes::RPL_STATSSLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSSLINE"); break;
    case IRC_Codes::RPL_STATSPING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSPING"); break;
    case IRC_Codes::RPL_STATSBLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSBLINE"); break;
    case IRC_Codes::RPL_STATSDEFINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSDEFINE"); break;
    case IRC_Codes::RPL_STATSDEBUG:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSDEBUG"); break;
    case IRC_Codes::RPL_STATSDLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSDLINE"); break;

    case IRC_Codes::RPL_LUSERCLIENT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSERCLIENT"); break;
    case IRC_Codes::RPL_LUSEROP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSEROP"); break;
    case IRC_Codes::RPL_LUSERUNKNOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSERUNKNOWN"); break;
    case IRC_Codes::RPL_LUSERCHANNELS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSERCHANNELS"); break;
    case IRC_Codes::RPL_LUSERME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSERME"); break;
    case IRC_Codes::RPL_ADMINME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ADMINME"); break;
    case IRC_Codes::RPL_ADMINLOC1:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ADMINLOC1"); break;
    case IRC_Codes::RPL_ADMINLOC2:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ADMINLOCTo"); break;
    case IRC_Codes::RPL_ADMINEMAIL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ADMINEMAIL"); break;

    case IRC_Codes::RPL_TRACELOG:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACELOG"); break;
    case IRC_Codes::RPL_TRACEEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEEND"); break;
    case IRC_Codes::RPL_TRYAGAIN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRYAGAIN"); break;
    case IRC_Codes::RPL_LOCALUSERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LOCALUSERS"); break;
    case IRC_Codes::RPL_GLOBALUSERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_GLOBALUSERS"); break;
    case IRC_Codes::RPL_START_NETSTAT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_START_NETSTAT"); break;
    case IRC_Codes::RPL_NETSTAT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NETSTAT"); break;
    case IRC_Codes::RPL_END_NETSTAT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_END_NETSTAT"); break;
    case IRC_Codes::RPL_PRIVS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_PRIVS"); break;

    case IRC_Codes::RPL_SILELIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SILELIST"); break;
    case IRC_Codes::RPL_ENDOFSILELIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFSILELIST"); break;
    case IRC_Codes::RPL_NOTIFY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOTIFY"); break;
    case IRC_Codes::RPL_STATSDELTA:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_STATSDELTA"); break;
    case IRC_Codes::RPL_VCHANEXIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_VCHANEXIST"); break;
    case IRC_Codes::RPL_VCHANLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_VCHANLIST"); break;
    case IRC_Codes::RPL_VCHANHELP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_VCHANHELP"); break;

    case IRC_Codes::RPL_GLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_GLIST"); break;
    case IRC_Codes::RPL_ENDOFGLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFGLIST"); break;
    case IRC_Codes::RPL_ENDOFACCEPT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFACCEPT"); break;
    case IRC_Codes::RPL_ALIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ALIST"); break;
    case IRC_Codes::RPL_ENDOFALIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFALIST"); break;
    case IRC_Codes::RPL_GLIST_HASH:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_GLIST_HASH"); break;
    case IRC_Codes::RPL_CHANINFO_USERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANINFO_USERS"); break;
    case IRC_Codes::RPL_CHANINFO_CHOPS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANINFO_CHOPS"); break;
    case IRC_Codes::RPL_CHANINFO_VOICES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANINFO_VOICES"); break;
    case IRC_Codes::RPL_CHANINFO_AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANINFO_AWAY"); break;

    case IRC_Codes::RPL_HELPHDR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPHDR"); break;
    case IRC_Codes::RPL_HELPOP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPOP"); break;
    case IRC_Codes::RPL_HELPTLR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPTLR"); break;
    case IRC_Codes::RPL_HELPHLP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPHLP"); break;
    case IRC_Codes::RPL_HELPFWD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPFWD"); break;
    case IRC_Codes::RPL_HELPIGN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPIGN"); break;
    case IRC_Codes::RPL_CHANINFO_KICKS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANINFO_KICKS"); break;
    case IRC_Codes::RPL_END_CHANINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_END_CHANINFO"); break;

    case IRC_Codes::RPL_NONE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NONE"); break;
    case IRC_Codes::RPL_AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_AWAY"); break;
    case IRC_Codes::RPL_USERHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_USERHOST"); break;
    case IRC_Codes::RPL_ISON:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ISON"); break;
    case IRC_Codes::RPL_TEXT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TEXT"); break;
    case IRC_Codes::RPL_UNAWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_UNAWAY"); break;
    case IRC_Codes::RPL_NOWAWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOWAWAY"); break;
    case IRC_Codes::RPL_USERIP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_USERIP"); break;
    case IRC_Codes::RPL_WHOISADMIN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISADMIN"); break;
    case IRC_Codes::RPL_WHOISSADMIN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISSADMIN"); break;

    case IRC_Codes::RPL_WHOISHELPOP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISHELPOP"); break;
    case IRC_Codes::RPL_WHOISUSER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISUSER"); break;
    case IRC_Codes::RPL_WHOISSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISSERVER"); break;
    case IRC_Codes::RPL_WHOISOPERATOR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISOPERATOR"); break;
    case IRC_Codes::RPL_WHOWASUSER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOWASUSER"); break;
    case IRC_Codes::RPL_ENDOFWHO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFWHO"); break;
    case IRC_Codes::RPL_WHOISCHANOP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISCHANOP"); break;
    case IRC_Codes::RPL_WHOISIDLE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISIDLE"); break;
    case IRC_Codes::RPL_ENDOFWHOIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFWHOIS"); break;
    case IRC_Codes::RPL_WHOISCHANNELS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISCHANNELS"); break;

    case IRC_Codes::RPL_WHOISVIRT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISVIRT"); break;
    case IRC_Codes::RPL_LISTSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LISTSTART"); break;
    case IRC_Codes::RPL_LIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LIST"); break;
    case IRC_Codes::RPL_LISTEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LISTEND"); break;
    case IRC_Codes::RPL_CHANNELMODEIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANNELMODEIS"); break;
    case IRC_Codes::RPL_UNIQOPIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_UNIQOPIS"); break;
    case IRC_Codes::RPL_NOCHANPASS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOCHANPASS"); break;
    case IRC_Codes::RPL_CHPASSUNKNOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHPASSUNKNOWN"); break;
    case IRC_Codes::RPL_CHANNEL_URL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANNEL_URL"); break;
    case IRC_Codes::RPL_CREATIONTIME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CREATIONTIME"); break;

    case IRC_Codes::RPL_WHOWAS_TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOWAS_TIME"); break;
    case IRC_Codes::RPL_NOTOPIC:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOTOPIC"); break;
    case IRC_Codes::RPL_TOPIC:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TOPIC"); break;
    case IRC_Codes::RPL_TOPICWHOTIME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TOPICWHOTIME"); break;
    case IRC_Codes::RPL_LISTUSAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LISTUSAGE"); break;
    case IRC_Codes::RPL_WHOISBOT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISBOT"); break;
    case IRC_Codes::RPL_CHANPASSOK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANPASSOK"); break;
    case IRC_Codes::RPL_BADCHANPASS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_BADCHANPASS"); break;

    case IRC_Codes::RPL_INVITING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_INVITING"); break;
    case IRC_Codes::RPL_SUMMONING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SUMMONING"); break;
    case IRC_Codes::RPL_INVITED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_INVITED"); break;
    case IRC_Codes::RPL_INVITELIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_INVITELIST"); break;
    case IRC_Codes::RPL_ENDOFINVITELIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFINVITELIST"); break;
    case IRC_Codes::RPL_EXCEPTLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_EXCEPTLIST"); break;
    case IRC_Codes::RPL_ENDOFEXCEPTLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFEXCEPTLIST"); break;

    case IRC_Codes::RPL_VERSION:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_VERSION"); break;
    case IRC_Codes::RPL_WHOREPLY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOREPLY"); break;
    case IRC_Codes::RPL_NAMREPLY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NAMREPLY"); break;
    case IRC_Codes::RPL_WHOSPCRPL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOSPCRPL"); break;
    case IRC_Codes::RPL_NAMREPLY_:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NAMREPLY_"); break;

    case IRC_Codes::RPL_KILLDONE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KILLDONE"); break;
    case IRC_Codes::RPL_CLOSING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CLOSING"); break;
    case IRC_Codes::RPL_CLOSEEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CLOSEEND"); break;
    case IRC_Codes::RPL_LINKS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LINKS"); break;
    case IRC_Codes::RPL_ENDOFLINKS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFLINKS"); break;
    case IRC_Codes::RPL_ENDOFNAMES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFNAMES"); break;
    case IRC_Codes::RPL_BANLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_BANLIST"); break;
    case IRC_Codes::RPL_ENDOFBANLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFBANLIST"); break;
    case IRC_Codes::RPL_ENDOFWHOWAS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFWHOWAS"); break;

    case IRC_Codes::RPL_INFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_INFO"); break;
    case IRC_Codes::RPL_MOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MOTD"); break;
    case IRC_Codes::RPL_INFOSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_INFOSTART"); break;
    case IRC_Codes::RPL_ENDOFINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFINFO"); break;
    case IRC_Codes::RPL_MOTDSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MOTDSTART"); break;
    case IRC_Codes::RPL_ENDOFMOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFMOTD"); break;
    case IRC_Codes::RPL_FORCE_MOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_FORCE_MOTD"); break;
    case IRC_Codes::RPL_BANEXPIRED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_BANEXPIRED"); break;
    case IRC_Codes::RPL_KICKLINKED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KICKLINKED"); break;
    case IRC_Codes::RPL_BANLINKED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_BANLINKED"); break;

    case IRC_Codes::RPL_YOUREOPER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOUREOPER"); break;
    case IRC_Codes::RPL_REHASHING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_REHASHING"); break;
    case IRC_Codes::RPL_YOURESERVICE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOURESERVICE"); break;
    case IRC_Codes::RPL_MYPORTIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MYPORTIS"); break;
    case IRC_Codes::RPL_NOTOPERANYMORE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOTOPERANYMORE"); break;
    case IRC_Codes::RPL_QLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_QLIST"); break;
    case IRC_Codes::RPL_ENDOFQLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFQLIST"); break;

    case IRC_Codes::RPL_TIME:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TIME"); break;
    case IRC_Codes::RPL_USERSSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_USERSSTART"); break;
    case IRC_Codes::RPL_USERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_USERS"); break;
    case IRC_Codes::RPL_ENDOFUSERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFUSERS"); break;
    case IRC_Codes::RPL_NOUSERS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOUSERS"); break;
    case IRC_Codes::RPL_HOSTHIDDEN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HOSTHIDDEN"); break;

    case IRC_Codes::ERR_UNKNOWNERROR:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNKNOWNERROR"); break;
    case IRC_Codes::ERR_NOSUCHNICK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSUCHNICK"); break;
    case IRC_Codes::ERR_NOSUCHSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSUCHSERVER"); break;
    case IRC_Codes::ERR_NOSUCHCHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSUCHCHANNEL"); break;
    case IRC_Codes::ERR_CANNOTSENDTOCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTSENDTOCHAN"); break;
    case IRC_Codes::ERR_TOOMANYCHANNELS:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TOOMANYCHANNELS"); break;
    case IRC_Codes::ERR_WASNOSUCHNICK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_WASNOSUCHNICK"); break;
    case IRC_Codes::ERR_TOOMANYTARGETS:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TOOMANYTARGETS"); break;
    case IRC_Codes::ERR_NOSUCHSERVICE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSUCHSERVICE"); break;
    case IRC_Codes::ERR_NOORIGIN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOORIGIN"); break;

    case IRC_Codes::ERR_NORECIPIENT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NORECIPIENT"); break;
    case IRC_Codes::ERR_NOTEXTTOSEND:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTEXTTOSEND"); break;
    case IRC_Codes::ERR_NOTOPLEVEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTOPLEVEL"); break;
    case IRC_Codes::ERR_WILDTOPLEVEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_WILDTOPLEVEL"); break;
    case IRC_Codes::ERR_BADMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADMASK"); break;
    case IRC_Codes::ERR_TOOMANYMATCHES:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TOOMANYMATCHES"); break;
    case IRC_Codes::ERR_LENGTHTRUNCATED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_LENGTHTRUNCATED"); break;

    case IRC_Codes::ERR_UNKNOWNCOMMAND:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNKNOWNCOMMAND"); break;
    case IRC_Codes::ERR_NOMOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOMOTD"); break;
    case IRC_Codes::ERR_NOADMININFO:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOADMININFO"); break;
    case IRC_Codes::ERR_FILEERROR:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_FILEERROR"); break;
    case IRC_Codes::ERR_NOOPERMOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOOPERMOTD"); break;
    case IRC_Codes::ERR_TOOMANYAWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TOOMANYAWAY"); break;

    case IRC_Codes::ERR_EVENTNICKCHANGE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_EVENTNICKCHANGE"); break;
    case IRC_Codes::ERR_NONICKNAMEGIVEN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NONICKNAMEGIVEN"); break;
    case IRC_Codes::ERR_ERRONEUSNICKNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ERRONEUSNICKNAME"); break;
    case IRC_Codes::ERR_NICKNAMEINUSE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NICKNAMEINUSE"); break;
    case IRC_Codes::ERR_SERVICENAMEINUSE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SERVICENAMEINUSE"); break;
    case IRC_Codes::ERR_SERVICECONFUSED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SERVICECONFUSED"); break;
    case IRC_Codes::ERR_NICKCOLLISION:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NICKCOLLISION"); break;
    case IRC_Codes::ERR_UNAVAILRESOURCE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNAVAILRESOURCE"); break;
    case IRC_Codes::ERR_NICKTOOFAST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NICKTOOFAST"); break;
    case IRC_Codes::ERR_TARGETTOOFAST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TARGETTOOFAST"); break;

    case IRC_Codes::ERR_SERVICESDOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SERVICESDOWN"); break;
    case IRC_Codes::ERR_USERNOTINCHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USERNOTINCHANNEL"); break;
    case IRC_Codes::ERR_NOTONCHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTONCHANNEL"); break;
    case IRC_Codes::ERR_USERONCHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USERONCHANNEL"); break;
    case IRC_Codes::ERR_NOLOGIN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOLOGIN"); break;
    case IRC_Codes::ERR_SUMMONDISABLED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SUMMONDISABLED"); break;
    case IRC_Codes::ERR_USERSDISABLED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USERSDISABLED"); break;
    case IRC_Codes::ERR_NONICKCHANGE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NONICKCHANGE"); break;
    case IRC_Codes::ERR_NOTIMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTIMPLEMENTED"); break;

    case IRC_Codes::ERR_NOTREGISTERED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTREGISTERED"); break;
    case IRC_Codes::ERR_IDCOLLISION:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_IDCOLLISION"); break;
    case IRC_Codes::ERR_NICKLOST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NICKLOST"); break;
    case IRC_Codes::ERR_HOSTILENAME:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_HOSTILENAME"); break;
    case IRC_Codes::ERR_ACCEPTFULL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ACCEPTFULL"); break;
    case IRC_Codes::ERR_ACCEPTEXIST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ACCEPTEXIST"); break;
    case IRC_Codes::ERR_ACCEPTNOT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ACCEPTNOT"); break;
    case IRC_Codes::ERR_NOHIDING:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOHIDING"); break;

    case IRC_Codes::ERR_NOTFORHALFOPS:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOTFORHALFOPS"); break;
    case IRC_Codes::ERR_NEEDMOREPARAMS:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NEEDMOREPARAMS"); break;
    case IRC_Codes::ERR_ALREADYREGISTRED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ALREADYREGISTRED"); break;
    case IRC_Codes::ERR_NOPERMFORHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOPERMFORHOST"); break;
    case IRC_Codes::ERR_PASSWDMISMATCH:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_PASSWDMISMATCH"); break;
    case IRC_Codes::ERR_YOUREBANNEDCREEP:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_YOUREBANNEDCREEP"); break;
    case IRC_Codes::ERR_YOUWILLBEBANNED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_YOUWILLBEBANNED"); break;
    case IRC_Codes::ERR_KEYSET:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_KEYSET"); break;
    case IRC_Codes::ERR_ONLYSERVERSCANCHANGE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ONLYSERVERSCANCHANGE"); break;
    case IRC_Codes::ERR_LINKSET:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_LINKSET"); break;

    case IRC_Codes::ERR_LINKCHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_LINKCHANNEL"); break;
    case IRC_Codes::ERR_CHANNELISFULL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CHANNELISFULL"); break;
    case IRC_Codes::ERR_UNKNOWNMODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNKNOWNMODE"); break;
    case IRC_Codes::ERR_INVITEONLYCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_INVITEONLYCHAN"); break;
    case IRC_Codes::ERR_BANNEDFROMCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BANNEDFROMCHAN"); break;
    case IRC_Codes::ERR_BADCHANNELKEY:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADCHANNELKEY"); break;
    case IRC_Codes::ERR_BADCHANMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADCHANMASK"); break;
    case IRC_Codes::ERR_NOCHANMODES:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOCHANMODES"); break;
    case IRC_Codes::ERR_BANLISTFULL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BANLISTFULL"); break;
    case IRC_Codes::ERR_BADCHANNAME:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADCHANNAME"); break;

    case IRC_Codes::ERR_CANNOTKNOCK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTKNOCK"); break;
    case IRC_Codes::ERR_NOPRIVILEGES:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOPRIVILEGES"); break;
    case IRC_Codes::ERR_CHANOPRIVSNEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CHANOPRIVSNEEDED"); break;
    case IRC_Codes::ERR_CANTKILLSERVER:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANTKILLSERVER"); break;
    case IRC_Codes::ERR_RESTRICTED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_RESTRICTED"); break;
    case IRC_Codes::ERR_UNIQOPPRIVSNEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNIQOPPRIVSNEEDED"); break;
    case IRC_Codes::ERR_NONONREG:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NONONREG"); break;
    case IRC_Codes::ERR_CHANTOORECENT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CHANTOORECENT"); break;
    case IRC_Codes::ERR_TSLESSCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TSLESSCHAN"); break;
    case IRC_Codes::ERR_VOICENEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_VOICENEEDED"); break;

    case IRC_Codes::ERR_NOOPERHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOOPERHOST"); break;
    case IRC_Codes::ERR_NOSERVICEHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSERVICEHOST"); break;
    case IRC_Codes::ERR_NOFEATURE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOFEATURE"); break;
    case IRC_Codes::ERR_BADFEATURE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADFEATURE"); break;
    case IRC_Codes::ERR_BADLOGTYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADLOGTYPE"); break;
    case IRC_Codes::ERR_BADLOGSYS:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADLOGSYS"); break;
    case IRC_Codes::ERR_BADLOGVALUE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADLOGVALUE"); break;
    case IRC_Codes::ERR_ISOPERLCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ISOPERLCHAN"); break;
    case IRC_Codes::ERR_CHANOWNPRIVNEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CHANOWNPRIVNEEDED"); break;

    case IRC_Codes::ERR_UMODEUNKNOWNFLAG:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UMODEUNKNOWNFLAG"); break;
    case IRC_Codes::ERR_USERSDONTMATCH:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USERSDONTMATCH"); break;
    case IRC_Codes::ERR_GHOSTEDCLIENT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_GHOSTEDCLIENT"); break;
    case IRC_Codes::ERR_USERNOTONSERV:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USERNOTONSERV"); break;

    case IRC_Codes::ERR_SILELISTFULL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SILELISTFULL"); break;
    case IRC_Codes::ERR_NOSUCHGLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOSUCHGLINE"); break;
    case IRC_Codes::ERR_BADPING:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADPING"); break;
    case IRC_Codes::ERR_INVALID_ERR:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_INVALID_ERR"); break;
    case IRC_Codes::ERR_BADEXPIRE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADEXPIRE"); break;
    case IRC_Codes::ERR_DONTCHEAT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_DONTCHEAT"); break;
    case IRC_Codes::ERR_DISABLED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_DISABLED"); break;
    case IRC_Codes::ERR_NOINVITE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOINVITE"); break;
    case IRC_Codes::ERR_ADMONLY:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_ADMONLY"); break;

    case IRC_Codes::ERR_OPERONLY:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_OPERONLY"); break;
    case IRC_Codes::ERR_LISTSYNTAX:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_LISTSYNTAX"); break;
    case IRC_Codes::ERR_WHOSYNTAX:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_WHOSYNTAX"); break;
    case IRC_Codes::ERR_WHOLIMEXCEED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_WHOLIMEXCEED"); break;
    case IRC_Codes::ERR_OPERSPVERIFY:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_OPERSPVERIFY"); break;
    case IRC_Codes::ERR_REMOTEPFX:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_REMOTEPFX"); break;
    case IRC_Codes::ERR_PFXUNROUTABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_PFXUNROUTABLE"); break;

    case IRC_Codes::ERR_BADHOSTMASK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADHOSTMASK"); break;
    case IRC_Codes::ERR_HOSTUNAVAIL:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_HOSTUNAVAIL"); break;
    case IRC_Codes::ERR_USINGSLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_USINGSLINE"); break;
    case IRC_Codes::ERR_STATSSLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_STATSSLINE"); break;

    case IRC_Codes::RPL_LOGON:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LOGON"); break;
    case IRC_Codes::RPL_LOGOFF:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LOGOFF"); break;
    case IRC_Codes::RPL_WATCHOFF:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WATCHOFF"); break;
    case IRC_Codes::RPL_WATCHSTAT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WATCHSTAT"); break;
    case IRC_Codes::RPL_NOWON:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOWON"); break;
    case IRC_Codes::RPL_NOWOFF:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOWOFF"); break;
    case IRC_Codes::RPL_WATCHLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WATCHLIST"); break;
    case IRC_Codes::RPL_ENDOFWATCHLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFWATCHLIST"); break;
    case IRC_Codes::RPL_WATCHCLEAR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WATCHCLEAR"); break;

    case IRC_Codes::RPL_ISOPER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ISOPER"); break;
    case IRC_Codes::RPL_ISLOCOP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ISLOCOP"); break;
    case IRC_Codes::RPL_ISNOTOPER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ISNOTOPER"); break;
    case IRC_Codes::RPL_ENDOFISOPER:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFISOPER"); break;
    case IRC_Codes::RPL_WHOISMODES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISMODES"); break;
    case IRC_Codes::RPL_WHOISHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISHOST"); break;
    case IRC_Codes::RPL_DCCSTATUS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_DCCSTATUS"); break;
    case IRC_Codes::RPL_DCCLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_DCCLIST"); break;
    case IRC_Codes::RPL_WHOWASHOST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOWASHOST"); break;

    case IRC_Codes::RPL_RULESSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_RULESSTART"); break;
    case IRC_Codes::RPL_RULES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_RULES"); break;
    case IRC_Codes::RPL_ENDOFRULES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFRULES"); break;
    case IRC_Codes::RPL_OMOTDSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_OMOTDSTART"); break;
    case IRC_Codes::RPL_OMOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_OMOTD"); break;
    case IRC_Codes::RPL_ENDOFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFO"); break;

    case IRC_Codes::RPL_SETTINGS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SETTINGS"); break;
    case IRC_Codes::RPL_ENDOFSETTINGS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFSETTINGS"); break;

    case IRC_Codes::RPL_DUMPING:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_DUMPING"); break;
    case IRC_Codes::RPL_DUMPRPL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_DUMPRPL"); break;
    case IRC_Codes::RPL_EODUMP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_EODUMP"); break;

    case IRC_Codes::RPL_TRACEROUTE_HOP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEROUTE_HOP"); break;
    case IRC_Codes::RPL_TRACEROUTE_START:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TRACEROUTE_START"); break;
    case IRC_Codes::RPL_MODECHANGEWARN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MODECHANGEWARN"); break;
    case IRC_Codes::RPL_CHANREDIR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANREDIR"); break;
    case IRC_Codes::RPL_SERVMODEIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_SERVMODEIS"); break;
    case IRC_Codes::RPL_OTHERUMODEIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_OTHERUMODEIS"); break;
    case IRC_Codes::RPL_ENDOF_GENERIC:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOF_GENERIC"); break;

    case IRC_Codes::RPL_WHOWASDETAILS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOWASDETAILS"); break;
    case IRC_Codes::RPL_WHOISSECURE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISSECURE"); break;
    case IRC_Codes::RPL_UNKNOWNMODES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_UNKNOWNMODES"); break;
    case IRC_Codes::RPL_CANNOTSETMODES:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CANNOTSETMODES"); break;
    case IRC_Codes::RPL_LUSERSTAFF:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LUSERSTAFF"); break;
    case IRC_Codes::RPL_TIMEONSERVERIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TIMEONSERVERIS"); break;

    case IRC_Codes::RPL_NETWORKS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NETWORKS"); break;
    case IRC_Codes::RPL_YOURLANGUAGEIS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_YOURLANGUAGEIS"); break;
    case IRC_Codes::RPL_LANGUAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_LANGUAGE"); break;
    case IRC_Codes::RPL_WHOISSTAFF:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISSTAFF"); break;

    case IRC_Codes::RPL_WHOISLANGUAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_WHOISLANGUAGE"); break;

    case IRC_Codes::RPL_MODLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_MODLIST"); break;
    case IRC_Codes::RPL_ENDOFMODLIST:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFMODLIST"); break;
    case IRC_Codes::RPL_HELPSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPSTART"); break;
    case IRC_Codes::RPL_HELPTXT:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_HELPTXT"); break;
    case IRC_Codes::RPL_ENDOFHELP:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFHELP"); break;
    case IRC_Codes::RPL_ETRACEFULL:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ETRACEFULL"); break;
    case IRC_Codes::RPL_ETRACE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ETRACE"); break;

    case IRC_Codes::RPL_KNOCK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KNOCK"); break;
    case IRC_Codes::RPL_KNOCKDLVR:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KNOCKDLVR"); break;
    case IRC_Codes::RPL_TOOMANYKNOCK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TOOMANYKNOCK"); break;
    case IRC_Codes::RPL_CHANOPEN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_CHANOPEN"); break;
    case IRC_Codes::RPL_KNOCKONCHAN:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KNOCKONCHAN"); break;
    case IRC_Codes::RPL_KNOCKDISABLED:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_KNOCKDISABLED"); break;
    case IRC_Codes::RPL_TARGUMODEG:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TARGUMODEG"); break;
    case IRC_Codes::RPL_TARGNOTIFY:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TARGNOTIFY"); break;
    case IRC_Codes::RPL_UMODEGMSG:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_UMODEGMSG"); break;

    case IRC_Codes::RPL_ENDOFOMOTD:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_ENDOFOMOTD"); break;
    case IRC_Codes::RPL_NOPRIVS:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOPRIVS"); break;
    case IRC_Codes::RPL_TESTMARK:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TESTMARK"); break;
    case IRC_Codes::RPL_TESTLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_TESTLINE"); break;
    case IRC_Codes::RPL_NOTESTLINE:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_NOTESTLINE"); break;

    case IRC_Codes::RPL_XINFO:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_XINFO"); break;
    case IRC_Codes::RPL_XINFOSTART:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_XINFOSTART"); break;
    case IRC_Codes::RPL_XINFOEND:
      result = ACE_TEXT_ALWAYS_CHAR ("RPL_XINFOEND"); break;

    case IRC_Codes::ERR_CANNOTDOCOMMAND:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTDOCOMMAND"); break;
    case IRC_Codes::ERR_CANNOTCHANGEUMODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTCHANGEUMODE"); break;
    case IRC_Codes::ERR_CANNOTCHANGECHANMODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTCHANGECHANMODE"); break;
    case IRC_Codes::ERR_CANNOTCHANGESERVERMODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTCHANGESERVERMODE"); break;
    case IRC_Codes::ERR_CANNOTSENDTONICK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_CANNOTSENDTONICK"); break;
    case IRC_Codes::ERR_UNKNOWNSERVERMODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_UNKNOWNSERVERMODE"); break;
    case IRC_Codes::ERR_SERVERMODELOCK:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_SERVERMODELOCK"); break;

    case IRC_Codes::ERR_BADCHARENCODING:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_BADCHARENCODING"); break;
    case IRC_Codes::ERR_TOOMANYLANGUAGES:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TOOMANYLANGUAGES"); break;
    case IRC_Codes::ERR_NOLANGUAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NOLANGUAGE"); break;
    case IRC_Codes::ERR_TEXTTOOSHORT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_TEXTTOOSHORT"); break;

    case IRC_Codes::ERR_NUMERIC_ERR:
      result = ACE_TEXT_ALWAYS_CHAR ("ERR_NUMERIC_ERR"); break;

    default:
    {
      // *NOTE*: according to the RFCTo81To, codes between 001-099 are reserved
      // for client-server connections...
      // --> maybe in use by some extension
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("invalid/unknown numeric (was: %d), aborting\n"),
                  numeric_in));
      break;
    }
  } // end SWITCH

  return result;
}

IRC_ChannelMode
IRC_Tools::CharToChannelMode (char mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CharToChannelMode"));

  // initialize result
  IRC_ChannelMode result = CHANNELMODE_INVALID;

  switch (mode_in)
  {
    case 'a':
      result = CHANNELMODE_ANONYMOUS; break;
    case 'b':
      result = CHANNELMODE_BAN; break;
    case 'i':
      result = CHANNELMODE_INVITEONLY; break;
    case 'k':
      result = CHANNELMODE_PASSWORD; break;
    case 'l':
      result = CHANNELMODE_USERLIMIT; break;
    case 'm':
      result = CHANNELMODE_MODERATED; break;
    case 'n':
      result = CHANNELMODE_BLOCKFOREIGNMSGS; break;
    case 'o':
      result = CHANNELMODE_OPERATOR; break;
    case 'p':
      result = CHANNELMODE_PRIVATE; break;
    case 'q':
      result = CHANNELMODE_QUIET; break;
    case 'r':
      result = CHANNELMODE_REOP; break;
    case 's':
      result = CHANNELMODE_SECRET; break;
    case 't':
      result = CHANNELMODE_RESTRICTEDTOPIC; break;
    case 'v':
      result = CHANNELMODE_VOICE; break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("unknown/invalid channel mode (was: \"%c\"), aborting\n"),
                  mode_in));
      break;
    }
  } // end SWITCH

  return result;
}

IRC_UserMode
IRC_Tools::CharToUserMode (char mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CharToUserMode"));

  // initialize result
  IRC_UserMode result = USERMODE_INVALID;

  switch (mode_in)
  {
    case 'a':
      result = USERMODE_AWAY; break;
    case 'H':
      result = USERMODE_HIDE_OPER; break;
    case 'i':
      result = USERMODE_INVISIBLE; break;
    case 'o':
      result = USERMODE_OPERATOR; break;
    case 'O':
      result = USERMODE_LOCALOPERATOR; break;
    case 'r':
      result = USERMODE_RESTRICTEDCONN; break;
    case 's':
      result = USERMODE_RECVNOTICES; break;
    case 'w':
      result = USERMODE_RECVWALLOPS; break;
    case 'x':
      result = USERMODE_HOSTHIDING; break;
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("unknown/invalid user mode (was: \"%c\"), aborting\n"),
                  mode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
IRC_Tools::ChannelModeToString (enum IRC_ChannelMode mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::ChannelModeToString"));

  // initialize result
  std::string result;

  switch (mode_in)
  {
    case CHANNELMODE_ANONYMOUS:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_ANONYMOUS"); break;
    case CHANNELMODE_BAN:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_BAN"); break;
    case CHANNELMODE_INVITEONLY:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_INVITEONLY"); break;
    case CHANNELMODE_PASSWORD:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_PASSWORD"); break;
    case CHANNELMODE_USERLIMIT:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_USERLIMIT"); break;
    case CHANNELMODE_MODERATED:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_MODERATED"); break;
    case CHANNELMODE_BLOCKFOREIGNMSGS:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_BLOCKFOREIGNMSGS"); break;
    case CHANNELMODE_OPERATOR:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_OPERATOR"); break;
    case CHANNELMODE_PRIVATE:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_PRIVATE"); break;
    case CHANNELMODE_QUIET:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_QUIET"); break;
    case CHANNELMODE_REOP:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_REOP"); break;
    case CHANNELMODE_SECRET:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_SECRET"); break;
    case CHANNELMODE_RESTRICTEDTOPIC:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_RESTRICTEDTOPIC"); break;
    case CHANNELMODE_VOICE:
      result = ACE_TEXT_ALWAYS_CHAR ("CHANNELMODE_VOICE"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid channel mode (was: %d), aborting\n"),
                  mode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
IRC_Tools::ChannelModeToString (const IRC_ChannelModes_t& modes_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::ChannelModeToString"));

  std::string result;

  // sanity check(s)
  if (modes_in.none ())
    return result;

  result = ACE_TEXT_ALWAYS_CHAR ("+");
  for (unsigned int i = 0;
       i < modes_in.size ();
       ++i)
    if (modes_in.test (i))
      result += IRC_Tools::ChannelModeToChar (static_cast<enum IRC_ChannelMode> (i));

  return result;
}

std::string
IRC_Tools::UserModeToString (enum IRC_UserMode mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::UserModeToString"));

  // initialize result
  std::string result;

  switch (mode_in)
  {
    case USERMODE_AWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("AWAY"); break;
    case USERMODE_HIDE_OPER:
      result = ACE_TEXT_ALWAYS_CHAR ("HIDE_OPER"); break;
    case USERMODE_INVISIBLE:
      result = ACE_TEXT_ALWAYS_CHAR ("INVISIBLE"); break;
    case USERMODE_OPERATOR:
      result = ACE_TEXT_ALWAYS_CHAR ("OPERATOR"); break;
    case USERMODE_LOCALOPERATOR:
      result = ACE_TEXT_ALWAYS_CHAR ("LOCAL_OPERATOR"); break;
    case USERMODE_RESTRICTEDCONN:
      result = ACE_TEXT_ALWAYS_CHAR ("RESTRICTED_CONNECTION"); break;
    case USERMODE_RECVNOTICES:
      result = ACE_TEXT_ALWAYS_CHAR ("RECEIVE_NOTICES"); break;
    case USERMODE_RECVWALLOPS:
      result = ACE_TEXT_ALWAYS_CHAR ("RECEIVE_WALLOPS"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid user mode (was: %d), aborting\n"),
                  mode_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
IRC_Tools::UserModeToString (const IRC_UserModes_t& modes_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::UserModeToString"));

  std::string result;

  // sanity check(s)
  if (modes_in.none ())
    return result;

  result = ACE_TEXT_ALWAYS_CHAR ("+");
  for (unsigned int i = 0;
       i < modes_in.size ();
       ++i)
    if (modes_in.test (i))
      result += IRC_Tools::UserModeToChar (static_cast<enum IRC_UserMode> (i));

  return result;
}

std::string
IRC_Tools::RecordToString (const IRC_Record& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::RecordToString"));

  // initialize result
  std::string result;

  switch (message_in.command_.discriminator)
  {
    case IRC_Record::Command::STRING:
    {
      IRC_Record::CommandType command =
        IRC_Tools::CommandToType (*message_in.command_.string);
      switch (command)
      {
        case IRC_Record::NICK:
        {
          result = message_in.prefix_.origin;
          result += ACE_TEXT_ALWAYS_CHAR (" --> ");
          result += message_in.parameters_.back ();
          break;
        }
        case IRC_Record::QUIT:
        {
          result = message_in.prefix_.origin;
          result += ACE_TEXT_ALWAYS_CHAR (" has QUIT IRC (reason: \"");
          result += message_in.parameters_.back ();
          result += ACE_TEXT_ALWAYS_CHAR ("\")");
          break;
        }
        case IRC_Record::JOIN:
        {
          result = '"';
          result += message_in.prefix_.origin;
          result += ACE_TEXT_ALWAYS_CHAR ("\" (");
          result += message_in.prefix_.user;
          result += '@';
          result += message_in.prefix_.host;
          result += ACE_TEXT_ALWAYS_CHAR (") has joined channel \"");
          result += message_in.parameters_.back ();
          result += '"';
          break;
        }
        case IRC_Record::PART:
        {
          result = '"';
          result += message_in.prefix_.origin;
          result += ACE_TEXT_ALWAYS_CHAR ("\" (");
          result += message_in.prefix_.user;
          result += '@';
          result += message_in.prefix_.host;
          result += ACE_TEXT_ALWAYS_CHAR (") has left channel \"");
          IRC_ParametersIterator_t iterator = message_in.parameters_.begin ();
          result += *iterator;
          if (message_in.parameters_.size () >= 2)
          {
            ++iterator;
            result += ACE_TEXT_ALWAYS_CHAR ("\" (reason: \"");
            result += *iterator;
            result += ACE_TEXT_ALWAYS_CHAR ("\")");
          } // end IF
          else
            result += ACE_TEXT_ALWAYS_CHAR ("\"");
          break;
        }
        case IRC_Record::NOTICE:
        {
          result = IRC_Tools::stringify (message_in.parameters_,
                                         1);
          break;
        }
        case IRC_Record::MODE:
        case IRC_Record::TOPIC:
        case IRC_Record::INVITE:
        case IRC_Record::KICK:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#pragma message("applying quirk code for this compiler")
        case IRC_Record::__QUIRK__ERROR:
#else
        case IRC_Record::ERROR:
#endif // ACE_WIN32 || ACE_WIN64
        case IRC_Record::AWAY:
        case IRC_Record::USERS:
        case IRC_Record::USERHOST:
        {
          result = IRC_Tools::stringify (message_in.parameters_);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid command (was: \"%s\"), aborting\n"),
                      ACE_TEXT (message_in.command_.string->c_str ())));

          message_in.dump_state ();

          return result;
        }
      } // end SWITCH

      break;
    }
    case IRC_Record::Command::NUMERIC:
    {
      switch (message_in.command_.numeric)
      {
        case IRC_Codes::RPL_WELCOME:          //   1
        case IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Codes::RPL_CREATED:          //   3
        case IRC_Codes::RPL_LUSERCLIENT:      // 251
        case IRC_Codes::RPL_LUSERME:          // 255
        case IRC_Codes::RPL_TRYAGAIN:         // 263
        case IRC_Codes::RPL_LOCALUSERS:       // 265
        case IRC_Codes::RPL_GLOBALUSERS:      // 266
        case IRC_Codes::RPL_UNAWAY:           // 305
        case IRC_Codes::RPL_NOWAWAY:          // 306
        case IRC_Codes::RPL_ENDOFBANLIST:     // 368
        case IRC_Codes::RPL_MOTD:             // 372
        case IRC_Codes::RPL_MOTDSTART:        // 375
        case IRC_Codes::RPL_ENDOFMOTD:        // 376
        case IRC_Codes::RPL_HOSTHIDDEN:       // 396
        case IRC_Codes::ERR_NOMOTD:           // 422
        case IRC_Codes::ERR_ALREADYREGISTRED: // 462
        case IRC_Codes::ERR_YOUREBANNEDCREEP: // 465
        case IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        {
          result = IRC_Tools::stringify (message_in.parameters_,
                                         -1);
          break;
        }
        case IRC_Codes::RPL_BANLIST:          // 367
        {
          result = IRC_Tools::stringify (message_in.parameters_);
          break;
        }
        case IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Codes::RPL_SNOMASK:          //   8
        case IRC_Codes::RPL_YOURID:           //  42
        case IRC_Codes::RPL_STATSDLINE:       // 250
        case IRC_Codes::RPL_LUSEROP:          // 252
        case IRC_Codes::RPL_LUSERUNKNOWN:     // 253
        case IRC_Codes::RPL_LUSERCHANNELS:    // 254
        case IRC_Codes::RPL_USERHOST:         // 302
        case IRC_Codes::RPL_LISTSTART:        // 321
        case IRC_Codes::RPL_LIST:             // 322
        case IRC_Codes::RPL_LISTEND:          // 323
        case IRC_Codes::RPL_INVITING:         // 341
        case IRC_Codes::ERR_NOSUCHNICK:       // 401
        case IRC_Codes::ERR_ERRONEUSNICKNAME: // 432
        case IRC_Codes::ERR_NICKNAMEINUSE:    // 433
        case IRC_Codes::ERR_NOTREGISTERED:    // 451
        case IRC_Codes::ERR_NEEDMOREPARAMS:   // 461
        case IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          result = IRC_Tools::stringify (message_in.parameters_, 1);
          break;
        }
        case IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          result = IRC_Tools::stringify (message_in.parameters_, 2);
          break;
        }
        case IRC_Codes::RPL_WHOREPLY:         // 352
        {
          result = IRC_Tools::stringify (message_in.parameters_, 5);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid (numeric) command (was: %u [\"%s\"]), aborting\n"),
                      message_in.command_.numeric,
                      ACE_TEXT (IRC_Tools::CommandToString (message_in.command_.numeric).c_str ())));

          message_in.dump_state ();

          return result;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command discriminator (was: %d), aborting\n"),
                  message_in.command_.discriminator));
      return result;
    }
  } // end SWITCH

  return result;
}

std::string
IRC_Tools::stringify (const IRC_Parameters_t& parameters_in,
                      int index_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::stringify"));

  std::string result;

  // sanity check(s)
  if (parameters_in.empty () ||
      (index_in > static_cast<int> ((parameters_in.size () - 1))))
    return result;

  if (index_in == -1)
    return parameters_in.back ();

  IRC_ParametersIterator_t iterator = parameters_in.begin ();
  std::advance (iterator, index_in);
  for (;
       iterator != parameters_in.end ();
       iterator++)
  {
    result += *iterator;
    result += ACE_TEXT_ALWAYS_CHAR (" ");
  } // end FOR
  if (index_in < static_cast<int> ((parameters_in.size () - 1)))
    result.erase (--result.end ());

  return result;
}

IRC_CommandType_t
IRC_Tools::CharToCommand (char char_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Tools::CharToCommand"));

  switch (char_in)
  {
    case 'a':
      return IRC_Record::CommandType::AWAY;
    case 'i':
      return IRC_Record::CommandType::INVITE;
    case 'j':
      return IRC_Record::CommandType::JOIN;
    case 'k':
      return IRC_Record::CommandType::KICK;
    case 'l':
      return IRC_Record::CommandType::LIST;
    case 'm':
      return IRC_Record::CommandType::PRIVMSG;
    case 'n':
      return IRC_Record::CommandType::NOTICE;
    case 'o':
      return IRC_Record::CommandType::MODE;
    case 'p':
      return IRC_Record::CommandType::PART;
    case 'q':
      return IRC_Record::CommandType::QUIT;
    case 't':
      return IRC_Record::CommandType::TOPIC;
    case 'u':
      return IRC_Record::CommandType::USERHOST;
    case 'w':
      return IRC_Record::CommandType::WHO;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid command (was: %c), aborting\n"),
                  char_in));
      break;
    }
  } // end SWITCH

  return IRC_Record::CommandType::IRC_COMMANDTYPE_INVALID;
}
