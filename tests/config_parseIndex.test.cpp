#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseAutoIndex
static void callParseIndex(const std::string& input, Location* loc) {
  ConfigParser parser;
  parser.content_ = input;
  parser.ParseIndex(loc);
}

TEST(ConfigParser, ParseIndex_SingleIndex) {
  Location loc;
  EXPECT_NO_THROW(callParseIndex("index.html;", &loc));
  std::vector<std::string> indexes = loc.GetIndexFiles();
  ASSERT_EQ(indexes.size(), 1);
  EXPECT_EQ(indexes[0], "index.html");
}

TEST(ConfigParser, ParseIndex_MultipleIndexes) {
  Location loc;
  EXPECT_NO_THROW(callParseIndex("index.html index.htm index.php;", &loc));
  std::vector<std::string> indexes = loc.GetIndexFiles();
  ASSERT_EQ(indexes.size(), 3);
  EXPECT_EQ(indexes[0], "index.html");
  EXPECT_EQ(indexes[1], "index.htm");
  EXPECT_EQ(indexes[2], "index.php");
}

TEST(ConfigParser, ParseIndex_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseIndex("index.html index.htm index.php", &loc),
               std::runtime_error);
  EXPECT_THROW(callParseIndex("index.html", &loc), std::runtime_error);
}
