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

#ifndef PCP_SESSION_H
#define PCP_SESSION_H

#include "ace/Global_Macros.h"

#include "pcp_isession.h"

template <typename StateType,
          typename ConnectionConfigurationType, // outbound-
          typename ConnectionManagerType,       // (outbound-)
          typename MessageType>
class PCP_Session_T
 : public PCP_ISession_T<StateType,
                         ConnectionConfigurationType>
{
 public:
  PCP_Session_T ();
  virtual ~PCP_Session_T ();

  // implement PCP_ISession_T
  inline virtual const StateType& getR () const { return state_; }
  virtual void initialize (const ConnectionConfigurationType&,
                           PCP_IConnection_t*);

  virtual void announce ();
  virtual void map (const ACE_INET_Addr&,  // external address
                    const ACE_INET_Addr&); // internal address
  virtual void peer (const ACE_INET_Addr&,  // external address
                     const ACE_INET_Addr&,  // internal address
                     const ACE_INET_Addr&); // remote peer address
  virtual void authenticate ();

 private:
  ACE_UNIMPLEMENTED_FUNC (PCP_Session_T (const PCP_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (PCP_Session_T& operator= (const PCP_Session_T&))

  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void notify (struct PCP_Record*&); // response record

  ConnectionConfigurationType* configuration_;
  PCP_IConnection_t*           connection_;
  StateType                    state_;
};

// include template definition
#include "pcp_session.inl"

#endif
