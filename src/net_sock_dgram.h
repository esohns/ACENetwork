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

#ifndef NET_SOCK_DGRAM_H
#define NET_SOCK_DGRAM_H

#include "ace/Global_Macros.h"
#include "ace/SOCK_CODgram.h"
#include "ace/SOCK_Dgram.h"

class Net_SOCK_Dgram
 : public ACE_SOCK_Dgram
{
 public:
  Net_SOCK_Dgram ();
  virtual ~Net_SOCK_Dgram ();

  using ACE_SOCK::get_remote_addr;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: overwrite (part of) ACE_SOCK_Dgram to support pre-bind() socket
  //         options (e.g. SO_REUSEPORT) on Unixy systems
  int open (const ACE_Addr&,                // (local) SAP
            int = ACE_PROTOCOL_FAMILY_INET, // protocol family
            int = 0,                        // protocol
            int = 0);                       // reuse address ?
#endif

 private:
  typedef ACE_SOCK_Dgram inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Dgram (const Net_SOCK_Dgram&))
  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Dgram& operator= (const Net_SOCK_Dgram&))
};

//////////////////////////////////////////

class Net_SOCK_CODgram
 : public ACE_SOCK_CODgram
{
 public:
  Net_SOCK_CODgram ();
  virtual ~Net_SOCK_CODgram ();

  using ACE_SOCK::get_remote_addr;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: overwrite (part of) ACE_SOCK_Dgram to support pre-bind() socket
  //         options (e.g. SO_REUSEPORT) on Unixy systems
  int open (const ACE_Addr&,                     // remote SAP
            const ACE_Addr& = ACE_Addr::sap_any, // local SAP
            int = ACE_PROTOCOL_FAMILY_INET,      // protocol family
            int = 0,                             // protocol
            int = 0);                            // reuse address ?
#endif

 private:
  typedef ACE_SOCK_CODgram inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_CODgram (const Net_SOCK_CODgram&))
  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_CODgram& operator= (const Net_SOCK_CODgram&))
};

#endif
