#include <gtest/gtest.h>
#include "RequestHandler.hpp"

class RequestHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.AddDefaultLocation("/var/www/html", "index.html");
    }
    ServerConfig config;
};

TEST_F(RequestHandlerTest, ResolveFullPath_RootDirectory) {
    HttpRequest request;
    request.SetUri("/");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/index.html");
}

TEST_F(RequestHandlerTest, ResolveFullPath_SubDirectory) {
    HttpRequest request;
    request.SetUri("/images/");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/images/index.html");
}

TEST_F(RequestHandlerTest, ResolveFullPath_FileRequest) {
    HttpRequest request;
    request.SetUri("/about.html");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/about.html");
}

TEST_F(RequestHandlerTest, ResolveFullPath_SingleDot) {
    HttpRequest request;
    request.SetUri("/file.");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/file.");
}

TEST_F(RequestHandlerTest, ResolveFullPath_MultipleDotsAtEnd) {
    HttpRequest request;
    request.SetUri("/strange...");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/strange...");
}

TEST_F(RequestHandlerTest, ResolveFullPath_DotsInDirectoryName) {
    HttpRequest request;
    request.SetUri("/v2.1.0/about");
    RequestHandler handler(config, request);

    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/v2.1.0/about");
}

// Edge case: Directory with trailing slash that has dots in name
TEST_F(RequestHandlerTest, ResolveFullPath_DirectoryWithDotsTrailingSlash) {
    HttpRequest request;
    request.SetUri("/modules/package.v1.2.3/");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/modules/package.v1.2.3/index.html");
}