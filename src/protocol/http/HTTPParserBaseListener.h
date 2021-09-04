
// Generated from HTTPParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "HTTPParserListener.h"


namespace antlrcpptest {

/**
 * This class provides an empty implementation of HTTPParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  HTTPParserBaseListener : public HTTPParserListener {
public:

  virtual void enterMain(HTTPParser::MainContext * /*ctx*/) override { }
  virtual void exitMain(HTTPParser::MainContext * /*ctx*/) override { }

  virtual void enterMessage(HTTPParser::MessageContext * /*ctx*/) override { }
  virtual void exitMessage(HTTPParser::MessageContext * /*ctx*/) override { }

  virtual void enterHead(HTTPParser::HeadContext * /*ctx*/) override { }
  virtual void exitHead(HTTPParser::HeadContext * /*ctx*/) override { }

  virtual void enterHead_rest1(HTTPParser::Head_rest1Context * /*ctx*/) override { }
  virtual void exitHead_rest1(HTTPParser::Head_rest1Context * /*ctx*/) override { }

  virtual void enterRequest_line_rest1(HTTPParser::Request_line_rest1Context * /*ctx*/) override { }
  virtual void exitRequest_line_rest1(HTTPParser::Request_line_rest1Context * /*ctx*/) override { }

  virtual void enterRequest_line_rest2(HTTPParser::Request_line_rest2Context * /*ctx*/) override { }
  virtual void exitRequest_line_rest2(HTTPParser::Request_line_rest2Context * /*ctx*/) override { }

  virtual void enterHead_rest2(HTTPParser::Head_rest2Context * /*ctx*/) override { }
  virtual void exitHead_rest2(HTTPParser::Head_rest2Context * /*ctx*/) override { }

  virtual void enterStatus_line_rest1(HTTPParser::Status_line_rest1Context * /*ctx*/) override { }
  virtual void exitStatus_line_rest1(HTTPParser::Status_line_rest1Context * /*ctx*/) override { }

  virtual void enterStatus_line_rest2(HTTPParser::Status_line_rest2Context * /*ctx*/) override { }
  virtual void exitStatus_line_rest2(HTTPParser::Status_line_rest2Context * /*ctx*/) override { }

  virtual void enterHeaders(HTTPParser::HeadersContext * /*ctx*/) override { }
  virtual void exitHeaders(HTTPParser::HeadersContext * /*ctx*/) override { }

  virtual void enterBody(HTTPParser::BodyContext * /*ctx*/) override { }
  virtual void exitBody(HTTPParser::BodyContext * /*ctx*/) override { }

  virtual void enterChunked_body(HTTPParser::Chunked_bodyContext * /*ctx*/) override { }
  virtual void exitChunked_body(HTTPParser::Chunked_bodyContext * /*ctx*/) override { }

  virtual void enterChunks(HTTPParser::ChunksContext * /*ctx*/) override { }
  virtual void exitChunks(HTTPParser::ChunksContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

}  // namespace antlrcpptest
