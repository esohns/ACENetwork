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

#include "common_timer_manager_common.h"

#include "net_defines.h"
#include "net_macros.h"

#include "smtp_common.h"
#include "smtp_defines.h"
#include "smtp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename ConnectionType>
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   ConnectionType>::SMTP_Module_Send_T (ISTREAM_T* stream_in)
#else
                   ConnectionType>::SMTP_Module_Send_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::SMTP_Module_Send_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename ConnectionType>
bool
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType,
                   ConnectionType>::initialize (const ConfigurationType& configuration_in,
                                                Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.protocolConfiguration);

  if (unlikely (inherited::isInitialized_))
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
          typename ConnectionStateType,
          typename ConnectionType>
void
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType,
                   ConnectionType>::handleDataMessage (DataMessageType*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (message_inout->isInitialized ());

  const typename DataMessageType::DATA_T& data_r = message_inout->getR ();

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  int result = -1;
  struct SMTP_Record record_s;
  ACE_Message_Block* message_block_p = NULL;
  DataMessageType* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (session_data_r.connection);

  message_p = inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    return;
  } // end IF

  ConnectionStateType& state_r =
    const_cast<ConnectionStateType&> (session_data_r.connection->state ());
continue_:
  switch (state_r.protocolState)
  {
    case SMTP_STATE_INVALID:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> greeting has been received; send MAIL command
      state_r.protocolState = SMTP_STATE_GREETING_RECEIVED;
      ++state_r.protocolState;

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_EHLO;
      record_s.request.address = inherited::configuration_->request.address;
      break;
    }
    case SMTP_STATE_EHLO_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> EHLO has been sent; send MAIL
      ++state_r.protocolState;

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_MAIL;
      record_s.request.from = inherited::configuration_->request.from;
      record_s.request.parameters = inherited::configuration_->request.parameters;
      break;
    }
    case SMTP_STATE_MAIL_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> MAIL has been sent; send RCPT(s)
      ++state_r.protocolState;

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_RCPT;
      ACE_ASSERT (!inherited::configuration_->request.to.empty ());
      record_s.request.to.push_back (inherited::configuration_->request.to.front ());
      record_s.request.parameters = inherited::configuration_->request.parameters;
      inherited::configuration_->request.to.pop_front ();
      break;
    }
    case SMTP_STATE_RCPT_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> RCPT has been sent; send any other RCPT(s)

      if (inherited::configuration_->request.to.empty ())
      {
        // --> RCPT(s) have been sent; send DATA

        ++state_r.protocolState;
        goto continue_;
      } // end IF

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_RCPT;
      record_s.request.to.push_back (inherited::configuration_->request.to.front ());
      record_s.request.parameters = inherited::configuration_->request.parameters;
      inherited::configuration_->request.to.pop_front ();
      break;
    }
    case SMTP_STATE_RCPTS_SENT:
    {
      // --> RCPT(s) have been sent; send DATA
      ++state_r.protocolState;

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_DATA;
      record_s.request.data = inherited::configuration_->request.data;
      break;
    }
    case SMTP_STATE_DATA_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> DATA has been sent; send QUIT
      ++state_r.protocolState;

      record_s.request.command = SMTP_Codes::SMTP_COMMAND_QUIT;
      break;
    }
    case SMTP_STATE_QUIT_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> QUIT has been sent; reset state
      state_r.protocolState = SMTP_STATE_INVALID;
      message_p->release (); message_p = NULL;
      return;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown protocol state (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (SMTP_Tools::StateToString (state_r.protocolState).c_str ())));
      message_p->release (); message_p = NULL;
      return;
    }
  } // end SWITCH

//send:
  message_p->initialize (record_s,
                         message_p->id (),
                         NULL);

  message_block_p = message_p;
  result = inherited::reply (message_block_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::reply(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    --state_r.protocolState; // reset state
    return;
  } // end IF

  return;

protocol_error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s[%s]: received error reply code (was: \"%s\"):\n%s\n, returning\n"),
              inherited::mod_->name (),
              ACE_TEXT (SMTP_Tools::StateToString (state_r.protocolState).c_str ()),
              ACE_TEXT (SMTP_Tools::CodeToString (data_r.code).c_str ()),
              ACE_TEXT (SMTP_Tools::dump (data_r).c_str ())));
  message_p->release (); message_p = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename ConnectionType>
void
SMTP_Module_Send_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionStateType,
                       ConnectionType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      //ConnectionManagerType* connection_manager_p =
      //  ConnectionManagerType::SINGLETON_T::instance ();
      //ACE_ASSERT (connection_manager_p);
      //bool clone_module = false;//, delete_module = false;
      //typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      //bool reset_configuration = false;
      //ACE_Time_Value deadline = ACE_Time_Value::zero;
      //ACE_Time_Value timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S, 0);
      //enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;

      if (connection_)
        goto continue_;
      // *TODO*: remove type inferences
      if (session_data_r.connection)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: using session connection\n"),
                    inherited::mod_->name ()));

        connection_ = session_data_r.connection;
        connection_->increase ();
        goto continue_;
      } // end IF

//      // step2a: set up the (broadcast) connection configuration
//      iterator =
//        inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
//      if (iterator == inherited::configuration_->connectionConfigurations->end ())
//        iterator =
//          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR ("In"));
//      else
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: applying connection configuration\n"),
//                    inherited::mod_->name ()));
//      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());

//      // step2aa: set up the socket configuration
//      write_only = (*iterator).second->writeOnly;
//      socket_configuration_p->writeOnly = true;

//      // step2ab: set up the connection manager
//      // *NOTE*: the stream configuration may contain a module handle that is
//      //         meant to be the final module of 'this' processing stream. As
//      //         the (possibly external) DHCP (broadcast) connection probably
//      //         (!) is oblivious of 'this' pipeline, the connection probably
//      //         (!) should not enqueue that same module again
//      //         --> temporarily 'hide' the module handle, if any
//      // *TODO*: this 'measure' diminishes the generic utility of this module
//      //         --> to be removed ASAP
//      // *TODO*: remove type inferences
//      // sanity check(s)
//      //ACE_ASSERT (inherited::configuration_->connectionConfiguration);
//      ACE_ASSERT (inherited::configuration_->streamConfiguration);

//      // *TODO*: remove type inferences
//      clone_module =
//          inherited::configuration_->streamConfiguration->configuration->cloneModule;
//      delete_module =
//          inherited::configuration_->streamConfiguration->configuration->deleteModule;
//      module_p =
//        inherited::configuration_->streamConfiguration->configuration->module;
//      inherited::configuration_->streamConfiguration->configuration->cloneModule =
//        false;
//      inherited::configuration_->streamConfiguration->configuration->deleteModule =
//        false;
//      inherited::configuration_->streamConfiguration->configuration->module =
//        NULL;
//      reset_configuration = true;

//      //connection_manager_p->set (*configuration_,
//      //                           configuration_->userData);

//      handle_ =
//          this->connect (socket_configuration_p->peerAddress,
//                         use_reactor);
//      if (unlikely (broadcastConnectionHandle_ == ACE_INVALID_HANDLE))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Net_Common_Tools::IPAddressToString (socket_configuration_p->peerAddress).c_str ())));
//        goto error;
//      } // end IF
//      if (use_reactor)
//        session_data_r.connection =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//            connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle_));
//#else
//            connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_));
//#endif
//      else
//        session_data_r.connection =
//            connection_manager_p->get (socket_configuration_p->peerAddress);
//      if (unlikely (!session_data_r.connection))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Net_Common_Tools::IPAddressToString (socket_configuration_p->peerAddress).c_str ())));
//        goto error;
//      } // end IF

//      // step2e: reset the connection configuration
//      socket_configuration_p->writeOnly = write_only;
//      inherited::configuration_->streamConfiguration->configuration->cloneModule =
//        clone_module;
//      inherited::configuration_->streamConfiguration->configuration->deleteModule =
//        delete_module;
//      inherited::configuration_->streamConfiguration->configuration->module =
//        module_p;

      //connection_manager_p->set (*inherited::configuration_,
      //                           inherited::configuration_->userData);

continue_:
      break;

//error:
//      if (reset_configuration)
//      {
////        (*iterator_2).second->writeOnly = write_only;
//        inherited::configuration_->streamConfiguration->configuration->cloneModule =
//          clone_module;
//        inherited::configuration_->streamConfiguration->configuration->module =
//          module_p;
//
//        //connection_manager_p->set (*configuration_,
//        //                           configuration_->userData);
//      } // end IF

//continue_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (connection_)
      {
        connection_->decrease (); connection_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
