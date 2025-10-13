#include "config_parser.hpp"

void ConfigParser::parseExtensions(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->addExtension(token);
  token = tokenize(content_);
  if (token != ";") throw std::runtime_error("Syntax error: expected ';' after extension value" + token);
}
