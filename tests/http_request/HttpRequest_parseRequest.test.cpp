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
  const char* data = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
  req.Parse(data, strlen(data));
  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}


// Incremental parsing (calling ParseRequest multiple times with partial data)
TEST_F(HttpRequestParseRequest, ParseRequest_IncrementalParsing) {
  const char* data = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n";
  req.Parse(data, strlen(data));
  EXPECT_EQ(req.GetProgress(), HttpRequest::kHeader); // still parsing header

  const char* data2 = "\r\n"; // end of headers 
  req.Parse(data2, strlen(data2));
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);

  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
}

// Requests with body content (POST with Content-Length)
TEST_F(HttpRequestParseRequest, ParseRequest_IncrementalParsing_WithBodyContent) {
  // only header part in the first call
  const char* data = "POST /submit HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Length: 11\r\n"
    "\r\n";
  req.Parse(data, strlen(data));
  // progress should be kBody, body is empty at this point
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
  EXPECT_EQ(req.GetBody(), "");
  // partial body in the second call
  const char* data2 = "Hello ";
  req.Parse(data2, strlen(data2));
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);  // still not enough
  EXPECT_EQ(req.GetBody(), ""); // body not yet advanced at this point
  // complete body in the third call
  const char* data3 = "World";
  req.Parse(data3, strlen(data3));
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
  const char* data = "POST /submit HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\n";
  req.Parse(data, strlen(data));
  EXPECT_EQ(req.GetMethod(), lib::http::kPost);
  EXPECT_EQ(req.GetUri(), "/submit");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetHeader().at("transfer-encoding"), "chunked");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestParseRequest, ParseRequest_WithChunkedBody_CompleteInOneCall) {
  const char* data = "POST /submit HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5\r\n"
    "Hello\r\n"
    "6\r\n"
    " World\r\n"
    "0\r\n"
    "\r\n";
  req.Parse(data, strlen(data));
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
  const char* data = "POST /submit HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5\r\n"
    "Hello\r\n";
  req.Parse(data, strlen(data));
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
  EXPECT_EQ(req.GetBody(), "Hello");

  // second chunk + terminator
  const char* data2 = "6\r\n"
    " World\r\n"
    "0\r\n"
    "\r\n";
  req.Parse(data2, strlen(data2));
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
  EXPECT_EQ(req.GetBody(), "Hello World");
}

// =============== Error cases: parse request ===============
TEST_F(HttpRequestParseRequest, ParseRequest_Error_ExtraDataAfterDone) {
  const char* data = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n"; 
  req.Parse(data, strlen(data));
  const char* data2 = "EXTRA DATA";
  EXPECT_THROW(req.Parse(data2, strlen(data2)), lib::exception::ResponseStatusException);
}

// Malformed requests (invalid headers, missing required fields)
TEST_F(HttpRequestParseRequest, ParseRequest_Error_MissingHostHeader) {
  const char* data = "GET /index.html HTTP/1.1\r\n\r\n";
  EXPECT_THROW(req.Parse(data, strlen(data)), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParseRequest, ParseRequest_Error_InvalidHeaderFormat) {
  const char* data = "GET /index.html HTTP/1.1\r\nInvalid-Header\r\n\r\n";
  EXPECT_THROW(req.Parse(data, strlen(data)), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParseRequest, ParseRequest_Error_ExceedMaxHeaderSize) {
  std::string largeHeader = "GET /index.html HTTP/1.1\r\n";
  largeHeader += std::string(HttpRequest::kMaxHeaderSize, 'A') + ": value\r\n\r\n";
  EXPECT_THROW(req.Parse(largeHeader.c_str(), strlen(largeHeader.c_str())), lib::exception::ResponseStatusException);
}
