#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "ConfigParser.hpp"

// set up a helper to call ParseAutoIndex
static void callParseListen(const std::string& input, ServerConfig* sc) {
  ConfigParser parser;
  parser.content = input;
  parser.ParseListen(sc);
}

// ==================== happy path ====================

TEST(ConfigParser, Listen_HostColonPort) {
  ServerConfig sc;  // default: host=0.0.0.0, port=80
  EXPECT_NO_THROW(callParseListen("127.0.0.1:8080;", &sc));
  EXPECT_EQ(sc.GetHost(), "127.0.0.1");
  EXPECT_EQ(sc.GetPort(), "8080");
}

TEST(ConfigParser, Listen_PortOnly) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseListen("8081;", &sc));
  EXPECT_EQ(sc.GetHost(), "0.0.0.0");  // default host
  EXPECT_EQ(sc.GetPort(), "8081");
}

TEST(ConfigParser, Listen_HostOnly) {
  ServerConfig sc;
  EXPECT_NO_THROW(callParseListen("10.0.0.5;", &sc));
  EXPECT_EQ(sc.GetHost(), "10.0.0.5");
  EXPECT_EQ(sc.GetPort(), "80");  // default port
}

// ==================== error cases ====================

TEST(ConfigParser, Listen_MissingSemicolon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseListen("127.0.0.1:8080", &sc), std::runtime_error);
  EXPECT_THROW(callParseListen("8080", &sc), std::runtime_error);
  EXPECT_THROW(callParseListen("127.0.0.1", &sc), std::runtime_error);
}

// white space around ':'
TEST(ConfigParser, Listen_SpacesAroundColon_Throws) {
  ServerConfig sc;
  EXPECT_THROW(callParseListen("127.0.0.1 :8080;", &sc), std::runtime_error);
  EXPECT_THROW(callParseListen("127.0.0.1: 8080;", &sc), std::runtime_error);
  EXPECT_THROW(callParseListen("127.0.0.1 : 8080;", &sc), std::runtime_error);
}

TEST(ConfigParser, Listen_invalidHost_Throws) {
    ServerConfig sc;
    EXPECT_THROW(callParseListen("127.0.0.1:http;", &sc),
    std::runtime_error); EXPECT_THROW(callParseListen("http;", &sc),
    std::runtime_error);
}

TEST(ConfigParser, Listen_InvalidIPv4_Throws) {
  ServerConfig sc1, sc2, sc3;
  EXPECT_THROW(callParseListen("256.0.0.1:8080;", &sc1), std::runtime_error);
  EXPECT_THROW(callParseListen("192.168.1.300;", &sc2), std::runtime_error);
  EXPECT_THROW(callParseListen("192.168.1.-1;", &sc3), std::runtime_error);
  EXPECT_THROW(callParseListen("0.0.0.;", &sc3), std::runtime_error);
}

TEST(ConfigParser, Listen_InvalidDomain_Throws) {
  ServerConfig sc1, sc2, sc3;
  EXPECT_THROW(callParseListen("exa mple.com:8080;", &sc1), std::runtime_error);
  EXPECT_THROW(callParseListen("example!.com;", &sc2), std::runtime_error);
  EXPECT_THROW(callParseListen("-example.com;", &sc3), std::runtime_error);
}

// IsValidPortNumber: 1..65535
TEST(ConfigParser, Listen_PortOutOfRange_Throws) {
  ServerConfig sc1, sc2, sc3;
  EXPECT_THROW(callParseListen("0;", &sc1), std::runtime_error);
  EXPECT_THROW(callParseListen("65536;", &sc2), std::runtime_error);
  EXPECT_THROW(callParseListen("127.0.0.1:65536;", &sc3), std::runtime_error);
}

TEST(ConfigParser, Listen_EmptyOrMissingValue_Throws) {
  ServerConfig sc1, sc2;
  EXPECT_THROW(callParseListen("", &sc1), std::runtime_error);
  EXPECT_THROW(callParseListen(";", &sc2), std::runtime_error);
}
