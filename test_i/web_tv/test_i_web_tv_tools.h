/***************************************************************************
 *   Copyright (C) 2024 by Erik Sohns                                      *
 *   erik.sohns@posteo.de                                                  *
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

#ifndef TEST_I_WEBTV_TOOLS_H
#define TEST_I_WEBTV_TOOLS_H

#include "ace/Global_Macros.h"

#include "test_i_web_tv_common.h"

class Test_I_WebTV_Tools
{
 public:
  static void addSegmentURIs (unsigned int,                       // program#
                              Test_I_WebTV_ChannelSegmentURLs_t&, // return value: segment URLs
                              unsigned int,                       // max index
                              unsigned int);                      // #index positions

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_WebTV_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_WebTV_Tools (const Test_I_WebTV_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_WebTV_Tools& operator= (const Test_I_WebTV_Tools&))
};

#endif // TEST_I_WEBTV_TOOLS_H