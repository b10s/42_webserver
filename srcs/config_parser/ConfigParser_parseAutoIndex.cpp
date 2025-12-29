#include "ConfigParser.hpp"

// autoindex is off by default (see Location constructor)
void ConfigParser::ParseAutoIndex(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error: expected autoindex value" + token);
  }
  if (token != "on" && token != "off") {
    throw std::runtime_error("Invalid autoindex value: " + token);
  }
  bool value = (token == "on");
  location->SetAutoIndex(value);
  ConsumeExpectedSemicolon("autoindex");
}
