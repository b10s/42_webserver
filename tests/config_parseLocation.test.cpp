#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

static void callParseLocation(const std::string& input, ServerConfig* sc) {
  ConfigParser p;
  p.content = input;
  p.ParseLocation(sc);
}

// ==================== happy path ====================

// minimal empty blcok such as  `/name/ { }`
TEST(ConfigParser, Location_MinimalEmptyBlock_AddsLocation) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseLocation("/images/ { }", &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_EQ(locs[0].GetName(), "/images/");
}

// only one known directive such as `autoindex on;`
TEST(ConfigParser, Location_WithAutoIndexOn) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseLocation("/cgi/ { autoindex on; }", &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_EQ(locs[0].GetName(), "/cgi/");
  EXPECT_TRUE(locs[0].GetAutoIndex());
}

TEST(ConfigParser, Location_WithSeveralKnownDirectives) {
  ServerConfig sc;
  const std::string s =
      "/app/ {\n"
      "  autoindex off;\n"
      "  root /var/www/app;\n"
      "  index index.html;\n"
      "  extension .php;\n"
      "}\n";
  EXPECT_NO_THROW(callParseLocation(s, &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_EQ(locs[0].GetName(), "/app/");
}

// ==================== error cases ====================

TEST(ConfigParser, Location_InvalidName_NoLeadingSlash_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("images/ { }", &sc), std::runtime_error);
}

TEST(ConfigParser, Location_InvalidName_NoTrailingSlash_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("/images { }", &sc), std::runtime_error);
}

TEST(ConfigParser, Location_MissingOpeningBrace_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("/x/  x", &sc), std::runtime_error);
}

TEST(ConfigParser, Location_UnknownDirective_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("/x/ { unknown_token ; }", &sc),
               std::runtime_error);
}

TEST(ConfigParser, Location_MissingClosingBrace_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("/x/ { autoindex on; ", &sc),
               std::runtime_error);
}
