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

TEST(ConfigParser, Server_MatchLocation_BasicMatches) {
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
    const Location* loc =
        server.MatchLocation("/kapouet/pouic/toto");
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->GetName(), "/kapouet");
  }
  // Test matching /kapouet2/test
  {
    const Location* loc =
        server.MatchLocation("/kapouet2/test");
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->GetName(), "/");
  }
  // Test matching /images/cat.png
  {
    const Location* loc =
        server.MatchLocation("/images/cat.png");
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->GetName(), "/images");
  }
}
