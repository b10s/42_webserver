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
      "location / { root ./www; } "
      "location /kapouet { root /tmp/www; } "
      "location /images { root /var/img; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];
  // Test matching /kapouet/pouic/toto
  {
    const Location& loc =
        server.FindLocationForUri("/kapouet/pouic/toto");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/kapouet");
  }
  // Test matching /kapouet2/test
  {
    const Location& loc =
        server.FindLocationForUri("/kapouet2/test");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/");
  }
  // Test matching /images/cat.png
  {
    const Location& loc =
        server.FindLocationForUri("/images/cat.png");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/images");
  }

  // Find location that does not exist
  {
    const Location& loc =
        server.FindLocationForUri("/nonexistent/path");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/");
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
      "location / { root ./www; } "
      "location /kapouet { root /tmp/www; } "
      "location /images/// { root /var/img; } "
      "location /img//assets { root /var/img; } "
      "}";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  const ServerConfig& server = servers[0];
  // Test root
  {
    const Location& loc =
        server.FindLocationForUri("/");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/");
  }
  // Test matching /kapouet/pouic/toto
  {
    const Location& loc =
        server.FindLocationForUri("/kapouet/pouic/toto");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/kapouet");
  }
  // Test matching /images/cat.png
  {
    const Location& loc =
        server.FindLocationForUri("/images/cat.png");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/images///");
  }
  // Test matching /img//assets/cat.png
  {
    const Location& loc =
        server.FindLocationForUri("/img//assets/cat.png");
    ASSERT_NE(&loc, nullptr);
    EXPECT_EQ(loc.GetName(), "/img//assets");
  }
}
