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

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_UDPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::Net_UDPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                unsigned int statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::Net_UDPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_UDPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::Net_UDPConnectionBase_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::Net_UDPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_UDPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::~Net_UDPConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::~Net_UDPConnectionBase_T"));

}

//template <typename UserDataType,
//          typename StateType,
//          typename HandlerType>
//void
//Net_UDPConnectionBase_T<UserDataType,
//                    StateType,
//                    HandlerType>::info (ACE_HANDLE& handle_out,
//                                        ACE_INET_Addr& localSAP_out,
//                                        ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::info"));
//
//  inherited::info (handle_out,
//                   localSAP_out,
//                   remoteSAP_out);
//}

//template <typename HandlerType,
//          typename StateType,
//          typename HandlerConfigurationType,
//          typename UserDataType>
//int
//Net_UDPConnectionBase_T<HandlerType,
//                    StateType,
//                    HandlerConfigurationType,
//                    UserDataType>::close (u_long arg_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::close"));
//
//  ACE_UNUSED_ARG (arg_in);
//
//  int result = -1;
//
//  result = inherited::close (0);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_SocketConnectionBase_T::close(0): \"%m\", continuing\n")));
//
//  return result;
//}

/////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::Net_AsynchUDPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                            unsigned int statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::Net_AsynchUDPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::Net_AsynchUDPConnectionBase_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::Net_AsynchUDPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::~Net_AsynchUDPConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::~Net_AsynchUDPConnectionBase_T"));

}

//template <typename UserDataType,
//          typename StateType,
//          typename HandlerType>
//void
//Net_AsynchUDPConnectionBase_T<UserDataType,
//                          StateType,
//                          HandlerType>::info (ACE_HANDLE& handle_out,
//                                              ACE_INET_Addr& localSAP_out,
//                                              ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::info"));
//
//  inherited::info (handle_out,
//                   localSAP_out,
//                   remoteSAP_out);
//}
