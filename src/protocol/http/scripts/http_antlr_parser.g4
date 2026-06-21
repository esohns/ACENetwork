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

#include "http_antlr_scanner.h"
}

@members {
 public:
  std::vector<std::pair<ACE_UINT64, ACE_UINT32> > chunks_;
  size_t                                          content_length_;
  struct HTTP_Record                              record_;

  // Helper function to extract token text
  inline std::string getTxt (antlr4::Token* tok) { return tok ? tok->getText () : ACE_TEXT_ALWAYS_CHAR (""); }
}

// Actual grammar start.
main:               message EOF;
message:            head CRLF body;
head:               m=METHOD {
                      record_.method = HTTP_Tools::MethodToType ($m.text);
                    } head_request_rest
                    | v=VERSION {
                    { std::string input_string = $v.text;
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
head_request_rest:  u=URI v=VERSION CRLF {
                    { record_.URI = $u.text;

                      std::string input_string = $v.text;
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
head_response_rest: c=CODE r=REASON CRLF {
                    { std::istringstream converter;
                      converter.str ($c.text);
                      int code_i;
                      converter >> code_i;
                      record_.status =
                        static_cast<HTTP_Codes::StatusType> (code_i);

                      record_.reason = $r.text;
                    }
                    } headers;
headers:            <assoc = right> headers header
                    |;
header:             k=FIELD_KEY COLON v=FIELD_VALUE CRLF {
                      record_.headers[$k.text] = $v.text;
                    };
body:               b=BODY {
                    {
                      std::istringstream converter;
                      converter.str ($b.text);
                      converter >> content_length_;
                    }
                    }
                    | c=CHUNK {
                    {
                      http_antlr_scanner* scanner_p =
                        static_cast<http_antlr_scanner*> (getTokenStream ()->getTokenSource ());
                      ACE_ASSERT (scanner_p);
                      std::istringstream converter;
                      converter.str ($c.text);
                      int chunk_size_i;
                      converter >> chunk_size_i;
                      content_length_ += chunk_size_i;
                      chunks_.clear ();
                      chunks_.push_back (std::make_pair (scanner_p->offset, chunk_size_i));
                    }
                    } chunked_body;
chunked_body:       chunks headers CRLF;
chunks:             <assoc = right> chunks c=CHUNK {
                    {
                      http_antlr_scanner* scanner_p =
                        static_cast<http_antlr_scanner*> (getTokenStream ()->getTokenSource ());
                      ACE_ASSERT (scanner_p);
                      std::istringstream converter;
                      converter.str ($c.text);
                      int chunk_size_i;
                      converter >> chunk_size_i;
                      content_length_ += chunk_size_i;
                      chunks_.push_back (std::make_pair (scanner_p->offset, chunk_size_i));
                    }
                    }
                    |;
