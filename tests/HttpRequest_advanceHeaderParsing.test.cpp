#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestAdvanceHeaderParsing : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path: header parsing ===============
TEST_F(HttpRequestAdvanceHeaderParsing, AdvanceHeaderParsing_HappyPath) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");

  EXPECT_TRUE(req.AdvanceHeaderParsing());
  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetBufferForTest(), "");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// =============== Need more data ===============
TEST_F(HttpRequestAdvanceHeaderParsing, AdvanceHeaderParsing_NeedMoreData) {
  req.SetBufferForTest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n"); // incomplete header
  EXPECT_FALSE(req.AdvanceHeaderParsing());
  EXPECT_EQ(req.GetBufferForTest(), "GET /index.html HTTP/1.1\r\nHost: example.com\r\n");
  // EXPECT_EQ(req.GetProgress(), HttpRequest::kHeader);
}

// =============== Malformed requests ===============
TEST_F(HttpRequestAdvanceHeaderParsing, AdvanceHeaderParsing_Malformed_RequestLine_ThrowsBadRequest) {
  req.SetBufferForTest("GET /index.html\r\nHost: example.com\r\n\r\n"); // missing version 
  EXPECT_THROW(req.AdvanceHeaderParsing(), lib::exception::ResponseStatusException);
}
