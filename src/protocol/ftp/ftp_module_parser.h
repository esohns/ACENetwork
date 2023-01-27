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

#ifndef FTP_MODULE_PARSER_H
#define FTP_MODULE_PARSER_H

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_headmoduletask_base.h"

#include "stream_misc_parser.h"

#include "ftp_common.h"
#include "ftp_defines.h"
#include "ftp_parser_driver.h"

//extern NET_PROTOCOL_FTP_Export const char libacenetwork_protocol_default_ftp_parser_module_name_string[];
extern const char libacenetwork_protocol_default_ftp_parser_module_name_string[];

// forward declaration(s)
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class FTP_Module_Parser_T
 : public Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 FTP_ParserDriver_T<SessionMessageType>,
                                 struct Stream_UserData>
{
  typedef Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 FTP_ParserDriver_T<SessionMessageType>,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  FTP_Module_Parser_T (ISTREAM_T*); // stream handle
#else
  FTP_Module_Parser_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~FTP_Module_Parser_T () {}

  // implement (part of) FTP_IParser
  inline virtual ACE_UINT64 availableBytes () { ACE_ASSERT (inherited::headFragment_); return inherited::headFragment_->total_length (); }

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Parser_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Parser_T (const FTP_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Parser_T& operator= (const FTP_Module_Parser_T&))

  // implement (part of) FTP_IParser
  virtual void record (struct FTP_Record*&); // data record
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType>
class FTP_Module_ParserH_T
 : public Stream_Module_ParserH_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  StatisticContainerType,
                                  Common_Timer_Manager_t,
                                  struct Stream_UserData,
                                  FTP_ParserDriver_T<SessionMessageType> >
{
  typedef Stream_Module_ParserH_T <ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   Common_Timer_Manager_t,
                                   struct Stream_UserData,
                                   FTP_ParserDriver_T<SessionMessageType> > inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  FTP_Module_ParserH_T (ISTREAM_T*);                     // stream handle
#else
  FTP_Module_ParserH_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~FTP_Module_ParserH_T () {}

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_Module_ParserH_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                SessionDataType,
                                SessionDataContainerType,
                                StatisticContainerType,
                                Common_Timer_Manager_t,
                                struct Stream_UserData,
                                FTP_ParserDriver_T<SessionMessageType> >::initialize;

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_ParserH_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_ParserH_T (const FTP_Module_ParserH_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_ParserH_T& operator= (const FTP_Module_ParserH_T&))

  // implement (part of) FTP_IParser
  virtual void record (struct FTP_Record*&); // data record
};

// include template definition
#include "ftp_module_parser.inl"

#endif
