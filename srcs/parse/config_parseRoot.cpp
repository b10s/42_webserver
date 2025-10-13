#include "config_parser.hpp"

void ConfigParser::parseRoot(Location *location) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected root directory path" + token);
  }
  location->setRoot(token);
  token = tokenize(content_);
  if (token != ";")
    throw std::runtime_error("Syntax error: expected ';' after root directory path" + token);
}