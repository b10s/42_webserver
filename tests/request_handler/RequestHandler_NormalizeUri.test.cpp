#include <gtest/gtest.h>

#include "HttpRequest.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"

class RequestHandlerNormalizeTest : public ::testing::Test {
 protected:
  ServerConfig config;

  void SetUp() override {
    Location location;
    location.SetName("/");
    location.SetRoot("/var/www/html");
    location.SetIndexFile("index.html");
    config.AddLocation(location);

    Location dirlist;
    dirlist.SetName("/dirlist");
    dirlist.SetRoot("/var/www/dirlist");
    dirlist.SetIndexFile("index.html");
    config.AddLocation(dirlist);
  }
};

TEST_F(RequestHandlerNormalizeTest, NormalizeUri_MultipleSlashes) {
  HttpRequest request;
  request.SetUri("///dirlist/");
  RequestHandler handler(config, request);
  handler.PrepareRoutingContext();
  EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/dirlist/");
}

TEST_F(RequestHandlerNormalizeTest, NormalizeUri_MultipleSlashesInMiddle) {
  HttpRequest request;
  request.SetUri("/a///b/c");
  RequestHandler handler(config, request);
  handler.PrepareRoutingContext();
  EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/a/b/c");
}

TEST_F(RequestHandlerNormalizeTest, NormalizeUri_MixedSlashesAndDots) {
  HttpRequest request;
  request.SetUri("/a/./b///c");
  RequestHandler handler(config, request);
  handler.PrepareRoutingContext();
  EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/a/b/c");
}

TEST_F(RequestHandlerNormalizeTest, NormalizeUri_LeadingTripleSlash) {
  HttpRequest request;
  request.SetUri("///");
  RequestHandler handler(config, request);
  handler.PrepareRoutingContext();
  EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/");
}
