#include <gtest/gtest.h>
#include <algorithm>
#include "HttpResponse.hpp"
#include "enums.hpp"

TEST(HttpResponseTest, DefaultConstructor) {
  HttpResponse response;
  std::string output = response.ToString();
  
  EXPECT_NE(output.find("HTTP/1.1 200 OK"), std::string::npos);
  EXPECT_NE(output.find("content-length: 0"), std::string::npos);
  EXPECT_NE(output.find("date: "), std::string::npos);
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
  response.AddHeader("content-type", "text/html");

  std::string output = response.ToString();
  
  EXPECT_NE(output.find("content-type: text/html"), std::string::npos);
  EXPECT_NE(output.find("x-custom-header: MyValue"), std::string::npos);

  int count_content_type = 0;
  std::string::size_type pos = 0;
  while ((pos = output.find("content-type:", pos)) != std::string::npos) {
    count_content_type++;
    pos += std::string("content-type:").length();
  }
  
  EXPECT_EQ(count_content_type, 1);
}

TEST(HttpResponseTest, SetBody) {
  HttpResponse response;
  std::string body = "Hello, World!";
  response.SetBody(body);
  response.AddHeader("content-length", std::to_string(body.length()));

  std::string output = response.ToString();
  
  EXPECT_NE(output.find(body), std::string::npos);
  
  std::string lower_output = output;
  std::transform(lower_output.begin(), lower_output.end(), lower_output.begin(), ::tolower);
  
  int count_content_length = 0;
  std::string::size_type pos = 0;
  while ((pos = lower_output.find("content-length:", pos)) != std::string::npos) {
    count_content_length++;
    pos += std::string("content-length:").length();
  }
  
  EXPECT_EQ(count_content_length, 1);
}

TEST(HttpResponseTest, FullResponse) {
  HttpResponse response;
  response.SetStatus(201, "Created");
  response.AddHeader("Content-Type", "text/plain");
  std::string body = "Created Successfully";
  response.SetBody(body);
  
  std::string output = response.ToString();
  
  EXPECT_NE(output.find("HTTP/1.1 201 Created\r\n"), std::string::npos);
  EXPECT_NE(output.find("content-type: text/plain\r\n"), std::string::npos);
  EXPECT_NE(output.find("content-length: " + std::to_string(body.length()) + "\r\n"), std::string::npos);
  EXPECT_NE(output.find("\r\n\r\n" + body), std::string::npos);
}
