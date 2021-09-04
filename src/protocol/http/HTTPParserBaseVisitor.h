
// Generated from HTTPParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "HTTPParserVisitor.h"


namespace antlrcpptest {

/**
 * This class provides an empty implementation of HTTPParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  HTTPParserBaseVisitor : public HTTPParserVisitor {
public:

  virtual antlrcpp::Any visitMain(HTTPParser::MainContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMessage(HTTPParser::MessageContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHead(HTTPParser::HeadContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHead_rest1(HTTPParser::Head_rest1Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRequest_line_rest1(HTTPParser::Request_line_rest1Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRequest_line_rest2(HTTPParser::Request_line_rest2Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHead_rest2(HTTPParser::Head_rest2Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatus_line_rest1(HTTPParser::Status_line_rest1Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatus_line_rest2(HTTPParser::Status_line_rest2Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHeaders(HTTPParser::HeadersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBody(HTTPParser::BodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitChunked_body(HTTPParser::Chunked_bodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitChunks(HTTPParser::ChunksContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace antlrcpptest
