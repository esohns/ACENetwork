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

template <typename ControlAsynchConnectorType,
          typename ControlConnectorType,
          typename DataAsynchConnectorType, // PASV mode
          typename DataConnectorType,
          typename UserDataType>
class FTP_Control_T
 : public FTP_IControl
{
 public:
  FTP_Control_T (enum Common_EventDispatchType,                                                     // dispatch type
                 const typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T&, // connection configuration (control-)
                 const typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T&,    // connection configuration (data-)
                 const struct FTP_LoginOptions&);                                                   // login options
  inline virtual ~FTP_Control_T () {}

  // implement FTP_IControl
  virtual ACE_HANDLE connectControl ();
  virtual ACE_HANDLE connectData ();
  virtual void request (const struct FTP_Request&); // request
  inline virtual void queue (const struct FTP_Request& request_in) { ACE_GUARD (ACE_Thread_Mutex, aGuard, lock_); queue_.push_back (request_in); }

  ////////////////////////////////////////
  virtual void responseCB (const struct FTP_Record&);

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Control_T (const FTP_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Control_T& operator= (const FTP_Control_T&))

  // convenient types
  typedef std::deque<struct FTP_Request> QUEUE_T;
  typedef QUEUE_T::const_iterator        QUEUE_ITERATOR_T;

  bool getControlConnectionAndMessage (typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T*&,                       // return value: connection handle
                                       typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::STREAM_T::MESSAGE_T*&); // return value: message handle

  typename ControlAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T* connectionConfiguration_; // control-
  typename DataAsynchConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T*    connectionConfiguration_2; // data-
  ACE_HANDLE                                                                  controlConnection_;
  enum Common_EventDispatchType                                               dispatch_;
  ACE_Thread_Mutex                                                            lock_;
  struct FTP_LoginOptions                                                     loginOptions_;
  QUEUE_T                                                                     queue_;
};

// include template definition
#include "ftp_control.inl"

#endif
