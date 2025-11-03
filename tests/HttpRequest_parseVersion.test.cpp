#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestParse : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
TEST_F(HttpRequestParse, consumeVersion_HTTP11_SetsAndAdvances) {
  std::string s = "HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeVersion(s.c_str()));
  EXPECT_EQ("HTTP/1.1", req.getVersion());
  EXPECT_EQ(s.c_str() + 10, p);  // "HTTP/1.1\r\n"
}

TEST_F(HttpRequestParse, consumeVersion_HTTP10_SetsAndAdvances) {
  std::string s = "HTTP/1.0\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeVersion(s.c_str()));
  EXPECT_EQ("HTTP/1.0", req.getVersion());
  EXPECT_EQ(s.c_str() + 10, p);  // "HTTP/1.1\r\n"
}

// =============== Error path ===============
TEST_F(HttpRequestParse, consumeVersion_UnsupportedVersion20_ThrowsBadRequest) {
  std::string s = "HTTP/2.0\r\n";
  EXPECT_THROW(
      {
        try {
          req.consumeVersion(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestParse, consumeVersion_UnsupportedVersion111_ThrowsBadRequest) {
  std::string s = "HTTP/1.11\r\n";
  EXPECT_THROW(
      {
        try {
          req.consumeVersion(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestParse, consumeVersion_NonAsciiCharacter_ThrowsBadRequest) {
  std::string s = "HTTP/1.\xFF\r\n";  // Invalid character
  EXPECT_THROW(
      {
        try {
          req.consumeVersion(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestParse, consumeVersion_MissingCRLF_ThrowsBadRequest) {
  std::string s = "HTTP/1.1 ";  // Missing CRLF
  EXPECT_THROW(
      {
        try {
          req.consumeVersion(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}
