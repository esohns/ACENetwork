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

#include "common_math_tools.h"

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
          typename ConnectionStateType>
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   ConnectionStateType>::SMTP_Module_Send_T (ISTREAM_T* stream_in)
#else
                   ConnectionStateType>::SMTP_Module_Send_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , connection_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::SMTP_Module_Send_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType>
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType>::~SMTP_Module_Send_T ()
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::~SMTP_Module_Send_T"));

  if (unlikely (connection_))
  {
    connection_->decrease (); connection_ = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType>
void
SMTP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);
  ACE_ASSERT (inherited::configuration_->request);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (connection_);
  ACE_ASSERT (message_inout->isInitialized ());

  const typename DataMessageType::DATA_T& data_container_r =
    message_inout->getR ();
  const typename DataMessageType::DATA_T::DATA_T& data_r =
    data_container_r.getR ();

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  int result = -1;
  DataMessageType* message_p = NULL;
  typename DataMessageType::DATA_T* data_container_2 = NULL;
  typename DataMessageType::DATA_T::DATA_T* data_p = NULL;

  ACE_NEW_NORETURN (data_p,
                    typename DataMessageType::DATA_T::DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  ConnectionStateType& state_r =
    const_cast<ConnectionStateType&> (connection_->state ());
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

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_EHLO;
      data_p->request.domain =
        inherited::configuration_->protocolConfiguration->domain;
      break;
    }
    case SMTP_STATE_EHLO_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> EHLO has been sent; send AUTH/MAIL ?
      ++state_r.protocolState;

      // retrieve any supported authentication mechanisms
      std::vector<std::string> authentication_mechanisms_a;
      std::string authentication_mechanisms;
      for (SMTP_TextConstIterator_t iterator = data_r.text.begin ();
           iterator != data_r.text.end ();
           ++iterator)
      {
        if ((*iterator).find (ACE_TEXT_ALWAYS_CHAR ("AUTH ")))
          continue;
        authentication_mechanisms = *iterator;
        authentication_mechanisms.erase (0, 5);
        std::istringstream string_stream (authentication_mechanisms);
        std::string authentication_mechanism;
        while (!string_stream.eof ())
        {
          std::getline (string_stream, authentication_mechanism, ' ');
          authentication_mechanisms_a.push_back (authentication_mechanism);
        } // end WHILE
        break;
      } // end FOR
      if (authentication_mechanisms_a.empty ())
        goto no_authentication_;

      // try AUTH LOGIN ?
      if (std::find (authentication_mechanisms_a.begin (),
                     authentication_mechanisms_a.end (),
                     ACE_TEXT_ALWAYS_CHAR ("LOGIN")) != authentication_mechanisms_a.end ())
      {
        // --> send AUTH [LOGIN]
        // *TODO*: support more authentication mechanisms
        data_p->request.command = SMTP_Codes::SMTP_COMMAND_AUTH;
        break;
      } // end IF
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: no supported authentication mechanisms (was: %s), continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (authentication_mechanisms.c_str ())));

no_authentication_:
      // proceed without authentication
      state_r.protocolState = SMTP_STATE_MAIL_SENT;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_MAIL;
      data_p->request.from = inherited::configuration_->request->from;
      data_p->request.parameters = inherited::configuration_->request->parameters;
      break;
    }
    case SMTP_STATE_AUTH_SENT:
    {
      if (data_r.code != SMTP_Codes::SMTP_CODE_SERVER_CHALLENGE)
        goto protocol_error;

      // --> AUTH has been sent; send [LOGIN] username
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_AUTH;
      data_p->request.parameters.push_back (Common_Math_Tools::encodeBase64 (inherited::configuration_->protocolConfiguration->username.c_str (),
                                                                             inherited::configuration_->protocolConfiguration->username.size ()));
      break;
    }
    case SMTP_STATE_AUTH_LOGIN_USER_SENT:
    {
      if (data_r.code != SMTP_Codes::SMTP_CODE_SERVER_CHALLENGE)
        goto protocol_error;

      // --> AUTH [LOGIN] username has been sent; send [LOGIN] password
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_AUTH;
      data_p->request.parameters.push_back (Common_Math_Tools::encodeBase64 (inherited::configuration_->protocolConfiguration->password.c_str (),
                                                                             inherited::configuration_->protocolConfiguration->password.size ()));
      break;
    }
    case SMTP_STATE_AUTH_LOGIN_PASSWORD_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> AUTH [LOGIN] password has been sent; send MAIL
      ++state_r.protocolState; ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_MAIL;
      data_p->request.from = inherited::configuration_->request->from;
      data_p->request.parameters = inherited::configuration_->request->parameters;
      break;
    }
    case SMTP_STATE_MAIL_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> MAIL has been sent; send RCPT(s)
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_RCPT;
      ACE_ASSERT (!inherited::configuration_->request->to.empty ());
      data_p->request.to.push_back (inherited::configuration_->request->to.front ());
      data_p->request.parameters = inherited::configuration_->request->parameters;
      inherited::configuration_->request->to.erase (inherited::configuration_->request->to.begin ());
      break;
    }
    case SMTP_STATE_RCPT_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> RCPT has been sent; send any other RCPT(s)

      if (inherited::configuration_->request->to.empty ())
      {
        // --> RCPT(s) have been sent; send DATA

        ++state_r.protocolState;
        goto continue_;
      } // end IF

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_RCPT;
      data_p->request.to.push_back (inherited::configuration_->request->to.front ());
      data_p->request.parameters = inherited::configuration_->request->parameters;
      inherited::configuration_->request->to.erase (inherited::configuration_->request->to.begin ());
      break;
    }
    case SMTP_STATE_RCPTS_SENT:
    {
      // --> RCPT(s) have been sent; send DATA
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_DATA;
      break;
    }
    case SMTP_STATE_DATA_SENT:
    {
      if (data_r.code != SMTP_Codes::SMTP_CODE_START_MAIL_INPUT)
        goto protocol_error;

      // --> DATA has been sent; send message data
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_DATA_2;
      data_p->request.data = inherited::configuration_->request->data;
      break;
    }
    case SMTP_STATE_DATA_2_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> message has been sent; send QUIT
      ++state_r.protocolState;

      data_p->request.command = SMTP_Codes::SMTP_COMMAND_QUIT;
      break;
    }
    case SMTP_STATE_QUIT_SENT:
    {
      if (!SMTP_Tools::isSuccess (data_r.code))
        goto protocol_error;

      // --> QUIT has been sent; reset state
      state_r.protocolState = SMTP_STATE_INVALID;
      delete data_p; data_p = NULL;
      return;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown protocol state (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (SMTP_Tools::StateToString (state_r.protocolState).c_str ())));
      delete data_p; data_p = NULL;
      return;
    }
  } // end SWITCH

//send:
  message_p = inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    --state_r.protocolState; // reset state
    goto error;
  } // end IF

  ACE_NEW_NORETURN (data_container_2,
                    typename DataMessageType::DATA_T ());
  if (!data_container_2)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                inherited::mod_->name ()));
    --state_r.protocolState; // reset state
    goto error;
  } // end IF
  data_container_2->setPR (data_p);
  ACE_ASSERT (!data_p);
  message_p->initialize (data_container_2,
                         message_p->id (),
                         NULL);
  ACE_ASSERT (!data_container_2);

  result = inherited::reply (message_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::reply(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    --state_r.protocolState; // reset state
    goto error;
  } // end IF

  return;

protocol_error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s[%s]: received error reply code (was: \"%s\"):\n%s\n, returning\n"),
              inherited::mod_->name (),
              ACE_TEXT (SMTP_Tools::StateToString (state_r.protocolState).c_str ()),
              ACE_TEXT (SMTP_Tools::CodeToString (data_r.code).c_str ()),
              ACE_TEXT (SMTP_Tools::dump (data_r).c_str ())));
error:
  if (data_p)
    delete data_p;
  if (data_container_2)
    delete data_container_2;
  if (message_p)
    message_p->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType>
void
SMTP_Module_Send_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionStateType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Send_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    { ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      // sanity check(s)
      ACE_ASSERT (session_data_r.connection);
      ACE_ASSERT (!connection_);

      connection_ = session_data_r.connection;
      connection_->increase ();
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (connection_))
      {
        connection_->decrease (); connection_ = NULL;
      } // end IF
      break;
    }
    default:
      break;
  } // end SWITCH
}
