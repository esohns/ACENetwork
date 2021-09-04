
// Generated from HTTPParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"


namespace antlrcpptest {


class  HTTPParser : public antlr4::Parser {
public:
  enum {
    CRLF = 1, VERSION = 2, URI = 3, HEADER = 4, METHOD = 5, REASON = 6, 
    STATUS = 7, BODY = 8, CHUNK = 9
  };

  enum {
    RuleMain = 0, RuleMessage = 1, RuleHead = 2, RuleHead_rest1 = 3, RuleRequest_line_rest1 = 4, 
    RuleRequest_line_rest2 = 5, RuleHead_rest2 = 6, RuleStatus_line_rest1 = 7, 
    RuleStatus_line_rest2 = 8, RuleHeaders = 9, RuleBody = 10, RuleChunked_body = 11, 
    RuleChunks = 12
  };

  HTTPParser(antlr4::TokenStream *input);
  ~HTTPParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class MainContext;
  class MessageContext;
  class HeadContext;
  class Head_rest1Context;
  class Request_line_rest1Context;
  class Request_line_rest2Context;
  class Head_rest2Context;
  class Status_line_rest1Context;
  class Status_line_rest2Context;
  class HeadersContext;
  class BodyContext;
  class Chunked_bodyContext;
  class ChunksContext; 

  class  MainContext : public antlr4::ParserRuleContext {
  public:
    MainContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MessageContext *message();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MainContext* main();

  class  MessageContext : public antlr4::ParserRuleContext {
  public:
    MessageContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    HeadContext *head();
    antlr4::tree::TerminalNode *CRLF();
    BodyContext *body();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MessageContext* message();

  class  HeadContext : public antlr4::ParserRuleContext {
  public:
    HeadContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *METHOD();
    Head_rest1Context *head_rest1();
    antlr4::tree::TerminalNode *VERSION();
    Head_rest2Context *head_rest2();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  HeadContext* head();

  class  Head_rest1Context : public antlr4::ParserRuleContext {
  public:
    Head_rest1Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Request_line_rest1Context *request_line_rest1();
    HeadersContext *headers();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Head_rest1Context* head_rest1();

  class  Request_line_rest1Context : public antlr4::ParserRuleContext {
  public:
    Request_line_rest1Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *URI();
    Request_line_rest2Context *request_line_rest2();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Request_line_rest1Context* request_line_rest1();

  class  Request_line_rest2Context : public antlr4::ParserRuleContext {
  public:
    Request_line_rest2Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VERSION();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Request_line_rest2Context* request_line_rest2();

  class  Head_rest2Context : public antlr4::ParserRuleContext {
  public:
    Head_rest2Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Status_line_rest1Context *status_line_rest1();
    HeadersContext *headers();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Head_rest2Context* head_rest2();

  class  Status_line_rest1Context : public antlr4::ParserRuleContext {
  public:
    Status_line_rest1Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STATUS();
    Status_line_rest2Context *status_line_rest2();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Status_line_rest1Context* status_line_rest1();

  class  Status_line_rest2Context : public antlr4::ParserRuleContext {
  public:
    Status_line_rest2Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REASON();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Status_line_rest2Context* status_line_rest2();

  class  HeadersContext : public antlr4::ParserRuleContext {
  public:
    HeadersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    HeadersContext *headers();
    antlr4::tree::TerminalNode *HEADER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  HeadersContext* headers();
  HeadersContext* headers(int precedence);
  class  BodyContext : public antlr4::ParserRuleContext {
  public:
    BodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BODY();
    antlr4::tree::TerminalNode *CHUNK();
    Chunked_bodyContext *chunked_body();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Chunked_bodyContext* chunked_body();

  class  ChunksContext : public antlr4::ParserRuleContext {
  public:
    ChunksContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ChunksContext *chunks();
    antlr4::tree::TerminalNode *CHUNK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ChunksContext* chunks();
  ChunksContext* chunks(int precedence);

  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool headersSempred(HeadersContext *_localctx, size_t predicateIndex);
  bool chunksSempred(ChunksContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace antlrcpptest
