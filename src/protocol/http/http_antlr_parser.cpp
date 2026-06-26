
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


// Generated from http_antlr_parser.g4 by ANTLR 4.13.2


#include "http_antlr_parserListener.h"

#include "http_antlr_parser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct Http_antlr_parserStaticData final {
  Http_antlr_parserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  Http_antlr_parserStaticData(const Http_antlr_parserStaticData&) = delete;
  Http_antlr_parserStaticData(Http_antlr_parserStaticData&&) = delete;
  Http_antlr_parserStaticData& operator=(const Http_antlr_parserStaticData&) = delete;
  Http_antlr_parserStaticData& operator=(Http_antlr_parserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag http_antlr_parserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<Http_antlr_parserStaticData> http_antlr_parserParserStaticData = nullptr;

void http_antlr_parserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (http_antlr_parserParserStaticData != nullptr) {
    return;
  }
#else
  assert(http_antlr_parserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<Http_antlr_parserStaticData>(
    std::vector<std::string>{
      "document", "head", "head_request_rest", "head_response_rest", "headers", 
      "header", "body", "chunked_body", "chunks"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "'\\u000D\\u000A'"
    },
    std::vector<std::string>{
      "", "METHOD", "URI", "VERSION", "CODE", "REASON", "FIELD_KEY", "COLON", 
      "FIELD_VALUE", "CRLF_HEAD", "CRLF", "CHUNK", "SP_RESP", "SP_CODE", 
      "BODY", "CHUNK_DATA", "CRLF_CHUNKED_DATA"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,16,79,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,
  	7,7,2,8,7,8,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,3,1,30,8,1,1,
  	2,1,2,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,4,1,4,1,4,5,4,48,
  	8,4,10,4,12,4,51,9,4,1,5,1,5,1,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,1,6,3,6,
  	64,8,6,1,7,1,7,1,7,1,7,1,8,1,8,1,8,1,8,5,8,74,8,8,10,8,12,8,77,9,8,1,
  	8,0,2,8,16,9,0,2,4,6,8,10,12,14,16,0,0,73,0,18,1,0,0,0,2,29,1,0,0,0,4,
  	31,1,0,0,0,6,37,1,0,0,0,8,44,1,0,0,0,10,52,1,0,0,0,12,63,1,0,0,0,14,65,
  	1,0,0,0,16,69,1,0,0,0,18,19,3,2,1,0,19,20,5,10,0,0,20,21,3,12,6,0,21,
  	22,5,0,0,1,22,1,1,0,0,0,23,24,5,1,0,0,24,25,6,1,-1,0,25,30,3,4,2,0,26,
  	27,5,3,0,0,27,28,6,1,-1,0,28,30,3,6,3,0,29,23,1,0,0,0,29,26,1,0,0,0,30,
  	3,1,0,0,0,31,32,5,2,0,0,32,33,5,3,0,0,33,34,5,10,0,0,34,35,6,2,-1,0,35,
  	36,3,8,4,0,36,5,1,0,0,0,37,38,5,4,0,0,38,39,6,3,-1,0,39,40,5,5,0,0,40,
  	41,6,3,-1,0,41,42,5,10,0,0,42,43,3,8,4,0,43,7,1,0,0,0,44,49,6,4,-1,0,
  	45,46,10,2,0,0,46,48,3,10,5,0,47,45,1,0,0,0,48,51,1,0,0,0,49,47,1,0,0,
  	0,49,50,1,0,0,0,50,9,1,0,0,0,51,49,1,0,0,0,52,53,5,6,0,0,53,54,5,7,0,
  	0,54,55,5,8,0,0,55,56,5,10,0,0,56,57,6,5,-1,0,57,11,1,0,0,0,58,59,5,14,
  	0,0,59,64,6,6,-1,0,60,61,5,11,0,0,61,62,6,6,-1,0,62,64,3,14,7,0,63,58,
  	1,0,0,0,63,60,1,0,0,0,64,13,1,0,0,0,65,66,3,16,8,0,66,67,3,8,4,0,67,68,
  	5,10,0,0,68,15,1,0,0,0,69,75,6,8,-1,0,70,71,10,2,0,0,71,72,5,11,0,0,72,
  	74,6,8,-1,0,73,70,1,0,0,0,74,77,1,0,0,0,75,73,1,0,0,0,75,76,1,0,0,0,76,
  	17,1,0,0,0,77,75,1,0,0,0,4,29,49,63,75
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  http_antlr_parserParserStaticData = std::move(staticData);
}

}

http_antlr_parser::http_antlr_parser(TokenStream *input) : http_antlr_parser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

http_antlr_parser::http_antlr_parser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  http_antlr_parser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *http_antlr_parserParserStaticData->atn, http_antlr_parserParserStaticData->decisionToDFA, http_antlr_parserParserStaticData->sharedContextCache, options);
}

http_antlr_parser::~http_antlr_parser() {
  delete _interpreter;
}

const atn::ATN& http_antlr_parser::getATN() const {
  return *http_antlr_parserParserStaticData->atn;
}

std::string http_antlr_parser::getGrammarFileName() const {
  return "http_antlr_parser.g4";
}

const std::vector<std::string>& http_antlr_parser::getRuleNames() const {
  return http_antlr_parserParserStaticData->ruleNames;
}

const dfa::Vocabulary& http_antlr_parser::getVocabulary() const {
  return http_antlr_parserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView http_antlr_parser::getSerializedATN() const {
  return http_antlr_parserParserStaticData->serializedATN;
}


//----------------- DocumentContext ------------------------------------------------------------------

http_antlr_parser::DocumentContext::DocumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::HeadContext* http_antlr_parser::DocumentContext::head() {
  return getRuleContext<http_antlr_parser::HeadContext>(0);
}

tree::TerminalNode* http_antlr_parser::DocumentContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}

http_antlr_parser::BodyContext* http_antlr_parser::DocumentContext::body() {
  return getRuleContext<http_antlr_parser::BodyContext>(0);
}

tree::TerminalNode* http_antlr_parser::DocumentContext::EOF() {
  return getToken(http_antlr_parser::EOF, 0);
}


size_t http_antlr_parser::DocumentContext::getRuleIndex() const {
  return http_antlr_parser::RuleDocument;
}

void http_antlr_parser::DocumentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDocument(this);
}

void http_antlr_parser::DocumentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDocument(this);
}

http_antlr_parser::DocumentContext* http_antlr_parser::document() {
  DocumentContext *_localctx = _tracker.createInstance<DocumentContext>(_ctx, getState());
  enterRule(_localctx, 0, http_antlr_parser::RuleDocument);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(18);
    head();
    setState(19);
    match(http_antlr_parser::CRLF);
    setState(20);
    body();
    setState(21);
    match(http_antlr_parser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HeadContext ------------------------------------------------------------------

http_antlr_parser::HeadContext::HeadContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* http_antlr_parser::HeadContext::METHOD() {
  return getToken(http_antlr_parser::METHOD, 0);
}

http_antlr_parser::Head_request_restContext* http_antlr_parser::HeadContext::head_request_rest() {
  return getRuleContext<http_antlr_parser::Head_request_restContext>(0);
}

tree::TerminalNode* http_antlr_parser::HeadContext::VERSION() {
  return getToken(http_antlr_parser::VERSION, 0);
}

http_antlr_parser::Head_response_restContext* http_antlr_parser::HeadContext::head_response_rest() {
  return getRuleContext<http_antlr_parser::Head_response_restContext>(0);
}


size_t http_antlr_parser::HeadContext::getRuleIndex() const {
  return http_antlr_parser::RuleHead;
}

void http_antlr_parser::HeadContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead(this);
}

void http_antlr_parser::HeadContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead(this);
}

http_antlr_parser::HeadContext* http_antlr_parser::head() {
  HeadContext *_localctx = _tracker.createInstance<HeadContext>(_ctx, getState());
  enterRule(_localctx, 2, http_antlr_parser::RuleHead);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(29);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case http_antlr_parser::METHOD: {
        enterOuterAlt(_localctx, 1);
        setState(23);
        antlrcpp::downCast<HeadContext *>(_localctx)->methodToken = match(http_antlr_parser::METHOD);

                              record_.method = HTTP_Tools::MethodToType (antlrcpp::downCast<HeadContext *>(_localctx)->methodToken->getText ());
                            
        setState(25);
        head_request_rest();
        break;
      }

      case http_antlr_parser::VERSION: {
        enterOuterAlt(_localctx, 2);
        setState(26);
        antlrcpp::downCast<HeadContext *>(_localctx)->versionToken = match(http_antlr_parser::VERSION);

                            { std::string input_string = antlrcpp::downCast<HeadContext *>(_localctx)->versionToken->getText ();
                              std::smatch match_results;
                              std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^");
                              regex_string += ACE_TEXT_ALWAYS_CHAR ("(?:HTTP\\/)");
                              regex_string +=
                                ACE_TEXT_ALWAYS_CHAR ("([[:digit:]]{1}\\.[[:digit:]]{1})$");
                              std::regex regex (regex_string.c_str ());
                              if (!std::regex_match (input_string,
                                                     match_results,
                                                     regex,
                                                     std::regex_constants::match_default))
                              {
                                std::string error_message =
                                  ACE_TEXT_ALWAYS_CHAR ("invalid HTTP version string (was: \"");
                                error_message += input_string;
                                error_message += ACE_TEXT_ALWAYS_CHAR ("\"), throwing\n");
                                ACE_DEBUG ((LM_ERROR,
                                            ACE_TEXT (error_message.c_str ())));
                                throw new ParseCancellationException (/*"line " + line + ":" + charPositionInLine + " " + */error_message);
                              } // end IF
                              ACE_ASSERT (!match_results.empty ());
                              ACE_ASSERT (match_results[1].matched);
                              record_.version =
                                HTTP_Tools::VersionToType (match_results[1].str ());
                            }
                            
        setState(28);
        head_response_rest();
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

//----------------- Head_request_restContext ------------------------------------------------------------------

http_antlr_parser::Head_request_restContext::Head_request_restContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* http_antlr_parser::Head_request_restContext::URI() {
  return getToken(http_antlr_parser::URI, 0);
}

tree::TerminalNode* http_antlr_parser::Head_request_restContext::VERSION() {
  return getToken(http_antlr_parser::VERSION, 0);
}

tree::TerminalNode* http_antlr_parser::Head_request_restContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}

http_antlr_parser::HeadersContext* http_antlr_parser::Head_request_restContext::headers() {
  return getRuleContext<http_antlr_parser::HeadersContext>(0);
}


size_t http_antlr_parser::Head_request_restContext::getRuleIndex() const {
  return http_antlr_parser::RuleHead_request_rest;
}

void http_antlr_parser::Head_request_restContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead_request_rest(this);
}

void http_antlr_parser::Head_request_restContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead_request_rest(this);
}

http_antlr_parser::Head_request_restContext* http_antlr_parser::head_request_rest() {
  Head_request_restContext *_localctx = _tracker.createInstance<Head_request_restContext>(_ctx, getState());
  enterRule(_localctx, 4, http_antlr_parser::RuleHead_request_rest);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(31);
    antlrcpp::downCast<Head_request_restContext *>(_localctx)->uriToken = match(http_antlr_parser::URI);
    setState(32);
    antlrcpp::downCast<Head_request_restContext *>(_localctx)->versionToken = match(http_antlr_parser::VERSION);
    setState(33);
    match(http_antlr_parser::CRLF);

                        { record_.URI = antlrcpp::downCast<Head_request_restContext *>(_localctx)->uriToken->getText ();

                          std::string input_string = antlrcpp::downCast<Head_request_restContext *>(_localctx)->versionToken->getText ();
                          std::smatch match_results;
                          std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^");
                          regex_string += ACE_TEXT_ALWAYS_CHAR ("(?:HTTP\\/)");
                          regex_string +=
                            ACE_TEXT_ALWAYS_CHAR ("([[:digit:]]{1}\\.[[:digit:]]{1})$");
                          std::regex regex (regex_string.c_str ());
                          if (!std::regex_match (input_string,
                                                 match_results,
                                                 regex,
                                                 std::regex_constants::match_default))
                          {
                            std::string error_message =
                              ACE_TEXT_ALWAYS_CHAR ("invalid HTTP version string (was: \"");
                            error_message += input_string;
                            error_message += ACE_TEXT_ALWAYS_CHAR ("\"), throwing\n");
                            ACE_DEBUG ((LM_ERROR,
                                        ACE_TEXT (error_message.c_str ())));
                            throw new ParseCancellationException (/*"line " + line + ":" + charPositionInLine + " " + */error_message);
                          } // end IF
                          ACE_ASSERT (!match_results.empty ());
                          ACE_ASSERT (match_results[1].matched);
                          record_.version =
                            HTTP_Tools::VersionToType (match_results[1].str ());
                        }
                        
    setState(35);
    headers(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Head_response_restContext ------------------------------------------------------------------

http_antlr_parser::Head_response_restContext::Head_response_restContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* http_antlr_parser::Head_response_restContext::CODE() {
  return getToken(http_antlr_parser::CODE, 0);
}

tree::TerminalNode* http_antlr_parser::Head_response_restContext::REASON() {
  return getToken(http_antlr_parser::REASON, 0);
}

tree::TerminalNode* http_antlr_parser::Head_response_restContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}

http_antlr_parser::HeadersContext* http_antlr_parser::Head_response_restContext::headers() {
  return getRuleContext<http_antlr_parser::HeadersContext>(0);
}


size_t http_antlr_parser::Head_response_restContext::getRuleIndex() const {
  return http_antlr_parser::RuleHead_response_rest;
}

void http_antlr_parser::Head_response_restContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHead_response_rest(this);
}

void http_antlr_parser::Head_response_restContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHead_response_rest(this);
}

http_antlr_parser::Head_response_restContext* http_antlr_parser::head_response_rest() {
  Head_response_restContext *_localctx = _tracker.createInstance<Head_response_restContext>(_ctx, getState());
  enterRule(_localctx, 6, http_antlr_parser::RuleHead_response_rest);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(37);
    antlrcpp::downCast<Head_response_restContext *>(_localctx)->codeToken = match(http_antlr_parser::CODE);

                        { std::istringstream converter;
                          converter.str (antlrcpp::downCast<Head_response_restContext *>(_localctx)->codeToken->getText ());
                          int code_i;
                          converter >> code_i;
                          record_.status =
                            static_cast<HTTP_Codes::StatusType> (code_i);
                        }
                        
    setState(39);
    antlrcpp::downCast<Head_response_restContext *>(_localctx)->reasonToken = match(http_antlr_parser::REASON);

                          record_.reason = antlrcpp::downCast<Head_response_restContext *>(_localctx)->reasonToken->getText ();
                        
    setState(41);
    match(http_antlr_parser::CRLF);
    setState(42);
    headers(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HeadersContext ------------------------------------------------------------------

http_antlr_parser::HeadersContext::HeadersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::HeadersContext* http_antlr_parser::HeadersContext::headers() {
  return getRuleContext<http_antlr_parser::HeadersContext>(0);
}

http_antlr_parser::HeaderContext* http_antlr_parser::HeadersContext::header() {
  return getRuleContext<http_antlr_parser::HeaderContext>(0);
}


size_t http_antlr_parser::HeadersContext::getRuleIndex() const {
  return http_antlr_parser::RuleHeaders;
}

void http_antlr_parser::HeadersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHeaders(this);
}

void http_antlr_parser::HeadersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHeaders(this);
}


http_antlr_parser::HeadersContext* http_antlr_parser::headers() {
   return headers(0);
}

http_antlr_parser::HeadersContext* http_antlr_parser::headers(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  http_antlr_parser::HeadersContext *_localctx = _tracker.createInstance<HeadersContext>(_ctx, parentState);
  http_antlr_parser::HeadersContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 8;
  enterRecursionRule(_localctx, 8, http_antlr_parser::RuleHeaders, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _ctx->stop = _input->LT(-1);
    setState(49);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<HeadersContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleHeaders);
        setState(45);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(46);
        header(); 
      }
      setState(51);
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

//----------------- HeaderContext ------------------------------------------------------------------

http_antlr_parser::HeaderContext::HeaderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* http_antlr_parser::HeaderContext::FIELD_KEY() {
  return getToken(http_antlr_parser::FIELD_KEY, 0);
}

tree::TerminalNode* http_antlr_parser::HeaderContext::COLON() {
  return getToken(http_antlr_parser::COLON, 0);
}

tree::TerminalNode* http_antlr_parser::HeaderContext::FIELD_VALUE() {
  return getToken(http_antlr_parser::FIELD_VALUE, 0);
}

tree::TerminalNode* http_antlr_parser::HeaderContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}


size_t http_antlr_parser::HeaderContext::getRuleIndex() const {
  return http_antlr_parser::RuleHeader;
}

void http_antlr_parser::HeaderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterHeader(this);
}

void http_antlr_parser::HeaderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitHeader(this);
}

http_antlr_parser::HeaderContext* http_antlr_parser::header() {
  HeaderContext *_localctx = _tracker.createInstance<HeaderContext>(_ctx, getState());
  enterRule(_localctx, 10, http_antlr_parser::RuleHeader);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(52);
    antlrcpp::downCast<HeaderContext *>(_localctx)->field_keyToken = match(http_antlr_parser::FIELD_KEY);
    setState(53);
    match(http_antlr_parser::COLON);
    setState(54);
    antlrcpp::downCast<HeaderContext *>(_localctx)->field_valueToken = match(http_antlr_parser::FIELD_VALUE);
    setState(55);
    match(http_antlr_parser::CRLF);

                          record_.headers[antlrcpp::downCast<HeaderContext *>(_localctx)->field_keyToken->getText ()] = antlrcpp::downCast<HeaderContext *>(_localctx)->field_valueToken->getText ();
                        
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BodyContext ------------------------------------------------------------------

http_antlr_parser::BodyContext::BodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* http_antlr_parser::BodyContext::BODY() {
  return getToken(http_antlr_parser::BODY, 0);
}

tree::TerminalNode* http_antlr_parser::BodyContext::CHUNK() {
  return getToken(http_antlr_parser::CHUNK, 0);
}

http_antlr_parser::Chunked_bodyContext* http_antlr_parser::BodyContext::chunked_body() {
  return getRuleContext<http_antlr_parser::Chunked_bodyContext>(0);
}


size_t http_antlr_parser::BodyContext::getRuleIndex() const {
  return http_antlr_parser::RuleBody;
}

void http_antlr_parser::BodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBody(this);
}

void http_antlr_parser::BodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBody(this);
}

http_antlr_parser::BodyContext* http_antlr_parser::body() {
  BodyContext *_localctx = _tracker.createInstance<BodyContext>(_ctx, getState());
  enterRule(_localctx, 12, http_antlr_parser::RuleBody);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(63);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case http_antlr_parser::BODY: {
        enterOuterAlt(_localctx, 1);
        setState(58);
        antlrcpp::downCast<BodyContext *>(_localctx)->bodyToken = match(http_antlr_parser::BODY);

                            {
                              std::istringstream converter;
                              converter.str (antlrcpp::downCast<BodyContext *>(_localctx)->bodyToken->getText ());
                              converter >> content_length_;
                            }
                            
        break;
      }

      case http_antlr_parser::CHUNK: {
        enterOuterAlt(_localctx, 2);
        setState(60);
        antlrcpp::downCast<BodyContext *>(_localctx)->chunkToken = match(http_antlr_parser::CHUNK);

                            {
                              std::istringstream converter;
                              converter.str (antlrcpp::downCast<BodyContext *>(_localctx)->chunkToken->getText ());
                              ACE_UINT32 chunk_size_i;
                              converter >> chunk_size_i;
                              content_length_ += chunk_size_i;
                            }
                            
        setState(62);
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

http_antlr_parser::Chunked_bodyContext::Chunked_bodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::ChunksContext* http_antlr_parser::Chunked_bodyContext::chunks() {
  return getRuleContext<http_antlr_parser::ChunksContext>(0);
}

http_antlr_parser::HeadersContext* http_antlr_parser::Chunked_bodyContext::headers() {
  return getRuleContext<http_antlr_parser::HeadersContext>(0);
}

tree::TerminalNode* http_antlr_parser::Chunked_bodyContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}


size_t http_antlr_parser::Chunked_bodyContext::getRuleIndex() const {
  return http_antlr_parser::RuleChunked_body;
}

void http_antlr_parser::Chunked_bodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterChunked_body(this);
}

void http_antlr_parser::Chunked_bodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitChunked_body(this);
}

http_antlr_parser::Chunked_bodyContext* http_antlr_parser::chunked_body() {
  Chunked_bodyContext *_localctx = _tracker.createInstance<Chunked_bodyContext>(_ctx, getState());
  enterRule(_localctx, 14, http_antlr_parser::RuleChunked_body);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(65);
    chunks(0);
    setState(66);
    headers(0);
    setState(67);
    match(http_antlr_parser::CRLF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ChunksContext ------------------------------------------------------------------

http_antlr_parser::ChunksContext::ChunksContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::ChunksContext* http_antlr_parser::ChunksContext::chunks() {
  return getRuleContext<http_antlr_parser::ChunksContext>(0);
}

tree::TerminalNode* http_antlr_parser::ChunksContext::CHUNK() {
  return getToken(http_antlr_parser::CHUNK, 0);
}


size_t http_antlr_parser::ChunksContext::getRuleIndex() const {
  return http_antlr_parser::RuleChunks;
}

void http_antlr_parser::ChunksContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterChunks(this);
}

void http_antlr_parser::ChunksContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitChunks(this);
}


http_antlr_parser::ChunksContext* http_antlr_parser::chunks() {
   return chunks(0);
}

http_antlr_parser::ChunksContext* http_antlr_parser::chunks(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  http_antlr_parser::ChunksContext *_localctx = _tracker.createInstance<ChunksContext>(_ctx, parentState);
  http_antlr_parser::ChunksContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 16;
  enterRecursionRule(_localctx, 16, http_antlr_parser::RuleChunks, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _ctx->stop = _input->LT(-1);
    setState(75);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ChunksContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleChunks);
        setState(70);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(71);
        antlrcpp::downCast<ChunksContext *>(_localctx)->chunkToken = match(http_antlr_parser::CHUNK);

                                      {
                                        std::istringstream converter;
                                        converter.str (antlrcpp::downCast<ChunksContext *>(_localctx)->chunkToken->getText ());
                                        ACE_UINT32 chunk_size_i;
                                        converter >> chunk_size_i;
                                        content_length_ += chunk_size_i;
                                      }
                                       
      }
      setState(77);
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

bool http_antlr_parser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 4: return headersSempred(antlrcpp::downCast<HeadersContext *>(context), predicateIndex);
    case 8: return chunksSempred(antlrcpp::downCast<ChunksContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool http_antlr_parser::headersSempred(HeadersContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

bool http_antlr_parser::chunksSempred(ChunksContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 2);

  default:
    break;
  }
  return true;
}

void http_antlr_parser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  http_antlr_parserParserInitialize();
#else
  ::antlr4::internal::call_once(http_antlr_parserParserOnceFlag, http_antlr_parserParserInitialize);
#endif
}
