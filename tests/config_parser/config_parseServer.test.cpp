#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// ==================== happy path ====================
TEST(ConfigParser, Server_MinimalEmptyBlock_AddsServerConfig) {
  ConfigParser parser;
  parser.content = "{ }";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  EXPECT_EQ(parser.GetServerConfigs().size(), 1u);
  EXPECT_EQ(servers[0].GetPort(), 80);       // default port
  EXPECT_EQ(servers[0].GetHost(), "0.0.0.0");  // default host
}

TEST(ConfigParser, Server_WithListenDirective) {
  ConfigParser parser;
  parser.content = "{ listen 8080; }";
  EXPECT_NO_THROW(parser.ParseServer());
  const std::vector<ServerConfig>& servers = parser.GetServerConfigs();
  ASSERT_EQ(servers.size(), 1u);
  EXPECT_EQ(servers[0].GetPort(), 8080);
}

// ==================== error cases ====================
TEST(ConfigParser, Server_MissingOpeningBrace_Throws) {
  ConfigParser parser;
  parser.content = "listen 8080; }";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_MissingClosingBrace_Throws) {
  ConfigParser parser;
  parser.content = "listen 8080; }";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_UnknownDirective_Throws) {
  ConfigParser parser;
  parser.content = "{ foo bar; }";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

// ==================== duplicate directive error cases ====================

TEST(ConfigParser, Server_DuplicateListen_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "listen 8080; "
      "listen 8081; "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_DuplicateServerName_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "server_name example.com; "
      "server_name www.example.com; "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_DuplicateMaxBodySize_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "client_max_body_size 1000000; "
      "client_max_body_size 2000000; "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_DuplicateLocationPath_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "location /foo/ { } "
      "location /foo/ { } "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

// normalized duplicate location paths
TEST(ConfigParser, Server_DuplicateLocationPath_Normalised_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "location /foo/ { } "
      "location /foo { } "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

// reject multiple slash or dots in location path
TEST(ConfigParser, Server_Duplicate_Slash_In_LocationPath_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "location /foo// { } "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}

TEST(ConfigParser, Server_Duplicate_Dots_In_LocationPath_Throws) {
  ConfigParser parser;
  parser.content =
      "{ "
      "location /..foo { } "
      "}";
  EXPECT_THROW(parser.ParseServer(), std::runtime_error);
}
