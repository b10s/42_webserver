#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "enums.hpp"

class HttpRequestParseRequest : public ::testing::Test {
 protected:
  HttpRequest req;
};

// =============== Happy path: parse request ===============
TEST_F(HttpRequestParseRequest, ParseRequest_HappyPath) {
  req.ParseRequest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
  EXPECT_EQ(req.GetMethod(), lib::http::kGet);
  EXPECT_EQ(req.GetUri(), "/index.html");
  EXPECT_EQ(req.GetVersion(), "HTTP/1.1");
  EXPECT_EQ(req.GetHeader().at("host"), "example.com");
  EXPECT_EQ(req.GetProgress(), HttpRequest::kDone);
}

// =============== Error cases: parse request ===============
TEST_F(HttpRequestParseRequest, ParseRequest_Error_ExtraDataAfterDone) {
  req.ParseRequest("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
  EXPECT_THROW(req.ParseRequest("EXTRA DATA"), lib::exception::ResponseStatusException);
}
