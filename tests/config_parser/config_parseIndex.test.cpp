#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseIndex
static void callParseIndex(const std::string& input, Location* loc) {
  ConfigParser parser;
  parser.content = input;
  parser.ParseIndex(loc);
}

TEST(ConfigParser, ParseIndex_SingleIndex) {
  Location loc;
  EXPECT_NO_THROW(callParseIndex("index.html;", &loc));
  EXPECT_EQ(loc.GetIndexFile(), std::string("index.html"));
}

TEST(ConfigParser, ParseIndex_MultipleIndexes_Throws) {
  Location loc;
  EXPECT_THROW(callParseIndex("index.html index.htm index.php;", &loc),
               std::runtime_error);
}

TEST(ConfigParser, ParseIndex_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseIndex("index.html", &loc),
               std::runtime_error);
}

TEST(ConfigParser, ParseIndex_UnsafeFilename_Throws) {
  Location loc;
  EXPECT_THROW(callParseIndex("../etc/passwd;", &loc),
               std::runtime_error);
  EXPECT_THROW(callParseIndex("index file.html;", &loc),
               std::runtime_error);
  EXPECT_THROW(callParseIndex("index<file>.html;", &loc),
               std::runtime_error);
}
