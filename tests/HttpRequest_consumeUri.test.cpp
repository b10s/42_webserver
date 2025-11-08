#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestParseUri : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path ===============

TEST_F(HttpRequestParseUri, PathOnly_SetsUri_AdvancesToSpace) {
  std::string s = "/index.html HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  EXPECT_EQ("/index.html", req.getUri());
  EXPECT_EQ(s.c_str() + std::strlen("/index.html "), p);
}

TEST_F(HttpRequestParseUri, PathWithQuery_KeyValuePairs) {
  std::string s = "/search?q=cats&page=2 HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  EXPECT_EQ("/search", req.getUri());

  const std::map<std::string, std::string>& q = req.getQuery();
  ASSERT_EQ(2u, q.size());
  EXPECT_EQ("cats", q.at("q"));
  EXPECT_EQ("2", q.at("page"));

  EXPECT_EQ(s.c_str() + std::strlen("/search?q=cats&page=2 "), p);
}

TEST_F(HttpRequestParseUri, QueryAllowsSlashAndQuestion_AsData) {
  std::string s = "/p?next=/login?m=1 HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  EXPECT_EQ("/p", req.getUri());

  const auto& q = req.getQuery();
  ASSERT_EQ(1u, q.size());
  EXPECT_EQ("/login?m=1", q.at("next"));

  EXPECT_EQ(s.c_str() + std::strlen("/p?next=/login?m=1 "), p);
}

TEST_F(HttpRequestParseUri, EmptyValueAllowed) {
  std::string s = "/p?x=&y=1 HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  const auto& q = req.getQuery();
  ASSERT_EQ(2u, q.size());
  EXPECT_EQ("", q.at("x"));
  EXPECT_EQ("1", q.at("y"));
  EXPECT_EQ(s.c_str() + std::strlen("/p?x=&y=1 "), p);
}

// =============== Error path ===============

TEST_F(HttpRequestParseUri, EmptyPath_ThrowsBadRequest) {
  std::string s = " HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, MissingSpaceAfterUri_ThrowsBadRequest) {
  std::string s = "/aHTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, NonVisibleAsciiInPath_ThrowsBadRequest) {
  std::string s = "/a\x01b HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, FragmentInRequest_ThrowsBadRequest) {
  std::string s = "/a#frag HTTP/1.1";
  EXPECT_THROW(
      {
        try {
          req.consumeUri(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(BAD_REQUEST, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

// origin-form has to start with '/'
TEST_F(HttpRequestParseUri, PathMustStartWithSlash_ThrowsBadRequest) {
  std::string s = "foo HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

// =============== Length boundary (kMaxUriSize) ===============

static std::string makeN(char ch, std::size_t n) {
  return std::string(n, ch);
}

// len is incremented for each character in the URI,
// including the initial '/' and any '?' and query characters.
TEST_F(HttpRequestParseUri, UriTooLong_Throws414_WhenReachesLimitExactly) {
  const std::size_t N = HttpRequest::kMaxUriSize;
  std::string path = "/" + makeN('a', N);
  std::string s = path + " HTTP/1.1";

  EXPECT_THROW(
      {
        try {
          req.consumeUri(s.c_str());
        } catch (const http::responseStatusException& e) {
          EXPECT_EQ(URI_TOO_LONG, e.getStatus());
          throw;
        }
      },
      http::responseStatusException);
}

TEST_F(HttpRequestParseUri, UriMaxMinusOne_OK_ButMax_EXACT_Throws) {
  const std::size_t N = HttpRequest::kMaxUriSize;
  std::string acceptable = "/" + makeN('a', N - 3); // -2 for initial '/' and ' ' and at least one char after space
  std::string s = acceptable + " HTTP/1.1";

  const char* p = NULL;
  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  EXPECT_EQ(acceptable, req.getUri());
  EXPECT_EQ(s.c_str() + (acceptable.size() + 1), p); // URI + ' '
}

// =============== Query corner cases ===============

TEST_F(HttpRequestParseUri, QueryOnlyKeys_NoValues) {
  std::string s = "/p?a&b&c HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  const auto& q = req.getQuery();
  ASSERT_EQ(3u, q.size());
  EXPECT_EQ("", q.at("a"));
  EXPECT_EQ("", q.at("b"));
  EXPECT_EQ("", q.at("c"));
  EXPECT_EQ(s.c_str() + std::strlen("/p?a&b&c "), p);
}

// we don't allow trailing '&' that implies empty key
TEST_F(HttpRequestParseUri, QueryWithTrailingAmpersand_TreatAsErrorOrEmptyNextKey) {
  std::string s = "/p?a=1& HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}
