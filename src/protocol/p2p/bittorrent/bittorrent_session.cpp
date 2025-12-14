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
#include "stdafx.h"

#include "bittorrent_session.h"

#include "ace/Log_Msg.h"
#include "ace/Thread_Manager.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void
net_bittorrent_session_cleanup_nmlock_function (void* object_in, void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_cleanup_nmlock_function"));

  // sanity check(s)
  Net_IConnectionManagerBase* i_connection_manager_p =
    static_cast<Net_IConnectionManagerBase*> (arg_in);
  ACE_ASSERT (i_connection_manager_p);

  i_connection_manager_p->unlock (false);
}
#else
void
net_bittorrent_session_cleanup_nmlock_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_cleanup_nmlock_function"));

  // sanity check(s)
  Net_IConnectionManagerBase* i_connection_manager_p =
    static_cast<Net_IConnectionManagerBase*> (arg_in);
  ACE_ASSERT (i_connection_manager_p);

  i_connection_manager_p->unlock (false);
}
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void
net_bittorrent_session_cleanup_data_function (void* object_in, void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_cleanup_data_function"));

  // sanity check(s)
  struct BitTorrent_SessionInitiationThreadData* data_p =
    static_cast<struct BitTorrent_SessionInitiationThreadData*> (arg_in);
  ACE_ASSERT (data_p);

  delete data_p;
}
#else
void
net_bittorrent_session_cleanup_data_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_cleanup_data_function"));

  // sanity check(s)
  struct BitTorrent_SessionInitiationThreadData* data_p =
    static_cast<struct BitTorrent_SessionInitiationThreadData*> (arg_in);
  ACE_ASSERT (data_p);

  delete data_p;
}
#endif // ACE_WIN32 || ACE_WIN64

ACE_THR_FUNC_RETURN
net_bittorrent_session_setup_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_setup_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = -1;
#else
  result = arg_in;

  int old_type;
  pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, &old_type);
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  struct BitTorrent_SessionInitiationThreadData* data_p =
    static_cast<struct BitTorrent_SessionInitiationThreadData*> (arg_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->lock);
  ACE_ASSERT (data_p->session);

  ACE_INET_Addr peer_address;
  bool delete_thread_data_b = false;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *data_p->lock, result);
    peer_address = data_p->addresses.back ();
    data_p->addresses.pop_back ();
    delete_thread_data_b = data_p->addresses.empty ();
    if (delete_thread_data_b)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_At_Thread_Exit_Func* at_thread_exit_p = NULL;
      ACE_hthread_t handle_h;
      ACE_OS::thr_self (handle_h);
      ACE_NEW_NORETURN (at_thread_exit_p,
                        ACE_At_Thread_Exit_Func (handle_h,
                                                 net_bittorrent_session_cleanup_data_function,
                                                 arg_in));
      ACE_ASSERT (at_thread_exit_p);
      ACE_Thread_Manager::instance ()->at_exit (at_thread_exit_p);
#else
      pthread_cleanup_push (net_bittorrent_session_cleanup_data_function,
                            arg_in);
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connecting to peer \"%s\"...\n"),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address, false, false).c_str ())));

  ACE_ASSERT (data_p->peerConnectionManager);
  data_p->peerConnectionManager->lock (true);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_At_Thread_Exit_Func* at_thread_exit_p = NULL;
  ACE_hthread_t handle_h;
  ACE_OS::thr_self (handle_h);
  ACE_NEW_NORETURN (at_thread_exit_p,
                    ACE_At_Thread_Exit_Func (handle_h,
                                             net_bittorrent_session_cleanup_nmlock_function,
                                             data_p->peerConnectionManager));
  ACE_ASSERT (at_thread_exit_p);
  ACE_Thread_Manager::instance ()->at_exit (at_thread_exit_p);
#else
  pthread_cleanup_push (net_bittorrent_session_cleanup_nmlock_function,
                        data_p->peerConnectionManager);
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (data_p->session);
  data_p->session->connect (peer_address);

  //data_p->peerConnectionManager->unlock (false);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  pthread_cleanup_pop (1);
  pthread_setcanceltype (old_type, NULL);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  if (unlikely (delete_thread_data_b))
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *data_p->lock, result);
      data_p->state->connecting = false;
    } // end lock scope

    //delete data_p; data_p = NULL;
#if defined(ACE_WIN32) || defined(ACE_WIN64)
#else
    pthread_cleanup_pop (1);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  return result;
}
