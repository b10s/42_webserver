#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestAdvanceBodyParsing : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path: content-length mode ===============

// complete body arrives in one go
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_HappyPath) {
  req.SetBufferForTest("Hello World!");
  req.SetContentLengthForTest(12);

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "Hello World!");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// request body size is 11, but data arrives in two parts
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_SizeThenData) {
  req.SetBufferForTest("Hello");
  req.SetContentLengthForTest(11);
  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "Hello"); // still waiting for more data
  // next, the rest of the data arrives
  req.AppendToBufferForTest(" World");
  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "Hello World");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// ====== Happy path: chunked mode ((content_length_ = -1) ========

// complete chunks arrive in one go
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_HappyPath) {
  req.SetBufferForTest("5\r\nhello\r\n8\r\nworld123\r\n0\r\n\r\n");
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "helloworld123");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// size and data split across multiple recv calls
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeAndDataSplitAcrossBuffers) {
  // receive first chunk size only
  req.SetBufferForTest("5\r\n");
  req.SetContentLengthForTest(-1); // chunked
  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "5\r\n"); // still waiting for more data
  // next, the rest of the data arrives
  req.AppendToBufferForTest("hello\r\n0\r\n\r\n");
  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "hello");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_PartialRecv_DoesNotDuplicateBody) {
  req.SetContentLengthForTest(-1);
  // first recv: first chunk + 2nd chunk partial
  req.SetBufferForTest("3\r\nabc\r\n3\r\nxy");
  bool done = req.AdvanceBodyParsing();
  EXPECT_FALSE(done); // still waiting the second chunk to complete
  EXPECT_EQ("abc", req.GetBody()); // first chunk "abc" is expected to be in body_

  // second recv: remaining "z\r\n0\r\n\r\n" arrives
  req.AppendToBufferForTest("z\r\n0\r\n\r\n");
  done = req.AdvanceBodyParsing();
  // all chunks are expected to be complete now
  EXPECT_TRUE(done);
  EXPECT_EQ("abcxyz", req.GetBody());
}

// =============== Need more data ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_NeedMoreData) {
  req.SetBufferForTest("Hello");
  req.SetContentLengthForTest(12);

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "Hello");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_NeedMoreData) {
  req.SetBufferForTest("5\r\nhello\r\n8\r\nworld12"); // incomplete chunk data
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "hello");
  // EXPECT_EQ(req.GetBufferForTest(), "5\r\nhello\r\n8\r\nworld12");
  EXPECT_EQ(req.GetBufferForTest(), "8\r\nworld12");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_NeedMoreData_ChunkSize) {
  req.SetBufferForTest("5"); // incomplete chunk size line
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "5");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

// =============== Malformed requests ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_ChunkSize) {
  req.SetBufferForTest("G\r\nhello\r\n"); // invalid hex 'G'
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
} 

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_NoCRLF) {
  req.SetBufferForTest("5\nhello\r\n"); // missing CR in CRLF
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_InsufficientData) {
  req.SetBufferForTest("5\r\nhel"); // incomplete chunk data
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "5\r\nhel");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_NoFinalCRLF) {
  req.SetBufferForTest("5\r\nhello\r\n0\r\n"); // missing final CRLF after last chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "hello");
  // EXPECT_EQ(req.GetBufferForTest(), "5\r\nhello\r\n0\r\n");
  EXPECT_EQ(req.GetBufferForTest(), "0\r\n");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_ExtraDataAfterLastChunk) {
  req.SetBufferForTest("5\r\nhello\r\n0\r\n\r\nEXTRA"); // extra data after last chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_MissingLastChunk) {
  req.SetBufferForTest("5\r\nhello\r\n"); // missing last chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "hello");
  // EXPECT_EQ(req.GetBufferForTest(), "5\r\nhello\r\n");
  EXPECT_EQ(req.GetBufferForTest(), "");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

// =============== Edge cases ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_ZeroLength) {
  req.SetBufferForTest("");
  req.SetContentLengthForTest(0);

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ZeroLength) {
  req.SetBufferForTest("0\r\n\r\n");
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_SingleByteChunks) {
  req.SetBufferForTest("1\r\na\r\n1\r\nb\r\n1\r\nc\r\n0\r\n\r\n");
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "abc");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_LargeChunkSize) {
  req.SetBufferForTest("A\r\nabcdefghij\r\n0\r\n\r\n"); // 10 bytes chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "abcdefghij");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// TODO: maybe I should not throw bad request for extensions but just ignore them
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeWithExtensions) {
  req.SetBufferForTest("5;name=value;foo=bar\r\nhello\r\n0\r\n\r\n"); // chunk size with extensions
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeWithUppercaseHex) {
  req.SetBufferForTest("A\r\nabcdefghij\r\n0\r\n\r\n"); // uppercase hex chunk size
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "abcdefghij");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// =============== Boundary conditions ===============
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_ExactBufferSize) {
  req.SetBufferForTest("HelloWorld"); // 10 bytes
  req.SetContentLengthForTest(10);

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "HelloWorld");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ExactBufferSize) {
  req.SetBufferForTest("5\r\nHello\r\n5\r\nWorld\r\n0\r\n\r\n"); // exact size for two chunks
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "HelloWorld");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// =============== Content-Length: error / edge paths ===============

// Content-Length がボディより大きい -> 非ブロッキングなので「まだ足りない」として false
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_Incomplete_ReturnsFalse) {
  req.SetBufferForTest("Hello");      // 5 bytes
  req.SetContentLengthForTest(10);   // 10 bytes expected

  EXPECT_FALSE(req.AdvanceBodyParsing());
  // まだ何も確定させない
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "Hello");
  // progress_ は kDone になっていないことだけ確認（具体値は実装依存ならチェックしなくてもOK）
  EXPECT_NE(req.GetProgress(), HttpRequest::kDone);
}

// Content-Length ぶんだけ消費し、それ以降は次のリクエスト用に残す
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_LeavesExtraDataForNextRequest) {
  req.SetBufferForTest("Hello World!NEXT");
  req.SetContentLengthForTest(12);   // "Hello World!" だけ

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "Hello World!");
  EXPECT_EQ(req.GetBufferForTest(), "NEXT");  // 余りは残す
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// =============== Chunked: non-happy paths ===============

// チャンクサイズ行が途中までしか来ていない（例: "5\r" だけ） -> false を返して次の recv を待つ
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_IncompleteChunkSizeLine_ReturnsFalse) {
  req.SetBufferForTest("5\r");    // "5\r\n" すら揃ってない
  req.SetContentLengthForTest(-1);

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "5\r");
  // EXPECT_NE(req.GetProgress(), HttpRequest::kBody);
}

// チャンクサイズは読めたが、データが足りない -> false
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_IncompleteChunkData_ReturnsFalse) {
  req.SetBufferForTest("5\r\nhe");   // size=5, でも "he" しか来ていない
  req.SetContentLengthForTest(-1);

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "");
  EXPECT_EQ(req.GetBufferForTest(), "5\r\nhe");
  // EXPECT_NE(req.GetProgress(), HttpRequest::kDone);
}

// チャンクサイズが 16進数でない -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_InvalidHexSize_ThrowsBadRequest) {
  req.SetBufferForTest("G\r\nhello\r\n0\r\n\r\n");  // 'G' は 0-9,A-F,a-f 以外
  req.SetContentLengthForTest(-1);

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
  req.SetBufferForTest("5\r\nhello0\r\n");
  req.SetContentLengthForTest(-1);

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
  req.SetBufferForTest("5\r\nhello\r\n0\r\n\r\nGARBAGE");
  req.SetContentLengthForTest(-1);

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

