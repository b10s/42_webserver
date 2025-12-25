// test/gtest_RequestHandler.cpp
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

