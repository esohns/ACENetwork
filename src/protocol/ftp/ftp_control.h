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

#ifndef FTP_CONTROL_H
#define FTP_CONTROL_H

#include <string>

#include "ace/config-macros.h"
#include "ace/Global_Macros.h"

#include "ftp_icontrol.h"

template <typename DataMessageType>
class FTP_Control_T
 : public FTP_IControl
{
 public:
  FTP_Control_T ();
  inline virtual ~FTP_Control_T () { }

  // implement FTP_IControl
  virtual void cwd (const std::string&); // path
  virtual void list ();

 private:
  //ACE_UNIMPLEMENTED_FUNC (FTP_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Control_T (const FTP_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Control_T& operator= (const FTP_Control_T&))

  bool getDataConnectionAndMessage (ACE_HANDLE&,        // return value: connection handle
                                    DataMessageType*&); // return value: message handle

  ACE_HANDLE currentDataConnection_;
};

// include template definition
#include "ftp_control.inl"

#endif
