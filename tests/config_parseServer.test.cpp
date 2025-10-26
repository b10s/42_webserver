#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "config_parser.hpp"

// ==================== happy path ====================
TEST(ConfigParser, Server_MinimalEmptyBlock_AddsServerConfig) {
  ConfigParser parser;
  parser.content_ = "{ }";
  EXPECT_NO_THROW(parser.parseServer());
  const std::vector<ServerConfig>& servers = parser.getServerConfigs();
  EXPECT_EQ(parser.getServerConfigs().size(), 1u);
  EXPECT_EQ(servers[0].getPort(), "80");       // default port
  EXPECT_EQ(servers[0].getHost(), "0.0.0.0");  // default host
}

TEST(ConfigParser, Server_WithListenDirective) {
  ConfigParser parser;
  parser.content_ = "{ listen 8080; }";
  EXPECT_NO_THROW(parser.parseServer());
  const std::vector<ServerConfig>& servers = parser.getServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  EXPECT_EQ(servers[0].getPort(), "8080");
}

// ==================== error cases ====================
TEST(ConfigParser, Server_MissingOpeningBrace_Throws) {
  ConfigParser parser;
  parser.content_ = "listen 8080; }";
  EXPECT_THROW(parser.parseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_MissingClosingBrace_Throws) {
  ConfigParser parser;
  parser.content_ = "listen 8080; }";
  EXPECT_THROW(parser.parseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_UnknownDirective_Throws) {
  ConfigParser parser;
  parser.content_ = "{ foo bar; }";
  EXPECT_THROW(parser.parseServer(), std::runtime_error);
}
