/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef NET_SOCK_CONNECTOR_H
#define NET_SOCK_CONNECTOR_H

#include <ace/Addr.h>
#include <ace/Global_Macros.h>
//#include <ace/OS_QoS.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
//#include <ace/SSL/SSL_SOCK_Connector.h>
//#include <ace/SSL/SSL_SOCK_Stream.h>

#include "net_exports.h"

// forward declarations
class ACE_Time_Value;

// *TODO*: move this class to Net_Client. 'friend'liness of
//         Net_TCPConnectionBase_T currently precludes this

//class Net_Export Net_SOCK_Connector
class Net_SOCK_Connector
 : public ACE_SOCK_Connector
{
 public:
  Net_SOCK_Connector ();
  virtual ~Net_SOCK_Connector ();

  int connect (ACE_SOCK_Stream&,                    // stream
               const ACE_Addr&,                     // remote address
               const ACE_Time_Value* = 0,           // timeout
               const ACE_Addr& = ACE_Addr::sap_any, // local address
               int = 0,                             // SO_REUSEADDR ?
               int = 0,                             // flags
               int = 0,                             // permissions
               int = 0);                            // protocol
//#if !defined (ACE_HAS_WINCE)
//  int connect (ACE_SOCK_Stream&,                    // stream
//               const ACE_Addr&,                     // remote address
//               ACE_QoS_Params,                      // QoS parameters
//               const ACE_Time_Value* = 0,           // timeout
//               const ACE_Addr& = ACE_Addr::sap_any, // local address
//               ACE_Protocol_Info* = 0,              // protocol info
//               ACE_SOCK_GROUP = 0,                  // group
//               u_long = 0,                          // flags
//               int = 0,                             // SO_REUSEADDR ?
//               int = 0);                            // permissions
//#endif  // ACE_HAS_WINCE

 protected:
  // *NOTE*: called before (!) connect () (socket open)
  int shared_connect_start (ACE_SOCK_Stream&,      // stream
                            const ACE_Time_Value*, // timeout
                            const ACE_Addr&,       // local address
                            const ACE_Addr&);      // remote address
  //// *NOTE*: called after (!) connect ()
  //int shared_connect_finish (ACE_SOCK_Stream&,      // stream
  //                           const ACE_Time_Value*, // timeout
  //                           int);                  // result

 private:
  typedef ACE_SOCK_Connector inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Connector (const Net_SOCK_Connector&))
  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_Connector& operator= (const Net_SOCK_Connector&))
};

//////////////////////////////////////////

//class Net_Export Net_SOCK_SSL_Connector
// : public ACE_SSL_SOCK_Connector
//{
// public:
//  Net_SOCK_SSL_Connector ();
//  virtual ~Net_SOCK_SSL_Connector ();

//  int connect (ACE_SSL_SOCK_Stream&,                // stream
//               const ACE_Addr&,                     // remote address
//               const ACE_Time_Value* = 0,           // timeout
//               const ACE_Addr& = ACE_Addr::sap_any, // local address
//               int = 0,                             // SO_REUSEADDR ?
//               int = 0,                             // flags
//               int = 0);                            // permissions
////#if !defined (ACE_HAS_WINCE)
////  int connect (ACE_SSL_SOCK_Stream&,                // stream
////               const ACE_Addr&,                     // remote address
////               ACE_QoS_Params,                      // QoS parameters
////               const ACE_Time_Value* = 0,           // timeout
////               const ACE_Addr& = ACE_Addr::sap_any, // local address
////               ACE_Protocol_Info* = 0,              // protocol info
////               ACE_SOCK_GROUP = 0,                  // group
////               u_long = 0,                          // flags
////               int = 0,                             // SO_REUSEADDR ?
////               int = 0);                            // permissions
////#endif  // ACE_HAS_WINCE

// private:
//  typedef ACE_SSL_SOCK_Connector inherited;

//  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_SSL_Connector (const Net_SOCK_SSL_Connector&))
//  ACE_UNIMPLEMENTED_FUNC (Net_SOCK_SSL_Connector& operator= (const Net_SOCK_SSL_Connector&))
//};

#endif
