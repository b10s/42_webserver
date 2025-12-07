#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "HttpRequest.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"

// ============== HeaderStart struct and helper ==============
struct HeaderStart {
  std::string reqbuf;    // start line (request line) + headers + body
  const char* p_headers; // points to the beginning of headers in reqbuf
  lib::http::Method method;
};

static HeaderStart makeHeaderStart(const std::string& method,
                                   const std::string& uri,
                                   const std::string& version,
                                   const std::string& headers_and_after = "") {
  HeaderStart hs;
  std::string startLine = method + " " + uri + " " + version + "\r\n";
  hs.reqbuf = startLine + headers_and_after;
  hs.p_headers = hs.reqbuf.c_str() + startLine.size();
  if (method == "GET") hs.method = lib::http::kGet;
  else if (method == "HEAD") hs.method = lib::http::kHead;
  else if (method == "POST") hs.method = lib::http::kPost;
  else if (method == "DELETE") hs.method = lib::http::kDelete;
  else hs.method = lib::http::kUnknownMethod;
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

// if method is POST and neither Content-Length nor Transfer-Encoding is present, throw kLengthRequired
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kLengthRequired, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kNotImplemented, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
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
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kRequestHeaderFieldsTooLarge, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, MultipleHost_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Host: another.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

// =============== error: duplicate headers ===============

TEST_F(HttpRequestConsumeHeader, DuplicateHeader_IgnoringCase_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "HOST: example.com\r\n"
      "\r\n";
  auto hs = makeHeaderStart("GET", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, DuplicateContentLength_MixedCase_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Content-Length: 3\r\n"
      "CONTENT-LENGTH: 3\r\n"
      "\r\n";

  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestConsumeHeader, DuplicateTransferEncoding_DifferentCase_ThrowsBadRequest) {
  std::string headers_and_after =
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "transfer-encoding: chunked\r\n"
      "\r\n";

  auto hs = makeHeaderStart("POST", "/", "HTTP/1.1", headers_and_after);

  EXPECT_THROW(
      {
        try {
          req.ConsumeHeader(hs.p_headers);
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}
