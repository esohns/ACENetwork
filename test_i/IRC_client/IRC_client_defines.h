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

#ifndef IRC_CLIENT_DEFINES_H
#define IRC_CLIENT_DEFINES_H

#define IRC_CLIENT_CNF_DEFAULT_INI_FILE                 "IRC_client.ini"
// .ini configuration file
#define IRC_CLIENT_CNF_LOGIN_SECTION_HEADER             "login"
#define IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER        "connections"
#define IRC_CLIENT_CNF_PASSWORD_LABEL                   "password"
#define IRC_CLIENT_CNF_NICKNAME_LABEL                   "nickname"
#define IRC_CLIENT_CNF_USER_LABEL                       "user"
#define IRC_CLIENT_CNF_REALNAME_LABEL                   "realname"
#define IRC_CLIENT_CNF_CHANNEL_LABEL                    "channel"
#define IRC_CLIENT_CNF_SERVER_LABEL                     "server"
#define IRC_CLIENT_CNF_PORT_LABEL                       "port"

// phonebook
#define IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER         "timestamp"
#define IRC_CLIENT_CNF_DATE_SECTION_HEADER              "date"
#define IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER          "networks"
#define IRC_CLIENT_CNF_SERVERS_SECTION_HEADER           "servers"
#define IRC_CLIENT_PHONEBOOK_DEF_NETWORK_LABEL          "<none>"
// *TODO*: write a parser instead
#define IRC_CLIENT_PHONEBOOK_KEYWORD_GROUP              "GROUP:"
#define IRC_CLIENT_PHONEBOOK_KEYWORD_SERVER             "SERVER:"

#define IRC_CLIENT_DEFAULT_NUMBER_OF_TP_THREADS         3

//#define IRC_CLIENT_DEFAULT_STATISTIC_REPORTING_INTERVAL 0 // seconds: 0 --> OFF

// client
// asynchronous connections
#define IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT            60 // second(s)
#define IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT_INTERVAL   1  // second(s)
#define IRC_CLIENT_SESSION_LOG_FILENAME_PREFIX          "IRC_client_session"
#define IRC_CLIENT_SESSION_USE_CURSES                   true // use (PD|N)curses ?
#define IRC_CLIENT_SESSION_DEFAULT_LOG                  false // log to file ? : stdout

// UI/output
#define IRC_CLIENT_CURSES_COLOR_LOG                     1
#define IRC_CLIENT_CURSES_COLOR_STATUS                  2
#define IRC_CLIENT_CURSES_CURSOR_MODE                   2 // highlighted
#define IRC_CLIENT_DEFAULT_PRINT_PINGDOT                false

#define IRC_CLIENT_DEFAULT_SERVER_HOSTNAME              ACE_LOCALHOST

#endif
