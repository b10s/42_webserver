#include <gtest/gtest.h>

#include "ServerConfig.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "HttpRequest.hpp"

class RequestHandlerTest : public ::testing::Test {
protected:
    ServerConfig config;
    void SetUp() override {
        Location location;
        location.SetName("/");
        location.SetRoot("/var/www/html");
        location.SetIndexFile("index.html");
        config.AddLocation(location);
    }
};

TEST_F(RequestHandlerTest, ResolveFilesystemPath_RootDirectory) {
    HttpRequest request;
    request.SetUri("/");
    RequestHandler handler(config, request);

    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/index.html");
}

TEST_F(RequestHandlerTest, ResolveFilesystemPath_SubDirectory) {
    HttpRequest request;
    request.SetUri("/images/");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/images/index.html");
}

// TODO: enable this test after implementing file requests
// TEST_F(RequestHandlerTest, ResolveFilesystemPath_FileRequest) {
//     HttpRequest request;
//     request.SetUri("/about.html");
//     RequestHandler handler(config, request);
//     handler.PrepareRoutingContext(); 
//     EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/about.html");
// }

TEST_F(RequestHandlerTest, ResolveFilesystemPath_SingleDot) {
    HttpRequest request;
    request.SetUri("/file./");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/file./index.html");
}

TEST_F(RequestHandlerTest, ResolveFilesystemPath_MultipleDotsAtEnd) {
    HttpRequest request;
    request.SetUri("/strange.../");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/strange.../index.html");
}

TEST_F(RequestHandlerTest, ResolveFilesystemPath_DotsInDirectoryName) {
    HttpRequest request;
    request.SetUri("/v2.1.0/about/");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/v2.1.0/about/index.html");
}

// Edge case: Directory with trailing slash that has dots in name
TEST_F(RequestHandlerTest, ResolveFilesystemPath_DirectoryWithDotsTrailingSlash) {
    HttpRequest request;
    request.SetUri("/modules/package.v1.2.3/");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    EXPECT_EQ(handler.ResolveFilesystemPath(), "/var/www/html/modules/package.v1.2.3/index.html");
}

// ResolveFilesystemPath() correctly maps:
// /kapouet/pouic/toto → /tmp/www/pouic/toto
// /kapouet/ → /tmp/www/<index file>

TEST_F(RequestHandlerTest, ResolveFilesystemPath_LocationSpecificRoot) {
    // Add a new location with a different root
    Location kapouetLocation;
    kapouetLocation.SetName("/kapouet");
    kapouetLocation.SetRoot("/tmp/www");
    kapouetLocation.SetIndexFile("index.html");
    config.AddLocation(kapouetLocation);

    // Test URI /kapouet/pouic/toto
    {
        HttpRequest request;
        request.SetUri("/kapouet/pouic/toto/");
        RequestHandler handler(config, request);
        handler.PrepareRoutingContext(); 
        EXPECT_EQ(handler.ResolveFilesystemPath(), "/tmp/www/pouic/toto/index.html");
    }

    // Test URI /kapouet/
    {
        HttpRequest request;
        request.SetUri("/kapouet/");
        RequestHandler handler(config, request);
        handler.PrepareRoutingContext(); 
        EXPECT_EQ(handler.ResolveFilesystemPath(), "/tmp/www/index.html");
    }
}
