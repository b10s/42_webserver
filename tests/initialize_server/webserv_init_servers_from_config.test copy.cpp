#include <gtest/gtest.h>
#include "Webserv.hpp"
#include "ConfigParser.hpp"

static std::vector<ServerConfig> ParseConfigs(const std::string& text) {
  ConfigParser p;
  p.content = text;
  p.Parse();
  return p.GetServerConfigs();
}

class WebservConfigTest : public ::testing::Test {
protected:
  Webserv ws;
};

// Empty configuration
TEST_F(WebservConfigTest, InitServersFromConfigs_Empty) {
  std::vector<ServerConfig> configs;
  ws.InitServersFromConfigs(configs);

  EXPECT_TRUE(ws.GetPortConfigs().empty());
  EXPECT_EQ(ws.FindServerConfigByPort("8080"), (const ServerConfig*)NULL);
}

// Two unique port configurations
TEST_F(WebservConfigTest, InitServersFromConfigs_UniquePorts) {
  std::vector<ServerConfig> configs = ParseConfigs(
      "server { listen 8080; }\n"
      "server { listen 8000; }\n"
  );

  EXPECT_NO_THROW(ws.InitServersFromConfigs(configs));

  const std::map<std::string, ServerConfig>& m = ws.GetPortConfigs();
  ASSERT_EQ(m.size(), 2u);
  EXPECT_NE(m.find("8080"), m.end());
  EXPECT_NE(m.find("8000"), m.end());
}

// Duplicate port configurations - first one should be used
TEST_F(WebservConfigTest, InitServersFromConfigs_DuplicatePort_UsesFirstOnly) {
  std::vector<ServerConfig> configs = ParseConfigs(
      "server { listen 8080; server_name first; }\n"
      "server { listen 8080; server_name second; }\n"
  );

  ws.InitServersFromConfigs(configs);

  const ServerConfig* sc = ws.FindServerConfigByPort("8080");
  ASSERT_NE(sc, (const ServerConfig*)NULL);
  EXPECT_EQ(sc->GetServerName(), "first");
}

// Finding existing port configuration
TEST_F(WebservConfigTest, FindServerConfigByPort_FoundAndNotFound) {
  std::vector<ServerConfig> configs = ParseConfigs(
      "server { listen 8080; }\n"
  );

  ws.InitServersFromConfigs(configs);

  EXPECT_NE(ws.FindServerConfigByPort("8080"), (const ServerConfig*)NULL);
  EXPECT_EQ(ws.FindServerConfigByPort("9999"), (const ServerConfig*)NULL);
}

// Finding non-existing port configuration
TEST_F(WebservConfigTest, FindServerConfigByPort_NotFound) {
  std::vector<ServerConfig> configs;
  configs.push_back(MakeServerConfig("8080"));
  ws.InitServersFromConfigs(configs);

  EXPECT_EQ(ws.FindServerConfigByPort("9999"), (const ServerConfig*)NULL);
}

// GetPortConfigs returns the current map
TEST_F(WebservConfigTest, GetPortConfigs_ReturnsCurrentMap) {
  std::vector<ServerConfig> configs;
  configs.push_back(MakeServerConfig("8080"));
  configs.push_back(MakeServerConfig("8000"));
  ws.InitServersFromConfigs(configs);

  const std::map<std::string, ServerConfig>& m = ws.GetPortConfigs();
  EXPECT_EQ(m.size(), 2u);
  EXPECT_NE(m.find("8080"), m.end());
  EXPECT_NE(m.find("8000"), m.end());
}
