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

// =============== Happy path: Content-Length: 0（empty body） ===============
TEST(HttpRequestParse, HeaderOnly_ContentLengthZero_Completes) {
  HttpRequest req;

  const std::string part1 =
      "GET /hello HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

  EXPECT_NO_THROW(req.parseRequest(part1.c_str()));
  EXPECT_TRUE(req.isDone());
  EXPECT_EQ(req.getBody(), "");
}
