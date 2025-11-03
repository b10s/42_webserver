#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call parseAutoIndex
static void callconsumeMethods(const std::string& input, Location* loc) {
  ConfigParser parser;
  parser.content_ = input;
  parser.consumeMethods(loc);
}

// ==================== happy path ====================
TEST(ConfigParser, consumeMethod_Empty_OK) {
  Location loc;
  EXPECT_NO_THROW(callconsumeMethods(";", &loc));
}

TEST(ConfigParser, consumeMethod_SingleMethod_GET_OK) {
  Location loc;
  EXPECT_NO_THROW(callconsumeMethods("GET;", &loc));
  EXPECT_TRUE(loc.isMethodAllowed(GET));
}

TEST(ConfigParser, consumeMethods_State_Set) {
  Location loc;
  ASSERT_NO_THROW(callconsumeMethods("GET POST DELETE;", &loc));
  const std::set<RequestMethod>& methods = loc.getMethods();
  EXPECT_EQ(methods.count(GET), 1u);
  EXPECT_EQ(methods.count(POST), 1u);
  EXPECT_EQ(methods.count(DELETE), 1u);
  EXPECT_EQ(methods.size(), 3u);
}

// ==================== error cases ====================
TEST(ConfigParser, consumeMethods_InvalidMethod_Throws) {
  Location loc;
  EXPECT_THROW(callconsumeMethods("GET PUT;", &loc), std::runtime_error);
}
