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

#include "ace/INET_Addr.h"

#include "common_iget.h"

#include "http_common.h"

#include "ssdp_common.h"

template <typename StateType,
          typename ConnectionConfigurationType,
          typename UserData>
class SSDP_ISession_T
 : public Common_IGetR_T<StateType>
{
 public:
  inline virtual ~SSDP_ISession_T () {}

  virtual void initialize (ACE_UINT16,                         // 'preferred' UPnP server port
                           const ConnectionConfigurationType&, // HTTP-
                           const UserData&) = 0;

  // API
  virtual void close () = 0; // close all HTTP- connections
  virtual void wait () = 0; // wait for all HTTP- connections
  virtual void getURL (const std::string&) const = 0; // device/service description URLs

  virtual void externalAddress () const = 0;
  virtual void map (const ACE_INET_Addr&,            // external IP/port
                    const ACE_INET_Addr&) const = 0; // internal IP/port
  virtual std::string presentationURL () const = 0;

  ////////////////////////////////////////
  // callbacks
  // *TODO*: make these 'private'

  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void notifySSDPResponse (const struct HTTP_Record&) = 0; // SSDP response record
  virtual void notifyPresentationURL (const std::string&) = 0; // presentation URL
  virtual void notifyServiceDescriptionControlURIs (const std::string&,      // service description URI
                                                    const std::string&) = 0; // service control URI
  virtual void notifyServiceActionArguments (const SSDP_ServiceActionArguments_t&) = 0; // service action argument(s)
};

#endif
