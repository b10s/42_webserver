#include "cgi/CgiResponseParser.hpp"

#include <gtest/gtest.h>

#include "HttpResponse.hpp"
#include "lib/http/Status.hpp"

TEST(CgiResponseParserTest, BasicSuccess) {
  std::string output = "Content-Type: text/html\r\n\r\nHello, CGI!";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kOk);
  EXPECT_TRUE(res.HasHeader("Content-Type"));
  EXPECT_EQ(res.GetHeader("Content-Type").Value(), "text/html");
  EXPECT_EQ(res.GetBody(), "Hello, CGI!");
}

TEST(CgiResponseParserTest, WithStatusHeader) {
  std::string output =
      "Status: 404 Not Found\r\nContent-Type: text/plain\r\n\r\nError Page";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kNotFound);
  EXPECT_EQ(res.GetBody(), "Error Page");
}

TEST(CgiResponseParserTest, WithLocationHeader) {
  // If Location is present and status is OK, it should change to Found (302)
  std::string output =
      "Location: http://example.com\r\nContent-Type: text/plain\r\n\r\nRedirecting";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kFound);
  EXPECT_EQ(res.GetHeader("Location").Value(), "http://example.com");
}

TEST(CgiResponseParserTest, WithExtraHeaders) {
  std::string output =
      "Content-Type: text/html\r\nX-Custom-Header: "
      "MyValue\r\n\r\nBody";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetHeader("X-Custom-Header").Value(), "MyValue");
}

TEST(CgiResponseParserTest, MissingSeparator) {
  std::string output = "Content-Type: text/html\r\nBodyWithoutSeparator";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kInternalServerError);
}

TEST(CgiResponseParserTest, MissingContentType) {
  std::string output = "X-Custom: OnlyThis\r\n\r\nBody";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kInternalServerError);
}

TEST(CgiResponseParserTest, MalformedHeader) {
  // Line without colon
  std::string output = "Content-Type text/html\r\n\r\nBody";
  HttpResponse res = cgi::ParseCgiResponse(output);

  EXPECT_EQ(res.GetStatus(), lib::http::kInternalServerError);
}
