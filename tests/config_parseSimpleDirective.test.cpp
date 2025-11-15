#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call parseAutoIndex
static void callParseRoot(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content_ = input;
  p.parseRoot(loc);
}

static void callParseCgiPath(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content_ = input;
  p.parseCgiPath(loc);
}

static void callParseUploadPath(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content_ = input;
  p.parseUploadPath(loc);
}

static void callParseRedirect(const std::string& input, Location* loc) {
  ConfigParser p;
  p.content_ = input;
  p.parseRedirect(loc);
}

static void callParseServerName(const std::string& input, ServerConfig* sc) {
  ConfigParser p;
  p.content_ = input;
  p.parseServerName(sc);
}

/* ===================== parseRoot ===================== */
TEST(ConfigParser, SimpleDir_parseRoot_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRoot("/var/www/html;", &loc));
  EXPECT_EQ(loc.GetRoot(), "/var/www/html");
}

TEST(ConfigParser, SimpleDir_parseRoot_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var/www/html", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseRoot_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot(";", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseRoot_ExtraTokens_Throws) {
  Location loc;
  EXPECT_THROW(callParseRoot("/var /www;", &loc), std::runtime_error);
}

/* ===================== parseCgiPath ===================== */
TEST(ConfigParser, SimpleDir_parseCgiPath_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseCgiPath("cgi/cgi.py;", &loc));
  EXPECT_EQ(loc.getCgiPath(), "cgi/cgi.py");
}

TEST(ConfigParser, SimpleDir_parseCgiPath_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseCgiPath("cgi/cgi.py", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseCgiPath_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseCgiPath(";", &loc), std::runtime_error);
}

/* ===================== parseUploadPath ===================== */
TEST(ConfigParser, SimpleDir_parseUploadPath_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseUploadPath("/uploads;", &loc));
  EXPECT_EQ(loc.getUploadPath(), "/uploads");
}

TEST(ConfigParser, SimpleDir_parseUploadPath_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath("/uploads", &loc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseUploadPath_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseUploadPath(";", &loc), std::runtime_error);
}

/* ===================== parseRedirect ===================== */
TEST(ConfigParser, SimpleDir_parseRedirect_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseRedirect("https://example.com/;", &loc));
  EXPECT_EQ(loc.getRedirect(), "https://example.com/");
}

TEST(ConfigParser, SimpleDir_parseRedirect_MissingSemicolon_Throws) {
  Location loc;
  EXPECT_THROW(callParseRedirect("https://example.com/", &loc),
               std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseRedirect_EmptyValue_Throws) {
  Location loc;
  EXPECT_THROW(callParseRedirect(";", &loc), std::runtime_error);
}

/* ===================== parseServerName ===================== */
TEST(ConfigParser, SimpleDir_parseServerName_OK) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseServerName("myhost.local;", &sc));
  EXPECT_EQ(sc.getServerName(), "myhost.local");
}

TEST(ConfigParser, SimpleDir_parseServerName_MissingSemicolon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseServerName("myhost.local", &sc), std::runtime_error);
}

TEST(ConfigParser, SimpleDir_parseServerName_EmptyValue_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseServerName(";", &sc), std::runtime_error);
}
