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

#include <regex>
#include <string>

#include "ace/Log_Msg.h"

#include "common_math_tools.h"

#include "common_timer_manager_common.h"

#include "net_defines.h"
#include "net_macros.h"

#include "pop_common.h"
#include "pop_defines.h"
#include "pop_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType>
POP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   ConnectionStateType>::POP_Module_Send_T (ISTREAM_T* stream_in)
#else
                   ConnectionStateType>::POP_Module_Send_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , connection_ (NULL)
 , index_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Send_T::POP_Module_Send_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename ConnectionStateType>
POP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType>::~POP_Module_Send_T ()
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Send_T::~POP_Module_Send_T"));

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
POP_Module_Send_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   ControlMessageType,
                   DataMessageType,
                   SessionMessageType,
                   ConfigurationType,
                   ConnectionStateType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Send_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);
  ACE_ASSERT (inherited::configuration_->protocolConfiguration->parser);
  ACE_ASSERT (inherited::configuration_->request);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (connection_);
  ACE_ASSERT (message_inout->isInitialized ());

  const typename DataMessageType::DATA_T& data_container_r =
    message_inout->getR ();
  const typename DataMessageType::DATA_T::DATA_T& data_r =
    data_container_r.getR ();
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
  switch (state_r.protocolState)
  {
    case POP_STATE_INVALID:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      // --> greeting has been received; send AUTH command
      state_r.protocolState = POP_STATE_GREETING_RECEIVED;
      ++state_r.protocolState;

      // *TODO*: support more authentication mechanisms
      data_p->request.command = POP_Codes::POP_COMMAND_AUTH_USER;
      data_p->request.parameters.push_back (inherited::configuration_->protocolConfiguration->username);
      break;
    }
    case POP_STATE_AUTH_LOGIN_USER_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      // --> AUTH [LOGIN] username has been sent; send [LOGIN] password
      ++state_r.protocolState;

      data_p->request.command = POP_Codes::POP_COMMAND_AUTH_PASS;
      data_p->request.parameters.push_back (inherited::configuration_->protocolConfiguration->password);
      break;
    }
    case POP_STATE_AUTH_LOGIN_PASSWORD_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      // --> AUTH [LOGIN] password has been sent; send STAT
      state_r.protocolState = POP_STATE_TRANS_STAT_SENT;

      data_p->request.command = POP_Codes::POP_COMMAND_TRANS_STAT;
      break;
    }
    case POP_STATE_TRANS_STAT_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;
      ACE_ASSERT (!data_r.text.empty ());

      // process STAT response
      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR("^([[:digit:]]+) ([[:digit:]]+)$");
      std::regex regex (regex_string);
      std::smatch match_results;
      if (!std::regex_match (data_r.text[0],
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to parse STAT return value (was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (data_r.text[0].c_str ())));
        delete data_p; data_p = NULL;
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      std::istringstream converter;
      converter.str (match_results[1].str ());
      converter >> state_r.mailDropMessages;
      converter.clear ();
      converter.str (match_results[2].str ());
      unsigned int number_of_message_bytes = 0;
      converter >> number_of_message_bytes;

      if (!state_r.mailDropMessages)
      {
        // --> STAT has been sent; send QUIT
        state_r.protocolState = POP_STATE_TRANS_QUIT_SENT;

        data_p->request.command = POP_Codes::POP_COMMAND_TRANS_QUIT;

        break;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: maildrop consists of %u message(s) (%u byte(s))\n"),
                  inherited::mod_->name (),
                  state_r.mailDropMessages,
                  number_of_message_bytes));

      // --> STAT has been sent; send RETR(s)
      state_r.protocolState = POP_STATE_TRANS_RETR_SENT;

      data_p->request.command = POP_Codes::POP_COMMAND_TRANS_RETR;
      index_ = 1;
      std::ostringstream converter_2;
      converter_2 << index_;
      data_p->request.parameters.push_back (converter_2.str ());

      inherited::configuration_->protocolConfiguration->parser->expectMultiline (true);
      break;
    }
    case POP_STATE_TRANS_RETR_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      // process message(s) --> send downstream
      passMessageDownstream_out = true;

      std::ostringstream converter;
      ++index_;
      if (index_ > state_r.mailDropMessages)
      {
        // --> RETR(s) has/have been sent; send DELE(s)
        state_r.protocolState = POP_STATE_TRANS_DELE_SENT;

        data_p->request.command = POP_Codes::POP_COMMAND_TRANS_DELE;
        index_ = 1;
        converter << index_;
        data_p->request.parameters.push_back (converter.str ());
        inherited::configuration_->protocolConfiguration->parser->expectMultiline (false);
        break;
      } // end IF

      // --> RETR has/have been sent; retrieve next message(s)
      state_r.protocolState = POP_STATE_TRANS_RETR_SENT;

      data_p->request.command = POP_Codes::POP_COMMAND_TRANS_RETR;
      converter << index_;
      data_p->request.parameters.push_back (converter.str ());

      break;
    }
    case POP_STATE_TRANS_DELE_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      ++index_;
      if (index_ > state_r.mailDropMessages)
      {
        // --> DELE(s) has/have been sent; send QUIT
        state_r.protocolState = POP_STATE_TRANS_QUIT_SENT;

        data_p->request.command = POP_Codes::POP_COMMAND_TRANS_QUIT;

        break;
      } // end IF

      // --> DELE has been sent; delete next message(s)
      state_r.protocolState = POP_STATE_TRANS_DELE_SENT;

      std::ostringstream converter;
      converter << index_;
      data_p->request.parameters.push_back (converter.str ());

      break;
    }
    case POP_STATE_TRANS_QUIT_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      // --> QUIT has been sent --> UPDATE state --> send QUIT
      data_p->request.command = POP_Codes::POP_COMMAND_UPDAT_QUIT;
      return;
    }
    case POP_STATE_UPDAT_QUIT_SENT:
    {
      if (!POP_Tools::isSuccess (data_r.status[0]))
        goto protocol_error;

      //// --> QUIT has been sent
      //// close connection
      //ACE_ASSERT (connection_);
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: got QUIT reply, closing connection (id: %u)...\n"),
      //            inherited::mod_->name (),
      //            reinterpret_cast<unsigned int> (connection_->id ())));
      //connection_->abort ();

      delete data_p; data_p = NULL;
      return;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown protocol state (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (POP_Tools::StateToString (state_r.protocolState).c_str ())));
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

  if (!passMessageDownstream_out)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  return;

protocol_error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s[%s]: received error reply code:\n%s\n, returning\n"),
              inherited::mod_->name (),
              ACE_TEXT (POP_Tools::StateToString (state_r.protocolState).c_str ()),
              ACE_TEXT (POP_Tools::dump (data_r).c_str ())));
error:
  if (data_p)
    delete data_p;
  if (data_container_2)
    delete data_container_2;
  if (message_p)
    message_p->release ();
  if (!passMessageDownstream_out)
  {
    message_inout->release (); message_inout = NULL;
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
POP_Module_Send_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ConfigurationType,
                       ConnectionStateType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Send_T::handleSessionMessage"));

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
