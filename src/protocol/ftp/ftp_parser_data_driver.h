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

#ifndef FTP_PARSER_DATA_DRIVER_T_H
#define FTP_PARSER_DATA_DRIVER_T_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "location.hh"

#include "common_iscanner.h"
#include "common_parser_common.h"

#include "ftp_iparser.h"
#include "ftp_scanner_data.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
struct FTP_Record;
//class FTP_Scanner;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

template <typename SessionMessageType>
class FTP_ParserDataDriver_T
 : public FTP_IParserData
{
  //friend class FTP_Scanner;

 public:
  FTP_ParserDataDriver_T ();
  virtual ~FTP_ParserDataDriver_T ();

  // implement Common_IParser_T
  virtual bool initialize (const struct Common_FlexBisonParserConfiguration&);
  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  virtual bool parse (ACE_Message_Block*); // data

  // implement Common_IYaccParser_T
  virtual void error (const yy::location&, // location
                      const std::string&); // message
  virtual void error (const YYLTYPE&,      // location
                      const std::string&); // message

  // implement (part of) FTP_IParserData
  inline virtual enum FTP_ProtocolDataState state () const { return state_; }
  inline virtual void state (enum FTP_ProtocolDataState state_in) { state_ = state_in; }

  inline virtual void resetOffset () { offset_ = 0; }

  ////////////////////////////////////////
  //virtual void record (struct FTP_Record*&); // data record

  // *TODO*: remove ASAP
  inline virtual bool hasFinished () const { return true; }

  // implement Common_IScannerBase
  void error (const std::string&); // message

  // *NOTE*: to be invoked by the scanner (ONLY !)
  //inline bool getDebugScanner () const { return (FTP_Scanner_get_debug (scannerState_) != 0); }
  inline virtual const struct Common_FlexScannerState& getR () const { static struct Common_FlexScannerState dummy;  ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }

 protected:
  bool                                        error_;
  bool                                        finished_;
  // *NOTE*: current (unscanned) data fragment
  ACE_Message_Block*                          fragment_;
  unsigned int                                offset_; // parsed entity bytes

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_ParserDataDriver_T (const FTP_ParserDataDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_ParserDataDriver_T& operator= (const FTP_ParserDataDriver_T&))

  // implement Common_IScannerBase
  inline virtual ACE_Message_Block* buffer () { return fragment_; }
#if defined (_DEBUG)
  inline virtual bool debug () const { return !(FTP_Scanner_Data_get_debug (scannerState_) == 0); }
#endif // _DEBUG
  inline virtual bool isBlocking () const { return true; }
  inline virtual unsigned int offset () const { return offset_; }

  inline virtual bool begin (const char*, unsigned int) { return scan_begin (); }
  inline virtual void end () { scan_end (); }

  // *NOTE*: appends a new buffer to the ACE_Message_Block chain
  virtual bool switchBuffer (bool = false); // unlink current buffer ?
  virtual void waitBuffer ();

  ////////////////////////////////////////
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; }

  // implement Common_ILexScanner_T
  virtual void setDebug (yyscan_t, // state handle
                         bool);    // toggle
  virtual void reset (); // resets the offsets (line/column to 1,1)

  virtual bool initialize (yyscan_t&,                // return value: state handle
                           FTP_IParserData* = NULL); // 'extra' data handle
  virtual void finalize (yyscan_t&); // state handle

  virtual struct yy_buffer_state* create (yyscan_t, // state handle
                                          char*,    // buffer handle
                                          size_t);  // buffer size
  virtual void destroy (yyscan_t,                  // state handle
                        struct yy_buffer_state*&); // buffer handle
  inline virtual bool lex (yyscan_t) { ACE_ASSERT (false); return FTP_Scanner_Data_lex (NULL, NULL, this, scannerState_); }

  // implement Common_IYaccStreamParser_T
  inline virtual struct FTP_IParserDataDummy& current () { static struct FTP_IParserDataDummy dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }
  inline virtual void record (struct FTP_IParserDataDummy*&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // helper methods
  bool scan_begin ();
  void scan_end ();

  struct Common_FlexBisonParserConfiguration* configuration_;

  // scanner
  yyscan_t                                    scannerState_;
  YY_BUFFER_STATE                             bufferState_;

  enum FTP_ProtocolDataState                  state_;

  bool                                        initialized_;
};

// include template definition
#include "ftp_parser_data_driver.inl"

#endif
