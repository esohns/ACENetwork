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

#include <string>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_timer_manager_common.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::DHCP_Module_Discover_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , broadcastConnectionHandle_ (ACE_INVALID_HANDLE)
 , connectionHandle_ (ACE_INVALID_HANDLE)
 , isSessionConnection_ (false)
 , sendRequestOnOffer_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::DHCP_Module_Discover_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::~DHCP_Module_Discover_T ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::~DHCP_Module_Discover_T"));

  typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;

  if (!inherited::sessionData_)
    return;

  session_data_p =
      &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

  if (!session_data_p->broadcastConnection)
    return;

  if (broadcastConnectionHandle_ != ACE_INVALID_HANDLE)
  {
    // sanity check(s)
    ACE_ASSERT (session_data_p->broadcastConnection);

    session_data_p->broadcastConnection->close ();
    session_data_p->broadcastConnection->decrease ();
    session_data_p->broadcastConnection = NULL;
  } // end IF
  else
  {
    if (isSessionConnection_)
    {
      // sanity check(s)
      ACE_ASSERT (session_data_p->broadcastConnection);

      session_data_p->broadcastConnection->decrease ();
    } // end IF
    else
      session_data_p->broadcastConnection = NULL;
  } // end ELSE

  if (connectionHandle_ != ACE_INVALID_HANDLE)
  {
    // sanity check(s)
    ACE_ASSERT (session_data_p->connection);

    session_data_p->connection->close ();
    session_data_p->connection->decrease ();
    session_data_p->connection = NULL;
  } // end IF
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename ConnectionManagerType,
//          typename ConnectorTypeBcast,
//          typename ConnectorType>
//const ConfigurationType&
//DHCP_Module_Discover_T<ACE_SYNCH_USE,
//                       TimePolicyType,
//                       SessionMessageType,
//                       DataMessageType,
//                       ConfigurationType,
//                       ConnectionManagerType,
//                       ConnectorTypeBcast,
//                       ConnectorType>::get () const
//{
//  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::get"));
//
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
bool
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.protocolConfiguration);

  if (inherited::isInitialized_)
  {
    // clean up
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inference
  sendRequestOnOffer_ =
      configuration_in.protocolConfiguration->sendRequestOnOffer;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
void
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfigurations);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (message_inout->isInitialized ());

  const typename DataMessageType::DATA_T& data_r = message_inout->get ();

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

  // sanity check(s)
  ACE_ASSERT (session_data_r.broadcastConnection);

  u_short port_number = DHCP_DEFAULT_SERVER_PORT;
  ACE_INET_Addr address;
  bool clone_module, delete_module;
  typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
  bool reset_configuration = false;
  ConnectionManagerType* connection_manager_p = NULL;
  bool use_reactor = false;
  bool write_only = false;
  DHCP_OptionsIterator_t iterator;
  int result = -1;
  ConnectionConfigurationIteratorType iterator_2;

  iterator_2 =
    inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
  if (iterator_2 == inherited::configuration_->connectionConfigurations->end ())
    iterator_2 =
      inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: applying connection configuration\n"),
                inherited::mod_->name ()));
  ACE_ASSERT (iterator_2 != inherited::configuration_->connectionConfigurations->end ());

  // set server address ?
  if (!session_data_r.serverAddress.is_any ())
    goto continue_;

  iterator =
    data_r.options.find (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER);
  if (iterator == data_r.options.end ())
    goto continue_;

  ACE_ASSERT ((*iterator).second.size () == sizeof (ACE_UINT32));
  result =
      session_data_r.serverAddress.set (((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (port_number)
                                                                               : port_number),
                                        *reinterpret_cast<const ACE_UINT32*> ((*iterator).second.c_str ()),
                                        0, // encode ?
                                        0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_INET_Addr::set(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: DHCP server address: \"%s\"\n"),
  //            inherited::mod_->name (),
  //            ACE_TEXT (Net_Common_Tools::IPAddressToString (session_data_r.serverAddress).c_str ())));

  // step2a: set up the (broadcast) connection configuration
  // step2aa: set up the socket configuration
  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (!session_data_r.connection);

  address =
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.address;
  write_only =
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.writeOnly;
  (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.address =
      session_data_r.serverAddress;
  (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
    true;

  // step2ab: set up the connection manager
  // *NOTE*: the stream configuration may contain a module handle that is
  //         meant to be the final module of 'this' processing stream. As
  //         the (possibly external) DHCP (broadcast) connection probably
  //         (!) is oblivious of 'this' pipeline, the connection probably
  //         (!) should not enqueue that same module again
  //         --> temporarily 'hide' the module handle, if any
  // *TODO*: this 'measure' diminishes the generic utility of this module
  //         --> to be removed ASAP
  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // *TODO*: remove type inferences
  connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  clone_module = inherited::configuration_->streamConfiguration->cloneModule;
  delete_module = inherited::configuration_->streamConfiguration->deleteModule;
  module_p = inherited::configuration_->streamConfiguration->module;
  inherited::configuration_->streamConfiguration->cloneModule = false;
  inherited::configuration_->streamConfiguration->deleteModule = false;
  inherited::configuration_->streamConfiguration->module = NULL;
  reset_configuration = true;
  //connection_manager_p->set ((*iterator_2).second,
  //                           configuration_->userData);

  connectionHandle_ = this->connect (session_data_r.serverAddress,
                                     use_reactor);
  if (connectionHandle_ == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to %s, returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (session_data_r.serverAddress).c_str ())));
    goto error;
  } // end IF
  if (use_reactor)
    session_data_r.connection =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (connectionHandle_));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (connectionHandle_));
#endif
  else
    session_data_r.connection =
        connection_manager_p->get (session_data_r.serverAddress);
  if (!session_data_r.connection)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to %s, returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (session_data_r.serverAddress).c_str ())));
    goto error;
  } // end IF

  // step2e: reset the connection configuration
  (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.address =
    address;
  (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
    write_only;
  inherited::configuration_->streamConfiguration->cloneModule = clone_module;
  inherited::configuration_->streamConfiguration->deleteModule = delete_module;
  inherited::configuration_->streamConfiguration->module = module_p;

  goto continue_;

error:
  if (reset_configuration)
  {
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.address =
      address;
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
      write_only;
    inherited::configuration_->streamConfiguration->cloneModule = clone_module;
    inherited::configuration_->streamConfiguration->deleteModule =
      delete_module;
    inherited::configuration_->streamConfiguration->module = module_p;
    //connection_manager_p->set ((*iterator_2).second,
    //                           configuration_->userData);
  } // end IF

  return;

continue_:
  // sanity check(s)
  if  (!sendRequestOnOffer_ ||
       (DHCP_Tools::type (data_r) != DHCP_Codes::DHCP_MESSAGE_OFFER))
    return; // done

  if (data_r.xid != session_data_r.xid)
    return; // done

  DataMessageType* message_p =
      inherited::allocateMessage ((*iterator_2).second.PDUSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                (*iterator_2).second.PDUSize));
    return;
  } // end IF
  struct DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = data_r.xid;
  DHCP_record.secs = data_r.secs;
  if (inherited::configuration_->protocolConfiguration->requestBroadcastReplies)
    DHCP_record.flags = DHCP_FLAGS_BROADCAST;
  if (!Net_Common_Tools::interfaceToMACAddress ((*iterator_2).second.socketHandlerConfiguration.socketConfiguration.networkInterface,
                                                DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Net_Common_Tools::interfaceToMACAddress(\"%s\"), returning\n"),
                inherited::mod_->name (),
                ACE_TEXT ((*iterator_2).second.socketHandlerConfiguration.socketConfiguration.networkInterface.c_str ())));

    // clean up
    message_p->release ();

    return;
  } // end IF
  std::string buffer_2;
  buffer_2.append (reinterpret_cast<const char*> (&data_r.yiaddr), 4);
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_REQUESTEDIPADDRESS,
                                              buffer_2));
  // *TODO*: support optional options:
  //         - 'overload'                (52)
  iterator =
      data_r.options.find (DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME);
  ACE_ASSERT (iterator != data_r.options.end ());
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME,
                                              (*iterator).second));
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                              std::string (1,
                                                           static_cast<char> (DHCP_Codes::DHCP_MESSAGE_REQUEST))));
  iterator =
      data_r.options.find (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER);
  ACE_ASSERT (iterator != data_r.options.end ());
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER,
                                              (*iterator).second));
  //         - 'parameter request list'  (55) [include in all subsequent messages]
  //         - 'message'                 (56)
  //         - 'maximum message size'    (57)
  //         - 'vendor class identifier' (60)
  //         - 'client identifier'       (61)
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  //  message_p->initialize (message_data_container_p,
  message_p->initialize (DHCP_record,
                         NULL);

  typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
    dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (session_data_r.broadcastConnection);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Net_ISocketConnection_T>(%@): \"%m\", returning\n"),
                inherited::mod_->name (),
                session_data_r.broadcastConnection));

    // clean up
    message_p->release ();

    return;
  } // end IF
  ACE_Message_Block* message_block_p = message_p;
  istream_connection_p->send (message_block_p);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
void
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (broadcastConnectionHandle_ == ACE_INVALID_HANDLE);

      // step1: retrieve a handle to the session data
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

      // step2: set up a (UDP) connection ?
      int result = -1;
      ConnectionManagerType* connection_manager_p =
        ConnectionManagerType::SINGLETON_T::instance ();
      ACE_ASSERT (connection_manager_p);
      bool clone_module, delete_module;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      bool reset_configuration = false;
      ACE_Time_Value deadline = ACE_Time_Value::zero;
      ACE_Time_Value timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT, 0);
      enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
      bool use_reactor = false;
      ConnectionConfigurationIteratorType iterator;
      bool write_only = false;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // *TODO*: remove type inferences
      if (inherited::configuration_->broadcastConnection)
      {
        // sanity check(s)
        ACE_ASSERT (!session_data_r.broadcastConnection);

        session_data_r.broadcastConnection =
            inherited::configuration_->broadcastConnection;

        goto continue_;
      } // end IF
      else if (session_data_r.broadcastConnection)
      {
        session_data_r.broadcastConnection->increase ();

        isSessionConnection_ = true;

        goto continue_;
      } // end ELSE IF
      else
      {
        // sanity check(s)
        ACE_ASSERT (inherited::configuration_->streamConfiguration);

        // *NOTE*: this means that this module is part of a connection
        //         processing stream
        //         --> grab a handle to the connection
        // *IMPORTANT NOTE*: Note how this can work only as long as at least
        //                   one upstream (or this) module operates
        //                   asynchronously
        // *TODO*: Note how the session ID is being reused for a particular
        //         purpose (in this case, to hold the socket handle of the
        //         connection). This will not work in some scenarios (e.g.
        //         when a connection handles several consecutive sessions,
        //         and/or each session needs a reference to its' own specific
        //         and/or 'unique' ID...)
        if (inherited::configuration_->streamConfiguration->sessionID)
        {
          // *NOTE*: this may have to wait for the connection to finish
          //         initializing
          // *TODO*: avoid tight loop here
          deadline = COMMON_TIME_NOW + timeout;
          do
          {
            if (!session_data_r.broadcastConnection)
              session_data_r.broadcastConnection =
                connection_manager_p->get (static_cast<Net_ConnectionId_t> (inherited::configuration_->streamConfiguration->sessionID));
            if (!session_data_r.broadcastConnection)
              continue;

            status = session_data_r.broadcastConnection->status ();
            if (status == NET_CONNECTION_STATUS_OK)
              break;
          } while (COMMON_TIME_NOW < deadline);
          if (!session_data_r.broadcastConnection)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to retrieve connection handle (handle was: %u), aborting\n"),
                        ACE_TEXT (inherited::mod_->name ()),
                        inherited::configuration_->streamConfiguration->sessionID));
            goto error;
          } // end IF

          goto continue_;
        } // end IF
      } // end ELSE

      // step2a: set up the (broadcast) connection configuration
      iterator =
        inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
      if (iterator == inherited::configuration_->connectionConfigurations->end ())
        iterator =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("%s: applying connection configuration\n"),
      //              inherited::mod_->name ()));
      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());

      // step2aa: set up the socket configuration
      // *TODO*: remove type inferences
      write_only =
        (*iterator).second.socketHandlerConfiguration.socketConfiguration.writeOnly;
      (*iterator).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
        true;

      // step2ab: set up the connection manager
      // *NOTE*: the stream configuration may contain a module handle that is
      //         meant to be the final module of 'this' processing stream. As
      //         the (possibly external) DHCP (broadcast) connection probably
      //         (!) is oblivious of 'this' pipeline, the connection probably
      //         (!) should not enqueue that same module again
      //         --> temporarily 'hide' the module handle, if any
      // *TODO*: this 'measure' diminishes the generic utility of this module
      //         --> to be removed ASAP
      // *TODO*: remove type inferences
      // sanity check(s)
      //ACE_ASSERT (inherited::configuration_->connectionConfiguration);
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      // *TODO*: remove type inferences
      clone_module =
          inherited::configuration_->streamConfiguration->cloneModule;
      delete_module =
          inherited::configuration_->streamConfiguration->deleteModule;
      module_p = inherited::configuration_->streamConfiguration->module;
      inherited::configuration_->streamConfiguration->cloneModule = false;
      inherited::configuration_->streamConfiguration->deleteModule = false;
      inherited::configuration_->streamConfiguration->module = NULL;
      reset_configuration = true;

      //connection_manager_p->set (*configuration_,
      //                           configuration_->userData);

      broadcastConnectionHandle_ =
          this->connect ((*iterator).second.socketHandlerConfiguration.socketConfiguration.address,
                         use_reactor);
      if (broadcastConnectionHandle_ == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString ((*iterator).second.socketHandlerConfiguration.socketConfiguration.address).c_str ())));
        goto error;
      } // end IF
      if (use_reactor)
        session_data_r.broadcastConnection =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (broadcastConnectionHandle_));
#else
            connection_manager_p->get (static_cast<Net_ConnectionId_t> (broadcastConnectionHandle_));
#endif
      else
        session_data_r.broadcastConnection =
            connection_manager_p->get ((*iterator).second.socketHandlerConfiguration.socketConfiguration.address);
      if (!session_data_r.broadcastConnection)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString ((*iterator).second.socketHandlerConfiguration.socketConfiguration.address).c_str ())));
        goto error;
      } // end IF

      // step2e: reset the connection configuration
      (*iterator).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
        write_only;
      inherited::configuration_->streamConfiguration->cloneModule = clone_module;
      inherited::configuration_->streamConfiguration->deleteModule = delete_module;
      inherited::configuration_->streamConfiguration->module = module_p;

      //connection_manager_p->set (*inherited::configuration_,
      //                           inherited::configuration_->userData);

continue_:
      break;

error:
      if (reset_configuration)
      {
        (*iterator).second.socketHandlerConfiguration.socketConfiguration.writeOnly =
          write_only;
        inherited::configuration_->streamConfiguration->cloneModule =
            clone_module;
        inherited::configuration_->streamConfiguration->deleteModule =
            delete_module;
        inherited::configuration_->streamConfiguration->module = module_p;

        //connection_manager_p->set (*configuration_,
        //                           configuration_->userData);
      } // end IF

      if (!session_data_r.broadcastConnection)
        goto continue_2;

      if (broadcastConnectionHandle_ != ACE_INVALID_HANDLE)
      {
        session_data_r.broadcastConnection->close ();
        session_data_r.broadcastConnection->decrease ();
        session_data_r.broadcastConnection = NULL;

        broadcastConnectionHandle_ = ACE_INVALID_HANDLE;
      } // end IF
      else
      {
        if (isSessionConnection_)
        {
          session_data_r.broadcastConnection->decrease ();

          isSessionConnection_ = false;
        } // end ELSE IF
        else
          session_data_r.broadcastConnection = NULL;
      } // end ELSE

continue_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

      if (!session_data_r.broadcastConnection)
        goto continue_3;

      if (broadcastConnectionHandle_ != ACE_INVALID_HANDLE)
      {
        // sanity check(s)
        ACE_ASSERT (session_data_r.broadcastConnection);

        session_data_r.broadcastConnection->close ();
        session_data_r.broadcastConnection->decrease ();
        session_data_r.broadcastConnection = NULL;

        broadcastConnectionHandle_ = ACE_INVALID_HANDLE;
      } // end IF
      else
      {
        if (isSessionConnection_)
        {
          // sanity check(s)
          ACE_ASSERT (session_data_r.broadcastConnection);

          session_data_r.broadcastConnection->decrease ();

          isSessionConnection_ = false;
        } // end ELSE IF
        else
          session_data_r.broadcastConnection = NULL;
      } // end ELSE

      if (connectionHandle_ != ACE_INVALID_HANDLE)
      {
        // sanity check(s)
        ACE_ASSERT (session_data_r.connection);

        session_data_r.connection->close ();
        session_data_r.connection->decrease ();
        session_data_r.connection = NULL;

        connectionHandle_ = ACE_INVALID_HANDLE;
      } // end IF

continue_3:
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorTypeBcast,
          typename ConnectorType>
ACE_HANDLE
DHCP_Module_Discover_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionConfigurationIteratorType,
                       ConnectionManagerType,
                       ConnectorTypeBcast,
                       ConnectorType>::connect (const ACE_INET_Addr& address_in,
                                                bool& useReactor_out)
{
  STREAM_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::connect"));

  // initialize return value(s)
  ACE_HANDLE result = ACE_INVALID_HANDLE;
  useReactor_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfigurations);

  // step0: set up a connector
  ConnectionManagerType* connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
  ACE_Time_Value deadline = ACE_Time_Value::zero;
  ACE_Time_Value timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT, 0);
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename ConnectorTypeBcast::ISTREAM_CONNECTION_T* istream_connection_p =
      NULL;
  typename ConnectorType::ISTREAM_CONNECTION_T* isocket_connection_2 = NULL;
  bool is_broadcast = (address_in.get_ip_address () == INADDR_BROADCAST);
  typename ConnectorType::ICONNECTOR_T* iconnector_p = NULL;
  ConnectionConfigurationIteratorType iterator =
    inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
  if (iterator == inherited::configuration_->connectionConfigurations->end ())
    iterator =
      inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
  //else
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: applying connection configuration\n"),
  //              inherited::mod_->name ()));
  ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());

  if (is_broadcast)
    ACE_NEW_NORETURN (iconnector_p,
                      ConnectorTypeBcast (connection_manager_p,
                                          (*iterator).second.socketHandlerConfiguration.statisticReportingInterval));
  else
    ACE_NEW_NORETURN (iconnector_p,
                      ConnectorType (connection_manager_p,
                                     (*iterator).second.socketHandlerConfiguration.statisticReportingInterval));
  if (!iconnector_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return ACE_INVALID_HANDLE;
  } // end IF
  if (!iconnector_p->initialize ((*iterator).second))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize connector, aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  // step1: connect
  result = iconnector_p->connect (address_in);
  if (result == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  if (iconnector_p->useReactor ())
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (result));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (result));
#endif
  else
  {
    // step2: wait for the connection to register with the manager
    // *TODO*: avoid tight loop here
    deadline = (COMMON_TIME_NOW +
                ACE_Time_Value (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT,
                                0));
    //result = ACE_OS::sleep (timeout);
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
    //              &timeout));
    do
    {
      iconnection_p = connection_manager_p->get (address_in);
      if (iconnection_p)
        break;
    } while (COMMON_TIME_NOW < deadline);
  } // end IF
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  // step3: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  deadline = COMMON_TIME_NOW + timeout;
  do
  {
    status = iconnection_p->status ();
    if (status == NET_CONNECTION_STATUS_OK) break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize connection to %s (status was: %d), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                status));
    goto error;
  } // end IF
  // step4: wait for the connection stream to finish initializing
  // *TODO*: if the connection processing stream includes (an instance of)
  //         this module, this will always time out
  if (is_broadcast)
  {
    istream_connection_p =
        dynamic_cast<typename ConnectorTypeBcast::ISTREAM_CONNECTION_T*> (iconnection_p);
    if (!istream_connection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<ConnectorTypeBcast::ISTREAM_CONNECTION_T>(0x%@), aborting\n"),
                  inherited::mod_->name (),
                  iconnection_p));
      goto error;
    } // end IF
    istream_connection_p->wait (STREAM_STATE_RUNNING,
                                NULL); // <-- block
  } // end IF
  else
  {
    isocket_connection_2 =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
    if (!isocket_connection_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T>(0x%@), aborting\n"),
                  inherited::mod_->name (),
                  iconnection_p));
      goto error;
    } // end IF
    isocket_connection_2->wait (STREAM_STATE_RUNNING,
                                NULL); // <-- block
  } // end ELSE
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: connected to %s (id: %u)...\n"),
              inherited::mod_->name (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
              iconnection_p->id ()));

  useReactor_out = iconnector_p->useReactor ();

  // clean up
  if (iconnector_p)
    delete iconnector_p;
  if (iconnection_p)
    iconnection_p->decrease ();

  return result;

error:
  if (iconnector_p)
    delete iconnector_p;
  if (iconnection_p)
    iconnection_p->decrease ();

  return ACE_INVALID_HANDLE;
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::DHCP_Module_DiscoverH_T (ISTREAM_T* stream_in,
                                                                          bool autoStart_in,
                                                                          bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              STREAM_HEADMODULECONCURRENCY_PASSIVE,
              generateSessionMessages_in)
 , sendRequestOnOffer_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::DHCP_Module_DiscoverH_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::~DHCP_Module_DiscoverH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::~DHCP_Module_DiscoverH_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::initialize"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::isInitialized_)
  {
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // retain session ID for reporting
      const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      // start profile timer...
      //profile_.start ();

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
DHCP_Module_DiscoverH_T<ACE_SYNCH_USE,
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
                        StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::collect"));

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//DHCP_Module_DiscoverH_T<LockType,
//                        ACE_SYNCH_USE,
//                        TimePolicyType,
//                        SessionMessageType,
//                        DataMessageType,
//                        ConfigurationType,
//                        StreamStateType,
//                        SessionDataType,
//                        SessionDataContainerType,
//                        StatisticContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return);
//}

//template <typename LockType,
//          ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//DHCP_Module_DiscoverH_T<LockType,
//                        ACE_SYNCH_USE,
//                        TimePolicyType,
//                        SessionMessageType,
//                        DataMessageType,
//                        ConfigurationType,
//                        StreamStateType,
//                        SessionDataType,
//                        SessionDataContainerType,
//                        StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
////  // step1: initialize session data
////  IRC_StreamSessionData* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    IRC_StreamSessionData ());
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
////    return false;
////  } // end IF
////  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  session_data_r.currentStatistic = statisticData_in;
//
////  // step2: allocate session data container
////  IRC_StreamSessionData_t* session_data_container_p = NULL;
////  // *NOTE*: fire-and-forget stream_session_data_p
////  ACE_NEW_NORETURN (session_data_container_p,
////                    IRC_StreamSessionData_t (stream_session_data_p,
////                                                    true));
////  if (!session_data_container_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//
////    // clean up
////    delete stream_session_data_p;
//
////    return false;
////  } // end IF
//
//  // step3: send the data downstream...
//  // *NOTE*: fire-and-forget session_data_container_p
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}
