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

#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename StateType>
BitTorrent_Session_T<ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     StateType>::BitTorrent_Session_T (ConnectionManagerType* interfaceHandle_in,
                                                       bool asynchronous_in)
 : inherited (interfaceHandle_in,
              asynchronous_in)
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

}

template <typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename StateType>
BitTorrent_Session_T<ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     StateType>::~BitTorrent_Session_T ()
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

template <typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename StateType>
void
BitTorrent_Session_T<ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     StateType>::notify (const struct BitTorrent_Record& record_in,
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

template <typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename StateType>
void
BitTorrent_Session_T<ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     StateType>::error (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::error"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename StateType>
void
BitTorrent_Session_T<ConfigurationType,
                     ConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     ConnectionManagerType,
                     StateType>::log (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  log (std::string (), // --> server log
       message_text);
}
