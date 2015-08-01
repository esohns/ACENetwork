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

#include "net_macros.h"

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_TCPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::Net_TCPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                unsigned int statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::Net_TCPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_TCPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::Net_TCPConnectionBase_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::Net_TCPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_TCPConnectionBase_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        HandlerConfigurationType,
                        UserDataType>::~Net_TCPConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::~Net_TCPConnectionBase_T"));

}

/////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchTCPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::Net_AsynchTCPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                            unsigned int statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::Net_AsynchTCPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchTCPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::Net_AsynchTCPConnectionBase_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::Net_AsynchTCPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_AsynchTCPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              HandlerConfigurationType,
                              UserDataType>::~Net_AsynchTCPConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::~Net_AsynchTCPConnectionBase_T"));

}
