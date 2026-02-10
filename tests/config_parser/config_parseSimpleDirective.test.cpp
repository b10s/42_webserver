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

static void callParseUploadPath(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content = input;
  p.ParseUploadPath(loc);
}

static void callParseRedirect(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content = input;
  p.ParseRedirect(loc);
}

static void callParseServerName(const std::string& input, ServerConfig* sc) {
  ConfigParser p;
  p.content = input;
  p.ParseServerName(sc);
}

/* ===================== ParseRoot ===================== */
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

// TEST(ConfigParser, SimpleDir_ParseRoot_RelativePath_Throws) {
//   Location loc;
//   EXPECT_THROW(callParseRoot("./var/www/html;", &loc), std::runtime_error);
// }

TEST(ConfigParser, SimpleDir_ParseRoot_InvalidChar_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www\r/html;", &loc), std::runtime_error);
}

/* ===================== ParseUploadPath ===================== */
TEST(ConfigParser, SimpleDir_ParseUploadPath_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseUploadPath("/uploads;", &loc));
  EXPECT_EQ(loc.GetUploadPath(), "/uploads");
}

TEST(ConfigParser, SimpleDir_ParseUploadPath_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath("/uploads", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseUploadPath_RelativePath_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath("./uploads;", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseUploadPath_UnsafeChars_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath("/upload\ts;", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseUploadPath_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath(";", &loc), std::runtime_error);
}

/* ===================== ParseRedirect ===================== */
TEST(ConfigParser, SimpleDir_ParseRedirect_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRedirect("https://example.com/;", &loc));
  EXPECT_EQ(loc.GetRedirect(), "https://example.com/");
}

TEST(ConfigParser, SimpleDir_ParseRedirect_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseRedirect("https://example.com/", &loc),
               std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseRedirect_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseRedirect(";", &loc), std::runtime_error);
}

/* ===================== ParseServerName ===================== */
TEST(ConfigParser, SimpleDir_ParseServerName_OK) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseServerName("myhost.local;", &sc));
  EXPECT_EQ(sc.GetServerName(), "myhost.local");
}

TEST(ConfigParser, SimpleDir_ParseServerName_MissingSemicolon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseServerName("myhost.local", &sc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseServerName_EmptyValue_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseServerName(";", &sc), std::runtime_error);
}

/* ===================== ParseCgi ===================== */
TEST(ConfigParser, SimpleDir_ParseCgi_OK_On) {
  Location loc;
  ConfigParser p;
  p.content = "on;";
  EXPECT_NO_THROW(p.ParseCgi(&loc));
  EXPECT_TRUE(loc.GetCgiEnabled());
}

TEST(ConfigParser, SimpleDir_ParseCgi_OK_Off) {
  Location loc;
  ConfigParser p;
  p.content = "off;";
  EXPECT_NO_THROW(p.ParseCgi(&loc));
  EXPECT_FALSE(loc.GetCgiEnabled());
}

TEST(ConfigParser, SimpleDir_ParseCgi_MissingSemicolon_Throws) {
  Location loc;
  ConfigParser p;
  p.content = "on";
  EXPECT_THROW(p.ParseCgi(&loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_ParseCgi_EmptyValue_Throws) {
  Location loc;
  ConfigParser p;
  p.content = ";";
  EXPECT_THROW(p.ParseCgi(&loc), std::runtime_error);
}
