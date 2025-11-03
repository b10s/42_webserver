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
  // "/index.html " の直後（先頭から 12 + 1 = 13）
  EXPECT_EQ(s.c_str() + std::strlen("/index.html "), p);
}

TEST_F(HttpRequestParseUri, PathWithQuery_KeyValuePairs) {
  std::string s = "/search?q=cats&page=2 HTTP/1.1";
  const char* p = NULL;

  ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
  EXPECT_EQ("/search", req.getUri());

  const std::map<std::string,std::string>& q = req.getQuery();
  ASSERT_EQ(2u, q.size());
  EXPECT_EQ("cats", q.at("q"));
  EXPECT_EQ("2",    q.at("page"));

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
  EXPECT_EQ("",  q.at("x"));
  EXPECT_EQ("1", q.at("y"));
  EXPECT_EQ(s.c_str() + std::strlen("/p?x=&y=1 "), p);
}

// =============== Error path ===============

TEST_F(HttpRequestParseUri, EmptyPath_ThrowsBadRequest) {
  std::string s = " HTTP/1.1"; // URIが空
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, MissingSpaceAfterUri_ThrowsBadRequest) {
  std::string s = "/a\tHTTP/1.1"; // URI直後がスペース以外
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, NonVisibleAsciiInPath_ThrowsBadRequest) {
  std::string s = "/a\x01b HTTP/1.1"; // 制御文字混入
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

TEST_F(HttpRequestParseUri, FragmentInRequest_ThrowsBadRequest) {
  // consumeUntilStopChar で '#' を禁止している前提のテスト
  std::string s = "/a#frag HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

// 先頭が '/' 必須にしている場合のテスト（実装に合わせて有効化）
TEST_F(HttpRequestParseUri, PathMustStartWithSlash_ThrowsBadRequest) {
  std::string s = "foo HTTP/1.1";  // origin-form なのに先頭が '/'
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

TEST_F(HttpRequestParseUri, UriTooLong_Throws414_WhenReachesLimitExactly) {
  // len は '/', その後の 'a' 群すべてを数える想定。
  // bumpOrThrow は "len >= kMaxUriSize で throw" なので、
  // ちょうど到達で例外にしたい場合は "/" + (kMaxUriSize) 文字 読ませる。
  const std::size_t N = HttpRequest::kMaxUriSize;  // 公開静的定数の想定
  std::string path = "/" + makeN('a', N);          // '/' + N 個 = N+1 文字読む → 途中で throw
  std::string s = path + " HTTP/1.1";

  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(URI_TOO_LONG, e.getStatus());
        throw;
      }
    }, http::responseStatusException);
}

// TEST_F(HttpRequestParseUri, UriMaxMinusOne_OK_ButMax_EXACT_Throws) {
//   // "/" + (kMaxUriSize - 1) 文字 までは OK（このテストでは OK を確認）
//   const std::size_t N = HttpRequest::kMaxUriSize;
//   std::string acceptable = "/" + makeN('a', N - 1);  // 読む文字数 = N （'/'含む）→ 直前でOK
//   std::string s = acceptable + " HTTP/1.1";

//   const char* p = NULL;
//   ASSERT_NO_THROW(p = req.consumeUri(s.c_str()));
//   EXPECT_EQ(acceptable, req.getUri());
//   EXPECT_EQ(s.c_str() + (acceptable.size() + 1), p); // URI + ' ' の位置
// }

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

TEST_F(HttpRequestParseUri, QueryWithTrailingAmpersand_TreatAsErrorOrEmptyNextKey) {
  // 方針次第：末尾の '&' を許容するなら OK、禁止なら BAD_REQUEST にする。
  // ここでは「禁止（BAD_REQUEST）」例。実装に合わせて変更してOK。
  std::string s = "/p?a=1& HTTP/1.1";
  EXPECT_THROW({
      try { req.consumeUri(s.c_str()); }
      catch (const http::responseStatusException& e) {
        EXPECT_EQ(BAD_REQUEST, e.getStatus()); // 末尾'&'で次キー空 → 方針で変えてOK
        throw;
      }
    }, http::responseStatusException);
}
