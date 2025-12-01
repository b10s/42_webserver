#include <gtest/gtest.h>

#include <string>

#include "HttpRequest.hpp"
#include "lib/http/Status.hpp"
#include "lib/exception/ResponseStatusException.hpp"

class HttpRequestParse : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
TEST_F(HttpRequestParse, ConsumeVersion_HTTP11_SetsAndAdvances) {
  std::string s = "HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeVersion(s.c_str()));
  EXPECT_EQ("HTTP/1.1", req.GetVersion());
  EXPECT_EQ(s.c_str() + 10, p);  // "HTTP/1.1\r\n"
}

TEST_F(HttpRequestParse, ConsumeVersion_HTTP10_SetsAndAdvances) {
  std::string s = "HTTP/1.0\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeVersion(s.c_str()));
  EXPECT_EQ("HTTP/1.0", req.GetVersion());
  EXPECT_EQ(s.c_str() + 10, p);  // "HTTP/1.1\r\n"
}

// =============== Error path ===============
TEST_F(HttpRequestParse, ConsumeVersion_UnsupportedVersion20_ThrowsBadRequest) {
  std::string s = "HTTP/2.0\r\n";
  EXPECT_THROW(
      {
        try {
          req.ConsumeVersion(s.c_str());
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParse,
       ConsumeVersion_UnsupportedVersion111_ThrowsBadRequest) {
  std::string s = "HTTP/1.11\r\n";
  EXPECT_THROW(
      {
        try {
          req.ConsumeVersion(s.c_str());
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParse, ConsumeVersion_NonAsciiCharacter_ThrowsBadRequest) {
  std::string s = "HTTP/1.\xFF\r\n";  // Invalid character
  EXPECT_THROW(
      {
        try {
          req.ConsumeVersion(s.c_str());
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestParse, ConsumeVersion_MissingCRLF_ThrowsBadRequest) {
  std::string s = "HTTP/1.1 ";  // Missing CRLF
  EXPECT_THROW(
      {
        try {
          req.ConsumeVersion(s.c_str());
        } catch (const lib::exception::ResponseStatusException& e) {
          EXPECT_EQ(lib::http::kBadRequest, e.GetStatus());
          throw;
        }
      },
      lib::exception::ResponseStatusException);
}
