
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
      "main", "message", "head", "head_request_rest", "head_response_rest", 
      "headers", "header", "body", "chunked_body", "chunks"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "'\\u000D\\u000A'"
    },
    std::vector<std::string>{
      "", "METHOD", "URI", "VERSION", "CODE", "REASON", "FIELD_KEY", "COLON", 
      "FIELD_VALUE", "CRLF_HEAD", "CRLF", "CHUNK", "SP_RESP", "SP_CODE", 
      "BODY", "CHUNK_DATA"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,15,83,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,
  	7,7,2,8,7,8,2,9,7,9,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,2,1,2,1,
  	2,3,2,34,8,2,1,3,1,3,1,3,1,3,1,3,1,3,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,5,
  	1,5,1,5,5,5,52,8,5,10,5,12,5,55,9,5,1,6,1,6,1,6,1,6,1,6,1,6,1,7,1,7,1,
  	7,1,7,1,7,3,7,68,8,7,1,8,1,8,1,8,1,8,1,9,1,9,1,9,1,9,5,9,78,8,9,10,9,
  	12,9,81,9,9,1,9,0,2,10,18,10,0,2,4,6,8,10,12,14,16,18,0,0,76,0,20,1,0,
  	0,0,2,23,1,0,0,0,4,33,1,0,0,0,6,35,1,0,0,0,8,41,1,0,0,0,10,48,1,0,0,0,
  	12,56,1,0,0,0,14,67,1,0,0,0,16,69,1,0,0,0,18,73,1,0,0,0,20,21,3,2,1,0,
  	21,22,5,0,0,1,22,1,1,0,0,0,23,24,3,4,2,0,24,25,5,10,0,0,25,26,3,14,7,
  	0,26,3,1,0,0,0,27,28,5,1,0,0,28,29,6,2,-1,0,29,34,3,6,3,0,30,31,5,3,0,
  	0,31,32,6,2,-1,0,32,34,3,8,4,0,33,27,1,0,0,0,33,30,1,0,0,0,34,5,1,0,0,
  	0,35,36,5,2,0,0,36,37,5,3,0,0,37,38,5,10,0,0,38,39,6,3,-1,0,39,40,3,10,
  	5,0,40,7,1,0,0,0,41,42,5,4,0,0,42,43,6,4,-1,0,43,44,5,5,0,0,44,45,6,4,
  	-1,0,45,46,5,10,0,0,46,47,3,10,5,0,47,9,1,0,0,0,48,53,6,5,-1,0,49,50,
  	10,2,0,0,50,52,3,12,6,0,51,49,1,0,0,0,52,55,1,0,0,0,53,51,1,0,0,0,53,
  	54,1,0,0,0,54,11,1,0,0,0,55,53,1,0,0,0,56,57,5,6,0,0,57,58,5,7,0,0,58,
  	59,5,8,0,0,59,60,5,10,0,0,60,61,6,6,-1,0,61,13,1,0,0,0,62,63,5,14,0,0,
  	63,68,6,7,-1,0,64,65,5,11,0,0,65,66,6,7,-1,0,66,68,3,16,8,0,67,62,1,0,
  	0,0,67,64,1,0,0,0,68,15,1,0,0,0,69,70,3,18,9,0,70,71,3,10,5,0,71,72,5,
  	10,0,0,72,17,1,0,0,0,73,79,6,9,-1,0,74,75,10,2,0,0,75,76,5,11,0,0,76,
  	78,6,9,-1,0,77,74,1,0,0,0,78,81,1,0,0,0,79,77,1,0,0,0,79,80,1,0,0,0,80,
  	19,1,0,0,0,81,79,1,0,0,0,4,33,53,67,79
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


//----------------- MainContext ------------------------------------------------------------------

http_antlr_parser::MainContext::MainContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::MessageContext* http_antlr_parser::MainContext::message() {
  return getRuleContext<http_antlr_parser::MessageContext>(0);
}

tree::TerminalNode* http_antlr_parser::MainContext::EOF() {
  return getToken(http_antlr_parser::EOF, 0);
}


size_t http_antlr_parser::MainContext::getRuleIndex() const {
  return http_antlr_parser::RuleMain;
}

void http_antlr_parser::MainContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMain(this);
}

void http_antlr_parser::MainContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMain(this);
}

http_antlr_parser::MainContext* http_antlr_parser::main() {
  MainContext *_localctx = _tracker.createInstance<MainContext>(_ctx, getState());
  enterRule(_localctx, 0, http_antlr_parser::RuleMain);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(20);
    message();
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

//----------------- MessageContext ------------------------------------------------------------------

http_antlr_parser::MessageContext::MessageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

http_antlr_parser::HeadContext* http_antlr_parser::MessageContext::head() {
  return getRuleContext<http_antlr_parser::HeadContext>(0);
}

tree::TerminalNode* http_antlr_parser::MessageContext::CRLF() {
  return getToken(http_antlr_parser::CRLF, 0);
}

http_antlr_parser::BodyContext* http_antlr_parser::MessageContext::body() {
  return getRuleContext<http_antlr_parser::BodyContext>(0);
}


size_t http_antlr_parser::MessageContext::getRuleIndex() const {
  return http_antlr_parser::RuleMessage;
}

void http_antlr_parser::MessageContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMessage(this);
}

void http_antlr_parser::MessageContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<http_antlr_parserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMessage(this);
}

http_antlr_parser::MessageContext* http_antlr_parser::message() {
  MessageContext *_localctx = _tracker.createInstance<MessageContext>(_ctx, getState());
  enterRule(_localctx, 2, http_antlr_parser::RuleMessage);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(23);
    head();
    setState(24);
    match(http_antlr_parser::CRLF);
    setState(25);
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
  enterRule(_localctx, 4, http_antlr_parser::RuleHead);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(33);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case http_antlr_parser::METHOD: {
        enterOuterAlt(_localctx, 1);
        setState(27);
        antlrcpp::downCast<HeadContext *>(_localctx)->methodToken = match(http_antlr_parser::METHOD);

                              record_.method = HTTP_Tools::MethodToType (antlrcpp::downCast<HeadContext *>(_localctx)->methodToken->getText ());
                            
        setState(29);
        head_request_rest();
        break;
      }

      case http_antlr_parser::VERSION: {
        enterOuterAlt(_localctx, 2);
        setState(30);
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
                            
        setState(32);
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
  enterRule(_localctx, 6, http_antlr_parser::RuleHead_request_rest);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(35);
    antlrcpp::downCast<Head_request_restContext *>(_localctx)->uriToken = match(http_antlr_parser::URI);
    setState(36);
    antlrcpp::downCast<Head_request_restContext *>(_localctx)->versionToken = match(http_antlr_parser::VERSION);
    setState(37);
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
                        
    setState(39);
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
  enterRule(_localctx, 8, http_antlr_parser::RuleHead_response_rest);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(41);
    antlrcpp::downCast<Head_response_restContext *>(_localctx)->codeToken = match(http_antlr_parser::CODE);

                        { std::istringstream converter;
                          converter.str (antlrcpp::downCast<Head_response_restContext *>(_localctx)->codeToken->getText ());
                          int code_i;
                          converter >> code_i;
                          record_.status =
                            static_cast<HTTP_Codes::StatusType> (code_i);
                        }
                        
    setState(43);
    antlrcpp::downCast<Head_response_restContext *>(_localctx)->reasonToken = match(http_antlr_parser::REASON);

                          record_.reason = antlrcpp::downCast<Head_response_restContext *>(_localctx)->reasonToken->getText ();
                        
    setState(45);
    match(http_antlr_parser::CRLF);
    setState(46);
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
  size_t startState = 10;
  enterRecursionRule(_localctx, 10, http_antlr_parser::RuleHeaders, precedence);

    

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
    setState(53);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<HeadersContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleHeaders);
        setState(49);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(50);
        header(); 
      }
      setState(55);
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
  enterRule(_localctx, 12, http_antlr_parser::RuleHeader);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(56);
    antlrcpp::downCast<HeaderContext *>(_localctx)->field_keyToken = match(http_antlr_parser::FIELD_KEY);
    setState(57);
    match(http_antlr_parser::COLON);
    setState(58);
    antlrcpp::downCast<HeaderContext *>(_localctx)->field_valueToken = match(http_antlr_parser::FIELD_VALUE);
    setState(59);
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
  enterRule(_localctx, 14, http_antlr_parser::RuleBody);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(67);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case http_antlr_parser::BODY: {
        enterOuterAlt(_localctx, 1);
        setState(62);
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
        setState(64);
        antlrcpp::downCast<BodyContext *>(_localctx)->chunkToken = match(http_antlr_parser::CHUNK);

                            {
                              http_antlr_scanner* scanner_p =
                                static_cast<http_antlr_scanner*> (getTokenStream ()->getTokenSource ());
                              ACE_ASSERT (scanner_p);
                              std::istringstream converter;
                              converter.str (antlrcpp::downCast<BodyContext *>(_localctx)->chunkToken->getText ());
                              int chunk_size_i;
                              converter >> chunk_size_i;
                              content_length_ += chunk_size_i;
                              chunks_.clear ();
                              chunks_.push_back (std::make_pair (scanner_p->chunk_offset, chunk_size_i));
                            }
                            
        setState(66);
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
  enterRule(_localctx, 16, http_antlr_parser::RuleChunked_body);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(69);
    chunks(0);
    setState(70);
    headers(0);
    setState(71);
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
  size_t startState = 18;
  enterRecursionRule(_localctx, 18, http_antlr_parser::RuleChunks, precedence);

    

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
    setState(79);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ChunksContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleChunks);
        setState(74);

        if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
        setState(75);
        antlrcpp::downCast<ChunksContext *>(_localctx)->chunkToken = match(http_antlr_parser::CHUNK);

                                      {
                                        http_antlr_scanner* scanner_p =
                                          static_cast<http_antlr_scanner*> (getTokenStream ()->getTokenSource ());
                                        ACE_ASSERT (scanner_p);
                                        std::istringstream converter;
                                        converter.str (antlrcpp::downCast<ChunksContext *>(_localctx)->chunkToken->getText ());
                                        int chunk_size_i;
                                        converter >> chunk_size_i;
                                        content_length_ += chunk_size_i;
                                        chunks_.push_back (std::make_pair (scanner_p->chunk_offset, chunk_size_i));
                                      }
                                       
      }
      setState(81);
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
    case 5: return headersSempred(antlrcpp::downCast<HeadersContext *>(context), predicateIndex);
    case 9: return chunksSempred(antlrcpp::downCast<ChunksContext *>(context), predicateIndex);

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
