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

#ifndef TEST_U_COMMON_TOOLS_H
#define TEST_U_COMMON_TOOLS_H

#include <functional>
#include <random>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "net_common.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

class Test_U_Common_Tools
 : public Common_SInitializeFinalize_T<Test_U_Common_Tools>
{
 public:
  // convenient types
//  typedef Net_IConnection_T<ACE_INET_Addr,
//                            Net_TCPStreamConnectionConfiguration_t,
//                            struct Net_StreamConnectionState,
//                            struct Net_StreamStatistic> CONNECTION_T;
//  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
//                                   ACE_INET_Addr,
//                                   Net_TCPStreamConnectionConfiguration_t,
//                                   struct Net_StreamConnectionState,
//                                   struct Net_StreamStatistic,
//                                   struct Net_UserData> IMANAGER_T;

  static bool initialize ();
  inline static bool finalize () { return true; }

  template <typename ConnectionConfigurationType>
  static Net_IConnection_T<ACE_INET_Addr,
                           struct Net_StreamConnectionState,
                           Net_StreamStatistic_t>* getRandomConnection (Net_IConnectionManager_T<ACE_INET_Addr,
                                                                                                 ConnectionConfigurationType,
                                                                                                 struct Net_StreamConnectionState,
                                                                                                 Net_StreamStatistic_t,
                                                                                                 struct Net_UserData>*);

  static std::function<int ()>              randomGenerator_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Common_Tools (const Test_U_Common_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Common_Tools& operator= (const Test_U_Common_Tools&))

  // probability
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static char                               randomStateInitializationBuffer_[BUFSIZ];
  static struct random_data                 randomState_;
#endif // ACE_WIN32 || ACE_WIN64
  static std::uniform_int_distribution<int> randomDistribution_;
  static std::default_random_engine         randomEngine_;
};

// include template definition
#include "test_u_common_tools.inl"

#endif
