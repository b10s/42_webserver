#include <gtest/gtest.h>

#include <stdexcept>
#include <sstream>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"
#include "HttpRequest.hpp"

// ============== HeaderStart struct and helper ==============
struct HeaderStart {
  std::string reqbuf;    // start line (request line) + headers + body
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

class HttpRequestConsumeHeader : public ::testing::Test { // fixture class for google test
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
TEST_F(HttpRequestConsumeHeader, Basic_HostOnly_NoBody_ParsesAndAdvances) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "\r\n"
      "ABC";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.ConsumeHeader(hs.p_headers));
  EXPECT_EQ("example.com", req.GetHostName());
  EXPECT_EQ(HttpRequest::kDefaultPort, req.GetHostPort());
  EXPECT_EQ(0, req.GetContentLength());
  EXPECT_STREQ("ABC", ret);
}

TEST_F(HttpRequestConsumeHeader, HostWithPort_ParsesHostnameAndPort) {
  std::string headers_and_after =
      "Host: example.com:8080\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.ConsumeHeader(hs.p_headers));
  EXPECT_EQ("example.com", req.GetHostName());
  EXPECT_EQ("8080", req.GetHostPort());
  EXPECT_EQ(0, req.GetContentLength());
}

// =============== Case-insensitive keys ===============
TEST_F(HttpRequestConsumeHeader, HeaderFieldName_IsCaseInsensitive) {
  std::string headers_and_after =
      "host: example.org\r\n"
      "cOnNeCtIoN: close\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.ConsumeHeader(hs.p_headers));
  EXPECT_EQ("example.org", req.GetHostName());
  EXPECT_FALSE(req.IsKeepAlive());
}

// =============== Error path: Host ===============
TEST_F(HttpRequestConsumeHeader, MissingHost_ThrowsBadRequest) {
  std::string headers_and_after =
      "Connection: keep-alive\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, EmptyHost_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: \r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// =============== Content-Length / Transfer-Encoding ===============
TEST_F(HttpRequestConsumeHeader, ContentLengthAndTransferEncodingTogether_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// if method is POST and neither Content-Length nor Transfer-Encoding is present, throw LENGTH_REQUIRED
TEST_F(HttpRequestConsumeHeader, PostWithoutCLorTE_ThrowsLengthRequired) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);
  req.SetMethod(hs.method);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(LENGTH_REQUIRED, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// =============== Content-Length の検証 ===============
TEST_F(HttpRequestConsumeHeader, ContentLength_Valid_SetsContentLength) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "12345";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.ConsumeHeader(hs.p_headers));
  EXPECT_EQ(5, req.GetContentLength());
  EXPECT_EQ('1', *ret);
}

TEST_F(HttpRequestConsumeHeader, ContentLength_NonNumeric_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: x\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, ContentLength_TooLarge_ThrowsBadRequest) {
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
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// =============== Transfer-Encoding ===============
TEST_F(HttpRequestConsumeHeader, TransferEncoding_Chunked_SetsMinusOne) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  const char* ret = NULL;
  ASSERT_NO_THROW(ret = req.ConsumeHeader(hs.p_headers));
  EXPECT_EQ(-1, req.GetContentLength());
}

TEST_F(HttpRequestConsumeHeader, TransferEncoding_Unknown_ThrowsNotImplemented) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Transfer-Encoding: gzip\r\n"
      "\r\n";
  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(NOT_IMPLEMENTED, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// =============== Connection ===============
TEST_F(HttpRequestConsumeHeader, Connection_KeepAliveAndClose) {
  { // keep-alive
    HttpRequest r1;
    std::string headers_and_after =
        "Host: example.com\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);
    ASSERT_NO_THROW(r1.ConsumeHeader(hs.p_headers));
    EXPECT_TRUE(r1.IsKeepAlive());
  }
  { // close
    HttpRequest r2;
    std::string headers_and_after =
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);
    ASSERT_NO_THROW(r2.ConsumeHeader(hs.p_headers));
    EXPECT_FALSE(r2.IsKeepAlive());
  }
}

TEST_F(HttpRequestConsumeHeader, Connection_InvalidToken_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Connection: upgrade\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// =============== Bad requests ===============
TEST_F(HttpRequestConsumeHeader, MissingColon_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, NoSpaceAfterColon_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host:example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, MissingCRLF_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\n"
      "\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, ExceedMaxHeaderSize_ThrowsRequestHeaderFieldsTooLarge) {
  std::string bigValue(HttpRequest::kMaxHeaderSize, 'a');
  std::string headers_and_after =
      "Host: example.com\r\n"
      "X-Long: " + bigValue + "\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(REQUEST_HEADER_FIELDS_TOO_LARGE, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}
