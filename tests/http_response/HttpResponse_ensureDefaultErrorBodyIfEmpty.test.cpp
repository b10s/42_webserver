#include <gtest/gtest.h>
#include <algorithm>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "enums.hpp"
#include "lib/http/Status.hpp"

// setStatus does not automatically generate a default error body
TEST(HttpResponse, SetStatus_DoesNotGenerateBody) {
  HttpResponse res(lib::http::kNotFound);
  res.SetBody("");
  EXPECT_EQ(res.GetBody(), "");
}

// EnsureDefaultErrorContent sets a default error body if the body is empty and status is error
TEST(HttpResponse, EnsureDefaultErrorBodyIfEmpty_GeneratesForErrorStatus) {
  HttpResponse res(lib::http::kNotFound);
  res.SetBody("");
  res.EnsureDefaultErrorContent();
  EXPECT_FALSE(res.GetBody().empty());
  EXPECT_NE(res.GetBody().find("404 Not Found"), std::string::npos);
}

// EnsureDefaultErrorContent does not set a body for non-error status codes
TEST(HttpResponse, EnsureDefaultErrorBodyIfEmpty_DoesNotOverrideExistingBody) {
  HttpResponse res(lib::http::kInternalServerError);
  res.SetBody("custom");
  res.EnsureDefaultErrorContent();
  EXPECT_EQ(res.GetBody(), "custom");
}

// ToString outputs the correct content-length header if body is set
TEST(HttpResponse, ToString_AddsContentLengthIfMissingAndNoTransferEncoding) {
  HttpResponse res(lib::http::kOk);
  res.SetBody("hello");
  std::string s = res.ToHttpString();
  EXPECT_NE(s.find("content-length: 5\r\n"), std::string::npos);
}

// if transfer-encoding header exists, content-length should not be added
TEST(HttpResponse, ToString_DoesNotAddContentLengthWhenTransferEncodingExists) {
  HttpResponse res(lib::http::kOk);
  res.SetBody("hello");
  res.AddHeader("transfer-encoding", "chunked");
  std::string s = res.ToHttpString();
  EXPECT_EQ(s.find("content-length:"), std::string::npos);
}
