#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseAutoIndex
static void callParseRoot(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content = input;
  p.ParseRoot(loc);
}

TEST(ConfigParser, SimpleDir_ParseRoot_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRoot("/var/www/html;", &loc));
  EXPECT_EQ(loc.GetRoot(), "/var/www/html");
}

TEST(ConfigParser, SimpleDir_ParseRoot_DotdotInFileName_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRoot("/var/ww..w/html;", &loc));
  EXPECT_EQ(loc.GetRoot(), "/var/ww..w/html");
}

TEST(ConfigParser, SimpleDir_ParseRoot_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www/html", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot(";", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_ExtraTokens_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var /www;", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRoot_InvalidChar_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www\r/html;", &loc), std::runtime_error);
}


  
