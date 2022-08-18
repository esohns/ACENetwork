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

#ifndef SSDP_ISESSION_T_H
#define SSDP_ISESSION_T_H

#include <string>

#include "common_iget.h"

#include "http_common.h"

template <typename StateType,
          typename ConnectionConfigurationType,
          typename UserData>
class SSDP_ISession_T
 : public Common_IGetR_T<StateType>
{
 public:
  inline virtual ~SSDP_ISession_T () {}

  virtual void initialize (const ConnectionConfigurationType&, // HTTP-
                           const UserData&) = 0;

  // API
  virtual void getDeviceDescription (const std::string&) = 0; // device description URL
  virtual void getServiceDescription (const std::string&) = 0; // service description URL

  ////////////////////////////////////////
  // callbacks
  // *TODO*: make these 'private'

  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void notifySSDPResponse (const struct HTTP_Record&) = 0; // SSDP response record
  virtual void notifyServiceControlURI (const std::string&) = 0; // service control URI
};

#endif
