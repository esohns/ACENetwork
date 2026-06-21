
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "common_string_tools.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

#include "http_antlr_scanner.h"


// Generated from /mnt/win_d/projects/ACENetwork/src/protocol/http/scripts/http_antlr_parser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "http_antlr_parserListener.h"


/**
 * This class provides an empty implementation of http_antlr_parserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  http_antlr_parserBaseListener : public http_antlr_parserListener {
public:

  virtual void enterMain(http_antlr_parser::MainContext * /*ctx*/) override { }
  virtual void exitMain(http_antlr_parser::MainContext * /*ctx*/) override { }

  virtual void enterMessage(http_antlr_parser::MessageContext * /*ctx*/) override { }
  virtual void exitMessage(http_antlr_parser::MessageContext * /*ctx*/) override { }

  virtual void enterHead(http_antlr_parser::HeadContext * /*ctx*/) override { }
  virtual void exitHead(http_antlr_parser::HeadContext * /*ctx*/) override { }

  virtual void enterHead_request_rest(http_antlr_parser::Head_request_restContext * /*ctx*/) override { }
  virtual void exitHead_request_rest(http_antlr_parser::Head_request_restContext * /*ctx*/) override { }

  virtual void enterHead_response_rest(http_antlr_parser::Head_response_restContext * /*ctx*/) override { }
  virtual void exitHead_response_rest(http_antlr_parser::Head_response_restContext * /*ctx*/) override { }

  virtual void enterHeaders(http_antlr_parser::HeadersContext * /*ctx*/) override { }
  virtual void exitHeaders(http_antlr_parser::HeadersContext * /*ctx*/) override { }

  virtual void enterHeader(http_antlr_parser::HeaderContext * /*ctx*/) override { }
  virtual void exitHeader(http_antlr_parser::HeaderContext * /*ctx*/) override { }

  virtual void enterBody(http_antlr_parser::BodyContext * /*ctx*/) override { }
  virtual void exitBody(http_antlr_parser::BodyContext * /*ctx*/) override { }

  virtual void enterChunked_body(http_antlr_parser::Chunked_bodyContext * /*ctx*/) override { }
  virtual void exitChunked_body(http_antlr_parser::Chunked_bodyContext * /*ctx*/) override { }

  virtual void enterChunks(http_antlr_parser::ChunksContext * /*ctx*/) override { }
  virtual void exitChunks(http_antlr_parser::ChunksContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

