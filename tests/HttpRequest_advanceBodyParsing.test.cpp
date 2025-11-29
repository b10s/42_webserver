#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestAdvanceBodyParsing : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_HappyPath) {
  req.buffer_ = "Hello World!";
  req.content_length_ = 12;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello World!");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_HappyPath) {
  req.buffer_ = "5\r\nhello\r\n8\r\nworld123\r\n0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "helloworld123");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Need more data ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_NeedMoreData) {
  req.buffer_ = "Hello";
  req.content_length_ = 12;

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "Hello");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_NeedMoreData) {
  req.buffer_ = "5\r\nhello\r\n8\r\nworld12"; // incomplete chunk data
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "5\r\nhello\r\n8\r\nworld12");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_NeedMoreData_ChunkSize) {
  req.buffer_ = "5"; // incomplete chunk size line
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

// =============== Malformed requests ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_ChunkSize) {
  req.buffer_ = "G\r\nhello\r\n"; // invalid hex 'G'
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
} 

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_NoCRLF) {
  req.buffer_ = "5\nhello\r\n"; // missing CR in CRLF
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_InsufficientData) {
  req.buffer_ = "5\r\nhel"; // incomplete chunk data
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5\r\nhel");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_NoFinalCRLF) {
  req.buffer_ = "5\r\nhello\r\n0\r\n"; // missing final CRLF after last chunk
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "5\r\nhello\r\n0\r\n");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_ExtraDataAfterLastChunk) {
  req.buffer_ = "5\r\nhello\r\n0\r\n\r\nEXTRA"; // extra data after last chunk
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_MissingLastChunk) {
  req.buffer_ = "5\r\nhello\r\n"; // missing last chunk
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "5\r\nhello\r\n");
  // EXPECT_EQ(req.progress_, HttpRequest::kBody);
}

// =============== Edge cases ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_ZeroLength) {
  req.buffer_ = "";
  req.content_length_ = 0;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ZeroLength) {
  req.buffer_ = "0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_SingleByteChunks) {
  req.buffer_ = "1\r\na\r\n1\r\nb\r\n1\r\nc\r\n0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abc");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_LargeChunkSize) {
  req.buffer_ = "A\r\nabcdefghij\r\n0\r\n\r\n"; // 10 bytes chunk
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abcdefghij");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// TODO: maybe I should not throw bad request for extensions but just ignore them
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeWithExtensions) {
  req.buffer_ = "5;name=value;foo=bar\r\nhello\r\n0\r\n\r\n"; // chunk size with extensions
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeWithUppercaseHex) {
  req.buffer_ = "A\r\nabcdefghij\r\n0\r\n\r\n"; // uppercase hex chunk size
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abcdefghij");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Boundary conditions ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_ExactBufferSize) {
  req.buffer_ = "HelloWorld"; // 10 bytes
  req.content_length_ = 10;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "HelloWorld");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ExactBufferSize) {
  req.buffer_ = "5\r\nHello\r\n5\r\nWorld\r\n0\r\n\r\n"; // exact size for two chunks
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "HelloWorld");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Content-Length: error / edge paths ===============

// Content-Length がボディより大きい -> 非ブロッキングなので「まだ足りない」として false
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_Incomplete_ReturnsFalse) {
  req.buffer_ = "Hello";      // 5 bytes
  req.content_length_ = 10;   // 10 bytes expected

  EXPECT_FALSE(req.AdvanceBodyParsing());
  // まだ何も確定させない
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "Hello");
  // progress_ は kDone になっていないことだけ確認（具体値は実装依存ならチェックしなくてもOK）
  EXPECT_NE(req.progress_, HttpRequest::kDone);
}

// Content-Length ぶんだけ消費し、それ以降は次のリクエスト用に残す
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_LeavesExtraDataForNextRequest) {
  req.buffer_ = "Hello World!NEXT";
  req.content_length_ = 12;   // "Hello World!" だけ

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello World!");
  EXPECT_EQ(req.buffer_, "NEXT");  // 余りは残す
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Chunked: non-happy paths ===============

// チャンクサイズ行が途中までしか来ていない（例: "5\r" だけ） -> false を返して次の recv を待つ
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_IncompleteChunkSizeLine_ReturnsFalse) {
  req.buffer_ = "5\r";    // "5\r\n" すら揃ってない
  req.content_length_ = -1;

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5\r");
  // EXPECT_NE(req.progress_, HttpRequest::kBody);
}

// チャンクサイズは読めたが、データが足りない -> false
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_IncompleteChunkData_ReturnsFalse) {
  req.buffer_ = "5\r\nhe";   // size=5, でも "he" しか来ていない
  req.content_length_ = -1;

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5\r\nhe");
  // EXPECT_NE(req.progress_, HttpRequest::kDone);
}

// チャンクサイズが 16進数でない -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_InvalidHexSize_ThrowsBadRequest) {
  req.buffer_ = "G\r\nhello\r\n0\r\n\r\n";  // 'G' は 0-9,A-F,a-f 以外
  req.content_length_ = -1;

  EXPECT_THROW(
      {
        try {
          req.AdvanceBodyParsing();
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(kBadRequest, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// チャンクデータの後に CRLF がない -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_MissingCRLFAfterData_ThrowsBadRequest) {
  // 本来は "5\r\nhello\r\n" だが、最後の CRLF がない
  req.buffer_ = "5\r\nhello0\r\n";
  req.content_length_ = -1;

  EXPECT_THROW(
      {
        try {
          req.AdvanceBodyParsing();
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(kBadRequest, e.GetStatus());
          throw;
        }
      },
      http::ResponseStatusException);
}

// "0\r\n\r\n" のあとに余計なデータがある -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ExtraDataAfterTerminator_ThrowsBadRequest) {
  req.buffer_ = "5\r\nhello\r\n0\r\n\r\nGARBAGE";
  req.content_length_ = -1;

  EXPECT_THROW(
      {
        try {
          req.AdvanceBodyParsing();
        } catch (const http::ResponseStatusException& e) {
          EXPECT_EQ(kBadRequest, e.GetStatus()); // HandleLastChunk/FinishChunkedBody で検知
          throw;
        }
      },
      http::ResponseStatusException);
}

// TCPの特性上、チャンクサイズ行とデータが別々に届くこともある。
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeAndDataSplitAcrossBuffers) {
  // 1回目の受信でチャンクサイズ行だけ受信
  req.buffer_ = "5\r\n";
  req.content_length_ = -1; // chunked
  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5\r\n"); // まだ消費されない
  // 2回目の受信でチャンクデータと終端チャンクを受信
  req.buffer_ += "hello\r\n0\r\n\r\n";
  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_PartialRecv_DoesNotDuplicateBody) {
  // chunked モードを想定（content_length_ < 0）
  req.content_length_ = -1;

  // ◆ 1回目の recv: 1チャンク目 + 2チャンク目の途中まで
  req.buffer_ = "3\r\nabc\r\n3\r\nxy";

  bool done = req.AdvanceBodyParsing();
  // まだ 2チャンク目が揃っていないので、false のはず
  EXPECT_FALSE(done);

  // 1チャンク目 "abc" だけが body_ に入っている想定
  EXPECT_EQ("abc", req.body_);

  // ◆ 2回目の recv: 残りの "z\r\n0\r\n\r\n" が届く
  req.buffer_ += "z\r\n0\r\n\r\n";

  done = req.AdvanceBodyParsing();

  // ここでは全チャンクが揃うので true になる想定
  EXPECT_TRUE(done);

  // 本来の正しい結果は "abcxyz"
  // （今の実装だと "abcabcxyz" になって、この EXPECT が FAIL するはず）
  EXPECT_EQ("abcxyz", req.body_);
}

