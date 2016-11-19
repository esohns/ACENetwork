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
#include <ace/Log_Msg.h>

#include "net_macros.h"

#include "http_tools.h"

#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::BitTorrent_Session_T (const HandlerConfigurationType& configuration_in,
                                                          ConnectionManagerType* interfaceHandle_in,
                                                          bool asynchronous_in)
 : inherited (configuration_in,
              interfaceHandle_in,
              asynchronous_in)
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

}

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::~BitTorrent_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::~BitTorrent_Session_T"));

//  int result = -1;

//  if (close_)
//  {
    //result = file_.close ();
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_FILE_Stream::close(): \"%m\", continuing\n")));
//    result = output_.close ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to BitTorrent_Client_IOStream_t::close(): \"%m\", continuing\n")));
//  } // end IF
}

//////////////////////////////////////////

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
void
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::trackerConnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  TrackerConnectorType connector (inherited::connectionManager_,
                                  ACE_Time_Value::zero);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // debug info
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = address_in.addr_to_string (buffer,
                                          sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // step0: retrive default configuration
  ConfigurationType* configuration_p = NULL;
  UserDataType* user_data_p = NULL;
  bool clone_module = false;
  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  if (inherited::connectionManager_)
  {
    inherited::connectionManager_->get (configuration_p,
                                        user_data_p);
    ACE_ASSERT (user_data_p);
    // *TODO*: remove type inferences
    ACE_ASSERT (user_data_p->configuration);

    // step1: set up configuration
    user_data_p->configuration->socketConfiguration.address = address_in;

    clone_module = user_data_p->configuration->streamConfiguration.cloneModule;
    delete_module =
        user_data_p->configuration->streamConfiguration.deleteModule;
    module_p = user_data_p->configuration->streamConfiguration.module;

    user_data_p->configuration->streamConfiguration.cloneModule = true;
    user_data_p->configuration->streamConfiguration.deleteModule = false;
    user_data_p->configuration->streamConfiguration.module =
        user_data_p->configuration->streamConfiguration.trackerModule;
  } // end IF

  // step1: initialize connector
  typename TrackerConnectorType::ICONNECTOR_T* iconnector_p = &connector;
  ACE_ASSERT (inherited::configuration_);
  if (!iconnector_p->initialize (*inherited::configuration_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    goto clean;
  } // end IF

  // step2: try to connect
  handle = iconnector_p->connect (address_in);
  if (inherited::isAsynch_)
  {

  } // end ELSE
  else
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));
    goto clean;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to \"%s\"...\n"),
              buffer));

clean:
  if (inherited::connectionManager_)
  {
    user_data_p->configuration->streamConfiguration.cloneModule = clone_module;
    user_data_p->configuration->streamConfiguration.deleteModule =
        delete_module;
    user_data_p->configuration->streamConfiguration.module = module_p;
  } // end IF
};

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
void
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::notify (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (HTTP_Tools::dump (record_in).c_str ())));
}
template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
void
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::notify (const struct BitTorrent_Record& record_in,
                                            ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_PIECE:
    {
      // sanity check(s)
      ACE_ASSERT (messageBlock_in);

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), returning\n"),
                  record_in.type));
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
void
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::error (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::error"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
void
BitTorrent_Session_T<HandlerConfigurationType,
                     ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     StateType,
                     UserDataType>::log (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  log (std::string (), // --> server log
       message_text);
}
