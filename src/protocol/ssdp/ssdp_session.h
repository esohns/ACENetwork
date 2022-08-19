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

#ifndef SSDP_SESSION_H
#define SSDP_SESSION_H

#include <string>

#include "ace/Global_Macros.h"

#include "http_network.h"

#include "ssdp_isession.h"

template <typename StateType,
          typename ConnectionConfigurationType, // HTTP-
          typename ConnectionManagerType,       // HTTP-
          typename MessageType,
          typename ConnectorType, // HTTP-
          typename UserData>
class SSDP_Session_T
 : public SSDP_ISession_T<StateType,
                          ConnectionConfigurationType,
                          UserData>
{
 public:
  SSDP_Session_T ();
  virtual ~SSDP_Session_T ();

  // implement SSDP_ISession_T
  inline virtual const StateType& getR () const { return state_; }
  virtual void initialize (ACE_UINT16,                         // 'preferred' UPnP server port
                           const ConnectionConfigurationType&,
                           const UserData&);

  virtual void close (); // close all HTTP- connections
  virtual void wait (); // wait for all HTTP- connections
  virtual void getURL (const std::string&) const; // device/service description URLs

  virtual void map (const ACE_INET_Addr&,        // external IP/port
                    const ACE_INET_Addr&) const; // internal IP/port

 private:
  ACE_UNIMPLEMENTED_FUNC (SSDP_Session_T (const SSDP_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (SSDP_Session_T& operator= (const SSDP_Session_T&))

  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void notifySSDPResponse (const struct HTTP_Record&); // SSDP response record
  virtual void notifyServiceDescriptionControlURIs (const std::string&,  // service description URI
                                                    const std::string&); // service control URI
  virtual void notifyServiceActionArguments(const SSDP_ServiceActionArguments_t&); // service action argument(s)

  ConnectionConfigurationType* configuration_;
  ACE_UINT16                   preferredServerPort_;
  StateType                    state_;
  UserData*                    userData_;
};

// include template definition
#include "ssdp_session.inl"

#endif
