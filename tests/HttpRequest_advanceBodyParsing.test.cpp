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

// consume only Content-Length bytes, leaving the rest for the next request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_ContentLength_LeavesExtraDataForNextRequest) {
  req.SetBufferForTest("Hello World!NEXT");
  req.SetContentLengthForTest(12);   // "Hello World!" is 12 bytes

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "Hello World!");
  EXPECT_EQ(req.GetBufferForTest(), "NEXT");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// ====== Happy path: chunked mode (content_length_ = -1) ========

// complete chunks arrive in one go
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_HappyPath) {
  req.SetBufferForTest("5\r\nhello\r\nB\r\nworld123456\r\n0\r\n\r\n");
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "helloworld123456");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// multiple recv calls for chunked body
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_MultipleRecvs) {
  req.SetContentLengthForTest(-1);
  // first recv: first chunk
  req.SetBufferForTest("a\r\nxxxxxxxxxx\r\n");
  bool done = req.AdvanceBodyParsing();
  EXPECT_FALSE(done); // still waiting for more chunks
  EXPECT_EQ("xxxxxxxxxx", req.GetBody());

  // second recv: second chunk
  req.AppendToBufferForTest("3\r\n123\r\n");
  done = req.AdvanceBodyParsing();
  EXPECT_FALSE(done); // still waiting for last chunk
  EXPECT_EQ("xxxxxxxxxx123", req.GetBody());

  // third recv: 
  req.AppendToBufferForTest("2\r\n45\r\n0\r\n\r\n");
  done = req.AdvanceBodyParsing();
  EXPECT_TRUE(done); // all chunks complete
  EXPECT_EQ("xxxxxxxxxx12345", req.GetBody());
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
  EXPECT_EQ(req.GetBufferForTest(), "5\r\nhello\r\n8\r\nworld12");
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

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_NoFinalCRLF) {
  req.SetBufferForTest("5\r\nhello\r\n0\r\n"); // missing final CRLF after last chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.GetBody(), "hello");
  EXPECT_EQ(req.GetBufferForTest(), "5\r\nhello\r\n0\r\n");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_Malformed_ExtraDataAfterLastChunk) {
  req.SetBufferForTest("5\r\nhello\r\n0\r\n\r\nEXTRA"); // extra data after last chunk
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

// TODO: maybe I should not throw bad request for extensions but just ignore them
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeWithExtensions) {
  req.SetBufferForTest("5;name=value;foo=bar\r\nhello\r\n0\r\n\r\n"); // chunk size with extensions
  req.SetContentLengthForTest(-1); // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
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

// extra data after "0\r\n\r\n" -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ExtraDataAfterTerminator_ThrowsBadRequest) {
  req.SetBufferForTest("5\r\nhello\r\n0\r\n\r\nGARBAGE");
  req.SetContentLengthForTest(-1);

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

// chunk size is greater than size_t max (overflow) -> 400 Bad Request
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeOverflow_ThrowsBadRequest) {
  req.SetContentLengthForTest(-1);
  std::string huge_hex(100, 'F');
  std::string payload = huge_hex + "\r\n";

  req.SetBufferForTest(payload);
  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

// chunk size is huge but valid (no overflow) -> works correctly
TEST_F(HttpRequestAdvanceBodyParsing, AdvanceBodyParsing_Chunked_ChunkSizeLargeButValid_WorksCorrectly) {
  req.SetContentLengthForTest(-1);
  // calculate the same boundary value as the parser on the test side
  const size_t max_before_shift =
      std::numeric_limits<size_t>::max() >> 4;
  std::ostringstream oss;
  oss << std::hex << max_before_shift;  // smallcase hex
  std::string hex = oss.str();
  std::string payload = hex + "\r\n";
  req.SetBufferForTest(payload);
  bool done = false;
  EXPECT_NO_THROW(done = req.AdvanceBodyParsing());
  EXPECT_FALSE(done); // still waiting for body data
  EXPECT_EQ("", req.GetBody());
}
