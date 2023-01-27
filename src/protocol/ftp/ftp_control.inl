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

#include "ace/Log_Msg.h"

#include "net_defines.h"
#include "net_macros.h"

#include "net_client_common_tools.h"

#include "ftp_iparser.h"
#include "ftp_tools.h"

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::FTP_Control_T (enum Common_EventDispatchType dispatch_in,
                                            const typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T& connectionConfiguration_in,
                                            const typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T& connectionConfiguration2_in,
                                            const struct FTP_LoginOptions& loginOptions_in)
 : connectionConfiguration_ (&const_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T&> (connectionConfiguration_in))
 , connectionConfiguration_2 (&const_cast<typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T&> (connectionConfiguration2_in))
 , controlConnection_ (ACE_INVALID_HANDLE)
 , dispatch_ (dispatch_in)
 , loginOptions_ (loginOptions_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::FTP_Control_T"));

}

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
ACE_HANDLE
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::connectControl ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::connectControl"));

  // sanity check(s)
  ACE_ASSERT (connectionConfiguration_);

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  ControlConnectorType connector (true);
  ControlAsynchConnectorType asynch_connector (true);
  UserDataType user_data;
  if (dispatch_ == COMMON_EVENT_DISPATCH_REACTOR)
    handle_h =
      Net_Client_Common_Tools::connect<ControlConnectorType> (connector,
                                                       *connectionConfiguration_,
                                                       user_data,
                                                       connectionConfiguration_->socketConfiguration.address,
                                                       true,  // wait ?
                                                       true); // is peer address ?
  else
    handle_h =
      Net_Client_Common_Tools::connect<ControlAsynchConnectorType> (asynch_connector,
                                                             *connectionConfiguration_,
                                                             user_data,
                                                             connectionConfiguration_->socketConfiguration.address,
                                                             true,  // wait ?
                                                             true); // is peer address ?
  if (unlikely (handle_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_->socketConfiguration.address).c_str ())));
    return 0;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened TCP socket: %s\n"),
              handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened TCP socket: %s\n"),
              handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (controlConnection_ == ACE_INVALID_HANDLE);

  controlConnection_ = handle_h;

  return handle_h;
}

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
ACE_HANDLE
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::connectData ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::connectData"));

    // sanity check(s)
  ACE_ASSERT (connectionConfiguration_2);

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  DataConnectorType connector (true);
  DataAsynchConnectorType asynch_connector (true);
  UserDataType user_data;
  if (dispatch_ == COMMON_EVENT_DISPATCH_REACTOR)
    handle_h =
      Net_Client_Common_Tools::connect<DataConnectorType> (connector,
                                                           *connectionConfiguration_2,
                                                           user_data,
                                                           connectionConfiguration_2->socketConfiguration.address,
                                                           true,  // wait ?
                                                           true); // is peer address ?
  else
    handle_h =
      Net_Client_Common_Tools::connect<DataAsynchConnectorType> (asynch_connector,
                                                                 *connectionConfiguration_2,
                                                                 user_data,
                                                                 connectionConfiguration_2->socketConfiguration.address,
                                                                 true,  // wait ?
                                                                 true); // is peer address ?
  if (unlikely (handle_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_2->socketConfiguration.address).c_str ())));
    return 0;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened TCP socket: %s\n"),
              handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_2->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened TCP socket: %s\n"),
              handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_2->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  return handle_h;
}

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
void
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::request (const struct FTP_Request& request_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::request"));

  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;

  // step4: send request
  if (unlikely (!getControlConnectionAndMessage (istream_connection_p,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_Control_T::getControlConnectionAndMessage(), aborting\n")));
    goto error;
  } // end IF
  data_container_p =
    &const_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
    &const_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
  data_p->request = request_in;

  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  message_block_p = message_p;
  try {
    istream_connection_p->send (message_block_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IStreamConnection_T::send(), returning\n")));
    goto error;
  }
  message_p = NULL;

  // step5: clean up
  istream_connection_p->decrease ();

  return;

error:
  if (istream_connection_p)
    istream_connection_p->decrease ();
  if (message_p)
    message_p->release ();
}

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
void
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::responseCB (const struct FTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::responseCB"));

  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  struct FTP_Request request_s;

  switch (record_in.code)
  {
    case FTP_Codes::FTP_CODE_USER_OK_NEED_PASSWORD:
    {
      if (unlikely (!getControlConnectionAndMessage (istream_connection_p,
                                                     message_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to FTP_Control_T::getControlConnectionAndMessage(), returning\n")));
        return;
      } // end IF
      data_container_p =
        &const_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
      data_p =
        &const_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
      data_p->request.command = FTP_Codes::CommandType::FTP_COMMAND_PASS;
      data_p->request.parameters.push_back (loginOptions_.password);

      message_block_p = message_p;
      typedef typename ControlAsynchConnectorType::CONNECTION_MANAGER_T::SINGLETON_T CONNECTION_MANAGER_SINGLETON;
      typename ControlAsynchConnectorType::CONNECTION_MANAGER_T* connection_manager_p =
        CONNECTION_MANAGER_SINGLETON::instance ();
      ACE_ASSERT (connection_manager_p);
      typename ControlAsynchConnectorType::CONNECTION_MANAGER_T::ICONNECTION_T* iconnection_p =
        connection_manager_p->get (controlConnection_);
      ACE_ASSERT (iconnection_p);
      typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
      ACE_ASSERT (istream_connection_p);
      istream_connection_p->send (message_block_p);
      message_block_p = NULL;
      iconnection_p->decrease (); iconnection_p = NULL;

      goto default_;
    }
    case FTP_Codes::FTP_CODE_ENTERING_PASSIVE_MODE:
    {
      // parse ip address and port
      ACE_ASSERT (!record_in.text.empty ());
      connectionConfiguration_2->socketConfiguration.address =
        FTP_Tools::parsePASVResponse (record_in.text.front ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("PASV mode response: server is listening at \"%s\"\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_2->socketConfiguration.address).c_str ())));

      // connect to PASV server address
      ACE_HANDLE handle_h = connectData ();
      if (unlikely (!handle_h))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to %s, returning\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (connectionConfiguration_2->socketConfiguration.address).c_str ())));
        return;
      } // end IF

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, lock_);
        ACE_ASSERT (!queue_.empty ());
        request_s = queue_.front ();
        queue_.pop_front ();
      } // end lock scope

      // set data parser state
      typedef typename DataAsynchConnectorType::CONNECTION_MANAGER_T::SINGLETON_T CONNECTION_MANAGER_2_SINGLETON;
      typename DataAsynchConnectorType::CONNECTION_MANAGER_T* connection_manager_p =
        CONNECTION_MANAGER_2_SINGLETON::instance ();
      ACE_ASSERT (connection_manager_p);
      typename DataAsynchConnectorType::CONNECTION_MANAGER_T::ICONNECTION_T* iconnection_p =
        connection_manager_p->get (handle_h);
      ACE_ASSERT (iconnection_p);
      typename DataAsynchConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename DataAsynchConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
      ACE_ASSERT (istream_connection_p);
      typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T& stream_r =
        const_cast<typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T&> (istream_connection_p->stream ());
      Stream_Module_t* module_p =
        const_cast<Stream_Module_t*> (stream_r.find (ACE_TEXT_ALWAYS_CHAR (FTP_DEFAULT_MODULE_PARSER_DATA_NAME_STRING), false, false));
      ACE_ASSERT (module_p);
      FTP_IParserData* iparser_data_p =
        dynamic_cast<FTP_IParserData*> (module_p->writer ());
      ACE_ASSERT (iparser_data_p);
      iparser_data_p->state (request_s.command == FTP_Codes::FTP_COMMAND_LIST ? (request_s.is_directory_list ? FTP_STATE_DATA_LIST_DIRECTORY
                                                                                                             : FTP_STATE_DATA_LIST_FILE)
                                                                              : FTP_STATE_DATA_DATA);

      goto default_;
    }
    default:
    {
default_:
      std::string text_string = FTP_Tools::CodeToString (record_in.code);
      text_string += ACE_TEXT_ALWAYS_CHAR (": ");
      for (FTP_TextConstIterator_t iterator_2 = record_in.text.begin ();
           iterator_2 != record_in.text.end ();
           ++iterator_2)
        text_string += *iterator_2;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received response: \"%s\"\n"),
                  ACE_TEXT (text_string.c_str ())));

      break;
    }
  } // end SWITCH

  if (request_s.command != FTP_Codes::FTP_COMMAND_INVALID)
    request (request_s);
}

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType,
          typename DataConnectorType,
          typename UserDataType>
bool
FTP_Control_T<ControlAsynchConnectorType,
              ControlConnectorType,
              DataAsynchConnectorType,
              DataConnectorType,
              UserDataType>::getControlConnectionAndMessage (typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T*& connection_out,
                                                             typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::getControlConnectionAndMessage"));

  // sanity check(s)
  ACE_ASSERT (connectionConfiguration_);
  ACE_ASSERT (controlConnection_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (!connection_out);
  ACE_ASSERT (!message_out);

  const typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T* stream_p =
    NULL;
  const typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  const typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::SESSION_DATA_T* session_data_p =
    NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
    NULL;
  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
    NULL;
  Net_ConnectionId_t tracker_connection_id = 0;
  typename ControlAsynchConnectorType::ICONNECTION_T* iconnection_p = NULL;
  unsigned int buffer_size_i = 0;

  ACE_NEW_NORETURN (data_p,
                    typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (data_p,
                                                                                                     true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (!data_p);

  ACE_ASSERT (connectionConfiguration_->allocatorConfiguration);
  ACE_ASSERT (connectionConfiguration_->messageAllocator);
  buffer_size_i =
    connectionConfiguration_->allocatorConfiguration->defaultBufferSize +
    connectionConfiguration_->allocatorConfiguration->paddingBytes;

allocate:
  message_out =
    static_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T*> (connectionConfiguration_->messageAllocator->malloc (buffer_size_i));
  // keep retrying ?
  if (!message_out &&
      !connectionConfiguration_->messageAllocator->block ())
    goto allocate;
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate request message: \"%m\", returning\n")));
    data_container_p->decrease (); data_container_p = NULL;
    goto error;
  } // end IF

  typedef typename ControlAsynchConnectorType::CONNECTION_MANAGER_T::SINGLETON_T CONNECTION_MANAGER_SINGLETON_2;
  iconnection_p =
    CONNECTION_MANAGER_SINGLETON_2::instance ()->get (controlConnection_);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve control connection handle, returning\n")));
    goto error;
  } // end IF
  connection_out =
    dynamic_cast<typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!connection_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    iconnection_p->decrease (); iconnection_p = NULL;
    goto error;
  } // end IF

  stream_p = &connection_out->stream ();
  session_data_container_p = &stream_p->getR_2 ();
  session_data_p = &session_data_container_p->getR ();

  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_out->initialize (data_container_p,
                           session_data_p->sessionId,
                           NULL);
  ACE_ASSERT (!data_container_p);

  return true;

error:
  if (message_out)
  {
    message_out->release (); message_out = NULL;
  } // end IF
  if (data_container_p)
    data_container_p->decrease ();
  if (data_p)
    delete data_p;
  if (connection_out)
  {
    connection_out->decrease (); connection_out = NULL;
  } // end IF

  return false;
}
