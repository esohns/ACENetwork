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

#ifndef BITTORRENT_ICONTROL_H
#define BITTORRENT_ICONTROL_H

#include <map>
#include <string>

#include "common_iget.h"

#include "bittorrent_common.h"

template <typename SessionInterfaceType>
class BitTorrent_IControl_T
 : public Common_IGetR_T<std::map<std::string, SessionInterfaceType*> >
{
 public:
  inline virtual ~BitTorrent_IControl_T () {}

  // *NOTE*: sending a request to the tracker allocates a session and initiates
  //         communication with the swarm of registered peers
  virtual void request (const std::string&) = 0; // metainfo (aka '.torrent') file URI
  virtual SessionInterfaceType* get (const std::string&) = 0; // metainfo (aka '.torrent') file URI

  virtual void stop (bool = false) = 0; // wait ?
  virtual void wait () = 0;

  ////////////////////////////////////////
  // callbacks
  virtual void notify (const std::string&,         // metainfo (aka '.torrent') file URI
                       enum BitTorrent_Event) = 0; // event
};

#endif
