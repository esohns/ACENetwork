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
#include "ace/OS.h"

#include "net_macros.h"

template <typename ConnectionConfigurationType>
Net_IConnection_T<ACE_INET_Addr,
                  struct Net_StreamConnectionState,
                  Net_StreamStatistic_t>*
Test_U_Common_Tools::getRandomConnection (Net_IConnectionManager_T<ACE_INET_Addr,
                                                                   ConnectionConfigurationType,
                                                                   struct Net_StreamConnectionState,
                                                                   Net_StreamStatistic_t,
                                                                   struct Net_UserData>* imanager_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Common_Tools::getRandomConnection"));

  // sanity check(s)
  ACE_ASSERT (imanager_in);

  Net_IConnection_T<ACE_INET_Addr,
                    struct Net_StreamConnectionState,
                    Net_StreamStatistic_t>* connection_p = NULL;
  int number_of_connections_i = 0;
  int index_i = 0;
  int result = -1;

retry:
  number_of_connections_i = imanager_in->count ();
  if (unlikely (!number_of_connections_i)) // sanity check (see below)
    return NULL;
  // *PORTABILITY*: outside glibc, this is not very portable
  // *TODO*: use STL random funcionality instead
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  index_i =
    (ACE_OS::rand_r (&Common_Tools::randomSeed) % number_of_connections_i);
#else
  result = ::random_r (&Test_U_Common_Tools::randomState_, &index_i);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::random_r(): \"%s\", aborting\n")));
    return NULL;
  } // end IF
  index_i = (index_i % number_of_connections_i);
#endif // ACE_WIN32 || ACE_WIN64

  connection_p = imanager_in->operator[] (index_i);
  if (unlikely (!connection_p))
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to retrieve connection #%d/%d, retrying\n"),
//                index_i, number_of_connections_i));
    goto retry;
  } // end IF

  return connection_p;
}
