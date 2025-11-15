#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call parseAutoIndex
static void callParseMaxBody(const std::string& input, ServerConfig* sc) {
  ConfigParser parser;
  parser.content_ = input;
  parser.parseMaxBody(sc);
}

// ==================== happy path ====================
// zero is valid
TEST(ConfigParser, ParseMaxBody_Zero_OK) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseMaxBody("0;", &sc));
  EXPECT_EQ(sc.getMaxBodySize(), 0);
}

TEST(ConfigParser, ParseMaxBody_Normal_OK) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseMaxBody("12345;", &sc));
  EXPECT_EQ(sc.getMaxBodySize(), 12345);
}

TEST(ConfigParser, ParseMaxBody_UpperBound_OK) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseMaxBody("100000000;", &sc));
  EXPECT_EQ(sc.getMaxBodySize(), 100000000);
}

// ==================== error cases ====================

TEST(ConfigParser, ParseMaxBody_Negative_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseMaxBody("-1;", &sc), std::runtime_error);
}

TEST(ConfigParser, ParseMaxBody_OverUpperBound_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseMaxBody("100000001;", &sc), std::runtime_error);
}

TEST(ConfigParser, ParseMaxBody_MissingSemicolon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseMaxBody("12345", &sc), std::runtime_error);
}

/*
std::atoi(token.c_str()) accepts "abd; " and returns 0.
TODO: introduce IsAllDigits(token) to validate the token before calling atoi.
*/
TEST(ConfigParser, ParseMaxBody_NonNumeric_TreatedAsZero_CurrentBehavior) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseMaxBody("abc;", &sc));
  EXPECT_EQ(sc.getMaxBodySize(), 0);
}
