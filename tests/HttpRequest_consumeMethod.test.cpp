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

TEST_F(HttpRequestParse, consumeMethod_GET_SetsAndAdvances) {
  std::string s = "GET / HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeMethod(s.c_str()));
  EXPECT_EQ(RequestMethod::kGet, req.GetMethod());
  // "GET " の4文字分だけ進む
  EXPECT_EQ(s.c_str() + 4, p);
}

TEST_F(HttpRequestParse, consumeMethod_HEAD_SetsAndAdvances) {
  std::string s = "HEAD / HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeMethod(s.c_str()));
  EXPECT_EQ(RequestMethod::kHead, req.GetMethod());
  EXPECT_EQ(s.c_str() + 5, p);  // "HEAD "
}

TEST_F(HttpRequestParse, consumeMethod_POST_SetsAndAdvances) {
  std::string s = "POST /path HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeMethod(s.c_str()));
  EXPECT_EQ(RequestMethod::kPost, req.GetMethod());
  EXPECT_EQ(s.c_str() + 5, p);  // "POST "
}

TEST_F(HttpRequestParse, consumeMethod_DELETESetsAndAdvances) {
  std::string s = "DELETE /x HTTP/1.1\r\n";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.ConsumeMethod(s.c_str()));
  EXPECT_EQ(RequestMethod::kDelete, req.GetMethod());
  EXPECT_EQ(s.c_str() + 7, p);  // "DELETE "
}

// =============== Error path ===============
TEST_F(HttpRequestParse, consumeMethod_UnsupportedMethod_ThrowsNotImplemented) {
  std::string s = "PUT / HTTP/1.1\r\n";

  EXPECT_THROW(
      {
        try {
          req.ConsumeMethod(s.c_str());
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(kNotImplemented, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}
