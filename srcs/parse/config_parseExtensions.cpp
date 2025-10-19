#include "config_parser.hpp"

void ConfigParser::parseExtensions(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setExtension(token);
  token = tokenize(content_);
  // we are not doing bonus so only one extension is allowed here
  if (token != ";") throw std::runtime_error("Syntax error: expected ';' after extension value" + token);
}

// I think I should handle the case where multiple extensions are given
// for example:
// extensions .php;
// extensions .py;
// I will handle this later