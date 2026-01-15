#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// Unit tests
// ServerConfig::MatchLocation():
// locations: / root=./www, /kapouet root=/tmp/www, /images root=/var/img
// /matches /
// /kapouet/pouic/toto → /kapouet
// /kapouet2/test→ /
// /images/cat.png → /images

TEST(ConfigParser, Server_FindLocation_BasicMatches) {
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 8080; "
      "location / { root /www; } "
      "location /kapouet { root /tmp/www; } "
      "location /images { root /var/img; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];
  // "/" -> location "/" remainder "/"
  {
    LocationMatch m = server.FindLocationForUri("/");
    ASSERT_NE(m.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m.loc->GetName(), "/");
    EXPECT_EQ(m.remainder, "/");
  }
  
  // "/kapouet/pouic/toto" -> "/kapouet" + "/pouic/toto"
  {
    LocationMatch m = server.FindLocationForUri("/kapouet/pouic/toto");
    ASSERT_NE(m.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m.loc->GetName(), "/kapouet");
    EXPECT_EQ(m.remainder, "/pouic/toto");
  }
  // boundary: "/kapouet2/test" should fall back to "/"
  {
    LocationMatch m = server.FindLocationForUri("/kapouet2/test");
    ASSERT_NE(m.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m.loc->GetName(), "/");
    EXPECT_EQ(m.remainder, "/kapouet2/test");
  }
  // "/images/cat.png" -> "/images" + "/cat.png"
  {
    LocationMatch m = server.FindLocationForUri("/images/cat.png");
    ASSERT_NE(m.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m.loc->GetName(), "/images");
    EXPECT_EQ(m.remainder, "/cat.png");
  }
  // Find location that does not exist
  {
    LocationMatch m = server.FindLocationForUri("/nonexistent/path");
    ASSERT_NE(m.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m.loc->GetName(), "/");
  }
}

// throws if no root location is defined
// TODO: change behavior to return 404 location instead
TEST(ConfigParser, Server_FindLocation_NoRootLocation) {
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 8080; "
      "location /kapouet { root /tmp/www; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];

  // /kapouet2/test -> no match -> throws (current behavior)
  {
    EXPECT_THROW(
      server.FindLocationForUri("/kapouet2/test"),
      std::runtime_error
    );
  }
}

// matching with trailing slashes in URI and location definitions
TEST(ConfigParser, Server_FindLocation_TrailingSlashes) {
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 8080; "
      "location / { root /www; } "
      "location /kapouet { root /tmp/www; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];
  // Exact match with trailing slash variants: "/kapouet" and "/kapouet/"
  // Both should pick the same location and remainder "/"
  {
    LocationMatch m1 = server.FindLocationForUri("/kapouet");
    LocationMatch m2 = server.FindLocationForUri("/kapouet/");
    LocationMatch m3 = server.FindLocationForUri("/kapouet////");
    ASSERT_NE(m1.loc, static_cast<const Location*>(NULL));
    ASSERT_NE(m2.loc, static_cast<const Location*>(NULL));
    ASSERT_NE(m3.loc, static_cast<const Location*>(NULL));
    EXPECT_EQ(m1.loc->GetName(), "/kapouet");
    EXPECT_EQ(m2.loc->GetName(), "/kapouet");
    EXPECT_EQ(m3.loc->GetName(), "/kapouet");
    EXPECT_EQ(m1.remainder, "/");
    EXPECT_EQ(m2.remainder, "/");
    EXPECT_EQ(m3.remainder, "/");
  }
}

// throw not found if no matching location
TEST(ConfigParser, Server_FindLocation_NoMatchingLocation) {
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 8080; "
      "location /images { root /var/img; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];
  // "/videos/movie.mp4" -> no match -> throw not found
  {
    EXPECT_THROW(
      server.FindLocationForUri("/videos/movie.mp4"),
      lib::exception::ResponseStatusException
    );
  }
}
