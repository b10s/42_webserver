#include <gtest/gtest.h>
#include "config_parser.hpp"

TEST(ConfigParserTest, BasicParse) {
    // テスト用の設定ファイル内容
    std::string configContent =
        "server {\n"
        "    listen 8080;\n"
        "    server_name test_server;\n"
        "    client_max_body_size 1024;\n"
        "    error_page 404 /404.html;\n"
        "    location / {\n"
        "        root /var/www/html;\n"
        "        autoindex on;\n"
        "    }\n"
        "}\n";

    // テスト用の一時ファイルを作成
    std::ofstream ofs("test.conf");
    ofs << configContent;
    ofs.close();

    ConfigParser parser("test.conf");
    const std::vector<ServerConfig>& servers = parser.getServerConfigs();

    ASSERT_EQ(servers.size(), 1);
    EXPECT_EQ(servers[0].getPort(), "8080");
    EXPECT_EQ(servers[0].getServerName(), "test_server");
    EXPECT_EQ(servers[0].getMaxBodySize(), 1024);

    const std::vector<Location>& locations = servers[0].getLocations();
    ASSERT_EQ(locations.size(), 1);
    EXPECT_EQ(locations[0].getName(), "/");
    EXPECT_EQ(locations[0].getRoot(), "/var/www/html");
    EXPECT_TRUE(locations[0].getAutoIndex());
}