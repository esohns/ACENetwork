parser grammar http_antlr_parser;

options {
/*  defaultErrorHandler = false;*/
  tokenVocab = http_antlr_scanner;
}

@header {
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
}

@members {
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
}

document:           head CRLF body EOF;
head:               METHOD {
                      record_.method = HTTP_Tools::MethodToType ($METHOD->getText ());
                    } head_request_rest
                    | VERSION {
                    { std::string input_string = $VERSION->getText ();
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
                    } head_response_rest;
head_request_rest:  URI VERSION CRLF {
                    { record_.URI = $URI->getText ();

                      std::string input_string = $VERSION->getText ();
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
                    } headers;
head_response_rest: CODE {
                    { std::istringstream converter;
                      converter.str ($CODE->getText ());
                      int code_i;
                      converter >> code_i;
                      record_.status =
                        static_cast<HTTP_Codes::StatusType> (code_i);
                    }
                    } REASON {
                      record_.reason = $REASON->getText ();
                    } CRLF headers;
headers:            headers header
                    |;
header:             FIELD_KEY COLON FIELD_VALUE CRLF {
                      record_.headers[$FIELD_KEY->getText ()] = $FIELD_VALUE->getText ();
                    };
body:               BODY {
                    {
                      std::istringstream converter;
                      converter.str ($BODY->getText ());
                      converter >> content_length_;
                    }
                    }
                    | CHUNK {
                    {
                      std::istringstream converter;
                      converter.str ($CHUNK->getText ());
                      ACE_UINT32 chunk_size_i;
                      converter >> chunk_size_i;
                      content_length_ += chunk_size_i;
                    }
                    } chunked_body;
chunked_body:       chunks headers CRLF;
chunks:             chunks CHUNK {
                    {
                      std::istringstream converter;
                      converter.str ($CHUNK->getText ());
                      ACE_UINT32 chunk_size_i;
                      converter >> chunk_size_i;
                      content_length_ += chunk_size_i;
                    }
                    }
                    |;
