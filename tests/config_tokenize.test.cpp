#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include "config_parser.hpp"

TEST(ConfigParser, TokenizeBasic) {
    // デフォルトコンストラクタを使用して直接文字列を渡す
    ConfigParser parser;
    parser.content_ = "server { listen 8080; }";

    EXPECT_EQ(parser.tokenize(parser.content_), "server");
    EXPECT_EQ(parser.tokenize(parser.content_), "{");
    EXPECT_EQ(parser.tokenize(parser.content_), "listen");
    EXPECT_EQ(parser.tokenize(parser.content_), "8080");
    EXPECT_EQ(parser.tokenize(parser.content_), ";");
    EXPECT_EQ(parser.tokenize(parser.content_), "}");
}

// TEST(ConfigParser, LoadFromFile) {
//     // 実際のファイルから読み込むテスト
//     ConfigParser parser("../sample_config/server_test_for_parse.conf");
//     ASSERT_FALSE(parser.content_.empty());
// }

