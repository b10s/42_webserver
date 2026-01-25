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

TEST_F(RequestHandlerTest, AppendIndexFileIfDirectoryOrThrow_RootDirectory) {
    HttpRequest request;
    request.SetUri("/");
    request.SetMethod(lib::http::kGet);
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext(); 
    std::string base = handler.ResolveFilesystemPath();
    EXPECT_EQ(handler.AppendIndexFileIfDirectoryOrThrow(base), "/var/www/html/index.html");
}

// ResolveFilesystemPath() correctly maps:
// /kapouet/pouic/toto → /tmp/www/pouic/toto
// /kapouet/ → /tmp/www/<index file>
TEST_F(RequestHandlerTest, AppendIndexFileIfDirectoryOrThrow_LocationSpecificRoot) {
    // Add a new location with a different root
    Location kapouetLocation;
    kapouetLocation.SetName("/kapouet");
    kapouetLocation.SetRoot("/tmp/www");
    kapouetLocation.SetIndexFile("index.html");
    config.AddLocation(kapouetLocation);

    // Test URI /kapouet/pouic/toto
    {
        HttpRequest request;
        request.SetUri("/kapouet/pouic/toto");
        request.SetMethod(lib::http::kGet);
        RequestHandler handler(config, request);
        handler.PrepareRoutingContext(); 
        std::string base = handler.ResolveFilesystemPath();
        EXPECT_EQ(handler.AppendIndexFileIfDirectoryOrThrow(base), "/tmp/www/pouic/toto");
    }

    // Test URI /kapouet/
    {
        HttpRequest request;
        request.SetUri("/kapouet/");
        request.SetMethod(lib::http::kGet);
        RequestHandler handler(config, request);
        handler.PrepareRoutingContext(); 
        std::string base = handler.ResolveFilesystemPath();
        EXPECT_EQ(handler.AppendIndexFileIfDirectoryOrThrow(base), "/tmp/www/index.html");
    }
}

// error when no index file is configured
// TODO: set appropriate HTTP status (500 Internal Server Error?)
TEST_F(RequestHandlerTest, AppendIndexFileIfDirectoryOrThrow_NoIndexFileConfigured) {
    // Add a new location without an index file
    Location noIndexLocation;
    noIndexLocation.SetName("/noindex");
    noIndexLocation.SetRoot("/tmp/noindex");
    config.AddLocation(noIndexLocation);
    HttpRequest request;
    request.SetUri("/noindex/");
    RequestHandler handler(config, request);
    handler.PrepareRoutingContext();
    std::string base = handler.ResolveFilesystemPath();
    EXPECT_THROW(
        handler.AppendIndexFileIfDirectoryOrThrow(base),
        std::runtime_error // TODO: shoudl be lib::http::kInternalServerError ??
    );
}

