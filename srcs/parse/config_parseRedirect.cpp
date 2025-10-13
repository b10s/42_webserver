#include "config_parser.hpp"

void ConfigParser::parseRedirect(Location *location) {
  std::string token;

  token = tokenize(content_);
  location->setRedirect(token);
  token = tokenize(content_);
  if (token != ";") throw std::runtime_error("Syntax error: expected ';' after redirect value" + token);
}
