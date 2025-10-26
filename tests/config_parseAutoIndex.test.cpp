#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "config_parser.hpp"

// set up a helper to call parseAutoIndex
static void callParseAutoIndex(const std::string& input, Location* loc) {
  ConfigParser
      parser;  // call default constructor to initialize currentPos_ to 0
  parser.content_ = input;  // parseAutoIndex は content_ から tokenize する想定
  parser.parseAutoIndex(loc);
}

TEST(ConfigParser, ParseAutoIndex_On_SetsTrue) {
  Location loc;  // autoindex is false by default
  EXPECT_NO_THROW(callParseAutoIndex("on;", &loc));
  EXPECT_TRUE(loc.getAutoIndex());
}

TEST(ConfigParser, ParseAutoIndex_Off_StaysFalse) {
  Location loc;
  EXPECT_NO_THROW(callParseAutoIndex("off;", &loc));
  EXPECT_FALSE(loc.getAutoIndex());
}

TEST(ConfigParser, ParseAutoIndex_MissingValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseAutoIndex(";", &loc), std::runtime_error);
  EXPECT_THROW(callParseAutoIndex("", &loc), std::runtime_error);
}

TEST(ConfigParser, ParseAutoIndex_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseAutoIndex("on", &loc), std::runtime_error);
  EXPECT_THROW(callParseAutoIndex("off", &loc), std::runtime_error);
}
