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

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::Net_UDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                       unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));

}

//template <typename SessionDataType,
//          typename HandlerType>
//Net_UDPConnection_T<SessionDataType,
//                    HandlerType>::Net_UDPConnection_T ()
// : inherited (NULL)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));
//
//}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::~Net_UDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::~Net_UDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
int
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::open (void* args_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::open"));

  int result = -1;

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  result = inherited::open (args_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  ACE_INET_Addr local_SAP, remote_SAP;
  try
  {
    info (handle,
          local_SAP,
          remote_SAP);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), aborting\n")));
    return -1;
  }
  if (local_SAP.addr_to_string (buffer,
                                sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  if (remote_SAP.addr_to_string (buffer,
                                 sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered UDP connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, reinterpret_cast<unsigned int> (handle),
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered UDP connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif

  return 0;
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
int
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::close (u_long flags_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::close"));

  int result = -1;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr address1, address2;
  try
  {
    info (handle,
          address1,
          address2);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
  }

  void* this_p = this;
  int num_connections =
      (inherited::manager_ ? (inherited::manager_->numConnections () - 1) : -1);
  // *WARNING*: this invokes 'delete this' !
  result = inherited::close ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::close(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered UDP connection [%@/%u] (total: %u)\n"),
              this_p, reinterpret_cast<unsigned int> (handle),
              num_connections));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered UDP connection [%@/%d] (total: %d)\n"),
              this_p, handle,
              num_connections));
#endif

  return 0;
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
bool
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::initialize (Net_ClientServerRole_t role_in,
                                              const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::initialize"));

  ACE_UNUSED_ARG (role_in);
  ACE_UNUSED_ARG (configuration_in);

  //  typename inherited::CONNECTION_BASE_T* connection_base_p = this;
  //  return connection_base_p->initialize (role_in,
  //                                        configuration_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (true);

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
void
Net_UDPConnection_T<UserDataType,
                    SessionDataType,
                    HandlerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::finalize"));

  int result = -1;

  result = close (0);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_UDPConnection_T::close(0): \"%m\", returning\n")));
}

//template <typename SessionDataType,
//          typename HandlerType>
//int
//Net_UDPConnection_T<SessionDataType>::handle_close (ACE_HANDLE handle_in,
//                                                    ACE_Reactor_Mask mask_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::handle_close"));

//  switch (mask_in)
//  {
//    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
//    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
//      //     accept failed (e.g. too many connections) /
//      //     select failed (EBADF see Select_Reactor_T.cpp)
//      //     asynch abort
//      break;
//    case ACE_Event_Handler::EXCEPT_MASK:
//      // *IMPORTANT NOTE*: the TP_Reactor dispatches notifications in parallel
//      // to regular socket events, thus several notifications may be in flight
//      // at the same time. In order to avoid confusion when the socket closes,
//      // proper synchronization is handled through the reference counting
//      // mechanism, i.e. the handler is only deleted after the last reference
//      // has been released. Still, handling notifications after this occurs
//      // will fail, and the reactor will invoke this method each time. As there
//      // is no need to go through the shutdown procedure several times, bail
//      // out early here
//      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
//      //  ACE_DEBUG((LM_ERROR,
//      //             ACE_TEXT("notification completed, continuing\n")));
//      return 0;
//    default:
//      // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
//                  handle_in,
//                  mask_in));
//#else
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
//                  handle_in,
//                  mask_in));
//#endif
//      break;
//  } // end SWITCH

//  //// step1: connection shutting down --> signal any worker(s)
//  //if (inherited::configuration_.streamConfiguration.useThreadPerConnection)
//  //  shutdown ();

//  //  // step2: de-register this connection
//  //  if (inherited::manager_ &&
//  //      inherited::isRegistered_)
//  //  { // (try to) de-register with the connection manager
//  //    try
//  //    {
//  //      inherited::manager_->deregisterConnection (this);
//  //    }
//  //    catch (...)
//  //    {
//  //      ACE_DEBUG ((LM_ERROR,
//  //                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
//  //    }
//  //  } // end IF

//  // step3: invoke base-class maintenance
//  int result = inherited::handle_close (handle_in, // event handle
//                                        mask_in);  // event mask
//  if (result == -1)
//    // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#endif

//  return result;
//}

/////////////////////////////////////////

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::Net_AsynchUDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                   unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::Net_AsynchUDPConnection_T ()
 : inherited (NULL,
              0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::~Net_AsynchUDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::~Net_AsynchUDPConnection_T"));

}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
bool
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::initialize (Net_ClientServerRole_t role_in,
                                                    const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::initialize"));

  ACE_UNUSED_ARG (role_in);
  ACE_UNUSED_ARG (configuration_in);

  //  typename inherited::CONNECTION_BASE_T* connection_base_p = this;
  //  return connection_base_p->initialize (role_in,
  //                                        configuration_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (true);

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::finalize"));

  int result = -1;

  result = inherited::handle_close (inherited::handle (),
                                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::handle_close(): \"%m\", continuing\n")));

  inherited::finalize ();
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::open (ACE_HANDLE handle_in,
                                              ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::open"));

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  inherited::open (handle_in,
                   messageBlock_in);

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  ACE_INET_Addr local_SAP, remote_SAP;
  try
  {
    info (handle,
          local_SAP,
          remote_SAP);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), returning\n")));

    return;
  }
  if (local_SAP.addr_to_string (buffer,
                                sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  if (remote_SAP.addr_to_string (buffer,
                                 sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered UDP connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, reinterpret_cast<unsigned int> (ACE_IPC_SAP::get_handle ()),
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered UDP connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, ACE_IPC_SAP::get_handle (),
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif
}

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
int
Net_AsynchUDPConnection_T<UserDataType,
                          SessionDataType,
                          HandlerType>::handle_close (ACE_HANDLE handle_in,
                                                      ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::handle_close"));

  int result = -1;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr address1, address2;
  try
  {
    info (handle,
          address1,
          address2);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
  }

  // step1: invoke base-class maintenance
  result = inherited::handle_close (handle_in, // event handle
                                    mask_in);  // event mask
  if (result == -1)
    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%@, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%d, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#endif

  // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered UDP connection [%@/%u] (total: %u)\n"),
              this, reinterpret_cast<unsigned int> (handle_in),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered UDP connection [%@/%d] (total: %d)\n"),
              this, handle_in,
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif

  return result;
}
