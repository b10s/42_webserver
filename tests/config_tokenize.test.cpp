#include <gtest/gtest.h>
#include "config_parser.hpp"

TEST(tokenize, basicTokens) {
    ConfigParser parser("dummy"); // Filename won't be used in this test
    std::string content = "server { listen 80; server_name example.com; }";
    std::string st = parser.tokenize(content); // Initialize currentPos_
    EXPECT_EQ(st, "server");
}
