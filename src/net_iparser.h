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

#ifndef NET_IPARSER_H
#define NET_IPARSER_H

#include <string>

#include "location.hh"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_iscanner.h"

#include "net_defines.h"

// forward declarations
struct yy_buffer_state;
class ACE_Message_Block;
class ACE_Message_Queue_Base;
typedef void* yyscan_t;

template <typename ConfigurationType,
          typename RecordType>
class Net_IYaccStreamParser_T
 : public Common_IYaccParser_T<ConfigurationType>
{
 public:
  // convenient types
  typedef Net_IYaccStreamParser_T<ConfigurationType,
                                  RecordType> IPARSER_T;

  virtual RecordType& current () = 0;

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (RecordType*&) = 0; // data record
};

//////////////////////////////////////////

template <typename ConfigurationType,
          typename RecordType>
class Net_IYaccRecordParser_T
 : public Net_IYaccStreamParser_T<ConfigurationType,
                                  RecordType>
{
 public:
  // convenient types
  typedef Net_IYaccRecordParser_T<ConfigurationType,
                                  RecordType> IPARSER_T;

  virtual bool hasFinished () const = 0;
};

#endif
