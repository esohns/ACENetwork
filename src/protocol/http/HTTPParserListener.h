
// Generated from HTTPParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "HTTPParser.h"


namespace antlrcpptest {

/**
 * This interface defines an abstract listener for a parse tree produced by HTTPParser.
 */
class  HTTPParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterMain(HTTPParser::MainContext *ctx) = 0;
  virtual void exitMain(HTTPParser::MainContext *ctx) = 0;

  virtual void enterMessage(HTTPParser::MessageContext *ctx) = 0;
  virtual void exitMessage(HTTPParser::MessageContext *ctx) = 0;

  virtual void enterHead(HTTPParser::HeadContext *ctx) = 0;
  virtual void exitHead(HTTPParser::HeadContext *ctx) = 0;

  virtual void enterHead_rest1(HTTPParser::Head_rest1Context *ctx) = 0;
  virtual void exitHead_rest1(HTTPParser::Head_rest1Context *ctx) = 0;

  virtual void enterRequest_line_rest1(HTTPParser::Request_line_rest1Context *ctx) = 0;
  virtual void exitRequest_line_rest1(HTTPParser::Request_line_rest1Context *ctx) = 0;

  virtual void enterRequest_line_rest2(HTTPParser::Request_line_rest2Context *ctx) = 0;
  virtual void exitRequest_line_rest2(HTTPParser::Request_line_rest2Context *ctx) = 0;

  virtual void enterHead_rest2(HTTPParser::Head_rest2Context *ctx) = 0;
  virtual void exitHead_rest2(HTTPParser::Head_rest2Context *ctx) = 0;

  virtual void enterStatus_line_rest1(HTTPParser::Status_line_rest1Context *ctx) = 0;
  virtual void exitStatus_line_rest1(HTTPParser::Status_line_rest1Context *ctx) = 0;

  virtual void enterStatus_line_rest2(HTTPParser::Status_line_rest2Context *ctx) = 0;
  virtual void exitStatus_line_rest2(HTTPParser::Status_line_rest2Context *ctx) = 0;

  virtual void enterHeaders(HTTPParser::HeadersContext *ctx) = 0;
  virtual void exitHeaders(HTTPParser::HeadersContext *ctx) = 0;

  virtual void enterBody(HTTPParser::BodyContext *ctx) = 0;
  virtual void exitBody(HTTPParser::BodyContext *ctx) = 0;

  virtual void enterChunked_body(HTTPParser::Chunked_bodyContext *ctx) = 0;
  virtual void exitChunked_body(HTTPParser::Chunked_bodyContext *ctx) = 0;

  virtual void enterChunks(HTTPParser::ChunksContext *ctx) = 0;
  virtual void exitChunks(HTTPParser::ChunksContext *ctx) = 0;


};

}  // namespace antlrcpptest
