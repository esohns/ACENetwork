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

#ifndef PCP_ISESSION_T_H
#define PCP_ISESSION_T_H

#include "ace/INET_Addr.h"

#include "common_iget.h"

#include "pcp_common.h"
#include "pcp_network.h"

template <typename StateType,
          typename ConnectionConfigurationType>
class PCP_ISession_T
 : public Common_IGetR_T<StateType>
{
 public:
  inline virtual ~PCP_ISession_T () {}

  virtual void initialize (const ConnectionConfigurationType&,
                           PCP_IConnection_t*) = 0;

  // API
  virtual void announce () = 0;
  // *TODO*: support filter option
  virtual void map (const ACE_INET_Addr&,      // external address
                    const ACE_INET_Addr&) = 0; // internal address
  virtual void peer (const ACE_INET_Addr&,      // external address
                     const ACE_INET_Addr&,      // internal address
                     const ACE_INET_Addr&) = 0; // remote peer address
  virtual void authenticate () = 0;

  ////////////////////////////////////////
  // callbacks
  // *TODO*: make these 'private'

  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void notify (struct PCP_Record*&) = 0; // response record
};

#endif
