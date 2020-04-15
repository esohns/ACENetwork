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

#include "test_u_common_tools.h"

#include "ace/OS.h"

#include "common_tools.h"

// initialize statics
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
char Test_U_Common_Tools::randomStateInitializationBuffer_[BUFSIZ];
struct random_data Test_U_Common_Tools::randomState_;
#endif // ACE_WIN32 || ACE_WIN64
std::uniform_int_distribution<int> Test_U_Common_Tools::randomDistribution_;
std::default_random_engine Test_U_Common_Tools::randomEngine_;
std::function<int ()> Test_U_Common_Tools::randomGenerator_;

bool
Test_U_Common_Tools::initialize ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Common_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (Test_U_Common_Tools::randomStateInitializationBuffer_,
                  0,
                  sizeof (char[BUFSIZ]));
  int result =
    ::initstate_r (Common_Tools::randomSeed,
                   Test_U_Common_Tools::randomStateInitializationBuffer_, sizeof (char[BUFSIZ]),
                   &Test_U_Common_Tools::randomState_);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::initstate_r(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = ::srandom_r (Common_Tools::randomSeed, &randomState_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::srandom_r(): \"%m\", aborting\n")));
    return false;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  Test_U_Common_Tools::randomGenerator_ =
      std::bind (Test_U_Common_Tools::randomDistribution_,
                 Test_U_Common_Tools::randomEngine_);

  return true;
}
