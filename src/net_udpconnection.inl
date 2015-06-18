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

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType,
                    HandlerConfigurationType>::Net_UDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                    unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));

}

//template <typename SessionDataType,
//          typename HandlerType>
//Net_UDPConnection_T<SessionDataType,
//                    HandlerType>::Net_UDPConnection_T ()
// : inherited (NULL)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));
//
//}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType,
                    HandlerConfigurationType>::~Net_UDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::~Net_UDPConnection_T"));

}

//template <typename UserDataType,
//          typename SessionDataType,
//          typename HandlerType>
//void
//Net_UDPConnection_T<UserDataType,
//                    SessionDataType,
//                    HandlerType>::info (ACE_HANDLE& handle_out,
//                                        ACE_INET_Addr& localSAP_out,
//                                        ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::info"));
//
//  inherited::info (handle_out,
//                   localSAP_out,
//                   remoteSAP_out);
//}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
void
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType,
                    HandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::close"));

  int result = -1;

  result = inherited::close (0);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::close(0): \"%m\", continuing\n")));
}

/////////////////////////////////////////

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType,
                          HandlerConfigurationType>::Net_AsynchUDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType,
                          HandlerConfigurationType>::Net_AsynchUDPConnection_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType,
          typename HandlerConfigurationType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType,
                          HandlerConfigurationType>::~Net_AsynchUDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::~Net_AsynchUDPConnection_T"));

}

//template <typename UserDataType,
//          typename SessionDataType,
//          typename HandlerType>
//void
//Net_AsynchUDPConnection_T<UserDataType,
//                          SessionDataType,
//                          HandlerType>::info (ACE_HANDLE& handle_out,
//                                              ACE_INET_Addr& localSAP_out,
//                                              ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::info"));
//
//  inherited::info (handle_out,
//                   localSAP_out,
//                   remoteSAP_out);
//}
