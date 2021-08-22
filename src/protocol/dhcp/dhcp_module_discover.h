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

#ifndef DHCP_MODULE_DISCOVER_H
#define DHCP_MODULE_DISCOVER_H

#include "ace/Global_Macros.h"

#include "stream_headmoduletask_base.h"
#include "stream_task_base_asynch.h"

#include "dhcp_network.h"

extern const char libacenetwork_protocol_default_dhcp_discover_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
class DHCP_Module_Discover_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  DHCP_Module_Discover_T (ISTREAM_T*);                     // stream handle
#else
  DHCP_Module_Discover_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~DHCP_Module_Discover_T ();

  // override (part of) Stream_IModuleHandler_T
  //virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_Discover_T ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_Discover_T (const DHCP_Module_Discover_T&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_Discover_T& operator= (const DHCP_Module_Discover_T&))

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // (requested) size
  ACE_HANDLE connect (const ACE_INET_Addr&, // peer (i.e. server) address
                      bool&);               // return value: use reactor ?

  DHCP_IConnection_t* connection_;
  bool                sendRequestOnOffer_;
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType>
class DHCP_Module_DiscoverH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      struct Stream_UserData>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  DHCP_Module_DiscoverH_T (ISTREAM_T*, // stream handle
#else
  DHCP_Module_DiscoverH_T (typename inherited::ISTREAM_T*,   // stream handle
#endif
                           bool = false, // auto-start ?
                           bool = true); // generate session messages ?
  inline virtual ~DHCP_Module_DiscoverH_T () {}

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using inherited::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_DiscoverH_T ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_DiscoverH_T (const DHCP_Module_DiscoverH_T&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_DiscoverH_T& operator= (const DHCP_Module_DiscoverH_T&))

  // convenience types
  //typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> STATISTICHANDLER_T;
  typedef typename DataMessageType::DATA_T DATA_T;

  bool sendRequestOnOffer_;
};

// include template definition
#include "dhcp_module_discover.inl"

#endif
