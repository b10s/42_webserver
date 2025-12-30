#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestAdvanceHeader : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path: header parsing ===============
TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_HappyPath) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");

  EXPECT_TRUE(req.AdvanceHeader());
  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kBody);
}

// =============== Need more data ===============
// header line is split across multiple calls
TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_HeaderLine_split_NeedMoreData) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: exam");

  EXPECT_FALSE(req.AdvanceHeader());
  EXPECT_EQ(req.GetBufferForTest(),
            "GET /index.html HTTP/1.1\r\nHost: exam");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kHeader);
}

TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_PartialHeaderLine_NeedMoreData) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n");
  EXPECT_FALSE(req.AdvanceHeader());
  EXPECT_EQ(req.GetBufferForTest(), "GET /index.html HTTP/1.1\r\nHost: example.com\r\n");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kHeader);
}

// =============== Malformed requests ===============
TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_Malformed_RequestLine_ThrowsBadRequest) {
  req.SetBufferForTest("GET /index.html\r\nHost: example.com\r\n\r\n"); // missing version 
  EXPECT_THROW(req.AdvanceHeader(), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_Malformed_HeaderLine_ThrowsBadRequest) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost example.com\r\n\r\n"); // missing colon
  EXPECT_THROW(req.AdvanceHeader(), lib::exception::ResponseStatusException);
}

TEST_F(HttpRequestAdvanceHeader, AdvanceHeader_Malformed_NonAsciiInHeader_ThrowsBadRequest) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: exam\x01ple.com\r\n\r\n"); // non-ASCII char
  EXPECT_THROW(req.AdvanceHeader(), lib::exception::ResponseStatusException);
}
