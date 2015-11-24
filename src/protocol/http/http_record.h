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

#ifndef HTTP_RECORD_H
#define HTTP_RECORD_H

#include <map>
#include <string>

#include "ace/Global_Macros.h"

#include "common_idumpstate.h"
#include "common_referencecounter_base.h"

#include "http_exports.h"
#include "http_codes.h"

typedef std::map<std::string, std::string> HTTP_Headers_t;
typedef HTTP_Headers_t::const_iterator HTTP_HeadersIterator_t;

class HTTP_Export HTTP_Record
 : public Common_ReferenceCounterBase,
   public Common_IDumpState
{
 public:
  HTTP_Record ();
  virtual ~HTTP_Record ();

  // implement Common_IDumpState
  virtual void dump_state () const;

  HTTP_Codes::MethodType  method_;
  std::string             URI_;
  HTTP_Codes::VersionType version_;
  HTTP_Codes::StatusType  status_;
  HTTP_Headers_t          headers_;

 private:
  typedef Common_ReferenceCounterBase inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Record (const HTTP_Record&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Record& operator= (const HTTP_Record&))
};

#endif
