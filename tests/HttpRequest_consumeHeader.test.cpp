#include <gtest/gtest.h>

#include <stdexcept>
#include <sstream>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

// ============== HeaderStart struct and helper ==============
struct HeaderStart {
  std::string reqbuf;    //　start line (request line) + headers + body
  const char* p_headers; // points to the beginning of headers in reqbuf
  RequestMethod method;
};

static HeaderStart makeHeaderStart(const std::string& method,
                                   const std::string& uri,
                                   const std::string& version,
                                   const std::string& headers_and_after = "") {
  HeaderStart hs;
  std::string startLine = method + " " + uri + " " + version + "\r\n";
  hs.reqbuf = startLine + headers_and_after;
  hs.p_headers = hs.reqbuf.c_str() + startLine.size();
  if (method == "GET") hs.method = GET;
  else if (method == "HEAD") hs.method = HEAD;
  else if (method == "POST") hs.method = POST;
  else if (method == "DELETE") hs.method = DELETE;
  else hs.method = UNKNOWN_METHOD;
  return hs;
}

class HttpRequestconsumeHeader : public ::testing::Test { // fixture class for google test
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
TEST_F(HttpRequestconsumeHeader, Basic_HostOnly_NoBody_ParsesAndAdvances) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "\r\n"
      "ABC";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.consumeHeader(hs.p_headers));
  EXPECT_EQ("example.com", req.getHostName());
  EXPECT_EQ(DEFAULT_PORT, req.getHostPort());
  EXPECT_EQ(0, req.getContentLength());
  EXPECT_STREQ("ABC", std::string(ret).c_str());
}

TEST_F(HttpRequestconsumeHeader, HostWithPort_ParsesHostnameAndPort) {
  std::string headers_and_after =
      "Host: example.com:8080\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.consumeHeader(hs.p_headers));
  EXPECT_EQ("example.com", req.getHostName());
  EXPECT_EQ("8080", req.getHostPort());
  EXPECT_EQ(0, req.getContentLength());
}

// =============== Case-insensitive keys ===============
TEST_F(HttpRequestconsumeHeader, HeaderFieldName_IsCaseInsensitive) {
  std::string headers_and_after =
      "host: example.org\r\n"
      "cOnNeCtIoN: close\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.consumeHeader(hs.p_headers));
  EXPECT_EQ("example.org", req.getHostName());
  EXPECT_FALSE(req.isKeepAlive());
}

// =============== Error path: Host ===============
TEST_F(HttpRequestconsumeHeader, MissingHost_ThrowsBadRequest) {
  std::string headers_and_after =
      "Connection: keep-alive\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestconsumeHeader, EmptyHost_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: \r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// =============== Content-Length / Transfer-Encoding ===============
TEST_F(HttpRequestconsumeHeader, ContentLengthAndTransferEncodingTogether_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// if method is POST and neither Content-Length nor Transfer-Encoding is present, throw LENGTH_REQUIRED
TEST_F(HttpRequestconsumeHeader, PostWithoutCLorTE_ThrowsLengthRequired) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);
  req.setMethod(hs.method);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(LENGTH_REQUIRED, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// =============== Content-Length の検証 ===============
TEST_F(HttpRequestconsumeHeader, ContentLength_Valid_SetsContentLength) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "12345";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.consumeHeader(hs.p_headers));
  EXPECT_EQ(5, req.getContentLength());
  EXPECT_EQ('1', *ret);
}

TEST_F(HttpRequestconsumeHeader, ContentLength_NonNumeric_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: x\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestconsumeHeader, ContentLength_TooLarge_ThrowsBadRequest) {
  std::stringstream ss;
  ss << (HttpRequest::kMaxPayloadSize + 1);
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: " + ss.str() + "\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// =============== Transfer-Encoding ===============
TEST_F(HttpRequestconsumeHeader, TransferEncoding_Chunked_SetsMinusOne) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.consumeHeader(hs.p_headers));
  EXPECT_EQ(-1, req.getContentLength());
}

TEST_F(HttpRequestconsumeHeader, TransferEncoding_Unknown_ThrowsNotImplemented) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Transfer-Encoding: gzip\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(NOT_IMPLEMENTED, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// =============== Connection ===============
TEST_F(HttpRequestconsumeHeader, Connection_KeepAliveAndClose) {
  { // keep-alive
    HttpRequest r1;
    std::string headers_and_after =
        "Host: example.com\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);
    ASSERT_NO_THROW(r1.consumeHeader(hs.p_headers));
    EXPECT_TRUE(r1.isKeepAlive());
  }
  { // close
    HttpRequest r2;
    std::string headers_and_after =
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);
    ASSERT_NO_THROW(r2.consumeHeader(hs.p_headers));
    EXPECT_FALSE(r2.isKeepAlive());
  }
}

TEST_F(HttpRequestconsumeHeader, Connection_InvalidToken_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Connection: upgrade\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// =============== Bad requests ===============
TEST_F(HttpRequestconsumeHeader, MissingColon_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestconsumeHeader, NoSpaceAfterColon_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host:example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestconsumeHeader, MissingCRLF_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\n"
      "\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestconsumeHeader, ExceedMaxHeaderSize_ThrowsRequestHeaderFieldsTooLarge) {
  std::string bigValue(HttpRequest::kMaxHeaderSize, 'a');
  std::string headers_and_after =
      "Host: example.com\r\n"
      "X-Long: " + bigValue + "\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.consumeHeader(hs.p_headers);
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(REQUEST_HEADER_FIELDS_TOO_LARGE, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}
