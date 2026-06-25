
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

#pragma once


#include "antlr4-runtime.h"




class  http_antlr_parser : public antlr4::Parser {
public:
  enum {
    METHOD = 1, URI = 2, VERSION = 3, CODE = 4, REASON = 5, FIELD_KEY = 6, 
    COLON = 7, FIELD_VALUE = 8, CRLF_HEAD = 9, CRLF = 10, CHUNK = 11, SP_RESP = 12, 
    SP_CODE = 13, BODY = 14, CHUNK_DATA = 15
  };

  enum {
    RuleDocument = 0, RuleHead = 1, RuleHead_request_rest = 2, RuleHead_response_rest = 3, 
    RuleHeaders = 4, RuleHeader = 5, RuleBody = 6, RuleChunked_body = 7, 
    RuleChunks = 8
  };

  explicit http_antlr_parser(antlr4::TokenStream *input);

  http_antlr_parser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~http_antlr_parser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


   public:
    size_t              content_length_;
    HTTP_ANTLR_IParser* parser_;
    struct HTTP_Record  record_;

    void reset_2 ()
    {
      content_length_ = 0;
      parser_ = NULL;
      record_.reset ();
    }

    // inline std::string getTxt (antlr4::Token* tok) { return tok ? tok->getText () : ACE_TEXT_ALWAYS_CHAR (""); }


  class DocumentContext;
  class HeadContext;
  class Head_request_restContext;
  class Head_response_restContext;
  class HeadersContext;
  class HeaderContext;
  class BodyContext;
  class Chunked_bodyContext;
  class ChunksContext; 

  class  DocumentContext : public antlr4::ParserRuleContext {
  public:
    DocumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    HeadContext *head();
    antlr4::tree::TerminalNode *CRLF();
    BodyContext *body();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  DocumentContext* document();

  class  HeadContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *methodToken = nullptr;
    antlr4::Token *versionToken = nullptr;
    HeadContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *METHOD();
    Head_request_restContext *head_request_rest();
    antlr4::tree::TerminalNode *VERSION();
    Head_response_restContext *head_response_rest();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  HeadContext* head();

  class  Head_request_restContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *uriToken = nullptr;
    antlr4::Token *versionToken = nullptr;
    Head_request_restContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *URI();
    antlr4::tree::TerminalNode *VERSION();
    antlr4::tree::TerminalNode *CRLF();
    HeadersContext *headers();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Head_request_restContext* head_request_rest();

  class  Head_response_restContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *codeToken = nullptr;
    antlr4::Token *reasonToken = nullptr;
    Head_response_restContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CODE();
    antlr4::tree::TerminalNode *REASON();
    antlr4::tree::TerminalNode *CRLF();
    HeadersContext *headers();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Head_response_restContext* head_response_rest();

  class  HeadersContext : public antlr4::ParserRuleContext {
  public:
    HeadersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    HeadersContext *headers();
    HeaderContext *header();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  HeadersContext* headers();
  HeadersContext* headers(int precedence);
  class  HeaderContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *field_keyToken = nullptr;
    antlr4::Token *field_valueToken = nullptr;
    HeaderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FIELD_KEY();
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *FIELD_VALUE();
    antlr4::tree::TerminalNode *CRLF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  HeaderContext* header();

  class  BodyContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *bodyToken = nullptr;
    antlr4::Token *chunkToken = nullptr;
    BodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BODY();
    antlr4::tree::TerminalNode *CHUNK();
    Chunked_bodyContext *chunked_body();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  BodyContext* body();

  class  Chunked_bodyContext : public antlr4::ParserRuleContext {
  public:
    Chunked_bodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ChunksContext *chunks();
    HeadersContext *headers();
    antlr4::tree::TerminalNode *CRLF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Chunked_bodyContext* chunked_body();

  class  ChunksContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *chunkToken = nullptr;
    ChunksContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ChunksContext *chunks();
    antlr4::tree::TerminalNode *CHUNK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ChunksContext* chunks();
  ChunksContext* chunks(int precedence);

  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool headersSempred(HeadersContext *_localctx, size_t predicateIndex);
  bool chunksSempred(ChunksContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

