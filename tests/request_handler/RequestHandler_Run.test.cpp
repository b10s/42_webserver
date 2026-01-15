#include <gtest/gtest.h>

#include "ServerConfig.hpp"
#include "Location.hpp"
#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "lib/http/Status.hpp"
#include "lib/http/Method.hpp"

TEST(RequestHandler, Run_NoMatchingLocation_Returns404) {
  // Arrange: server has only /images location
  ServerConfig config;
  Location images;
  images.SetName("/images");
  images.SetRoot("/var/img");
  images.SetIndexFile("index.html");
  config.AddLocation(images);

  HttpRequest req;
  req.SetMethod(lib::http::kGet);   // just to get past method check
  req.SetUri("/videos/movie.mp4");  // no matching location

  RequestHandler handler(config, req);
  ExecResult result = handler.Run();
  EXPECT_FALSE(result.is_async);
  EXPECT_EQ(result.new_socket, (ASocket*)NULL);
  EXPECT_EQ(result.response.GetStatus(), lib::http::kNotFound);
}
