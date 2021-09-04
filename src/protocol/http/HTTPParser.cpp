
// Generated from HTTPParser.g4 by ANTLR 4.8


#include "HTTPParserListener.h"
#include "HTTPParserVisitor.h"

#include "HTTPParser.h"


using namespace antlrcpp;
using namespace antlrcpptest;
using namespace antlr4;

HTTPParser::HTTPParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

HTTPParser::~HTTPParser() {
  delete _interpreter;
}

std::string HTTPParser::getGrammarFileName() const {
  return "HTTPParser.g4";
}

const std::vector<std::string>& HTTPParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& HTTPParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- MainContext ------------------------------------------------------------------

HTTPParser::MainContext::MainContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::MessageContext* HTTPParser::MainContext::message() {
  return getRuleContext<HTTPParser::MessageContext>(0);
}

tree::TerminalNode* HTTPParser::MainContext::EOF() {
  return getToken(HTTPParser::EOF, 0);
}


size_t HTTPParser::MainContext::getRuleIndex() const {
  return HTTPParser::RuleMain;
}

void HTTPParser::MainContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMain(this);
}

void HTTPParser::MainContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMain(this);
}


antlrcpp::Any HTTPParser::MainContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitMain(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::MainContext* HTTPParser::main() {
  MainContext *_localctx = _tracker.createInstance<MainContext>(_ctx, getState());
  enterRule(_localctx, 0, HTTPParser::RuleMain);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(26);
    message();
    setState(27);
    match(HTTPParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MessageContext ------------------------------------------------------------------

HTTPParser::MessageContext::MessageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::HeadContext* HTTPParser::MessageContext::head() {
  return getRuleContext<HTTPParser::HeadContext>(0);
}

tree::TerminalNode* HTTPParser::MessageContext::CRLF() {
  return getToken(HTTPParser::CRLF, 0);
}

HTTPParser::BodyContext* HTTPParser::MessageContext::body() {
  return getRuleContext<HTTPParser::BodyContext>(0);
}


size_t HTTPParser::MessageContext::getRuleIndex() const {
  return HTTPParser::RuleMessage;
}

void HTTPParser::MessageContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMessage(this);
}

void HTTPParser::MessageContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMessage(this);
}


antlrcpp::Any HTTPParser::MessageContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitMessage(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::MessageContext* HTTPParser::message() {
  MessageContext *_localctx = _tracker.createInstance<MessageContext>(_ctx, getState());
  enterRule(_localctx, 2, HTTPParser::RuleMessage);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(29);
    head();
    setState(30);
    match(HTTPParser::CRLF);
    setState(31);
    body();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HeadContext ------------------------------------------------------------------

HTTPParser::HeadContext::HeadContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::HeadContext::METHOD() {
  return getToken(HTTPParser::METHOD, 0);
}

HTTPParser::Head_rest1Context* HTTPParser::HeadContext::head_rest1() {
  return getRuleContext<HTTPParser::Head_rest1Context>(0);
}

tree::TerminalNode* HTTPParser::HeadContext::VERSION() {
  return getToken(HTTPParser::VERSION, 0);
}

HTTPParser::Head_rest2Context* HTTPParser::HeadContext::head_rest2() {
  return getRuleContext<HTTPParser::Head_rest2Context>(0);
}


size_t HTTPParser::HeadContext::getRuleIndex() const {
  return HTTPParser::RuleHead;
}

void HTTPParser::HeadContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead(this);
}

void HTTPParser::HeadContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead(this);
}


antlrcpp::Any HTTPParser::HeadContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitHead(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::HeadContext* HTTPParser::head() {
  HeadContext *_localctx = _tracker.createInstance<HeadContext>(_ctx, getState());
  enterRule(_localctx, 4, HTTPParser::RuleHead);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(37);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case HTTPParser::METHOD: {
        enterOuterAlt(_localctx, 1);
        setState(33);
        match(HTTPParser::METHOD);
        setState(34);
        head_rest1();
        break;
      }

      case HTTPParser::VERSION: {
        enterOuterAlt(_localctx, 2);
        setState(35);
        match(HTTPParser::VERSION);
        setState(36);
        head_rest2();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Head_rest1Context ------------------------------------------------------------------

HTTPParser::Head_rest1Context::Head_rest1Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::Request_line_rest1Context* HTTPParser::Head_rest1Context::request_line_rest1() {
  return getRuleContext<HTTPParser::Request_line_rest1Context>(0);
}

HTTPParser::HeadersContext* HTTPParser::Head_rest1Context::headers() {
  return getRuleContext<HTTPParser::HeadersContext>(0);
}


size_t HTTPParser::Head_rest1Context::getRuleIndex() const {
  return HTTPParser::RuleHead_rest1;
}

void HTTPParser::Head_rest1Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead_rest1(this);
}

void HTTPParser::Head_rest1Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead_rest1(this);
}


antlrcpp::Any HTTPParser::Head_rest1Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitHead_rest1(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Head_rest1Context* HTTPParser::head_rest1() {
  Head_rest1Context *_localctx = _tracker.createInstance<Head_rest1Context>(_ctx, getState());
  enterRule(_localctx, 6, HTTPParser::RuleHead_rest1);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(39);
    request_line_rest1();
    setState(40);
    headers(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Request_line_rest1Context ------------------------------------------------------------------

HTTPParser::Request_line_rest1Context::Request_line_rest1Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::Request_line_rest1Context::URI() {
  return getToken(HTTPParser::URI, 0);
}

HTTPParser::Request_line_rest2Context* HTTPParser::Request_line_rest1Context::request_line_rest2() {
  return getRuleContext<HTTPParser::Request_line_rest2Context>(0);
}


size_t HTTPParser::Request_line_rest1Context::getRuleIndex() const {
  return HTTPParser::RuleRequest_line_rest1;
}

void HTTPParser::Request_line_rest1Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRequest_line_rest1(this);
}

void HTTPParser::Request_line_rest1Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRequest_line_rest1(this);
}


antlrcpp::Any HTTPParser::Request_line_rest1Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitRequest_line_rest1(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Request_line_rest1Context* HTTPParser::request_line_rest1() {
  Request_line_rest1Context *_localctx = _tracker.createInstance<Request_line_rest1Context>(_ctx, getState());
  enterRule(_localctx, 8, HTTPParser::RuleRequest_line_rest1);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    match(HTTPParser::URI);
    setState(43);
    request_line_rest2();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Request_line_rest2Context ------------------------------------------------------------------

HTTPParser::Request_line_rest2Context::Request_line_rest2Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::Request_line_rest2Context::VERSION() {
  return getToken(HTTPParser::VERSION, 0);
}


size_t HTTPParser::Request_line_rest2Context::getRuleIndex() const {
  return HTTPParser::RuleRequest_line_rest2;
}

void HTTPParser::Request_line_rest2Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRequest_line_rest2(this);
}

void HTTPParser::Request_line_rest2Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRequest_line_rest2(this);
}


antlrcpp::Any HTTPParser::Request_line_rest2Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitRequest_line_rest2(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Request_line_rest2Context* HTTPParser::request_line_rest2() {
  Request_line_rest2Context *_localctx = _tracker.createInstance<Request_line_rest2Context>(_ctx, getState());
  enterRule(_localctx, 10, HTTPParser::RuleRequest_line_rest2);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(45);
    match(HTTPParser::VERSION);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Head_rest2Context ------------------------------------------------------------------

HTTPParser::Head_rest2Context::Head_rest2Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::Status_line_rest1Context* HTTPParser::Head_rest2Context::status_line_rest1() {
  return getRuleContext<HTTPParser::Status_line_rest1Context>(0);
}

HTTPParser::HeadersContext* HTTPParser::Head_rest2Context::headers() {
  return getRuleContext<HTTPParser::HeadersContext>(0);
}


size_t HTTPParser::Head_rest2Context::getRuleIndex() const {
  return HTTPParser::RuleHead_rest2;
}

void HTTPParser::Head_rest2Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead_rest2(this);
}

void HTTPParser::Head_rest2Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead_rest2(this);
}


antlrcpp::Any HTTPParser::Head_rest2Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitHead_rest2(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Head_rest2Context* HTTPParser::head_rest2() {
  Head_rest2Context *_localctx = _tracker.createInstance<Head_rest2Context>(_ctx, getState());
  enterRule(_localctx, 12, HTTPParser::RuleHead_rest2);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(47);
    status_line_rest1();
    setState(48);
    headers(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Status_line_rest1Context ------------------------------------------------------------------

HTTPParser::Status_line_rest1Context::Status_line_rest1Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::Status_line_rest1Context::STATUS() {
  return getToken(HTTPParser::STATUS, 0);
}

HTTPParser::Status_line_rest2Context* HTTPParser::Status_line_rest1Context::status_line_rest2() {
  return getRuleContext<HTTPParser::Status_line_rest2Context>(0);
}


size_t HTTPParser::Status_line_rest1Context::getRuleIndex() const {
  return HTTPParser::RuleStatus_line_rest1;
}

void HTTPParser::Status_line_rest1Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStatus_line_rest1(this);
}

void HTTPParser::Status_line_rest1Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStatus_line_rest1(this);
}


antlrcpp::Any HTTPParser::Status_line_rest1Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitStatus_line_rest1(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Status_line_rest1Context* HTTPParser::status_line_rest1() {
  Status_line_rest1Context *_localctx = _tracker.createInstance<Status_line_rest1Context>(_ctx, getState());
  enterRule(_localctx, 14, HTTPParser::RuleStatus_line_rest1);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(50);
    match(HTTPParser::STATUS);
    setState(51);
    status_line_rest2();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Status_line_rest2Context ------------------------------------------------------------------

HTTPParser::Status_line_rest2Context::Status_line_rest2Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::Status_line_rest2Context::REASON() {
  return getToken(HTTPParser::REASON, 0);
}


size_t HTTPParser::Status_line_rest2Context::getRuleIndex() const {
  return HTTPParser::RuleStatus_line_rest2;
}

void HTTPParser::Status_line_rest2Context::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStatus_line_rest2(this);
}

void HTTPParser::Status_line_rest2Context::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStatus_line_rest2(this);
}


antlrcpp::Any HTTPParser::Status_line_rest2Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitStatus_line_rest2(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Status_line_rest2Context* HTTPParser::status_line_rest2() {
  Status_line_rest2Context *_localctx = _tracker.createInstance<Status_line_rest2Context>(_ctx, getState());
  enterRule(_localctx, 16, HTTPParser::RuleStatus_line_rest2);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(53);
    match(HTTPParser::REASON);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HeadersContext ------------------------------------------------------------------

HTTPParser::HeadersContext::HeadersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::HeadersContext* HTTPParser::HeadersContext::headers() {
  return getRuleContext<HTTPParser::HeadersContext>(0);
}

tree::TerminalNode* HTTPParser::HeadersContext::HEADER() {
  return getToken(HTTPParser::HEADER, 0);
}


size_t HTTPParser::HeadersContext::getRuleIndex() const {
  return HTTPParser::RuleHeaders;
}

void HTTPParser::HeadersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHeaders(this);
}

void HTTPParser::HeadersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHeaders(this);
}


antlrcpp::Any HTTPParser::HeadersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitHeaders(this);
  else
    return visitor->visitChildren(this);
}


HTTPParser::HeadersContext* HTTPParser::headers() {
   return headers(0);
}

HTTPParser::HeadersContext* HTTPParser::headers(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  HTTPParser::HeadersContext *_localctx = _tracker.createInstance<HeadersContext>(_ctx, parentState);
  HTTPParser::HeadersContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 18;
  enterRecursionRule(_localctx, 18, HTTPParser::RuleHeaders, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _ctx->stop = _input->LT(-1);
    setState(60);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<HeadersContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleHeaders);
        setState(56);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(57);
        match(HTTPParser::HEADER); 
      }
      setState(62);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- BodyContext ------------------------------------------------------------------

HTTPParser::BodyContext::BodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* HTTPParser::BodyContext::BODY() {
  return getToken(HTTPParser::BODY, 0);
}

tree::TerminalNode* HTTPParser::BodyContext::CHUNK() {
  return getToken(HTTPParser::CHUNK, 0);
}

HTTPParser::Chunked_bodyContext* HTTPParser::BodyContext::chunked_body() {
  return getRuleContext<HTTPParser::Chunked_bodyContext>(0);
}


size_t HTTPParser::BodyContext::getRuleIndex() const {
  return HTTPParser::RuleBody;
}

void HTTPParser::BodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBody(this);
}

void HTTPParser::BodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBody(this);
}


antlrcpp::Any HTTPParser::BodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitBody(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::BodyContext* HTTPParser::body() {
  BodyContext *_localctx = _tracker.createInstance<BodyContext>(_ctx, getState());
  enterRule(_localctx, 20, HTTPParser::RuleBody);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(66);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case HTTPParser::BODY: {
        enterOuterAlt(_localctx, 1);
        setState(63);
        match(HTTPParser::BODY);
        break;
      }

      case HTTPParser::CHUNK: {
        enterOuterAlt(_localctx, 2);
        setState(64);
        match(HTTPParser::CHUNK);
        setState(65);
        chunked_body();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Chunked_bodyContext ------------------------------------------------------------------

HTTPParser::Chunked_bodyContext::Chunked_bodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::ChunksContext* HTTPParser::Chunked_bodyContext::chunks() {
  return getRuleContext<HTTPParser::ChunksContext>(0);
}

HTTPParser::HeadersContext* HTTPParser::Chunked_bodyContext::headers() {
  return getRuleContext<HTTPParser::HeadersContext>(0);
}

tree::TerminalNode* HTTPParser::Chunked_bodyContext::CRLF() {
  return getToken(HTTPParser::CRLF, 0);
}


size_t HTTPParser::Chunked_bodyContext::getRuleIndex() const {
  return HTTPParser::RuleChunked_body;
}

void HTTPParser::Chunked_bodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterChunked_body(this);
}

void HTTPParser::Chunked_bodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitChunked_body(this);
}


antlrcpp::Any HTTPParser::Chunked_bodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitChunked_body(this);
  else
    return visitor->visitChildren(this);
}

HTTPParser::Chunked_bodyContext* HTTPParser::chunked_body() {
  Chunked_bodyContext *_localctx = _tracker.createInstance<Chunked_bodyContext>(_ctx, getState());
  enterRule(_localctx, 22, HTTPParser::RuleChunked_body);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(68);
    chunks(0);
    setState(69);
    headers(0);
    setState(70);
    match(HTTPParser::CRLF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ChunksContext ------------------------------------------------------------------

HTTPParser::ChunksContext::ChunksContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

HTTPParser::ChunksContext* HTTPParser::ChunksContext::chunks() {
  return getRuleContext<HTTPParser::ChunksContext>(0);
}

tree::TerminalNode* HTTPParser::ChunksContext::CHUNK() {
  return getToken(HTTPParser::CHUNK, 0);
}


size_t HTTPParser::ChunksContext::getRuleIndex() const {
  return HTTPParser::RuleChunks;
}

void HTTPParser::ChunksContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterChunks(this);
}

void HTTPParser::ChunksContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<HTTPParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitChunks(this);
}


antlrcpp::Any HTTPParser::ChunksContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<HTTPParserVisitor*>(visitor))
    return parserVisitor->visitChunks(this);
  else
    return visitor->visitChildren(this);
}


HTTPParser::ChunksContext* HTTPParser::chunks() {
   return chunks(0);
}

HTTPParser::ChunksContext* HTTPParser::chunks(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  HTTPParser::ChunksContext *_localctx = _tracker.createInstance<ChunksContext>(_ctx, parentState);
  HTTPParser::ChunksContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 24;
  enterRecursionRule(_localctx, 24, HTTPParser::RuleChunks, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _ctx->stop = _input->LT(-1);
    setState(77);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ChunksContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleChunks);
        setState(73);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(74);
        match(HTTPParser::CHUNK); 
      }
      setState(79);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

bool HTTPParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 9: return headersSempred(dynamic_cast<HeadersContext *>(context), predicateIndex);
    case 12: return chunksSempred(dynamic_cast<ChunksContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool HTTPParser::headersSempred(HeadersContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

bool HTTPParser::chunksSempred(ChunksContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> HTTPParser::_decisionToDFA;
atn::PredictionContextCache HTTPParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN HTTPParser::_atn;
std::vector<uint16_t> HTTPParser::_serializedATN;

std::vector<std::string> HTTPParser::_ruleNames = {
  "main", "message", "head", "head_rest1", "request_line_rest1", "request_line_rest2", 
  "head_rest2", "status_line_rest1", "status_line_rest2", "headers", "body", 
  "chunked_body", "chunks"
};

std::vector<std::string> HTTPParser::_literalNames = {
  "", "'\u000D\u000A'"
};

std::vector<std::string> HTTPParser::_symbolicNames = {
  "", "CRLF", "VERSION", "URI", "HEADER", "METHOD", "REASON", "STATUS", 
  "BODY", "CHUNK"
};

dfa::Vocabulary HTTPParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> HTTPParser::_tokenNames;

HTTPParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0xb, 0x53, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 0x9, 
    0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 0x3, 
    0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 
    0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0x28, 0xa, 0x4, 0x3, 0x5, 
    0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x7, 0x3, 0x7, 
    0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0xa, 
    0x3, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x7, 0xb, 0x3d, 0xa, 0xb, 0xc, 
    0xb, 0xe, 0xb, 0x40, 0xb, 0xb, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 
    0x45, 0xa, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xe, 0x3, 
    0xe, 0x3, 0xe, 0x7, 0xe, 0x4e, 0xa, 0xe, 0xc, 0xe, 0xe, 0xe, 0x51, 0xb, 
    0xe, 0x3, 0xe, 0x2, 0x4, 0x14, 0x1a, 0xf, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 
    0xe, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x2, 0x2, 0x2, 0x49, 0x2, 0x1c, 
    0x3, 0x2, 0x2, 0x2, 0x4, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x6, 0x27, 0x3, 0x2, 
    0x2, 0x2, 0x8, 0x29, 0x3, 0x2, 0x2, 0x2, 0xa, 0x2c, 0x3, 0x2, 0x2, 0x2, 
    0xc, 0x2f, 0x3, 0x2, 0x2, 0x2, 0xe, 0x31, 0x3, 0x2, 0x2, 0x2, 0x10, 
    0x34, 0x3, 0x2, 0x2, 0x2, 0x12, 0x37, 0x3, 0x2, 0x2, 0x2, 0x14, 0x39, 
    0x3, 0x2, 0x2, 0x2, 0x16, 0x44, 0x3, 0x2, 0x2, 0x2, 0x18, 0x46, 0x3, 
    0x2, 0x2, 0x2, 0x1a, 0x4a, 0x3, 0x2, 0x2, 0x2, 0x1c, 0x1d, 0x5, 0x4, 
    0x3, 0x2, 0x1d, 0x1e, 0x7, 0x2, 0x2, 0x3, 0x1e, 0x3, 0x3, 0x2, 0x2, 
    0x2, 0x1f, 0x20, 0x5, 0x6, 0x4, 0x2, 0x20, 0x21, 0x7, 0x3, 0x2, 0x2, 
    0x21, 0x22, 0x5, 0x16, 0xc, 0x2, 0x22, 0x5, 0x3, 0x2, 0x2, 0x2, 0x23, 
    0x24, 0x7, 0x7, 0x2, 0x2, 0x24, 0x28, 0x5, 0x8, 0x5, 0x2, 0x25, 0x26, 
    0x7, 0x4, 0x2, 0x2, 0x26, 0x28, 0x5, 0xe, 0x8, 0x2, 0x27, 0x23, 0x3, 
    0x2, 0x2, 0x2, 0x27, 0x25, 0x3, 0x2, 0x2, 0x2, 0x28, 0x7, 0x3, 0x2, 
    0x2, 0x2, 0x29, 0x2a, 0x5, 0xa, 0x6, 0x2, 0x2a, 0x2b, 0x5, 0x14, 0xb, 
    0x2, 0x2b, 0x9, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x2d, 0x7, 0x5, 0x2, 0x2, 
    0x2d, 0x2e, 0x5, 0xc, 0x7, 0x2, 0x2e, 0xb, 0x3, 0x2, 0x2, 0x2, 0x2f, 
    0x30, 0x7, 0x4, 0x2, 0x2, 0x30, 0xd, 0x3, 0x2, 0x2, 0x2, 0x31, 0x32, 
    0x5, 0x10, 0x9, 0x2, 0x32, 0x33, 0x5, 0x14, 0xb, 0x2, 0x33, 0xf, 0x3, 
    0x2, 0x2, 0x2, 0x34, 0x35, 0x7, 0x9, 0x2, 0x2, 0x35, 0x36, 0x5, 0x12, 
    0xa, 0x2, 0x36, 0x11, 0x3, 0x2, 0x2, 0x2, 0x37, 0x38, 0x7, 0x8, 0x2, 
    0x2, 0x38, 0x13, 0x3, 0x2, 0x2, 0x2, 0x39, 0x3e, 0x8, 0xb, 0x1, 0x2, 
    0x3a, 0x3b, 0xc, 0x4, 0x2, 0x2, 0x3b, 0x3d, 0x7, 0x6, 0x2, 0x2, 0x3c, 
    0x3a, 0x3, 0x2, 0x2, 0x2, 0x3d, 0x40, 0x3, 0x2, 0x2, 0x2, 0x3e, 0x3c, 
    0x3, 0x2, 0x2, 0x2, 0x3e, 0x3f, 0x3, 0x2, 0x2, 0x2, 0x3f, 0x15, 0x3, 
    0x2, 0x2, 0x2, 0x40, 0x3e, 0x3, 0x2, 0x2, 0x2, 0x41, 0x45, 0x7, 0xa, 
    0x2, 0x2, 0x42, 0x43, 0x7, 0xb, 0x2, 0x2, 0x43, 0x45, 0x5, 0x18, 0xd, 
    0x2, 0x44, 0x41, 0x3, 0x2, 0x2, 0x2, 0x44, 0x42, 0x3, 0x2, 0x2, 0x2, 
    0x45, 0x17, 0x3, 0x2, 0x2, 0x2, 0x46, 0x47, 0x5, 0x1a, 0xe, 0x2, 0x47, 
    0x48, 0x5, 0x14, 0xb, 0x2, 0x48, 0x49, 0x7, 0x3, 0x2, 0x2, 0x49, 0x19, 
    0x3, 0x2, 0x2, 0x2, 0x4a, 0x4f, 0x8, 0xe, 0x1, 0x2, 0x4b, 0x4c, 0xc, 
    0x4, 0x2, 0x2, 0x4c, 0x4e, 0x7, 0xb, 0x2, 0x2, 0x4d, 0x4b, 0x3, 0x2, 
    0x2, 0x2, 0x4e, 0x51, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x4d, 0x3, 0x2, 0x2, 
    0x2, 0x4f, 0x50, 0x3, 0x2, 0x2, 0x2, 0x50, 0x1b, 0x3, 0x2, 0x2, 0x2, 
    0x51, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x6, 0x27, 0x3e, 0x44, 0x4f, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

HTTPParser::Initializer HTTPParser::_init;
