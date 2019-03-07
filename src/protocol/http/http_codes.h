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

#ifndef HTTP_CODES_H
#define HTTP_CODES_H

#include "ace/Global_Macros.h"

class HTTP_Codes
{
 public:
  enum MethodType
  {
    HTTP_METHOD_GET   = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD,
    /////////////////////////////////////
    HTTP_METHOD_MAX,
    HTTP_METHOD_INVALID
  };

  enum VersionType
  {
    HTTP_VERSION_0_9  = 0,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2,
    /////////////////////////////////////
    HTTP_VERSION_MAX,
    HTTP_VERSION_INVALID
  };

  enum StatusType
  {
    // 1xx: Informational
    /////////////////////////////////////
    // 2xx: Successful
    HTTP_STATUS_OK                  = 200,
    HTTP_STATUS_CREATED             = 201,
    HTTP_STATUS_ACCEPTED            = 202,
    HTTP_STATUS_NOCONTENT           = 204,
    /////////////////////////////////////
    // 3xx Redirection
    HTTP_STATUS_MULTIPLECHOICES     = 300,
    HTTP_STATUS_MOVEDPERMANENTLY    = 301,
    HTTP_STATUS_MOVEDTEMPORARILY    = 302,
    HTTP_STATUS_NOTMODIFIED         = 304,
    /////////////////////////////////////
    // 4xx Client Error
    HTTP_STATUS_BADREQUEST          = 400,
    HTTP_STATUS_UNAUTHORIZED        = 401,
    HTTP_STATUS_FORBIDDEN           = 403,
    HTTP_STATUS_NOTFOUND            = 404,
    /////////////////////////////////////
    // 5xx Server Error
    HTTP_STATUS_INTERNALSERVERERROR = 500,
    HTTP_STATUS_NOTIMPLEMENTED      = 501,
    HTTP_STATUS_BADGATEWAY          = 502,
    HTTP_STATUS_SERVICEUNAVAILABLE  = 503,
    /////////////////////////////////////
    HTTP_STATUS_MAX   = 1000,
    HTTP_STATUS_INVALID
  };

  enum HeaderType
  {
    HTTP_HEADER_GENERAL = 0,
    HTTP_HEADER_REQUEST,
    HTTP_HEADER_RESPONSE,
    HTTP_HEADER_ENTITY,
    /////////////////////////////////////
    HTTP_HEADER_MAX,
    HTTP_HEADER_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_Codes (const HTTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Codes& operator= (const HTTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~HTTP_Codes ())
};

// convenience typedefs
typedef HTTP_Codes::MethodType HTTP_Method_t;
typedef HTTP_Codes::VersionType HTTP_Version_t;
typedef HTTP_Codes::StatusType HTTP_Status_t;

#endif
