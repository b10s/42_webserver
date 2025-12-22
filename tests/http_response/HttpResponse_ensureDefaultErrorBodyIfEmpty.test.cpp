#include <gtest/gtest.h>
#include <algorithm>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "enums.hpp"

// setStatus does not automatically generate a default error body
TEST(HttpResponse, SetStatus_DoesNotGenerateBody) {
  HttpResponse res;
  res.SetBody("");
  res.SetStatus(404, "Not Found");
  EXPECT_EQ(res.GetBody(), "");
}

// EnsureDefaultBodyIfEmpty sets a default error body if the body is empty and status is error
TEST(HttpResponse, EnsureDefaultErrorBodyIfEmpty_GeneratesForErrorStatus) {
  HttpResponse res;
  res.SetBody("");
  res.SetStatus(404, "Not Found");
  res.EnsureDefaultBodyIfEmpty();
  EXPECT_FALSE(res.GetBody().empty());
  EXPECT_NE(res.GetBody().find("404 Not Found"), std::string::npos);
}

// EnsureDefaultBodyIfEmpty does not set a body for non-error status codes
TEST(HttpResponse, EnsureDefaultErrorBodyIfEmpty_DoesNotOverrideExistingBody) {
  HttpResponse res;
  res.SetBody("custom");
  res.SetStatus(500, "Internal Server Error");
  res.EnsureDefaultBodyIfEmpty();
  EXPECT_EQ(res.GetBody(), "custom");
}

// ToString outputs the correct content-length header if body is set
TEST(HttpResponse, ToString_AddsContentLengthIfMissingAndNoTransferEncoding) {
  HttpResponse res;
  res.SetBody("hello");
  res.SetStatus(200, "OK");
  std::string s = res.ToHttpString();
  EXPECT_NE(s.find("content-length: 5\r\n"), std::string::npos);
}

// if transfer-encoding header exists, content-length should not be added
TEST(HttpResponse, ToString_DoesNotAddContentLengthWhenTransferEncodingExists) {
  HttpResponse res;
  res.SetBody("hello");
  res.SetStatus(200, "OK");
  res.AddHeader("transfer-encoding", "chunked");
  std::string s = res.ToHttpString();
  EXPECT_EQ(s.find("content-length:"), std::string::npos);
}
