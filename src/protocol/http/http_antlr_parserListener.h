
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "common_string_tools.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

#include "http_antlr_iparser.h"
#include "http_antlr_scanner.h"


// Generated from /mnt/win_d/projects/ACENetwork/src/protocol/http/scripts/http_antlr_parser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "http_antlr_parser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by http_antlr_parser.
 */
class  http_antlr_parserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterMain(http_antlr_parser::MainContext *ctx) = 0;
  virtual void exitMain(http_antlr_parser::MainContext *ctx) = 0;

  virtual void enterMessage(http_antlr_parser::MessageContext *ctx) = 0;
  virtual void exitMessage(http_antlr_parser::MessageContext *ctx) = 0;

  virtual void enterHead(http_antlr_parser::HeadContext *ctx) = 0;
  virtual void exitHead(http_antlr_parser::HeadContext *ctx) = 0;

  virtual void enterHead_request_rest(http_antlr_parser::Head_request_restContext *ctx) = 0;
  virtual void exitHead_request_rest(http_antlr_parser::Head_request_restContext *ctx) = 0;

  virtual void enterHead_response_rest(http_antlr_parser::Head_response_restContext *ctx) = 0;
  virtual void exitHead_response_rest(http_antlr_parser::Head_response_restContext *ctx) = 0;

  virtual void enterHeaders(http_antlr_parser::HeadersContext *ctx) = 0;
  virtual void exitHeaders(http_antlr_parser::HeadersContext *ctx) = 0;

  virtual void enterHeader(http_antlr_parser::HeaderContext *ctx) = 0;
  virtual void exitHeader(http_antlr_parser::HeaderContext *ctx) = 0;

  virtual void enterBody(http_antlr_parser::BodyContext *ctx) = 0;
  virtual void exitBody(http_antlr_parser::BodyContext *ctx) = 0;

  virtual void enterChunked_body(http_antlr_parser::Chunked_bodyContext *ctx) = 0;
  virtual void exitChunked_body(http_antlr_parser::Chunked_bodyContext *ctx) = 0;

  virtual void enterChunks(http_antlr_parser::ChunksContext *ctx) = 0;
  virtual void exitChunks(http_antlr_parser::ChunksContext *ctx) = 0;


};

