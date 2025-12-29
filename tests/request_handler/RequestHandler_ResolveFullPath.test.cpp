#include <gtest/gtest.h>
#include "RequestHandler.hpp"

class RequestHandlerTest : public ::testing::Test {
protected:
    ServerConfig config;
    void SetUp() override {
        Location location;
        location.SetRoot("/var/www/html");
        location.AddIndex("index.html");
        config.AddLocation(location);
    }
};

TEST_F(RequestHandlerTest, ResolveFullPath_RootDirectory) {
    HttpRequest request;
    request.SetUri("/");
    RequestHandler handler(config, request);
    
    EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/index.html");
}

// TEST_F(RequestHandlerTest, ResolveFullPath_SubDirectory) {
//     HttpRequest request;
//     request.SetUri("/images/");
//     RequestHandler handler(config, request);
    
//     EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/images/index.html");
// }

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
// TEST_F(RequestHandlerTest, ResolveFullPath_DirectoryWithDotsTrailingSlash) {
//     HttpRequest request;
//     request.SetUri("/modules/package.v1.2.3/");
//     RequestHandler handler(config, request);
    
//     EXPECT_EQ(handler.ResolveFullPath(), "/var/www/html/modules/package.v1.2.3/index.html");
// }

// ResolveFullPath() correctly maps:
// /kapouet/pouic/toto → /tmp/www/pouic/toto
// /kapouet/ → /tmp/www/<index file>

TEST_F(RequestHandlerTest, ResolveFullPath_LocationSpecificRoot) {
    // Add a new location with a different root
    Location kapouetLocation;
    kapouetLocation.SetName("/kapouet");
    kapouetLocation.SetRoot("/tmp/www");
    kapouetLocation.AddIndex("index.html");
    config.AddLocation(kapouetLocation);

    // Test URI /kapouet/pouic/toto
    {
        HttpRequest request;
        request.SetUri("/kapouet/pouic/toto");
        RequestHandler handler(config, request);
        
        EXPECT_EQ(handler.ResolveFullPath(), "/tmp/www/pouic/toto");
    }

    // Test URI /kapouet/
    {
        HttpRequest request;
        request.SetUri("/kapouet/");
        RequestHandler handler(config, request);
        
        EXPECT_EQ(handler.ResolveFullPath(), "/tmp/www/index.html");
    }
}
