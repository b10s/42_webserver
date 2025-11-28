#include <gtest/gtest.h>
#include <algorithm>
#include "HttpResponse.hpp"

TEST(HttpResponseTest, DefaultConstructor) {
  HttpResponse response;
  std::string output = response.ToString();
  
  EXPECT_NE(output.find("HTTP/1.1 200 OK"), std::string::npos);
  EXPECT_NE(output.find("Content-Length: 0"), std::string::npos);
  EXPECT_NE(output.find("Date: "), std::string::npos) << "HTTP response must include a Date header.";
}

TEST(HttpResponseTest, SetStatus) {
  HttpResponse response;
  response.SetStatus(404, "Not Found");
  std::string output = response.ToString();
  
  EXPECT_NE(output.find("HTTP/1.1 404 Not Found"), std::string::npos);
}

TEST(HttpResponseTest, AddHeader) {
  HttpResponse response;
  response.AddHeader("Content-Type", "application/json");
  response.AddHeader("X-Custom-Header", "MyValue");
  
  // RFC 7230: Header fields are case-insensitive.
  // Adding "content-type" (lowercase) should ideally overwrite or be treated as the same key.
  response.AddHeader("content-type", "text/html");

  std::string output = response.ToString();
  
  EXPECT_NE(output.find("Content-Type: application/json"), std::string::npos);
  EXPECT_NE(output.find("X-Custom-Header: MyValue"), std::string::npos);

  // Check for case-insensitivity: count occurrences of "content-type" (normalized to lowercase)
  std::string lower_output = output;
  std::transform(lower_output.begin(), lower_output.end(), lower_output.begin(), ::tolower);
  
  int count_content_type = 0;
  std::string::size_type pos = 0;
  while ((pos = lower_output.find("content-type:", pos)) != std::string::npos) {
    count_content_type++;
    pos += 13; // length of "content-type:"
  }
  
  EXPECT_EQ(count_content_type, 1) << "Headers should be case-insensitive. Found duplicate 'Content-Type'.";
}

TEST(HttpResponseTest, SetBody) {
  HttpResponse response;
  std::string body = "Hello, World!";
  response.SetBody(body);
  // Manually set content-length in lowercase
  response.AddHeader("content-length", std::to_string(body.length()));

  std::string output = response.ToString();
  
  EXPECT_NE(output.find(body), std::string::npos);
  
  // Check for auto-calculated Content-Length (should not duplicate if manually set with different case)
  std::string lower_output = output;
  std::transform(lower_output.begin(), lower_output.end(), lower_output.begin(), ::tolower);
  
  int count_content_length = 0;
  std::string::size_type pos = 0;
  while ((pos = lower_output.find("content-length:", pos)) != std::string::npos) {
    count_content_length++;
    pos += 15; // length of "content-length:"
  }
  
  EXPECT_EQ(count_content_length, 1) << "Should not duplicate Content-Length if manually set with different case.";
}

TEST(HttpResponseTest, FullResponse) {
  HttpResponse response;
  response.SetStatus(201, "Created");
  response.AddHeader("Content-Type", "text/plain");
  response.SetBody("Created Successfully");
  
  std::string output = response.ToString();
  
  // Check structure
  EXPECT_NE(output.find("HTTP/1.1 201 Created\r\n"), std::string::npos);
  EXPECT_NE(output.find("Content-Type: text/plain\r\n"), std::string::npos);
  // "Created Successfully" length is 20
  EXPECT_NE(output.find("Content-Length: 20\r\n"), std::string::npos);
  EXPECT_NE(output.find("\r\n\r\nCreated Successfully"), std::string::npos);
}
