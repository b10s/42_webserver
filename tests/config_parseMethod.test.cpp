#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call parseAutoIndex
static void callparseMethods(const std::string& input, Location* loc) {
  ConfigParser parser;
  parser.content_ = input;
  parser.parseMethods(loc);
}

// ==================== happy path ====================
TEST(ConfigParser, ParseMethod_Empty_OK) {
  Location loc;
  EXPECT_NO_THROW(callparseMethods(";", &loc));
}

TEST(ConfigParser, ParseMethod_SingleMethod_GET_OK) {
  Location loc;
  EXPECT_NO_THROW(callparseMethods("GET;", &loc));
  EXPECT_TRUE(loc.isMethodAllowed(GET));
}

TEST(ConfigParser, ParseMethods_State_Set) {
  Location loc;
  ASSERT_NO_THROW(callparseMethods("GET POST DELETE;", &loc));
  const std::set<RequestMethod>& methods = loc.getMethods();
  EXPECT_EQ(methods.count(GET), 1u);
  EXPECT_EQ(methods.count(POST), 1u);
  EXPECT_EQ(methods.count(DELETE_), 1u);
  EXPECT_EQ(methods.size(), 3u);
}

// ==================== error cases ====================
TEST(ConfigParser, ParseMethods_InvalidMethod_Throws) {
  Location loc;
  EXPECT_THROW(callparseMethods("GET PUT;", &loc), std::runtime_error);
}
