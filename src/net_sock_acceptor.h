/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef NET_SOCK_ACCEPTOR_H
#define NET_SOCK_ACCEPTOR_H

#include "ace/Addr.h"
#include "ace/Global_Macros.h"
//#include "ace/OS_QoS.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"

#include "net_exports.h"

// forward declarations
class ACE_Time_Value;

// *TODO*: move this class to Net_Client. 'friend'liness of
//         Net_TCPConnectionBase_T currently precludes this

class Net_Export Net_SOCK_Acceptor
 : public ACE_SOCK_Acceptor
{
 public:
  Net_SOCK_Acceptor ();
  virtual ~Net_SOCK_Acceptor ();

  int open (const ACE_Addr&,           // local address
            int = 0,                   // SO_REUSEADDR ?
            int = PF_UNSPEC,           // protocol family
            int = ACE_DEFAULT_BACKLOG, // backlog
            int = 0);                  // protocol
  //int open (const ACE_Addr&,           // local address
  //          ACE_Protocol_Info*,        // protocol info
  //          ACE_SOCK_GROUP,            // socket group
  //          u_long,                    // flags
  //          int,                       // SO_REUSEADDR ?
  //          int = PF_UNSPEC,           // protocol family
  //          int = ACE_DEFAULT_BACKLOG, // backlog
  //          int = 0);                  // protocol

 protected:
  int shared_open (const ACE_Addr&, // local address
                   int,             // protocol family
                   int);            // backlog

 private:
  typedef ACE_SOCK_Acceptor inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Acceptor (const Net_SOCK_Acceptor&))
  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Acceptor& operator= (const Net_SOCK_Acceptor&))
};

#endif
