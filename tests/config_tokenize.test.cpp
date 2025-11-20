#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include "ConfigParser.hpp"

// TEST(ConfigParser, TokenizeBasic) {
//   // デフォルトコンストラクタを使用して直接文字列を渡す
//   ConfigParser parser;
//   parser.content = "server { listen 8080; }";

//   EXPECT_EQ(parser.Tokenize(parser.content), "server");
//   EXPECT_EQ(parser.Tokenize(parser.content), "{");
//   EXPECT_EQ(parser.Tokenize(parser.content), "listen");
//   EXPECT_EQ(parser.Tokenize(parser.content), "8080");
//   EXPECT_EQ(parser.Tokenize(parser.content), ";");
//   EXPECT_EQ(parser.Tokenize(parser.content), "}");
// }

// TEST(ConfigParser, LoadFromFile) {
//     // 実際のファイルから読み込むテスト
//     ConfigParser parser("../sample_config/server_test_for_parse.conf");
//     ASSERT_FALSE(parser.content.empty());
// }
