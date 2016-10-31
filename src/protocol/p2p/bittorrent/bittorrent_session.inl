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
#include <ace/Assert.h>
#include <ace/FILE_Addr.h>
#include <ace/FILE_Connector.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/OS_Memory.h>

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_macros.h"

#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::BitTorrent_Session_T (PeerConnectionManagerType* interfaceHandle_in,
                                                                          TrackerConnectionManagerType* interfaceHandle_2_in)
 : lock_ ()
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

}

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::~BitTorrent_Session_T ()
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
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::trackerConnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  ACE_UNUSED_ARG (address_in);
}

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::trackerDisconnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerDisconnect"));

  ACE_UNUSED_ARG (address_in);
}

//////////////////////////////////////////

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::peerConnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::peerConnect"));

  ACE_UNUSED_ARG (address_in);
}

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::peerDisconnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::peerDisconnect"));

  ACE_UNUSED_ARG (address_in);
}

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::notify (const struct BitTorrent_Record& record_in,
                                                            ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_PIECE:
    {
      // sanity check(s)
      ACE_ASSERT (messageBlock_in);

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

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

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::error (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::error"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType,
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
void
BitTorrent_Session_T<ConfigurationType,
                     StateType,
                     StreamType,
                     StreamStatusType,
                     SessionStateType,
                     PeerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionType,
                     TrackerConnectionManagerType>::log (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  log (std::string (), // --> server log
       message_text);
}
