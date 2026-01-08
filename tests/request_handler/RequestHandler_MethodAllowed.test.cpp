#include <gtest/gtest.h>

#include "ServerConfig.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "lib/http/Status.hpp"
#include "lib/http/Method.hpp"

class RequestHandlerMethodTest : public ::testing::Test {
protected:
    ServerConfig config;
    void SetUp() override {
        // Default location
        Location location;
        location.SetName("/");
        location.SetRoot("/var/www/html");
        location.SetIndexFile("index.html");
        config.AddLocation(location);

        // Location with specific allowed methods
        Location api_loc;
        api_loc.SetName("/api");
        api_loc.SetRoot("/var/www/api");
        api_loc.SetHasAllowedMethods(true);
        api_loc.AddAllowedMethod(lib::http::kGet);
        api_loc.SetIndexFile("index.html"); // Minimal config
        config.AddLocation(api_loc);
    }
};

TEST_F(RequestHandlerMethodTest, MethodNotAllowed_ExplicitBlocked) {
    HttpRequest request;
    request.SetUri("/api/resource");
    request.SetMethod(lib::http::kPost); // Only GET is allowed in /api

    RequestHandler handler(config, request);
    ExecResult res = handler.Run();

    EXPECT_EQ(res.response.GetStatus(), lib::http::kMethodNotAllowed);
}

TEST_F(RequestHandlerMethodTest, MethodNotAllowed_AnotherBlocked) {
    HttpRequest request;
    request.SetUri("/api/resource");
    request.SetMethod(lib::http::kDelete); // Only GET is allowed in /api

    RequestHandler handler(config, request);
    ExecResult res = handler.Run();

    EXPECT_EQ(res.response.GetStatus(), lib::http::kMethodNotAllowed);
}
