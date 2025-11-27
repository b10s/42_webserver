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
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_ContentLength_HappyPath) {
  req.buffer_ = "Hello World!";
  req.content_length_ = 12;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello World!");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_HappyPath) {
  req.buffer_ = "5\r\nhello\r\n8\r\nworld123\r\n0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "helloworld123");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Need more data ===============
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_ContentLength_NeedMoreData) {
  req.buffer_ = "Hello";
  req.content_length_ = 12;

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "Hello");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_NeedMoreData) {
  req.buffer_ = "5\r\nhello\r\n8\r\nworld12"; // incomplete chunk data
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "8\r\nworld12");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_NeedMoreData_ChunkSize) {
  req.buffer_ = "5"; // incomplete chunk size line
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

// =============== Malformed requests ===============
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_ChunkSize) {
  req.buffer_ = "G\r\nhello\r\n"; // invalid hex 'G'
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
} 

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_NoCRLF) {
  req.buffer_ = "5\nhello\r\n"; // missing CR in CRLF
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_InsufficientData) {
  req.buffer_ = "5\r\nhel"; // incomplete chunk data
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "5\r\nhel");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_NoFinalCRLF) {
  req.buffer_ = "5\r\nhello\r\n0\r\n"; // missing final CRLF after last chunk
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "0\r\n");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_ExtraDataAfterLastChunk) {
  req.buffer_ = "5\r\nhello\r\n0\r\n\r\nEXTRA"; // extra data after last chunk
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_MissingLastChunk) {
  req.buffer_ = "5\r\nhello\r\n"; // missing last chunk
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "5\r\nhello\r\n");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_Malformed_NonHexInChunkSize) {
  req.buffer_ = "5;name=value;foo=bar\r\nhello\r\n0\r\n\r\n"; // chunk extensions with non-hex chars
  req.content_length_ = -1; // chunked

  EXPECT_THROW(req.AdvanceBodyParsing(), http::ResponseStatusException);
}

// =============== Edge cases ===============
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_ContentLength_ZeroLength) {
  req.buffer_ = "";
  req.content_length_ = 0;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ZeroLength) {
  req.buffer_ = "0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_SingleByteChunks) {
  req.buffer_ = "1\r\na\r\n1\r\nb\r\n1\r\nc\r\n0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abc");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_LargeChunkSize) {
  req.buffer_ = "A\r\nabcdefghij\r\n0\r\n\r\n"; // 10 bytes chunk
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abcdefghij");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ChunkSizeWithExtensions) {
  req.buffer_ = "5;name=value;foo=bar\r\nhello\r\n0\r\n\r\n"; // chunk size with extensions
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "hello");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ChunkSizeWithUppercaseHex) {
  req.buffer_ = "A\r\nabcdefghij\r\n0\r\n\r\n"; // uppercase hex chunk size
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abcdefghij");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ChunkSizeWithMixedCaseHex) {
  req.buffer_ = "aBcD\r\n"  // mixed-case hex chunk size
                    "abcdefghijabcdefghijabcdefghijabcdefghij\r\n"
                    "0\r\n\r\n";
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "abcdefghijabcdefghijabcdefghijabcdefghij");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

// =============== Boundary conditions ===============
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_ContentLength_ExactBufferSize) {
  req.buffer_ = "HelloWorld"; // 10 bytes
  req.content_length_ = 10;

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "HelloWorld");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ExactBufferSize) {
  req.buffer_ = "5\r\nHello\r\n5\r\nWorld\r\n0\r\n\r\n"; // exact size for two chunks
  req.content_length_ = -1; // chunked

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "HelloWorld");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_ChunkSizeAtBufferEnd) {
  req.buffer_ = "5\r\nHello\r\nA"; // chunk size 'A' at buffer end
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello");
  EXPECT_EQ(req.buffer_, "A");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Chunked_CRLFAtBufferEnd) {
  req.buffer_ = "5\r\nHello\r\nA\r"; // CR at buffer end
  req.content_length_ = -1; // chunked

  EXPECT_FALSE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello");
  EXPECT_EQ(req.buffer_, "A\r");
  EXPECT_EQ(req.progress_, HttpRequest::kParsing);
}

// =============== Performance considerations ===============
TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Performance_AssignVsNew) {
  req.buffer_ = "Hello World!";
  req.content_length_ = 12;
  req.body_ = "old content";

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello World!");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

TEST_F(HttpRequestParseUri, AdvanceBodyParsing_Performance_Reserve) {
  req.buffer_ = "Hello World!";
  req.content_length_ = 12;
  req.body_.reserve(100); // 事前に大きめに確保

  EXPECT_TRUE(req.AdvanceBodyParsing());
  EXPECT_EQ(req.body_, "Hello World!");
  EXPECT_EQ(req.buffer_, "");
  EXPECT_EQ(req.progress_, HttpRequest::kDone);
}

