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

#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

#include <map>
#include <string>

#include "common_inotify.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "http_codes.h"

// forward declarations
struct HTTP_Configuration;
struct HTTP_Record;
class HTTP_SessionMessage;
struct HTTP_Stream_SessionData;
struct HTTP_Stream_UserData;
typedef Common_INotify_T<unsigned int,
                         HTTP_Stream_SessionData,
                         HTTP_Record,
                         HTTP_SessionMessage> HTTP_IStreamNotify_t;
typedef Stream_Statistic HTTP_RuntimeStatistic_t;

typedef std::map<std::string, std::string> HTTP_Headers_t;
typedef HTTP_Headers_t::const_iterator HTTP_HeadersConstIterator_t;
typedef HTTP_Headers_t::iterator HTTP_HeadersIterator_t;
typedef std::map<std::string, std::string> HTTP_Form_t;
typedef HTTP_Form_t::const_iterator HTTP_FormIterator_t;
struct HTTP_Record
{
  inline HTTP_Record ()
   : form ()
   , headers ()
   , method (HTTP_Codes::HTTP_METHOD_INVALID)
   , reason ()
   , status (HTTP_Codes::HTTP_STATUS_INVALID)
   , URI ()
   , version (HTTP_Codes::HTTP_VERSION_INVALID)
  {};

  HTTP_Form_t             form; // request
  HTTP_Headers_t          headers;
  HTTP_Codes::MethodType  method;
  std::string             reason; // response
  HTTP_Codes::StatusType  status; // response
  std::string             URI;
  HTTP_Codes::VersionType version;
};
struct HTTP_MessageData
{
  inline HTTP_MessageData ()
   : HTTPRecord (NULL)
  {};
  inline ~HTTP_MessageData ()
  {
    if (HTTPRecord)
      delete HTTPRecord;
  };

  HTTP_Record* HTTPRecord;
};
typedef Stream_DataBase_T<HTTP_MessageData> HTTP_MessageData_t;

struct HTTP_ConnectionState
 : Net_ConnectionState
{
  inline HTTP_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  HTTP_Configuration*   configuration;

  HTTP_Stream_UserData* userData;
};

#endif
