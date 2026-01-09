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

TEST(ConfigParser, Location_WithCgiOn) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseLocation("/cgi-bin/ { cgi on; }", &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_TRUE(locs[0].GetCgiEnabled());
}

TEST(ConfigParser, Location_WithCgiOff) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseLocation("/cgi-bin/ { cgi off; }", &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_FALSE(locs[0].GetCgiEnabled());
}

TEST(ConfigParser, Location_WithCgiAllowedExtensions) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseLocation("/cgi-bin/ { cgi_allowed_extensions .py .php; }", &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  EXPECT_EQ(locs[0].GetCgiAllowedExtensions()[0], ".py");
  EXPECT_EQ(locs[0].GetCgiAllowedExtensions()[1], ".php");
}

TEST(ConfigParser, Location_WithSeveralKnownDirectives) {
  ServerConfig sc;
  const std::string s =
      "/app/ {\n"
      "  autoindex off;\n"
      "  root /var/www/app;\n"
      "  index index.html;\n"
      "}\n";
  EXPECT_NO_THROW(callParseLocation(s, &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_EQ(locs[0].GetName(), "/app/");
}

TEST(ConfigParser, Location_WithRedirect) {
  ServerConfig sc;
  const std::string s =
      "/old/ {\n"
      "  redirect /new/page.html;\n"
      "}\n";
  EXPECT_NO_THROW(callParseLocation(s, &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 1u);
  EXPECT_TRUE(locs[0].HasRedirect());
  EXPECT_EQ(locs[0].GetRedirect(), "/new/page.html");
  EXPECT_EQ(locs[0].GetRedirectStatus(), lib::http::kFound);
}

// ==================== error cases ====================

TEST(ConfigParser, Location_InvalidName_NoLeadingSlash_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseLocation("images/ { }", &sc), std::runtime_error);
}

// TEST(ConfigParser, Location_InvalidName_NoTrailingSlash_Throws) {
//   ServerConfig sc;
//   EXPECT_THROW(callParseLocation("/images { }", &sc), std::runtime_error);
// }

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

// ==================== duplicate directive error cases ====================

TEST(ConfigParser, Location_DuplicateAllowedMethods_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  allowed_methods GET POST;\n"
      "  allowed_methods GET;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateRoot_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  root /var/www/html;\n"
      "  root /var/www/other;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateAutoIndex_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  autoindex on;\n"
      "  autoindex off;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateIndex_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  index index.html;\n"
      "  index index.htm;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateUploadPath_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  upload_path /tmp/uploads;\n"
      "  upload_path /var/uploads;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateRedirect_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  redirect https://example.com/;\n"
      "  redirect https://example.org/;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_DuplicateCgi_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  cgi on;\n"
      "  cgi off;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

TEST(ConfigParser, Location_InvalidCgiValue_Throws) {
  ServerConfig sc;
  const std::string s =
      "/x/ {\n"
      "  cgi invalid;\n"
      "}\n";
  EXPECT_THROW(callParseLocation(s, &sc), std::runtime_error);
}

// ==================== multiple locations ====================
TEST(ConfigParser, Location_MultipleLocations_AddsAll) {
  ServerConfig sc;
  const std::string s1 =
      "/images/ {\n"
      "  autoindex on;\n"
      "}\n";
  const std::string s2 =
      "/app/ {\n"
      "  root /var/www/app;\n"
      "}\n";
  EXPECT_NO_THROW(callParseLocation(s1, &sc));
  EXPECT_NO_THROW(callParseLocation(s2, &sc));

  const std::vector<Location>& locs = sc.GetLocations();
  ASSERT_EQ(locs.size(), 2u);
  EXPECT_EQ(locs[0].GetName(), "/images/");
  EXPECT_EQ(locs[1].GetName(), "/app/");
}

TEST(ConfigParser, Location_DuplicateLocationName_Throws) {
  ServerConfig sc;
  const std::string s1 =
      "/x/ {\n"
      "  autoindex on;\n"
      "}\n";
  const std::string s2 =
      "/x/ {\n"
      "  root /var/www/html;\n"
      "}\n";
  EXPECT_NO_THROW(callParseLocation(s1, &sc));
  EXPECT_THROW(callParseLocation(s2, &sc), std::runtime_error);
}

