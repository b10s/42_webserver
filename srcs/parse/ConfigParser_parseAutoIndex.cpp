#include "ConfigParser.hpp"

// autoindex is off by default (see Location constructor)
void ConfigParser::parseAutoIndex(Location* location) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected autoindex value" + token);
  }
  if (token != "on" && token != "off") {
    throw std::runtime_error("Invalid autoindex value: " + token);
  }
  if (token == "on") location->setAutoIndex(true);
  token = tokenize(content_);
  if (token != ";")
    throw std::runtime_error(
        "Syntax error: expected ';' after autoindex value" + token);
}
