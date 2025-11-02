#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "HttpRequest.hpp"

// 例外のステータスを検査する小ヘルパ
static void ExpectHttpStatusThrows(std::function<void()> fn, HttpStatus expected) {
  try {
    fn();
    FAIL() << "Expected http::responseStatusException";
  } catch (const http::responseStatusException& e) {
    EXPECT_EQ(e.getStatus(), expected);
  } catch (...) {
    FAIL() << "Expected http::responseStatusException, but got different exception type";
  }
}

// =============== Happy path: Content-Length: 0（ヘッダのみ） ===============
TEST(HttpRequestParse, HeaderOnly_ContentLengthZero_Completes) {
  HttpRequest req;

  // ヘッダは \r\n\r\n で終端。Content-Length: 0 を明示しておくと本文処理が即座に完了する
  const std::string part1 =
      "GET /hello HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

  EXPECT_NO_THROW(req.parseRequest(part1.c_str()));
  EXPECT_TRUE(req.isDone());
  EXPECT_EQ(req.getBody(), ""); // 本文は空
}

// // =============== Happy path: Content-Length あり + 分割到着 ===============
// TEST(HttpRequestParse, FixedLengthBody_FragmentedArrival_CompletesWhenEnough) {
//   HttpRequest req;

//   const std::string header =
//       "POST /submit HTTP/1.1\r\n"
//       "Host: example.com\r\n"
//       "Content-Length: 11\r\n"
//       "\r\n";

//   // まずヘッダだけ
//   EXPECT_NO_THROW(req.parseRequest(header.c_str()));
//   // まだボディ不足なので Done にはならない想定
//   EXPECT_FALSE(req.isDone());

//   // ボディ前半だけ到着
//   EXPECT_NO_THROW(req.parseRequest("hello ")); // 6 bytes
//   EXPECT_FALSE(req.isDone());

//   // 残り到着で完了
//   EXPECT_NO_THROW(req.parseRequest("world")); // 5bytes, total 11
//   EXPECT_TRUE(req.isDone());
//   EXPECT_EQ(req.getBody(), "hello world");

//   // 完了後にさらに到着したら BAD_REQUEST
//   ExpectHttpStatusThrows([&]{ req.parseRequest("!"); }, BAD_REQUEST);
// }

// // =============== Happy path: chunked 転送（複数チャンク + 分割入力） ===============
// TEST(HttpRequestParse, ChunkedBody_WithMultipleChunks_CompletesAndConcats) {
//   HttpRequest req;

//   const std::string header =
//       "POST /chunked HTTP/1.1\r\n"
//       "Host: example.com\r\n"
//       "Transfer-Encoding: chunked\r\n"
//       "\r\n";
//   EXPECT_NO_THROW(req.parseRequest(header.c_str()));
//   EXPECT_FALSE(req.isDone());

//   // "Wiki" (4) + CRLF
//   EXPECT_NO_THROW(req.parseRequest("4\r\nWi"));
//   EXPECT_FALSE(req.isDone());
//   EXPECT_NO_THROW(req.parseRequest("ki\r\n"));
//   EXPECT_FALSE(req.isDone());

//   // "pedia" (5) + CRLF
//   EXPECT_NO_THROW(req.parseRequest("5\r\npedia\r\n"));
//   EXPECT_FALSE(req.isDone());

//   // 終端チャンク "0" + CRLF + （この実装は trailer 未対応なので）CRLF
//   EXPECT_NO_THROW(req.parseRequest("0\r\n\r\n"));
//   EXPECT_TRUE(req.isDone());
//   EXPECT_EQ(req.getBody(), "Wikipedia");

//   // 完了後の追加入力は BAD_REQUEST
//   ExpectHttpStatusThrows([&]{ req.parseRequest("x"); }, BAD_REQUEST);
// }

// // =============== 不完全ヘッダ：途中で止まっても例外は投げず、次で続き ===============
// TEST(HttpRequestParse, IncompleteHeader_NoThrowUntilCRLFCRLFArrives) {
//   HttpRequest req;

//   EXPECT_NO_THROW(req.parseRequest("GET / HTTP/1.1\r\nHost: exa"));
//   EXPECT_FALSE(req.isDone()); // まだヘッダ終端に達していない

//   EXPECT_NO_THROW(req.parseRequest("mple.com\r\nContent-Length: 0\r\n"));
//   EXPECT_FALSE(req.isDone()); // まだ "空行" がない

//   // ヘッダ終端が届くと本文処理（Content-Length:0 なので即 Done）
//   EXPECT_NO_THROW(req.parseRequest("\r\n"));
//   EXPECT_TRUE(req.isDone());
//   EXPECT_EQ(req.getBody(), "");
// }

// // =============== エラー: 不正なチャンクサイズ（16進でない） ===============
// TEST(HttpRequestParse, Chunked_InvalidHexSize_ThrowsBadRequest) {
//   HttpRequest req;

//   const std::string header =
//       "POST /bad HTTP/1.1\r\n"
//       "Host: example.com\r\n"
//       "Transfer-Encoding: chunked\r\n"
//       "\r\n";
//   EXPECT_NO_THROW(req.parseRequest(header.c_str()));

//   // チャンクサイズ "Z"（不正） → BAD_REQUEST
//   ExpectHttpStatusThrows([&]{ req.parseRequest("Z\r\n"); }, BAD_REQUEST);
// }

// // =============== エラー: チャンク本文末尾の CRLF 欠落 ===============
// TEST(HttpRequestParse, Chunked_MissingCRLFAfterChunk_ThrowsBadRequest) {
//   HttpRequest req;

//   const std::string header =
//       "POST /bad2 HTTP/1.1\r\n"
//       "Host: example.com\r\n"
//       "Transfer-Encoding: chunked\r\n"
//       "\r\n";
//   EXPECT_NO_THROW(req.parseRequest(header.c_str()));

//   // サイズ 3 のはずだが、末尾の CRLF を入れない
//   EXPECT_NO_THROW(req.parseRequest("3\r\nabc"));
//   // ここで次の parse で CRLF を期待して BAD_REQUEST になる
//   ExpectHttpStatusThrows([&]{ req.parseRequest("X"); }, BAD_REQUEST);
// }
