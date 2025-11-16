#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// custom macro to check exception message contains substring
#define EXPECT_THROW_WHAT_CONTAINS(stmt, ex_type, substr)                  \
  do {                                                                     \
    try {                                                                  \
      stmt;                                                                \
      FAIL() << "Expected " #ex_type;                                      \
    } catch (const ex_type& e) {                                           \
      std::string _msg_(e.what());                                         \
      EXPECT_NE(_msg_.find(substr), std::string::npos)                     \
          << "expected substring: [" << substr << "], actual: [" << _msg_  \
          << "]";                                                          \
    } catch (...) {                                                        \
      FAIL() << "Expected " #ex_type ", but got different exception type"; \
    }                                                                      \
  } while (0)

// test fixture
class ConfigParserTest : public ::testing::Test {
 protected:
  ConfigParser parser;
  Location loc;

  virtual void SetUp() {
    parser.content.clear();
  }

  // boilerplate to call ParseAutoIndex
  void CallParseAutoIndex(const std::string& input) {
    parser.content = input;      // Tokenize は content から読む想定
    parser.ParseAutoIndex(&loc);  // ここで例外が出たらテスト側で検証
  }
};

TEST_F(ConfigParserTest, ParseAutoIndex_On_SetsTrue) {
  EXPECT_NO_THROW(CallParseAutoIndex("on;"));
  EXPECT_TRUE(loc.GetAutoIndex());
}

TEST_F(ConfigParserTest, ParseAutoIndex_Off_StaysFalse) {
  EXPECT_NO_THROW(CallParseAutoIndex("off;"));
  EXPECT_FALSE(loc.GetAutoIndex());
}

TEST_F(ConfigParserTest, ParseAutoIndex_MissingValue_Throws) {
  EXPECT_THROW(CallParseAutoIndex(";"), std::runtime_error);
  EXPECT_THROW(CallParseAutoIndex(""), std::runtime_error);
}

TEST_F(ConfigParserTest, ParseAutoIndex_MissingSemicolon_Throws) {
  EXPECT_THROW(CallParseAutoIndex("on"), std::runtime_error);
  EXPECT_THROW(CallParseAutoIndex("off"), std::runtime_error);
}

// Error message tests
TEST_F(ConfigParserTest, ErrorMessage_InvalidValue_Semicolon) {
  EXPECT_THROW_WHAT_CONTAINS(CallParseAutoIndex(";"), std::runtime_error,
                             "Invalid autoindex value");
}

TEST_F(ConfigParserTest, ErrorMessage_MissingSemicolon_On) {
  EXPECT_THROW_WHAT_CONTAINS(CallParseAutoIndex("on"), std::runtime_error,
                             "expected ';' after autoindex value");
}

TEST_F(ConfigParserTest, ErrorMessage_MissingValue_EmptyInput) {
  EXPECT_THROW_WHAT_CONTAINS(CallParseAutoIndex(""), std::runtime_error,
                             "expected autoindex value");
}
