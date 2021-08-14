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

#ifndef NET_CLIENT_COMMON_TOOLS_H
#define NET_CLIENT_COMMON_TOOLS_H

#include "ace/config-macros.h"
#include "ace/Global_Macros.h"

class Net_Client_Common_Tools
{
 public:
  template <typename AddressType,
            typename ConnectorType,
            typename ConnectionConfigurationType,
            typename UserDataType,
            typename ConnectionManagerType>
  static ACE_HANDLE connect (ConnectorType&,
                             const ConnectionConfigurationType&, // (connection-) configuration
                             const UserDataType&,                // user data
                             const AddressType&,                 // (peer) address
                             bool = true,                        // wait ?
                             bool = true);                       // peer address ? : local address

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Client_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Common_Tools (const Net_Client_Common_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Common_Tools& operator= (const Net_Client_Common_Tools&))
};

// include template definition
#include "net_client_common_tools.inl"

#endif
