
#include <regex>
#include <sstream>
#include <string>

#include "common_string_tools.h"

#include "http_common.h"
#include "http_defines.h"


// Generated from /mnt/win_d/projects/ACENetwork/src/protocol/http/scripts/http_antlr_scanner.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  http_antlr_scanner : public antlr4::Lexer {
public:
  enum {
    METHOD = 1, URI = 2, VERSION = 3, CODE = 4, REASON = 5, FIELD_KEY = 6, 
    COLON = 7, FIELD_VALUE = 8, CRLF_HEAD = 9, CRLF = 10, CHUNK = 11, SP_RESP = 12, 
    SP_CODE = 13, BODY = 14, CHUNK_DATA = 15
  };

  enum {
    RESPONSE_SP = 1, STATUS_CODE = 2, REASON_STRING = 3, HEADERS = 4, HEAD = 5, 
    HEAD_VALUE = 6, HEAD_END = 7, REGULAR_BODY = 8, CHUNKED_BODY = 9, CHUNKED_BODY_END = 10, 
    CHUNKED_DATA = 11
  };

  explicit http_antlr_scanner(antlr4::CharStream *input);

  ~http_antlr_scanner() override;


   public:
    size_t             chunk_offset;
    size_t             content_length;
    std::string        key;
    size_t             missing_body_or_chunk_bytes;
    size_t             offset;
    struct HTTP_Record record;

    void reset_2 ()
    {
      chunk_offset = 0;
      content_length = 0;
      missing_body_or_chunk_bytes = 0;
      offset = 0;
    }


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.
  void VERSIONAction(antlr4::RuleContext *context, size_t actionIndex);
  void SP_RESPAction(antlr4::RuleContext *context, size_t actionIndex);
  void CODEAction(antlr4::RuleContext *context, size_t actionIndex);
  void SP_CODEAction(antlr4::RuleContext *context, size_t actionIndex);
  void REASONAction(antlr4::RuleContext *context, size_t actionIndex);
  void CRLF_REASONAction(antlr4::RuleContext *context, size_t actionIndex);
  void KEY_HEADAction(antlr4::RuleContext *context, size_t actionIndex);
  void CRLF_HEADERSAction(antlr4::RuleContext *context, size_t actionIndex);
  void COLON_SPAction(antlr4::RuleContext *context, size_t actionIndex);
  void VALUE_HEADAction(antlr4::RuleContext *context, size_t actionIndex);
  void CRLF_HEADAction(antlr4::RuleContext *context, size_t actionIndex);
  void BODYAction(antlr4::RuleContext *context, size_t actionIndex);
  void CHUNK_LASTAction(antlr4::RuleContext *context, size_t actionIndex);
  void CHUNKAction(antlr4::RuleContext *context, size_t actionIndex);
  void CRLF_CHUNKED_BODYAction(antlr4::RuleContext *context, size_t actionIndex);
  void CHUNK_DATAAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.

};

