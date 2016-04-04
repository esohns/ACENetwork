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

#ifndef IRC_ICONTROL_H
#define IRC_ICONTROL_H

#include <string>

//#include "common_isubscribe.h"

#include "irc_record.h"

// forward declarations
struct IRC_LoginOptions;

//template <typename NotificationType>
class IRC_IControl
// : public Common_ISubscribe_T<NotificationType>
{
 public:
  inline virtual ~IRC_IControl () {};

  virtual bool registerc (const IRC_LoginOptions&) = 0; // login details

  virtual void nick (const std::string&) = 0; // nick
  virtual void quit (const std::string&) = 0; // reason
  virtual void join (const string_list_t&,      // channel(s)
                     const string_list_t&) = 0; // key(s)
  virtual void part (const string_list_t&) = 0; // channel(s)
  virtual void mode (const std::string&,        // nick/channel
                     char,                      // user/channel mode
                     bool,                      // enable ?
                     const string_list_t&) = 0; // any parameters
  virtual void topic (const std::string&,      // channel
                      const std::string&) = 0; // topic
  virtual void names (const string_list_t&) = 0; // channel(s)
  virtual void list (const string_list_t&) = 0; // channel(s)
  virtual void invite (const std::string&,      // nick
                       const std::string&) = 0; // channel
  virtual void kick (const std::string&,      // channel
                     const std::string&,      // nick
                     const std::string&) = 0; // comment
  virtual void send (const string_list_t&,    // receiver(s) [nick/channel]
                     const std::string&) = 0; // message
  virtual void who (const std::string&, // name
                    bool) = 0;          // query ops only ?
  virtual void whois (const std::string&,        // server
                      const string_list_t&) = 0; // nickmask(s)
  virtual void whowas (const std::string&,      // nick
                       unsigned int,            // count
                       const std::string&) = 0; // server
  virtual void away (const std::string&) = 0; // message
  virtual void users (const std::string&) = 0; // server
  virtual void userhost (const string_list_t&) = 0; // nicknames
};

#endif
