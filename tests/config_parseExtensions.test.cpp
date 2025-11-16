#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseAutoIndex
static void callParseExtensions(const std::string& input, Location* loc) {
  ConfigParser
      parser;  // call default constructor to initialize current_pos_ to 0
  parser.content = input;  // ParseAutoIndex は content から Tokenize する想定
  parser.ParseExtensions(loc);
}

TEST(ConfigParser, ParseExtensions) {
  Location loc;
  EXPECT_NO_THROW(callParseExtensions(".php;", &loc));
  std::string exts = loc.GetExtensions();
  EXPECT_EQ(exts, std::string(".php"));
}

TEST(ConfigParser, ParseExtensions_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseExtensions(".php", &loc), std::runtime_error);
}

TEST(ConfigParser, ParseExtensions_NoExtensions_Throws) {
  Location loc;
  EXPECT_THROW(callParseExtensions(";", &loc), std::runtime_error);
  EXPECT_THROW(callParseExtensions("", &loc), std::runtime_error);
}

// duplicate test: we have to discuss how to handle multiple extensions
// TEST(ConfigParser, ParseExtensions_DuplicatePolicy) {
//     Location loc;
//     EXPECT_NO_THROW(callParseExtentions(".php;", &loc));
//     EXPECT_NO_THROW(callParseExtentions(".php;", &loc));
//     auto exts = loc.GetExtensions();
//     // TODO: define expected behavior for duplicates
//     // EXPECT_EQ(exts.size(), 2);
// }
