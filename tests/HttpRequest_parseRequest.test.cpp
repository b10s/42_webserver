#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestParseRequest : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path: parse request ===============
TEST_F(HttpRequestParseRequest, ParseRequest_HappyPath) {
  req.ParseRequest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}


// Incremental parsing (calling ParseRequest multiple times with partial data)
TEST_F(HttpRequestParseRequest, ParseRequest_IncrementalParsing) {
  req.ParseRequest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kHeader); // still parsing header

  req.ParseRequest("\r\n"); // end of headers
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);

  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
}

// Requests with body content (POST with Content-Length)
TEST_F(HttpRequestParseRequest, ParseRequest_IncrementalParsing_WithBodyContent) {
  // only header part in the first call
  req.ParseRequest(
      "POST /submit HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 11\r\n"
      "\r\n");
  // progress should be kBody, body is empty at this point
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
  EXPECT_EQ(req.GetBody(), "");
  // partial body in the second call
  req.ParseRequest("Hello ");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);  // still not enough
  EXPECT_EQ(req.GetBody(), ""); // body not yet advanced at this point
  // complete body in the third call
  req.ParseRequest("World");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
  EXPECT_EQ(req.GetBody(), "Hello World");
  EXPECT_EQ(req.GetMethod(), lib::http::kPost);
  EXPECT_EQ(req.GetUri(), "/submit");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetHeader().at("content-length"), "11");
}

// Requests with chunked transfer encoding
TEST_F(HttpRequestParseRequest, ParseRequest_WithChunkedTransferEncoding) {
  req.ParseRequest(
      "POST /submit HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\n");
  EXPECT_EQ(req.GetMethod(), lib::http::kPost);
  EXPECT_EQ(req.GetUri(), "/submit");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetHeader().at("transfer-encoding"), "chunked");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestParseRequest, ParseRequest_WithChunkedBody_CompleteInOneCall) {
  req.ParseRequest(
      "POST /submit HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "5\r\n"
      "Hello\r\n"
      "6\r\n"
      " World\r\n"
      "0\r\n"
      "\r\n");

  EXPECT_EQ(req.GetMethod(), lib::http::kPost);
  EXPECT_EQ(req.GetUri(), "/submit");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetHeader().at("transfer-encoding"), "chunked");
  EXPECT_EQ(req.GetBody(), "Hello World");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestParseRequest, ParseRequest_IncrementalChunkedBody) {
  // header + first chunk
  req.ParseRequest(
      "POST /submit HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "5\r\n"
      "Hello\r\n");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
  EXPECT_EQ(req.GetBody(), "Hello");

  // second chunk + terminator
  req.ParseRequest(
      "6\r\n"
      " World\r\n"
      "0\r\n"
      "\r\n");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
  EXPECT_EQ(req.GetBody(), "Hello World");
}

// =============== Error cases: parse request ===============
TEST_F(HttpRequestParseRequest, ParseRequest_Error_ExtraDataAfterDone) {
  req.ParseRequest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
  EXPECT_THROW(req.ParseRequest("EXTRA DATA"), lib::exception::ResponseStatusException);
}

// Malformed requests (invalid headers, missing required fields)
TEST_F(HttpRequestParseRequest, ParseRequest_Error_MissingHostHeader) {
  EXPECT_THROW(req.ParseRequest("GET /index.html HTTP/1.1\r\n\r\n"), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParseRequest, ParseRequest_Error_InvalidHeaderFormat) {
  EXPECT_THROW(req.ParseRequest("GET /index.html HTTP/1.1\r\nInvalid-Header\r\n\r\n"), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParseRequest, ParseRequest_Error_ExceedMaxHeaderSize) {
  std::string largeHeader = "GET /index.html HTTP/1.1\r\n";
  largeHeader += std::string(HttpRequest::kMaxHeaderSize, 'A') + ": value\r\n\r\n";
  EXPECT_THROW(req.ParseRequest(largeHeader.c_str()), lib::exception::ResponseStatusException);
}