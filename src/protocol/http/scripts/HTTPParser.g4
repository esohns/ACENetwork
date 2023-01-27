parser grammar HTTPParser;

options {
	tokenVocab = HTTPScanner;
}

// Actual grammar start.
main:               message EOF;
message:            head CRLF body;
head:               METHOD head_rest1
                    | VERSION head_rest2;
head_rest1:         request_line_rest1 headers;
request_line_rest1: URI request_line_rest2;
request_line_rest2: VERSION;
head_rest2:         status_line_rest1 headers;
status_line_rest1:  STATUS status_line_rest2;
status_line_rest2:  REASON;
headers:            <assoc = right> headers HEADER
                    |;
body:               BODY
                    | CHUNK chunked_body;
chunked_body:       chunks headers CRLF;
chunks:             <assoc = right> chunks CHUNK
                    |;
