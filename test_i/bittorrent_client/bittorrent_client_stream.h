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

#ifndef BITTORRENT_CLIENT_STREAM_H
#define BITTORRENT_CLIENT_STREAM_H

#include <string>

#include <ace/Global_Macros.h>

#include "bittorrent_common.h"
#include "bittorrent_message.h"
#include "bittorrent_stream.h"
#include "bittorrent_stream_common.h"

#include "bittorrent_client_common_modules.h"
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_sessionmessage.h"
#include "bittorrent_client_stream_common.h"

class BitTorrent_Client_Stream
 : public BitTorrent_Stream_T<BitTorrent_Client_StreamState,
                              BitTorrent_Client_StreamConfiguration,
                              BitTorrent_RuntimeStatistic_t,
                              BitTorrent_Client_ModuleHandlerConfiguration,
                              BitTorrent_Client_SessionData,
                              BitTorrent_Client_SessionData_t,
                              ACE_Message_Block,
                              BitTorrent_Message,
                              BitTorrent_Client_SessionMessage>
{
 public:
  BitTorrent_Client_Stream (const std::string&); // name
  virtual ~BitTorrent_Client_Stream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const BitTorrent_Client_StreamConfiguration&, // configuration
                           bool = true,                           // setup pipeline ?
                           bool = true);                          // reset session data ?

  //// implement Common_IStatistic_T
  //// *NOTE*: delegate this to rntimeStatistic_
  //virtual bool collect (BitTorrent_RuntimeStatistic_t&); // return value: statistic data
  //// this is just a dummy (use statisticsReportingInterval instead)
  //virtual void report () const;

  // *TODO*: remove this API
//  void ping ();

 private:
  typedef BitTorrent_Stream_T<BitTorrent_Client_StreamState,
                              BitTorrent_Client_StreamConfiguration,
                              BitTorrent_RuntimeStatistic_t,
                              BitTorrent_Client_ModuleHandlerConfiguration,
                              BitTorrent_Client_SessionData,
                              BitTorrent_Client_SessionData_t,
                              ACE_Message_Block,
                              BitTorrent_Message,
                              BitTorrent_Client_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_Stream ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_Stream (const BitTorrent_Client_Stream&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_Stream& operator= (const BitTorrent_Client_Stream&))

  // modules
  //BitTorrent_Client_Module_Marshal_Module   marshal_;
  //BitTorrent_Module_Parser_Module           parser_;
  //BitTorrent_Module_RuntimeStatistic_Module runtimeStatistic_;
  // *NOTE*: the final module needs to be supplied to the stream from outside,
  //         otherwise data might be lost if event dispatch runs in (a) separate
  //         thread(s)
  //   BitTorrent_Client_Module_Handler_Module handler_;
};

#endif
