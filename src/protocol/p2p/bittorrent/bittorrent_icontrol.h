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

#include <string>

#include "common_isubscribe.h"

#include "bittorrent_common.h"

template <typename NotificationType>
class BitTorrent_IControl_T
 : public Common_ISubscribe_T<NotificationType>
{
 public:
  inline virtual ~BitTorrent_IControl_T () {};

  virtual void download (const std::string&) = 0; // metainfo (aka '.torrent') file
  virtual void seed (const std::string&) = 0; // metainfo (aka '.torrent') file

  virtual void notify (const std::string&) = 0; // nick
};

#endif
