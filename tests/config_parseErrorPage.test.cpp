#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

static void expectErrorPageEq(const ServerConfig& sc, int code,
                              const std::string& expected) {
  const std::map<HttpStatus, std::string>& pages = sc.GetErrorPages();
  HttpStatus key = static_cast<HttpStatus>(code);
  std::map<HttpStatus, std::string>::const_iterator it = pages.find(key);
  ASSERT_NE(it, pages.end()) << "error page for code " << code << " not found";
  EXPECT_EQ(it->second, expected);
}

static void callParseErrorPage(const std::string& input, ServerConfig* sc) {
  ConfigParser parser;
  parser.content_ = input;
  parser.ParseErrorPage(sc);
}

// ============ happy path ============

TEST(ConfigParser, ErrorPage_AbsolutePath) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseErrorPage("404 /errors/404.html;", &sc));
  expectErrorPageEq(sc, 404, "/errors/404.html");
}

TEST(ConfigParser, ErrorPage_RelativePathGetsPrefixedSlash) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseErrorPage("404 errors/404.html;", &sc));
  expectErrorPageEq(sc, 404, "/errors/404.html");
}

TEST(ConfigParser, ErrorPage_HttpUrlAccepted) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseErrorPage("502 http://example.com/502.html;", &sc));
  expectErrorPageEq(sc, 502, "http://example.com/502.html");
}

// ============ error path ============

TEST(ConfigParser, ErrorPage_CodeTooLow_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseErrorPage("200 /x;", &sc), std::runtime_error);
}

TEST(ConfigParser, ErrorPage_CodeTooHigh_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseErrorPage("600 /x;", &sc), std::runtime_error);
}

TEST(ConfigParser, ErrorPage_PathMissing_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseErrorPage("404 ;", &sc), std::runtime_error);
}

TEST(ConfigParser, ErrorPage_PathStartingWithDot_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseErrorPage("404 ./404.html;", &sc), std::runtime_error);
}

TEST(ConfigParser, ErrorPage_MissingSemicolon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseErrorPage("404 /errors/404.html", &sc),
               std::runtime_error);
}
