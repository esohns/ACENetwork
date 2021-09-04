
// Generated from HTTPParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "HTTPParser.h"


namespace antlrcpptest {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by HTTPParser.
 */
class  HTTPParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by HTTPParser.
   */
    virtual antlrcpp::Any visitMain(HTTPParser::MainContext *context) = 0;

    virtual antlrcpp::Any visitMessage(HTTPParser::MessageContext *context) = 0;

    virtual antlrcpp::Any visitHead(HTTPParser::HeadContext *context) = 0;

    virtual antlrcpp::Any visitHead_rest1(HTTPParser::Head_rest1Context *context) = 0;

    virtual antlrcpp::Any visitRequest_line_rest1(HTTPParser::Request_line_rest1Context *context) = 0;

    virtual antlrcpp::Any visitRequest_line_rest2(HTTPParser::Request_line_rest2Context *context) = 0;

    virtual antlrcpp::Any visitHead_rest2(HTTPParser::Head_rest2Context *context) = 0;

    virtual antlrcpp::Any visitStatus_line_rest1(HTTPParser::Status_line_rest1Context *context) = 0;

    virtual antlrcpp::Any visitStatus_line_rest2(HTTPParser::Status_line_rest2Context *context) = 0;

    virtual antlrcpp::Any visitHeaders(HTTPParser::HeadersContext *context) = 0;

    virtual antlrcpp::Any visitBody(HTTPParser::BodyContext *context) = 0;

    virtual antlrcpp::Any visitChunked_body(HTTPParser::Chunked_bodyContext *context) = 0;

    virtual antlrcpp::Any visitChunks(HTTPParser::ChunksContext *context) = 0;


};

}  // namespace antlrcpptest
