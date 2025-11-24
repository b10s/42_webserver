#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseAutoIndex
static void callParseMethods(const std::string& input, Location* loc) {
  ConfigParser parser;
  parser.content = input;
  parser.ParseMethods(loc);
}

// ==================== happy path ====================

TEST(ConfigParser, parseMethod_SingleMethod_GET_OK) {
  Location loc;
  EXPECT_NO_THROW(callParseMethods("GET;", &loc));
  EXPECT_TRUE(loc.IsMethodAllowed(kGet));
}

TEST(ConfigParser, parseMethods_State_Set) {
  Location loc;
  ASSERT_NO_THROW(callParseMethods("GET POST DELETE;", &loc));
  const std::set<RequestMethod>& methods = loc.GetMethods();
  EXPECT_EQ(methods.count(kGet), 1u);
  EXPECT_EQ(methods.count(kPost), 1u);
  EXPECT_EQ(methods.count(kDelete), 1u);
  EXPECT_EQ(methods.size(), 3u);
}

// ==================== error cases ====================
TEST(ConfigParser, parseMethods_Empty_Throws) {
  Location loc;
  EXPECT_THROW(callParseMethods(";", &loc), std::runtime_error);
}

TEST(ConfigParser, parseMethods_InvalidMethod_Throws) {
  Location loc;
  EXPECT_THROW(callParseMethods("GET PUT;", &loc), std::runtime_error);
}
