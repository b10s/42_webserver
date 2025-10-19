#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "config_parser.hpp"

// set up a helper to call parseAutoIndex
static void callParseExtensions(const std::string& input, Location* loc) {
    ConfigParser parser;     // call default constructor to initialize currentPos_ to 0
    parser.content_ = input; // parseAutoIndex は content_ から tokenize する想定
    parser.parseExtensions(loc);
}

TEST(ConfigParser, ParseExtensions) {
    Location loc;
    EXPECT_NO_THROW(callParseExtensions(".php;", &loc));
    std::string exts = loc.getExtensions();
    EXPECT_EQ(exts, std::string(".php"));
}

TEST(ConfigParser, ParseExtensions_MissingSemicolon_Throws) {
    Location loc;
    EXPECT_THROW(callParseExtensions(".php", &loc), std::runtime_error);
}

TEST(ConfigParser, ParseExtensions_NoExtensions_Throws) {
    Location loc;
    EXPECT_THROW(callParseExtensions(";", &loc), std::runtime_error);
    EXPECT_THROW(callParseExtensions("", &loc), std::runtime_error);
}

// duplicate test: we have to discuss how to handle multiple extensions
// TEST(ConfigParser, ParseExtensions_DuplicatePolicy) {
//     Location loc;
//     EXPECT_NO_THROW(callParseExtentions(".php;", &loc));
//     EXPECT_NO_THROW(callParseExtentions(".php;", &loc));
//     auto exts = loc.getExtensions();
//     // TODO: define expected behavior for duplicates
//     // EXPECT_EQ(exts.size(), 2);
// }
